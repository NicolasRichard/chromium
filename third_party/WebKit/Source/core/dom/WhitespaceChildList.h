/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
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

#ifndef WhitespaceChildList_h
#define WhitespaceChildList_h

#include "core/dom/Text.h"
#include "core/rendering/style/RenderStyleConstants.h"

namespace WebCore {

class Text;

// Tracks a limited number of whitespace text children during style recalc
// to postpone their style recalc as part of optimization to avoid creating
// unnecessary whitespace text renderers. If we hit the limit, it recalcs
// the whitespace text's style and clears the list.
class WhitespaceChildList {
public:
    WhitespaceChildList(StyleRecalcChange change)
        : m_change(change)
    { }

    void append(Text* textChild)
    {
        ASSERT(textChild->containsOnlyWhitespace());
        if (m_list.size() == maxWhitespaceChildrenToDefer) {
            recalcStyle();
            m_list.clear();
        }
        m_list.append(textChild);
    }

    void recalcStyle() const
    {
        for (unsigned i = 0; i < m_list.size(); ++i)
            m_list[i]->recalcTextStyle(m_change);
    }
private:
    StyleRecalcChange m_change;

    static const unsigned maxWhitespaceChildrenToDefer = 10;
    Vector<Text*, maxWhitespaceChildrenToDefer> m_list;
};

} // namespace WebCore

#endif // WhitespaceChildList_h
