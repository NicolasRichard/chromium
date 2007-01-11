/*
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2006 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef RenderView_h
#define RenderView_h

#include "RenderBlock.h"

namespace WebCore {

class FrameView;

class RenderView : public RenderBlock {
public:
    RenderView(Node*, FrameView*);
    virtual ~RenderView();

    virtual const char* renderName() const { return "RenderView"; }

    virtual bool isRenderView() const { return true; }

    virtual void layout();
    virtual void calcWidth();
    virtual void calcHeight();
    virtual void calcMinMaxWidth();
    virtual bool absolutePosition(int& xPos, int& yPos, bool fixed = false);

    int docHeight() const;
    int docWidth() const;

    FrameView* frameView() const { return m_frameView; }

    virtual bool hasOverhangingFloats() { return false; }

    virtual void computeAbsoluteRepaintRect(IntRect&, bool fixed = false);
    virtual void repaintViewRectangle(const IntRect&, bool immediate = false);

    virtual void paint(PaintInfo&, int tx, int ty);
    virtual void paintBoxDecorations(PaintInfo&, int tx, int ty);

    void setSelection(RenderObject* start, int startPos, RenderObject* end, int endPos);
    void clearSelection();
    virtual RenderObject* selectionStart() const { return m_selectionStart; }
    virtual RenderObject* selectionEnd() const { return m_selectionEnd; }

    bool printing() const;
    void setPrintImages(bool enable) { m_printImages = enable; }
    bool printImages() const { return m_printImages; }
    void setTruncatedAt(int y) { m_truncatedAt = y; m_bestTruncatedAt = m_truncatorWidth = 0; m_forcedPageBreak = false; }
    void setBestTruncatedAt(int y, RenderObject *forRenderer, bool forcedBreak = false);
    int bestTruncatedAt() const { return m_bestTruncatedAt; }

    int truncatedAt() const { return m_truncatedAt; }

    virtual void absoluteRects(Vector<IntRect>&, int tx, int ty);

    IntRect selectionRect() const;

    void setMaximalOutlineSize(int o) { m_maximalOutlineSize = o; }
    int maximalOutlineSize() const { return m_maximalOutlineSize; }

    virtual IntRect viewRect() const;

    virtual void selectionStartEnd(int& startPos, int& endPos) const;

    IntRect printRect() const { return m_printRect; }
    void setPrintRect(const IntRect& r) { m_printRect = r; }

    void updateWidgetPositions();
    void addWidget(RenderObject*);
    void removeWidget(RenderObject*);

    void setFlexBoxInFirstLayout(RenderObject* r) { m_flexBoxInFirstLayout = r; }
    RenderObject* flexBoxInFirstLayout() { return m_flexBoxInFirstLayout; }

protected:
    FrameView* m_frameView;

    RenderObject* m_selectionStart;
    RenderObject* m_selectionEnd;
    int m_selectionStartPos;
    int m_selectionEndPos;

    // used to ignore viewport width when printing to the printer
    bool m_printImages;
    int m_truncatedAt;

    int m_maximalOutlineSize; // Used to apply a fudge factor to dirty-rect checks on blocks/tables.
    IntRect m_printRect; // Used when printing.

    typedef HashSet<RenderObject*> RenderObjectSet;

    RenderObjectSet m_widgets;

    RenderObject* m_flexBoxInFirstLayout;

private:
    int m_bestTruncatedAt;
    int m_truncatorWidth;
    bool m_forcedPageBreak;
};

} // namespace WebCore

#endif // RenderView_h
