/*
 * Copyright (C) 2009, 2012 Google Inc. All rights reserved.
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
#include "SocketStreamHandle.h"
#include "SocketStreamHandleInternal.h"

#include "Logging.h"
#include "NotImplemented.h"
#include "SocketStreamError.h"
#include "SocketStreamHandleClient.h"
#include <public/Platform.h>
#include <public/WebData.h>
#include <public/WebSocketStreamError.h>
#include <public/WebSocketStreamHandle.h>
#include <wtf/PassOwnPtr.h>

namespace WebCore {

SocketStreamHandleInternal::SocketStreamHandleInternal(SocketStreamHandle* handle)
    : m_handle(handle)
    , m_maxPendingSendAllowed(0)
    , m_pendingAmountSent(0)
{
}

SocketStreamHandleInternal::~SocketStreamHandleInternal()
{
    m_handle = 0;
}

void SocketStreamHandleInternal::connect(const KURL& url)
{
    m_socket = adoptPtr(WebKit::Platform::current()->createSocketStreamHandle());
    LOG(Network, "SocketStreamHandleInternal %p connect()", this);
    ASSERT(m_socket);
    ASSERT(m_handle);
    if (m_handle->m_client)
        m_handle->m_client->willOpenSocketStream(m_handle);
    m_socket->connect(url, this);
}

int SocketStreamHandleInternal::send(const char* data, int len)
{
    LOG(Network, "SocketStreamHandleInternal %p send() len=%d", this, len);
    // FIXME: |m_socket| should not be null here, but it seems that there is the
    // case. We should figure out such a path and fix it rather than checking
    // null here.
    if (!m_socket) {
        LOG(Network, "SocketStreamHandleInternal %p send() m_socket is NULL", this);
        return 0;
    }
    if (m_pendingAmountSent + len > m_maxPendingSendAllowed)
        len = m_maxPendingSendAllowed - m_pendingAmountSent;

    if (len <= 0)
        return len;
    WebKit::WebData webdata(data, len);
    if (m_socket->send(webdata)) {
        m_pendingAmountSent += len;
        LOG(Network, "SocketStreamHandleInternal %p send() Sent %d bytes", this, len);
        return len;
    }
    LOG(Network, "SocketStreamHandleInternal %p send() m_socket->send() failed", this);
    return 0;
}

void SocketStreamHandleInternal::close()
{
    LOG(Network, "SocketStreamHandleInternal %p close()", this);
    if (m_socket)
        m_socket->close();
}

void SocketStreamHandleInternal::didOpenStream(WebKit::WebSocketStreamHandle* socketHandle, int maxPendingSendAllowed)
{
    LOG(Network, "SocketStreamHandleInternal %p didOpenStream() maxPendingSendAllowed=%d", this, maxPendingSendAllowed);
    ASSERT(maxPendingSendAllowed > 0);
    if (m_handle && m_socket) {
        ASSERT(socketHandle == m_socket.get());
        m_maxPendingSendAllowed = maxPendingSendAllowed;
        m_handle->m_state = SocketStreamHandleBase::Open;
        if (m_handle->m_client) {
            m_handle->m_client->didOpenSocketStream(m_handle);
            return;
        }
    }
    LOG(Network, "SocketStreamHandleInternal %p didOpenStream() m_handle or m_socket is NULL", this);
}

void SocketStreamHandleInternal::didSendData(WebKit::WebSocketStreamHandle* socketHandle, int amountSent)
{
    LOG(Network, "SocketStreamHandleInternal %p didSendData() amountSent=%d", this, amountSent);
    ASSERT(amountSent > 0);
    if (m_handle && m_socket) {
        ASSERT(socketHandle == m_socket.get());
        m_pendingAmountSent -= amountSent;
        ASSERT(m_pendingAmountSent >= 0);
        m_handle->sendPendingData();
    }
}

void SocketStreamHandleInternal::didReceiveData(WebKit::WebSocketStreamHandle* socketHandle, const WebKit::WebData& data)
{
    LOG(Network, "SocketStreamHandleInternal %p didReceiveData() Received %lu bytes", this, static_cast<unsigned long>(data.size()));
    if (m_handle && m_socket) {
        ASSERT(socketHandle == m_socket.get());
        if (m_handle->m_client)
            m_handle->m_client->didReceiveSocketStreamData(m_handle, data.data(), data.size());
    }
}

void SocketStreamHandleInternal::didClose(WebKit::WebSocketStreamHandle* socketHandle)
{
    LOG(Network, "SocketStreamHandleInternal %p didClose()", this);
    if (m_handle && m_socket) {
        ASSERT(socketHandle == m_socket.get());
        m_socket.clear();
        SocketStreamHandle* h = m_handle;
        m_handle = 0;
        if (h->m_client)
            h->m_client->didCloseSocketStream(h);
    }
}

void SocketStreamHandleInternal::didFail(WebKit::WebSocketStreamHandle* socketHandle, const WebKit::WebSocketStreamError& err)
{
    LOG(Network, "SocketStreamHandleInternal %p didFail()", this);
    if (m_handle && m_socket) {
        ASSERT(socketHandle == m_socket.get());
        if (m_handle->m_client)
            m_handle->m_client->didFailSocketStream(m_handle, *(PassRefPtr<SocketStreamError>(err)));
    }
}

// FIXME: auth

// SocketStreamHandle ----------------------------------------------------------

SocketStreamHandle::SocketStreamHandle(const KURL& url, SocketStreamHandleClient* client)
    : SocketStreamHandleBase(url, client)
{
    m_internal = SocketStreamHandleInternal::create(this);
    m_internal->connect(m_url);
}

SocketStreamHandle::~SocketStreamHandle()
{
    setClient(0);
    m_internal.clear();
}

int SocketStreamHandle::platformSend(const char* buf, int len)
{
    if (!m_internal)
        return 0;
    return m_internal->send(buf, len);
}

void SocketStreamHandle::platformClose()
{
    if (m_internal)
        m_internal->close();
}

}  // namespace WebCore
