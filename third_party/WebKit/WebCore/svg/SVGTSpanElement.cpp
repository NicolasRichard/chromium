/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2010 Rob Buis <buis@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#if ENABLE(SVG)
#include "SVGTSpanElement.h"

#include "RenderInline.h"
#include "RenderSVGTSpan.h"
#include "SVGNames.h"

namespace WebCore {

inline SVGTSpanElement::SVGTSpanElement(const QualifiedName& tagName, Document* document)
    : SVGTextPositioningElement(tagName, document)
{
}

PassRefPtr<SVGTSpanElement> SVGTSpanElement::create(const QualifiedName& tagName, Document* document)
{
    return adoptRef(new SVGTSpanElement(tagName, document));
}

bool SVGTSpanElement::childShouldCreateRenderer(Node* child) const
{
    if (child->isTextNode()
#if ENABLE(SVG_FONTS)
        || child->hasTagName(SVGNames::altGlyphTag)
#endif
        || child->hasTagName(SVGNames::tspanTag)
        || child->hasTagName(SVGNames::trefTag)
        || child->hasTagName(SVGNames::aTag)
        || child->hasTagName(SVGNames::textPathTag))
        return true;

    return false;
}

RenderObject* SVGTSpanElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderSVGTSpan(this);
}

}

#endif // ENABLE(SVG)

// vim:ts=4:noet
