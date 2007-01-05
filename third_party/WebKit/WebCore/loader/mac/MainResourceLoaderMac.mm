/*
 * Copyright (C) 2005, 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "config.h"
#import "MainResourceLoader.h"

#import "FrameLoader.h"
#import "FrameLoaderClient.h"
#import "KURL.h"
#import "PlatformString.h"
#import "ResourceError.h"
#import "ResourceHandle.h"
#import "ResourceRequest.h"
#import "ResourceResponse.h"
#import "WebCoreSystemInterface.h"
#import "WebDataProtocol.h"
#import <Foundation/NSHTTPCookie.h>
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSURLRequest.h>
#import <wtf/Assertions.h>
#import <wtf/PassRefPtr.h>
#import <wtf/Vector.h>

// FIXME: More that is in common with SubresourceLoader should move up into ResourceLoader.

using namespace WebCore;

namespace WebCore {

const size_t URLBufferLength = 2048;

MainResourceLoader::MainResourceLoader(Frame* frame)
    : ResourceLoader(frame)
    , m_loadingMultipartContent(false)
    , m_waitingForContentPolicy(false)
{
}

MainResourceLoader::~MainResourceLoader()
{
}

PassRefPtr<MainResourceLoader> MainResourceLoader::create(Frame* frame)
{
    return new MainResourceLoader(frame);
}

void MainResourceLoader::receivedError(const ResourceError& error)
{
    // Calling receivedMainResourceError will likely result in the last reference to this object to go away.
    RefPtr<MainResourceLoader> protect(this);

    if (!cancelled()) {
        ASSERT(!reachedTerminalState());
        frameLoader()->didFailToLoad(this, error);
    }

    frameLoader()->receivedMainResourceError(error, true);

    if (!cancelled())
        releaseResources();

    ASSERT(reachedTerminalState());
}

void MainResourceLoader::didCancel(const ResourceError& error)
{
    // Calling receivedMainResourceError will likely result in the last reference to this object to go away.
    RefPtr<MainResourceLoader> protect(this);

    if (m_waitingForContentPolicy) {
        frameLoader()->cancelContentPolicyCheck();
        ASSERT(m_waitingForContentPolicy);
        m_waitingForContentPolicy = false;
        deref(); // balances ref in didReceiveResponse
    }
    frameLoader()->receivedMainResourceError(error, true);
    ResourceLoader::didCancel(error);
}

ResourceError MainResourceLoader::interruptionForPolicyChangeError() const
{
    return frameLoader()->interruptionForPolicyChangeError(request());
}

void MainResourceLoader::stopLoadingForPolicyChange()
{
    cancel(interruptionForPolicyChangeError());
}

void MainResourceLoader::callContinueAfterNavigationPolicy(void* argument, const ResourceRequest& request, PassRefPtr<FormState>, bool shouldContinue)
{
    static_cast<MainResourceLoader*>(argument)->continueAfterNavigationPolicy(request, shouldContinue);
}

void MainResourceLoader::continueAfterNavigationPolicy(const ResourceRequest& request, bool shouldContinue)
{
    if (!shouldContinue)
        stopLoadingForPolicyChange();
    deref(); // balances ref in willSendRequest
}

bool MainResourceLoader::isPostOrRedirectAfterPost(const ResourceRequest& newRequest, const ResourceResponse& redirectResponse)
{
    if (newRequest.httpMethod() == "POST")
        return true;

    int status = redirectResponse.httpStatusCode();
    if (((status >= 301 && status <= 303) || status == 307)
        && frameLoader()->initialRequest().httpMethod() == "POST")
        return true;
    
    return false;
}

void MainResourceLoader::addData(const char* data, int length, bool allAtOnce)
{
    ResourceLoader::addData(data, length, allAtOnce);
    frameLoader()->receivedData(data, length);
}

void MainResourceLoader::willSendRequest(ResourceRequest& newRequest, const ResourceResponse& redirectResponse)
{
    // Note that there are no asserts here as there are for the other callbacks. This is due to the
    // fact that this "callback" is sent when starting every load, and the state of callback
    // deferrals plays less of a part in this function in preventing the bad behavior deferring 
    // callbacks is meant to prevent.
    ASSERT(!newRequest.isNull());
    
    // The additional processing can do anything including possibly removing the last
    // reference to this object; one example of this is 3266216.
    RefPtr<MainResourceLoader> protect(this);
    
    // Update cookie policy base URL as URL changes, except for subframes, which use the
    // URL of the main frame which doesn't change when we redirect.
    if (frameLoader()->isLoadingMainFrame())
        newRequest.setMainDocumentURL(newRequest.url());
    
    // If we're fielding a redirect in response to a POST, force a load from origin, since
    // this is a common site technique to return to a page viewing some data that the POST
    // just modified.
    // Also, POST requests always load from origin, but this does not affect subresources.
    if (newRequest.cachePolicy() == UseProtocolCachePolicy && isPostOrRedirectAfterPost(newRequest, redirectResponse))
        newRequest.setCachePolicy(ReloadIgnoringCacheData);

    ResourceLoader::willSendRequest(newRequest, redirectResponse);
    
    // Don't set this on the first request. It is set when the main load was started.
    frameLoader()->setRequest(newRequest);
    
    ref(); // balanced by deref in continueAfterNavigationPolicy
    frameLoader()->checkNavigationPolicy(newRequest, callContinueAfterNavigationPolicy, this);
}

static bool shouldLoadAsEmptyDocument(const KURL& URL)
{
    return URL.isEmpty() || equalIgnoringCase(URL.protocol(), "about");
}

void MainResourceLoader::continueAfterContentPolicy(PolicyAction contentPolicy, const ResourceResponse& r)
{
    KURL url = request().url();
    const String& mimeType = r.mimeType();
    
    switch (contentPolicy) {
    case PolicyUse: {
        // Prevent remote web archives from loading because they can claim to be from any domain and thus avoid cross-domain security checks (4120255).
        bool isRemote = !url.isLocalFile();
#if PLATFORM(MAC)
        isRemote = isRemote && ![WebDataProtocol _webIsDataProtocolURL:url.getNSURL()];
#endif
        bool isRemoteWebArchive = isRemote && equalIgnoringCase("application/x-webarchive", mimeType);
        if (!frameLoader()->canShowMIMEType(mimeType) || isRemoteWebArchive) {
            frameLoader()->cannotShowMIMEType(r);
            // Check reachedTerminalState since the load may have already been cancelled inside of _handleUnimplementablePolicyWithErrorCode::.
            if (!reachedTerminalState())
                stopLoadingForPolicyChange();
            return;
        }
        break;
    }

    case PolicyDownload:
        frameLoader()->client()->download(m_handle.get(), request(), r);
        receivedError(interruptionForPolicyChangeError());
        return;

    case PolicyIgnore:
        stopLoadingForPolicyChange();
        return;
    
    default:
        ASSERT_NOT_REACHED();
    }

    RefPtr<MainResourceLoader> protect(this);

    if (r.isHTTP()) {
        int status = r.httpStatusCode();
        if (status < 200 || status >= 300) {
            bool hostedByObject = frameLoader()->isHostedByObjectElement();

            frameLoader()->handleFallbackContent();
            // object elements are no longer rendered after we fallback, so don't
            // keep trying to process data from their load

            if (hostedByObject)
                cancel();
        }
    }

    // we may have cancelled this load as part of switching to fallback content
    if (!reachedTerminalState())
        ResourceLoader::didReceiveResponse(r);

    if (frameLoader() && !frameLoader()->isStopping()
            && (shouldLoadAsEmptyDocument(url)
                || frameLoader()->representationExistsForURLScheme(url.protocol())))
        didFinishLoading();
}

void MainResourceLoader::callContinueAfterContentPolicy(void* argument, PolicyAction policy)
{
    static_cast<MainResourceLoader*>(argument)->continueAfterContentPolicy(policy);
}

void MainResourceLoader::continueAfterContentPolicy(PolicyAction policy)
{
    ASSERT(m_waitingForContentPolicy);
    m_waitingForContentPolicy = false;
    if (!frameLoader()->isStopping())
        continueAfterContentPolicy(policy, m_response);
    deref(); // balances ref in didReceiveResponse
}

void MainResourceLoader::didReceiveResponse(const ResourceResponse& r)
{
    ASSERT(shouldLoadAsEmptyDocument(r.url()) || !defersLoading());

    if (m_loadingMultipartContent) {
        frameLoader()->setupForReplaceByMIMEType(r.mimeType());
        clearResourceData();
    }
    
    if (r.isMultipart())
        m_loadingMultipartContent = true;
        
    // The additional processing can do anything including possibly removing the last
    // reference to this object; one example of this is 3266216.
    RefPtr<MainResourceLoader> protect(this);

    frameLoader()->setResponse(r);

    m_response = r;

    ASSERT(!m_waitingForContentPolicy);
    m_waitingForContentPolicy = true;
    ref(); // balanced by deref in continueAfterContentPolicy and didCancel
    frameLoader()->checkContentPolicy(m_response.mimeType(), callContinueAfterContentPolicy, this);
}

void MainResourceLoader::didReceiveData(const char* data, int length, long long lengthReceived, bool allAtOnce)
{
    ASSERT(data);
    ASSERT(length != 0);
    ASSERT(!defersLoading());
 
    // The additional processing can do anything including possibly removing the last
    // reference to this object; one example of this is 3266216.
    RefPtr<MainResourceLoader> protect(this);

    ResourceLoader::didReceiveData(data, length, lengthReceived, allAtOnce);
}

void MainResourceLoader::didFinishLoading()
{
    ASSERT(shouldLoadAsEmptyDocument(frameLoader()->URL()) || !defersLoading());

    // The additional processing can do anything including possibly removing the last
    // reference to this object.
    RefPtr<MainResourceLoader> protect(this);

    frameLoader()->finishedLoading();
    ResourceLoader::didFinishLoading();
}

void MainResourceLoader::didFail(const ResourceError& error)
{
    ASSERT(!defersLoading());

    receivedError(error);
}

bool MainResourceLoader::loadNow(ResourceRequest& r)
{
    bool shouldLoadEmptyBeforeRedirect = shouldLoadAsEmptyDocument(r.url());

    ASSERT(!m_handle);
    ASSERT(shouldLoadEmptyBeforeRedirect || !defersLoading());

    // Send this synthetic delegate callback since clients expect it, and
    // we no longer send the callback from within NSURLConnection for
    // initial requests.
    willSendRequest(r, ResourceResponse());

    // <rdar://problem/4801066>
    // willSendRequest() is liable to make the call to frameLoader() return NULL, so we need to check that here
    if (!frameLoader())
        return false;
    
    const KURL& url = r.url();
    bool shouldLoadEmpty = shouldLoadAsEmptyDocument(url);

    if (shouldLoadEmptyBeforeRedirect && !shouldLoadEmpty && defersLoading())
        return true;

    if (shouldLoadEmpty || frameLoader()->representationExistsForURLScheme(url.protocol())) {
        String mimeType;
        if (shouldLoadEmpty)
            mimeType = "text/html";
        else
            mimeType = frameLoader()->generatedMIMETypeForURLScheme(url.protocol());

        ResourceResponse response(url, mimeType, 0, String(), String());
        didReceiveResponse(response);
    } else {
        m_handle = ResourceHandle::create(r, this, m_frame.get(), false, true);
    }

    return false;
}

bool MainResourceLoader::load(const ResourceRequest& r)
{
    ASSERT(!m_handle);

    ResourceRequest request(r);
    bool defer = defersLoading();
    if (defer) {
        bool shouldLoadEmpty = shouldLoadAsEmptyDocument(r.url());
        if (shouldLoadEmpty)
            defer = false;
    }
    if (!defer) {
        if (loadNow(request)) {
            // Started as an empty document, but was redirected to something non-empty.
            ASSERT(defersLoading());
            defer = true;
        }
    }
    if (defer) 
        m_initialRequest = request;

    return true;
}

void MainResourceLoader::setDefersLoading(bool defers)
{
    ResourceLoader::setDefersLoading(defers);
    if (!defers) {
        if (!m_initialRequest.isNull()) {
            ResourceRequest r(m_initialRequest);
            m_initialRequest = ResourceRequest();
            loadNow(r);
        }
    }
}

}
