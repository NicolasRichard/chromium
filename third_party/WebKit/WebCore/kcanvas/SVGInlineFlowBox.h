/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 *           (C) 2006 Apple Computer Inc.
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

#ifndef SVGInlineFlowBox_H
#define SVGInlineFlowBox_H

#ifdef SVG_SUPPORT

#include "InlineFlowBox.h"

namespace WebCore {

void paintSVGInlineFlow(InlineFlowBox* flow, RenderObject* object, RenderObject::PaintInfo& i, 
                        int _tx, int _ty);
int placeSVGFlowHorizontally(InlineFlowBox* flow, int x, int& leftPosition, int& rightPosition, bool& needsWordSpacing);
void placeSVGFlowVertically(InlineFlowBox* flow, int& heightOfBlock);

class SVGInlineFlowBox : public InlineFlowBox {
public:
    SVGInlineFlowBox(RenderObject* obj) : InlineFlowBox(obj) {}
    virtual void paint(RenderObject::PaintInfo& i, int _tx, int _ty);
    virtual int placeBoxesHorizontally(int x, int& leftPosition, int& rightPosition, bool& needsWordSpacing);
    virtual void verticallyAlignBoxes(int& heightOfBlock);
};

}

#endif // SVG_SUPPORT

#endif // !SVGInlineFlowBox_H
