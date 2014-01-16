/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#ifndef SVGCircleElement_h
#define SVGCircleElement_h

#include "SVGNames.h"
#include "core/svg/SVGAnimatedBoolean.h"
#include "core/svg/SVGAnimatedLength.h"
#include "core/svg/SVGGeometryElement.h"

namespace WebCore {

class SVGCircleElement FINAL : public SVGGeometryElement {
public:
    static PassRefPtr<SVGCircleElement> create(Document&);

    SVGAnimatedLength* cx() const { return m_cx.get(); }
    SVGAnimatedLength* cy() const { return m_cy.get(); }
    SVGAnimatedLength* r() const { return m_r.get(); }

private:
    explicit SVGCircleElement(Document&);

    virtual bool supportsFocus() const OVERRIDE { return hasFocusEventListeners(); }

    bool isSupportedAttribute(const QualifiedName&);
    virtual void parseAttribute(const QualifiedName&, const AtomicString&) OVERRIDE;
    virtual void svgAttributeChanged(const QualifiedName&) OVERRIDE;

    virtual bool selfHasRelativeLengths() const OVERRIDE;

    virtual RenderObject* createRenderer(RenderStyle*) OVERRIDE;

    RefPtr<SVGAnimatedLength> m_cx;
    RefPtr<SVGAnimatedLength> m_cy;
    RefPtr<SVGAnimatedLength> m_r;
    BEGIN_DECLARE_ANIMATED_PROPERTIES(SVGCircleElement)
    END_DECLARE_ANIMATED_PROPERTIES
};

DEFINE_NODE_TYPE_CASTS(SVGCircleElement, hasTagName(SVGNames::circleTag));

} // namespace WebCore

#endif // SVGCircleElement_h
