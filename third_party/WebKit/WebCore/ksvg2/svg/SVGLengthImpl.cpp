/*
    Copyright (C) 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>

    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "config.h"
#include <qrect.h>

#include <kdebug.h>

#include <kcanvas/KCanvas.h>
#include <kcanvas/RenderPath.h>
#include "KCanvasRenderingStyle.h"

#include "ksvg.h"
#include "svgpathparser.h"
#include "SVGLengthImpl.h"
#include "SVGElementImpl.h"
#include "SVGSVGElementImpl.h"
#include "SVGAnimatedRectImpl.h"
#include "SVGAnimatedLengthImpl.h"

#include <cmath>

using namespace KSVG;
using namespace std;

// keep track of textual description of the unit type
QString UnitText[] =
{
    QString::fromLatin1(""), QString::fromLatin1(""),
    QString::fromLatin1("%"), QString::fromLatin1("em"),
    QString::fromLatin1("ex"), QString::fromLatin1("px"),
    QString::fromLatin1("cm"), QString::fromLatin1("mm"),
    QString::fromLatin1("in"), QString::fromLatin1("pt"),
    QString::fromLatin1("pc")
};

SVGLengthImpl::SVGLengthImpl(const SVGStyledElementImpl *context, LengthMode mode, const SVGElementImpl *viewport) : KDOM::Shared<SVGLengthImpl>()
{
    m_mode = mode;
    m_context = context;
    m_viewportElement = viewport;

    m_value = 0;
    m_valueInSpecifiedUnits = 0;

    m_bboxRelative = false;
    m_unitType = SVG_LENGTHTYPE_UNKNOWN;
}

SVGLengthImpl::~SVGLengthImpl()
{
}

unsigned short SVGLengthImpl::unitType() const
{
    return m_unitType;
}

void SVGLengthImpl::setValue(float value)
{
    m_value = value;
    updateValueInSpecifiedUnits();
}

float SVGLengthImpl::value() const
{
    if(m_unitType != SVG_LENGTHTYPE_PERCENTAGE)
        return m_value;

    float value = m_valueInSpecifiedUnits / 100.0;
    if(m_bboxRelative)
        return value;

    // Spec: When percentages are used with attributes that define the
    // gradient vector, the pattern tile, the filter region or the masking
    // region, a percentage represents the same value as the corresponding
    // decimal value (e.g., 50% means the same as 0.5). 
    khtml::RenderObject *item = (m_context ? m_context->renderer() : 0);
    if(item)
    {
        QRect bbox = item->relativeBBox();

        float result = 0;
        if(m_mode == LM_WIDTH)
            result = value * (bbox.width() - 1);
        else if(m_mode == LM_HEIGHT)
            result = value * (bbox.height() - 1);
        else if(m_mode == LM_OTHER)
            result = value * sqrt(pow(double(bbox.width() - 1), 2) + pow(double(bbox.height() - 1), 2)) / sqrt(2.0);

        return result;
    }

    return SVGHelper::PercentageOfViewport(value, m_viewportElement, m_mode);
}

void SVGLengthImpl::setValueInSpecifiedUnits(float valueInSpecifiedUnits)
{
    m_valueInSpecifiedUnits = valueInSpecifiedUnits;
    updateValue();
}

float SVGLengthImpl::valueInSpecifiedUnits() const
{
    return m_valueInSpecifiedUnits;
}                                                

void SVGLengthImpl::setValueAsString(KDOM::DOMStringImpl *valueAsStringImpl)
{
    KDOM::DOMString valueAsString(valueAsStringImpl);
    if(valueAsString.isEmpty())
        return;

    QString valueAsQString = valueAsString.qstring();

    double convertedNumber = 0;
    const char *start = valueAsQString.latin1();
    const char *end = parseCoord(start, convertedNumber);
    m_valueInSpecifiedUnits = convertedNumber;

    unsigned int diff = end - start;
    if(diff < valueAsQString.length())
    {
        if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_PX]))
            m_unitType = SVG_LENGTHTYPE_PX;
        else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_CM]))
            m_unitType = SVG_LENGTHTYPE_CM;
        else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_PC]))
            m_unitType = SVG_LENGTHTYPE_PC;
        else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_MM]))
            m_unitType = SVG_LENGTHTYPE_MM;
        else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_IN]))
            m_unitType = SVG_LENGTHTYPE_IN;
        else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_PT]))
            m_unitType = SVG_LENGTHTYPE_PT;
         else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_PERCENTAGE]))
            m_unitType = SVG_LENGTHTYPE_PERCENTAGE;
        else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_EMS]))
            m_unitType = SVG_LENGTHTYPE_EMS;
        else if(valueAsQString.endsWith(UnitText[SVG_LENGTHTYPE_EXS]))
            m_unitType = SVG_LENGTHTYPE_EXS;
        else if(valueAsQString.isEmpty())
            m_unitType = SVG_LENGTHTYPE_NUMBER;
        else
            m_unitType = SVG_LENGTHTYPE_UNKNOWN;
    }
    else
        m_unitType = SVG_LENGTHTYPE_PX;

    updateValue();
}

KDOM::DOMStringImpl *SVGLengthImpl::valueAsString() const
{
    return new KDOM::DOMStringImpl(QString::number(m_valueInSpecifiedUnits) + UnitText[m_unitType]);
}

void SVGLengthImpl::newValueSpecifiedUnits(unsigned short unitType, float valueInSpecifiedUnits)
{
    m_valueInSpecifiedUnits = valueInSpecifiedUnits;
    m_unitType = unitType;
    updateValue();
}

void SVGLengthImpl::convertToSpecifiedUnits(unsigned short unitType)
{
    m_unitType = unitType;
    updateValueInSpecifiedUnits();
}

double SVGLengthImpl::dpi() const
{
    /* FIXME: DPI detection
    if(context && context->ownerDoc())
    {
        if(mode == LM_WIDTH)
            return 25.4 * context->ownerDoc()->screenPixelsPerMillimeterX();
        else if(mode == LM_HEIGHT)
            return 25.4 * context->ownerDoc()->screenPixelsPerMillimeterY();
        else if(mode == LM_OTHER)
            return 25.4 * context->ownerDoc()->screenPixelsPerMillimeterX();
    }
    */

    return 90.0;
}

