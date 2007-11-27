/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
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

#include "config.h"
#include "WebSecurityOrigin.h"
#include "WebKitDLL.h"

#include <WebCore/BString.h>
#include <WebCore/DatabaseTracker.h>

using namespace WebCore;

// WebSecurityOrigin ---------------------------------------------------------------
WebSecurityOrigin* WebSecurityOrigin::createInstance(const SecurityOriginData& securityOriginData)
{
    WebSecurityOrigin* origin = new WebSecurityOrigin(securityOriginData);
    origin->AddRef();
    return origin;
}

WebSecurityOrigin::WebSecurityOrigin(const SecurityOriginData& securityOriginData)
    : m_refCount(0)
    , m_securityOriginData(securityOriginData)
{
    gClassCount++;
}

WebSecurityOrigin::~WebSecurityOrigin()
{
    gClassCount--;
}

// IUnknown ------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE WebSecurityOrigin::QueryInterface(REFIID riid, void** ppvObject)
{
    *ppvObject = 0;
    if (IsEqualGUID(riid, IID_IUnknown))
        *ppvObject = static_cast<IWebSecurityOrigin*>(this);
    else if (IsEqualGUID(riid, IID_IWebSecurityOrigin))
        *ppvObject = static_cast<IWebSecurityOrigin*>(this);
    else if (IsEqualGUID(riid, __uuidof(this)))
        *ppvObject = this;
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE WebSecurityOrigin::AddRef()
{
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE WebSecurityOrigin::Release()
{
    ULONG newRef = --m_refCount;
    if (!newRef)
        delete this;

    return newRef;
}

// IWebSecurityOrigin --------------------------------------------------------------

HRESULT STDMETHODCALLTYPE WebSecurityOrigin::protocol( 
    /* [retval][out] */ BSTR* result)
{
    if (!result)
        return E_POINTER;

    *result = BString(m_securityOriginData.protocol()).release();

    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE WebSecurityOrigin::domain( 
    /* [retval][out] */ BSTR* result)
{
    if (!result)
        return E_POINTER;

    *result = BString(m_securityOriginData.host()).release();

    return S_OK;
}
      
HRESULT STDMETHODCALLTYPE WebSecurityOrigin::port( 
    /* [retval][out] */ unsigned short* result)
{
    if (!result)
        return E_POINTER;

    *result = m_securityOriginData.port();

    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE WebSecurityOrigin::usage( 
    /* [retval][out] */ unsigned long long* result)
{
    if (!result)
        return E_POINTER;

    *result = DatabaseTracker::tracker().usageForOrigin(m_securityOriginData);

    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE WebSecurityOrigin::quota( 
    /* [retval][out] */ unsigned long long* result)
{
    if (!result)
        return E_POINTER;

    *result = DatabaseTracker::tracker().quotaForOrigin(m_securityOriginData);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE WebSecurityOrigin::setQuota( 
    /* [in] */ unsigned long long quota) 
{
    DatabaseTracker::tracker().setQuota(m_securityOriginData, quota);

    return S_OK;
}
