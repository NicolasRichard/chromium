/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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

#ifndef ContextMenuClient_h
#define ContextMenuClient_h

#if ENABLE(CONTEXT_MENUS)

#include "ContextMenu.h"
#include <wtf/Forward.h>
#include <wtf/PassOwnPtr.h>

namespace WebCore {
    class ContextMenuItem;
    class Frame;
    class HitTestResult;
    class KURL;

    class ContextMenuClient {
    public:
        virtual ~ContextMenuClient() {  }
        virtual void contextMenuDestroyed() = 0;

        virtual PassOwnPtr<ContextMenu> customizeMenu(PassOwnPtr<ContextMenu>) = 0;

        virtual void contextMenuItemSelected(const ContextMenuItem*, const ContextMenu*) = 0;

        virtual void downloadURL(const KURL& url) = 0;
        virtual void searchWithGoogle(const Frame*) = 0;
        virtual void lookUpInDictionary(Frame*) = 0;
        virtual bool isSpeaking() = 0;
        virtual void speak(const String&) = 0;
        virtual void stopSpeaking() = 0;

#if USE(ACCESSIBILITY_CONTEXT_MENUS)
        virtual void showContextMenu() = 0;
#endif
    };
}

#endif // ENABLE(CONTEXT_MENUS)
#endif
