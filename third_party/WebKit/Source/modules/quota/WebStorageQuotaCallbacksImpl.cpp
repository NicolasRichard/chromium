/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "modules/quota/WebStorageQuotaCallbacksImpl.h"

#include "core/dom/DOMError.h"
#include "core/dom/ExceptionCode.h"
#include "modules/quota/StorageInfo.h"

namespace WebCore {

WebStorageQuotaCallbacksImpl::WebStorageQuotaCallbacksImpl(PassRefPtr<ScriptPromiseResolver> resolver, ExecutionContext* context)
    : m_resolver(resolver)
    , m_requestState(context)
{
}

WebStorageQuotaCallbacksImpl::~WebStorageQuotaCallbacksImpl()
{
}

void WebStorageQuotaCallbacksImpl::didQueryStorageUsageAndQuota(unsigned long long usageInBytes, unsigned long long quotaInBytes)
{
    OwnPtr<WebStorageQuotaCallbacksImpl> deleter = adoptPtr(this);
    DOMRequestState::Scope scope(m_requestState);
    m_resolver->resolve(StorageInfo::create(usageInBytes, quotaInBytes));
}

void WebStorageQuotaCallbacksImpl::didGrantStorageQuota(unsigned long long usageInBytes, unsigned long long grantedQuotaInBytes)
{
    OwnPtr<WebStorageQuotaCallbacksImpl> deleter = adoptPtr(this);
    DOMRequestState::Scope scope(m_requestState);
    m_resolver->resolve(StorageInfo::create(usageInBytes, grantedQuotaInBytes));
}

void WebStorageQuotaCallbacksImpl::didFail(blink::WebStorageQuotaError error)
{
    OwnPtr<WebStorageQuotaCallbacksImpl> deleter = adoptPtr(this);
    DOMRequestState::Scope scope(m_requestState);
    m_resolver->reject(DOMError::create(static_cast<ExceptionCode>(error)).get());
}

} // namespace WebCore
