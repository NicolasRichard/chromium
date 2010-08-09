/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
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
 *
 */

#include "config.h"
#include "TextEvent.h"

#include "EventNames.h"

namespace WebCore {

PassRefPtr<TextEvent> TextEvent::create()
{
    return adoptRef(new TextEvent);
}

PassRefPtr<TextEvent> TextEvent::create(PassRefPtr<AbstractView> view, const String& data)
{
    return adoptRef(new TextEvent(view, data));
}

PassRefPtr<TextEvent> TextEvent::createForPlainTextPaste(PassRefPtr<AbstractView> view, const String& data, bool shouldSmartReplace)
{
    return adoptRef(new TextEvent(view, data, 0, true, shouldSmartReplace));
}

PassRefPtr<TextEvent> TextEvent::createForFragmentPaste(PassRefPtr<AbstractView> view, PassRefPtr<DocumentFragment> data, bool shouldSmartReplace, bool shouldMatchStyle)
{
    return adoptRef(new TextEvent(view, "", data, true, shouldSmartReplace, shouldMatchStyle));
}

TextEvent::TextEvent()
    : m_isLineBreak(false)
    , m_isBackTab(false)
    , m_isPaste(false)
    , m_shouldSmartReplace(false)
    , m_shouldMatchStyle(false)
{
}

TextEvent::TextEvent(PassRefPtr<AbstractView> view, const String& data, PassRefPtr<DocumentFragment> pastingFragment,
                     bool isPaste, bool shouldSmartReplace, bool shouldMatchStyle)
    : UIEvent(eventNames().textInputEvent, true, true, view, 0)
    , m_data(data)
    , m_isLineBreak(false)
    , m_isBackTab(false)
    , m_pastingFragment(pastingFragment)
    , m_isPaste(isPaste)
    , m_shouldSmartReplace(shouldSmartReplace)
    , m_shouldMatchStyle(shouldMatchStyle)
{
}

TextEvent::~TextEvent()
{
}

void TextEvent::initTextEvent(const AtomicString& type, bool canBubble, bool cancelable, PassRefPtr<AbstractView> view, const String& data)
{
    if (dispatched())
        return;

    initUIEvent(type, canBubble, cancelable, view, 0);

    m_data = data;
}

bool TextEvent::isTextEvent() const
{
    return true;
}

} // namespace WebCore
