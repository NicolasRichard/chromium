/*
 * Copyright (C) 2010 Company 100, Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
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

#include "config.h"
#include "SearchPopupMenuBrew.h"

namespace WebCore {

// Save the past searches stored in 'searchItems' to a database associated with 'name'
void SearchPopupMenuBrew::saveRecentSearches(const AtomicString& name, const Vector<String>& searchItems)
{
}

// Load past searches associated with 'name' from the database to 'searchItems'
void SearchPopupMenuBrew::loadRecentSearches(const AtomicString& name, Vector<String>& searchItems)
{
}

// Create a search popup menu - not sure what else we have to do here
SearchPopupMenuBrew::SearchPopupMenuBrew(PopupMenuClient* client)
    : m_popup(client)
{
}

bool SearchPopupMenuBrew::enabled()
{
    return false;
}

PopupMenu* SearchPopupMenuBrew::popupMenu()
{
    return &m_popup;
}

} // namespace WebCore
