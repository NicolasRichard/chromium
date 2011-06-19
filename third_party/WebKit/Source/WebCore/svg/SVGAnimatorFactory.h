/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
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

#ifndef SVGAnimatorFactory_h
#define SVGAnimatorFactory_h

#if ENABLE(SVG) && ENABLE(SVG_ANIMATION)
#include "SVGAnimatedAngle.h"
#include "SVGAnimatedLength.h"
#include "SVGAnimatedNumber.h"

namespace WebCore {
    
class SVGAnimatorFactory {
public:
    static PassOwnPtr<SVGAnimatedTypeAnimator> create(SVGElement* contextElement, AnimatedAttributeType attributeType, const QualifiedName& attributeName)
    {
        // FIXME: Add animation support for all SVG units.
        switch (attributeType) {
        case AnimatedAngle:
            return adoptPtr(new SVGAnimatedAngleAnimator(contextElement, attributeName));
        case AnimatedLength:
            return adoptPtr(new SVGAnimatedLengthAnimator(contextElement, attributeName));
        case AnimatedNumber:
            return adoptPtr(new SVGAnimatedNumberAnimator(contextElement, attributeName));
        default:
            ASSERT_NOT_REACHED();
            return adoptPtr(new SVGAnimatedLengthAnimator(contextElement, attributeName));
        }
    }

private:
    SVGAnimatorFactory() { }

};
    
} // namespace WebCore

#endif // ENABLE(SVG) && ENABLE(SVG_ANIMATION)
#endif // SVGAnimatorFactory_h
