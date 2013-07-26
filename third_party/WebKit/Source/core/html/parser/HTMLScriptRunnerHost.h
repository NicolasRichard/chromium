/*
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HTMLScriptRunnerHost_h
#define HTMLScriptRunnerHost_h

#include "wtf/Forward.h"

namespace WebCore {

class CachedResource;
class Element;
class HTMLInputStream;
class ScriptSourceCode;

class HTMLScriptRunnerHost {
public:
    virtual ~HTMLScriptRunnerHost() { }

    // Implementors should call cachedResource->addClient() here or soon after.
    virtual void watchForLoad(CachedResource*) = 0;
    // Implementors must call cachedResource->removeClient() immediately.
    virtual void stopWatchingForLoad(CachedResource*) = 0;

    virtual HTMLInputStream& inputStream() = 0;

    virtual bool hasPreloadScanner() const = 0;
    virtual void appendCurrentInputStreamToPreloadScannerAndScan() = 0;

};

}

#endif
