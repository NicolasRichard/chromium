/*
 * Copyright (C) 2004, 2005, 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2008 Rob Buis <buis@kde.org>
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
#include "SVGPathSegArc.h"

namespace WebCore {

String SVGPathSegArc::toString() const
{
    return pathSegTypeAsLetter() + String::format(" %.6lg %.6lg %.6lg %d %d %.6lg %.6lg", m_r1, m_r2, m_angle, m_largeArcFlag, m_sweepFlag, m_x, m_y);
}

SVGPathSegArcAbs::SVGPathSegArcAbs(float x, float y, float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag)
    : SVGPathSegArc(x, y, r1, r2, angle, largeArcFlag, sweepFlag)
{
}

SVGPathSegArcRel::SVGPathSegArcRel(float x, float y, float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag)
    : SVGPathSegArc(x, y, r1, r2, angle, largeArcFlag, sweepFlag)
{
}

}

#endif // ENABLE(SVG)

// vim:ts=4:noet
