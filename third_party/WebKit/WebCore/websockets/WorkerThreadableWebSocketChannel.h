/*
 * Copyright (C) 2009, 2010 Google Inc.  All rights reserved.
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

#ifndef WorkerThreadableWebSocketChannel_h
#define WorkerThreadableWebSocketChannel_h

#if ENABLE(WEB_SOCKETS) && ENABLE(WORKERS)

#include "PlatformString.h"
#include "ThreadableWebSocketChannel.h"
#include "WebSocketChannelClient.h"

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Threading.h>

namespace WebCore {

class KURL;
class ScriptExecutionContext;
class ThreadableWebSocketChannelClientWrapper;
class WorkerContext;
class WorkerLoaderProxy;
class WorkerRunLoop;

class WorkerThreadableWebSocketChannel : public RefCounted<WorkerThreadableWebSocketChannel>, public ThreadableWebSocketChannel {
public:
    static PassRefPtr<ThreadableWebSocketChannel> create(WorkerContext* workerContext, WebSocketChannelClient* client, const String& taskMode, const KURL& url, const String& protocol)
    {
        return adoptRef(new WorkerThreadableWebSocketChannel(workerContext, client, taskMode, url, protocol));
    }
    virtual ~WorkerThreadableWebSocketChannel();

    virtual void connect();
    virtual bool send(const String& message);
    virtual unsigned long bufferedAmount() const;
    virtual void close();
    virtual void disconnect(); // Will suppress didClose().

    using RefCounted<WorkerThreadableWebSocketChannel>::ref;
    using RefCounted<WorkerThreadableWebSocketChannel>::deref;

protected:
    virtual void refThreadableWebSocketChannel() { ref(); }
    virtual void derefThreadableWebSocketChannel() { deref(); }

private:
    // Generated by the bridge.  The Peer and its bridge should have identical
    // lifetimes.
    class Peer : public WebSocketChannelClient, public Noncopyable {
    public:
        static Peer* create(RefPtr<ThreadableWebSocketChannelClientWrapper> clientWrapper, WorkerLoaderProxy& loaderProxy, ScriptExecutionContext* context, const String& taskMode, const KURL& url, const String& protocol)
        {
            return new Peer(clientWrapper, loaderProxy, context, taskMode, url, protocol);
        }
        ~Peer();

        void connect();
        void send(const String& message);
        void bufferedAmount();
        void close();
        void disconnect();

        virtual void didConnect();
        virtual void didReceiveMessage(const String& message);
        virtual void didClose();

    private:
        Peer(RefPtr<ThreadableWebSocketChannelClientWrapper>, WorkerLoaderProxy&, ScriptExecutionContext*, const String& taskMode, const KURL&, const String& protocol);

        RefPtr<ThreadableWebSocketChannelClientWrapper> m_workerClientWrapper;
        WorkerLoaderProxy& m_loaderProxy;
        RefPtr<ThreadableWebSocketChannel> m_mainWebSocketChannel;
        String m_taskMode;
    };

    // Bridge for Peer.  Running on the worker thread.
    class Bridge : public RefCounted<Bridge> {
    public:
        Bridge(PassRefPtr<ThreadableWebSocketChannelClientWrapper>, PassRefPtr<WorkerContext>, const String& taskMode, const KURL&, const String& protocol);
        ~Bridge();
        void connect();
        bool send(const String& message);
        unsigned long bufferedAmount();
        void close();
        void disconnect();

        using RefCounted<Bridge>::ref;
        using RefCounted<Bridge>::deref;

    private:
        static void setWebSocketChannel(ScriptExecutionContext*, Bridge* thisPtr, Peer*, RefPtr<ThreadableWebSocketChannelClientWrapper>);

        // Executed on the main thread to create a Peer for this bridge.
        static void mainThreadCreateWebSocketChannel(ScriptExecutionContext*, Bridge* thisPtr, RefPtr<ThreadableWebSocketChannelClientWrapper>, const String& taskMode, const KURL&, const String& protocol);

        // Executed on the worker context's thread.
        void clearClientWrapper();

        void setMethodNotCompleted();
        void waitForMethodCompletion();

        RefPtr<ThreadableWebSocketChannelClientWrapper> m_workerClientWrapper;
        RefPtr<WorkerContext> m_workerContext;
        WorkerLoaderProxy& m_loaderProxy;
        String m_taskMode;
        Peer* m_peer;
    };

    WorkerThreadableWebSocketChannel(WorkerContext*, WebSocketChannelClient*, const String& taskMode, const KURL&, const String& protocol);

    static void mainThreadConnect(ScriptExecutionContext*, Peer*);
    static void mainThreadSend(ScriptExecutionContext*, Peer*, const String& message);
    static void mainThreadBufferedAmount(ScriptExecutionContext*, Peer*);
    static void mainThreadClose(ScriptExecutionContext*, Peer*);
    static void mainThreadDestroy(ScriptExecutionContext*, Peer*);

    RefPtr<WorkerContext> m_workerContext;
    RefPtr<ThreadableWebSocketChannelClientWrapper> m_workerClientWrapper;
    RefPtr<Bridge> m_bridge;
};

} // namespace WebCore

#endif // ENABLE(WEB_SOCKETS)

#endif // WorkerThreadableWebSocketChannel_h
