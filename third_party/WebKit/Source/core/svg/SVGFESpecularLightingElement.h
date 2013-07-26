/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Oliver Hunt <oliver@nerget.com>
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

#ifndef SVGFESpecularLightingElement_h
#define SVGFESpecularLightingElement_h

#include "core/platform/graphics/filters/FESpecularLighting.h"
#include "core/svg/SVGAnimatedNumber.h"
#include "core/svg/SVGFELightElement.h"
#include "core/svg/SVGFilterPrimitiveStandardAttributes.h"

namespace WebCore {

class SVGFESpecularLightingElement FINAL : public SVGFilterPrimitiveStandardAttributes {
public:
    static PassRefPtr<SVGFESpecularLightingElement> create(const QualifiedName&, Document*);
    void lightElementAttributeChanged(const SVGFELightElement*, const QualifiedName&);

private:
    SVGFESpecularLightingElement(const QualifiedName&, Document*);

    bool isSupportedAttribute(const QualifiedName&);
    virtual void parseAttribute(const QualifiedName&, const AtomicString&) OVERRIDE;
    virtual bool setFilterEffectAttribute(FilterEffect*, const QualifiedName&);
    virtual void svgAttributeChanged(const QualifiedName&);
    virtual PassRefPtr<FilterEffect> build(SVGFilterBuilder*, Filter*);

    static const AtomicString& kernelUnitLengthXIdentifier();
    static const AtomicString& kernelUnitLengthYIdentifier();

    BEGIN_DECLARE_ANIMATED_PROPERTIES(SVGFESpecularLightingElement)
        DECLARE_ANIMATED_STRING(In1, in1)
        DECLARE_ANIMATED_NUMBER(SpecularConstant, specularConstant)
        DECLARE_ANIMATED_NUMBER(SpecularExponent, specularExponent)
        DECLARE_ANIMATED_NUMBER(SurfaceScale, surfaceScale)
        DECLARE_ANIMATED_NUMBER(KernelUnitLengthX, kernelUnitLengthX)
        DECLARE_ANIMATED_NUMBER(KernelUnitLengthY, kernelUnitLengthY)
    END_DECLARE_ANIMATED_PROPERTIES
};

} // namespace WebCore

#endif