void SVGLengthImpl::updateValue()
{
    switch(m_unitType)
    {
        case SVG_LENGTHTYPE_PX:
            m_value = m_valueInSpecifiedUnits;
            break;
        case SVG_LENGTHTYPE_CM:
            m_value = (m_valueInSpecifiedUnits / 2.54) * dpi();
            break;
        case SVG_LENGTHTYPE_MM:
            m_value = (m_valueInSpecifiedUnits / 25.4) * dpi();
            break;
        case SVG_LENGTHTYPE_IN:
            m_value = m_valueInSpecifiedUnits * dpi();
            break;
        case SVG_LENGTHTYPE_PT:
            m_value = (m_valueInSpecifiedUnits / 72.0) * dpi();
            break;
        case SVG_LENGTHTYPE_PC:
            m_value = (m_valueInSpecifiedUnits / 6.0) * dpi();
            break;
        case SVG_LENGTHTYPE_EMS: // Be careful here, always recheck coords-units-BE-01.svg after touching (Niko)
        case SVG_LENGTHTYPE_EXS:
            if(m_context)
            {
                /* FIXME: EMS/EXS handling
                KCanvasRenderingStyle *style = context->style();
                KCFontProperties *fontProperites = style->fontProperties();

                bool sizeLocal = (style->getFontSize() != -1);
                bool familyLocal = (style->getFontFamily() && style->getFontFamily()->getFirst());
                
                SVGStylableImpl *parentStyle = 0;
                if((!sizeLocal || !familyLocal) && context) {
                    parentStyle = dynamic_cast<SVGStylableImpl *>(context->ownerDoc()->getElementFromHandle(context->parentNode().handle()));
                
                // Look up font-size in a SAFE way, because at this place
                // processStyle() has NOT yet been called, so we need
                // a different solution (Niko)
                QString useFont = "Arial";
                double useSize = 12;
                
                if(sizeLocal)
                    useSize = style->getFontSize();
                else if(parentStyle && parentStyle->getFontSize() != -1)
                    useSize = parentStyle->getFontSize();
            
                if(familyLocal)
                    useFont = style->getFontFamily()->getFirst()->qstring();
                else if(parentStyle && parentStyle->getFontFamily() && parentStyle->getFontFamily()->getFirst())
                    useFont = parentStyle->getFontFamily()->getFirst()->qstring();

                if(unitType == SVG_LENGTHTYPE_EMS)
                    value = valueInSpecifiedUnits * useSize;
                else
                {
                    // Easiest way, use qfont (Niko)
                    QFont font(useFont);
                    font.setPixelSize(static_cast<int>(useSize));

                    QFontMetrics fm(font);
                    value = valueInSpecifiedUnits * fm.boundingRect('x').height();
                }
            }

            break;*/
        }
    }

    if(m_context)
        m_context->notifyAttributeChange();
}

bool SVGLengthImpl::updateValueInSpecifiedUnits()
{
    if(m_unitType == SVG_LENGTHTYPE_UNKNOWN)
        return false;

    switch(m_unitType)
    {
        case SVG_LENGTHTYPE_PERCENTAGE:
            kdError() << "updateValueInSpecifiedUnits() SVG_LENGTHTYPE_PERCENTAGE - UNSUPPORTED! Please report!" << endl;
            return false;
        case SVG_LENGTHTYPE_EMS:
            kdError() << "updateValueInSpecifiedUnits() SVG_LENGTHTYPE_EMS - UNSUPPORTED! Please report!" << endl;
            return false;
        case SVG_LENGTHTYPE_EXS:
            kdError() << "updateValueInSpecifiedUnits() SVG_LENGTHTYPE_EXS - UNSUPPORTED! Please report!" << endl;
            return false;
        case SVG_LENGTHTYPE_PX:
            m_valueInSpecifiedUnits = m_value;
            break;
        case SVG_LENGTHTYPE_CM:
            m_valueInSpecifiedUnits = m_value / dpi() * 2.54;
            break;
        case SVG_LENGTHTYPE_MM:
            m_valueInSpecifiedUnits = m_value / dpi() * 25.4;
            break;
        case SVG_LENGTHTYPE_IN:
            m_valueInSpecifiedUnits = m_value / dpi();
            break;
        case SVG_LENGTHTYPE_PT:
            m_valueInSpecifiedUnits = m_value / dpi() * 72.0;
            break;
        case SVG_LENGTHTYPE_PC:
            m_valueInSpecifiedUnits = m_value / dpi() * 6.0;
            break;
    };
    
    if(m_context)
        m_context->notifyAttributeChange();

    return true;
}

bool SVGLengthImpl::bboxRelative() const
{
    return m_bboxRelative;
}

void SVGLengthImpl::setBboxRelative(bool relative)
{
    m_bboxRelative = relative;
}

const SVGStyledElementImpl *SVGLengthImpl::context() const
{
    return m_context;
}

void SVGLengthImpl::setContext(const SVGStyledElementImpl *context)
{
    m_context = context;
}

// vim:ts=4:noet
