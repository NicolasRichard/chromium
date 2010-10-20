/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "FindIndicator.h"

#include "BackingStore.h"
#include <WebCore/Gradient.h>
#include <WebCore/GraphicsContext.h>
#include <WebCore/IntRect.h>
#include <WebCore/Path.h>

using namespace WebCore;

static const float cornerRadius = 3.0;

static const float shadowOffsetX = 0.0;
static const float shadowOffsetY = 1.0;
static const float shadowBlurRadius = 3.0;

static const int shadowRed = 0;
static const int shadowGreen = 0;
static const int shadowBlue = 0;
static const int shadowAlpha = 204;

static const float lightBorderThickness = 1.0;
static const float horizontalPaddingInsideLightBorder = 3.0;
static const float verticalPaddingInsideLightBorder = 1.0;

static const float horizontalBorderInsideShadow = lightBorderThickness + horizontalPaddingInsideLightBorder;
static const float verticalBorderInsideShadow = lightBorderThickness + verticalPaddingInsideLightBorder;

static const float leftBorderThickness = horizontalBorderInsideShadow + shadowOffsetX + shadowBlurRadius / 2.0;
static const float topBorderThickness = verticalBorderInsideShadow - shadowOffsetY + shadowBlurRadius / 2.0;
static const float rightBorderThickness = horizontalBorderInsideShadow - shadowOffsetX + shadowBlurRadius / 2.0;
static const float bottomBorderThickness = verticalBorderInsideShadow + shadowOffsetY + shadowBlurRadius / 2.0;

static const float horizontalOutsetToCenterOfLightBorder = horizontalBorderInsideShadow - lightBorderThickness / 2.0;
static const float verticalOutsetToCenterOfLightBorder = verticalBorderInsideShadow - lightBorderThickness / 2.0;

static const int lightBorderRed = 245;
static const int lightBorderGreen = 230;
static const int lightBorderBlue = 0;
static const int lightBorderAlpha = 255;

static const int gradientDarkRed = 237;
static const int gradientDarkGreen = 204;
static const int gradientDarkBlue = 0;
static const int gradientDarkAlpha = 255;

static const int gradientLightRed = 242;
static const int gradientLightGreen = 239;
static const int gradientLightBlue = 0;
static const int gradientLightAlpha = 255;

namespace WebKit {

PassRefPtr<FindIndicator> FindIndicator::create(const FloatRect& selectionRect, const Vector<FloatRect>& textRects, const SharedMemory::Handle& contentImageHandle)
{
    RefPtr<BackingStore> contentImage = BackingStore::create(enclosingIntRect(selectionRect).size(), contentImageHandle);
    if (!contentImage)
        return 0;

    return adoptRef(new FindIndicator(selectionRect, textRects, contentImage.release()));
}

FindIndicator::FindIndicator(const WebCore::FloatRect& selectionRect, const Vector<WebCore::FloatRect>& textRects, PassRefPtr<BackingStore> contentImage)
    : m_selectionRect(selectionRect)
    , m_textRects(textRects)
    , m_contentImage(contentImage)
{
}

FindIndicator::~FindIndicator()
{
}

static FloatRect inflateRect(const FloatRect& rect, float inflateX, float inflateY)
{
    FloatRect inflatedRect = rect;
    inflatedRect.inflateX(inflateX);
    inflatedRect.inflateY(inflateY);
    
    return inflatedRect;
}

FloatRect FindIndicator::frameRect() const
{
    return FloatRect(m_selectionRect.x() - leftBorderThickness, m_selectionRect.y() - topBorderThickness,
                     m_selectionRect.width() + rightBorderThickness + leftBorderThickness,
                     m_selectionRect.height() + topBorderThickness + bottomBorderThickness);
}

static Color lightBorderColor()
{
    return Color(lightBorderRed, lightBorderGreen, lightBorderBlue, lightBorderAlpha);
}

static Color shadowColor()
{
    return Color(shadowRed, shadowGreen, shadowBlue, shadowAlpha);
}

static Color gradientLightColor()
{
    return Color(gradientLightRed, gradientLightGreen, gradientLightBlue, gradientLightAlpha);
}

static Color gradientDarkColor()
{
    return Color(gradientDarkRed, gradientDarkGreen, gradientDarkBlue, gradientDarkAlpha);
}

static Path pathWithRoundedRect(const FloatRect& pathRect, float radius)
{
    Path path;
    path.addRoundedRect(pathRect, FloatSize(radius, radius));

    return path;
}
    
void FindIndicator::draw(GraphicsContext& graphicsContext, const IntRect& dirtyRect)
{
    for (size_t i = 0; i < m_textRects.size(); ++i) {
        FloatRect textRect = m_textRects[i];
        textRect.move(leftBorderThickness, topBorderThickness);

        graphicsContext.save();
        FloatRect outerPathRect = inflateRect(textRect, horizontalOutsetToCenterOfLightBorder, verticalOutsetToCenterOfLightBorder);
        graphicsContext.setShadow(FloatSize(shadowOffsetX, shadowOffsetY), shadowBlurRadius, shadowColor(), ColorSpaceSRGB);
        graphicsContext.addPath(pathWithRoundedRect(outerPathRect, cornerRadius));
        graphicsContext.setFillColor(lightBorderColor(), ColorSpaceDeviceRGB);
        graphicsContext.fillPath();
        graphicsContext.restore();

        graphicsContext.save();
        FloatRect innerPathRect = inflateRect(textRect, horizontalPaddingInsideLightBorder, verticalPaddingInsideLightBorder);
        graphicsContext.clip(pathWithRoundedRect(innerPathRect, cornerRadius));
        RefPtr<Gradient> gradient = Gradient::create(FloatPoint(innerPathRect.x(), innerPathRect.y()), FloatPoint(innerPathRect.x(), innerPathRect.bottom()));
        gradient->addColorStop(0, gradientLightColor());
        gradient->addColorStop(1, gradientDarkColor());
        graphicsContext.setFillGradient(gradient);
        graphicsContext.fillRect(outerPathRect);
        graphicsContext.restore();

        graphicsContext.save();
        graphicsContext.translate(FloatSize(roundf(leftBorderThickness), roundf(topBorderThickness) + m_contentImage->bounds().height()));
        graphicsContext.scale(FloatSize(1, -1));
        m_contentImage->paint(&graphicsContext, m_contentImage->bounds());
        graphicsContext.restore();
    }
}

} // namespace WebKit
