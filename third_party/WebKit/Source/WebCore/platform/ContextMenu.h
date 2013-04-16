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

#ifndef ContextMenu_h
#define ContextMenu_h

#if ENABLE(CONTEXT_MENUS)

#include <wtf/Noncopyable.h>

#include "ContextMenuItem.h"
#include <wtf/text/WTFString.h>

namespace WebCore {

    class ContextMenuController;

    typedef void* PlatformContextMenu;

    class ContextMenu {
        WTF_MAKE_NONCOPYABLE(ContextMenu); WTF_MAKE_FAST_ALLOCATED;
    public:
        ContextMenu();

        const ContextMenuItem* itemWithAction(unsigned);

        explicit ContextMenu(PlatformContextMenu);

        PlatformContextMenu platformContextMenu() const;

        static PlatformContextMenu createPlatformContextMenuFromItems(const Vector<ContextMenuItem>&);
        static void getContextMenuItems(PlatformContextMenu, Vector<ContextMenuItem>&);

        const ContextMenuItem* itemAtIndex(unsigned index) { return &m_items[index]; }

        void setItems(const Vector<ContextMenuItem>& items) { m_items = items; }
        const Vector<ContextMenuItem>& items() const { return m_items; }

        void appendItem(const ContextMenuItem& item) { m_items.append(item); }

    private:
        Vector<ContextMenuItem> m_items;
    };
}

#endif // ENABLE(CONTEXT_MENUS)
#endif // ContextMenu_h
