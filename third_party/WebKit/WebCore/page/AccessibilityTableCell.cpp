/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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
#include "AccessibilityTableCell.h"

#include "AXObjectCache.h"
#include "RenderObject.h"
#include "RenderTableCell.h"

using namespace std;

namespace WebCore {
    
AccessibilityTableCell::AccessibilityTableCell(RenderObject* renderer)
    : AccessibilityRenderObject(renderer)
{
}

AccessibilityTableCell::~AccessibilityTableCell()
{
}

PassRefPtr<AccessibilityTableCell> AccessibilityTableCell::create(RenderObject* renderer)
{
    return adoptRef(new AccessibilityTableCell(renderer));
}

bool AccessibilityTableCell::accessibilityIsIgnored() const
{
    if (!m_renderer)
        return true;
    
    AccessibilityObject* renderTable = axObjectCache()->get(static_cast<RenderTableCell*>(m_renderer)->table());
    return renderTable->accessibilityIsIgnored();
}
    
void AccessibilityTableCell::rowIndexRange(pair<int, int>& rowRange)
{
    if (!m_renderer)
        return;
    
    RenderTableCell *renderCell = static_cast<RenderTableCell*>(m_renderer);
    rowRange.first = renderCell->row();
    rowRange.second = renderCell->rowSpan();
    
    // since our table might have multiple sections, we have to offset our row appropriately
    RenderTableSection* section = renderCell->section();
    RenderTable* table = renderCell->table();
    if (!table || !section)
        return;
    
    RenderTableSection* tableSection = table->header();
    if (!tableSection)
        tableSection = table->firstBody();
    
    unsigned rowOffset = 0;
    while (tableSection) {
        if (tableSection == section)
            break;
        rowOffset += tableSection->numRows();
        tableSection = table->sectionBelow(tableSection, true); 
    }

    rowRange.first += rowOffset;
}
    
void AccessibilityTableCell::columnIndexRange(pair<int, int>& columnRange)
{
    if (!m_renderer)
        return;
    
    RenderTableCell *renderCell = static_cast<RenderTableCell*>(m_renderer);
    columnRange.first = renderCell->col();
    columnRange.second = renderCell->colSpan();    
}
    
} // namespace WebCore
