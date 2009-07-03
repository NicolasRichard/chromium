/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FrameLoaderClientGtk_h
#define FrameLoaderClientGtk_h

#include "FrameLoaderClient.h"
#include "ResourceResponse.h"
#include "PluginView.h"
#include "webkitwebpolicydecision.h"

typedef struct _WebKitWebFrame WebKitWebFrame;

namespace WebKit {

    class FrameLoaderClient : public WebCore::FrameLoaderClient {
    public:
        FrameLoaderClient(WebKitWebFrame*);
        virtual ~FrameLoaderClient();
        virtual void frameLoaderDestroyed();

        WebKitWebFrame*  webFrame() const { return m_frame; }

        virtual bool hasWebView() const;

        virtual void makeRepresentation(WebCore::DocumentLoader*);
        virtual void forceLayout();
        virtual void forceLayoutForNonHTML();

        virtual void setCopiesOnScroll();

        virtual void detachedFromParent2();
        virtual void detachedFromParent3();

        virtual void assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&);

        virtual void dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long  identifier, WebCore::ResourceRequest&, const WebCore::ResourceResponse& redirectResponse);
        virtual bool shouldUseCredentialStorage(WebCore::DocumentLoader*, unsigned long identifier);
        virtual void dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&);
        virtual void dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long  identifier, const WebCore::AuthenticationChallenge&);
        virtual void dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long  identifier, const WebCore::ResourceResponse&);
        virtual void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long identifier, int lengthReceived);
        virtual void dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long  identifier);
        virtual void dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long  identifier, const WebCore::ResourceError&);
        virtual bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int length);
        virtual void dispatchDidLoadResourceByXMLHttpRequest(unsigned long, const WebCore::ScriptString&);

        virtual void dispatchDidHandleOnloadEvents();
        virtual void dispatchDidReceiveServerRedirectForProvisionalLoad();
        virtual void dispatchDidCancelClientRedirect();
        virtual void dispatchWillPerformClientRedirect(const WebCore::KURL&, double, double);
        virtual void dispatchDidChangeLocationWithinPage();
        virtual void dispatchWillClose();
        virtual void dispatchDidReceiveIcon();
        virtual void dispatchDidStartProvisionalLoad();
        virtual void dispatchDidReceiveTitle(const WebCore::String&);
        virtual void dispatchDidCommitLoad();
        virtual void dispatchDidFailProvisionalLoad(const WebCore::ResourceError&);
        virtual void dispatchDidFailLoad(const WebCore::ResourceError&);
        virtual void dispatchDidFinishDocumentLoad();
        virtual void dispatchDidFinishLoad();
        virtual void dispatchDidFirstLayout();
        virtual void dispatchDidFirstVisuallyNonEmptyLayout();

        virtual WebCore::Frame* dispatchCreatePage();
        virtual void dispatchShow();

        virtual void dispatchDecidePolicyForMIMEType(WebCore::FramePolicyFunction, const WebCore::String& MIMEType, const WebCore::ResourceRequest&);
        virtual void dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, WTF::PassRefPtr<WebCore::FormState>, const WebCore::String& frameName);
        virtual void dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, WTF::PassRefPtr<WebCore::FormState>);
        virtual void cancelPolicyCheck();

        virtual void dispatchUnableToImplementPolicy(const WebCore::ResourceError&);

        virtual void dispatchWillSubmitForm(WebCore::FramePolicyFunction, WTF::PassRefPtr<WebCore::FormState>);

        virtual void dispatchDidLoadMainResource(WebCore::DocumentLoader*);
        virtual void revertToProvisionalState(WebCore::DocumentLoader*);
        virtual void setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&);

        virtual void postProgressStartedNotification();
        virtual void postProgressEstimateChangedNotification();
        virtual void postProgressFinishedNotification();

        virtual PassRefPtr<WebCore::Frame> createFrame(const WebCore::KURL& url, const WebCore::String& name, WebCore::HTMLFrameOwnerElement* ownerElement,
                                   const WebCore::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight);
        virtual WebCore::Widget* createPlugin(const WebCore::IntSize&, WebCore::HTMLPlugInElement*, const WebCore::KURL&, const WTF::Vector<WebCore::String>&, const WTF::Vector<WebCore::String>&, const WebCore::String&, bool);
        virtual void redirectDataToPlugin(WebCore::Widget* pluginWidget);
        virtual WebCore::Widget* createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL, const WTF::Vector<WebCore::String>& paramNames, const WTF::Vector<WebCore::String>& paramValues);
        virtual WebCore::String overrideMediaType() const;
        virtual void windowObjectCleared();
        virtual void documentElementAvailable();
        virtual void didPerformFirstNavigation() const;

        virtual void registerForIconNotification(bool);

        virtual WebCore::ObjectContentType objectContentType(const WebCore::KURL& url, const WebCore::String& mimeType);

        virtual void setMainFrameDocumentReady(bool);

        virtual void startDownload(const WebCore::ResourceRequest&);

        virtual void willChangeTitle(WebCore::DocumentLoader*);
        virtual void didChangeTitle(WebCore::DocumentLoader*);

        virtual void committedLoad(WebCore::DocumentLoader*, const char*, int);
        virtual void finishedLoading(WebCore::DocumentLoader*);

        virtual void updateGlobalHistory();
        virtual void updateGlobalHistoryRedirectLinks();
        virtual bool shouldGoToHistoryItem(WebCore::HistoryItem*) const;

        virtual WebCore::ResourceError cancelledError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError blockedError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError cannotShowURLError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError interruptForPolicyChangeError(const WebCore::ResourceRequest&);

        virtual WebCore::ResourceError cannotShowMIMETypeError(const WebCore::ResourceResponse&);
        virtual WebCore::ResourceError fileDoesNotExistError(const WebCore::ResourceResponse&);
        virtual WebCore::ResourceError pluginWillHandleLoadError(const WebCore::ResourceResponse&);

        virtual bool shouldFallBack(const WebCore::ResourceError&);

        virtual bool canHandleRequest(const WebCore::ResourceRequest&) const;
        virtual bool canShowMIMEType(const WebCore::String&) const;
        virtual bool representationExistsForURLScheme(const WebCore::String&) const;
        virtual WebCore::String generatedMIMETypeForURLScheme(const WebCore::String&) const;

        virtual void frameLoadCompleted();
        virtual void saveViewStateToItem(WebCore::HistoryItem*);
        virtual void restoreViewState();
        virtual void provisionalLoadStarted();
        virtual void didFinishLoad();
        virtual void prepareForDataSourceReplacement();

        virtual WTF::PassRefPtr<WebCore::DocumentLoader> createDocumentLoader(const WebCore::ResourceRequest&, const WebCore::SubstituteData&);
        virtual void setTitle(const WebCore::String& title, const WebCore::KURL&);

        virtual WebCore::String userAgent(const WebCore::KURL&);

        virtual void savePlatformDataToCachedFrame(WebCore::CachedFrame*);
        virtual void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*);
        virtual void transitionToCommittedForNewPage();

        virtual bool canCachePage() const;
        virtual void download(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&);
    private:
        WebKitWebFrame* m_frame;
        WebCore::ResourceResponse m_response;
        WebKitWebPolicyDecision* m_policyDecision;

        // Plugin view to redirect data to
        WebCore::PluginView* m_pluginView;
        bool m_hasSentResponseToPlugin;
    };

}

#endif
