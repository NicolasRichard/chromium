/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#import "KWQFontMetrics.h"

#import <Cocoa/Cocoa.h>
#import <kxmlcore/Noncopyable.h>

#import "Font.h"
#import "Logging.h"
#import "KWQExceptions.h"
#import "FoundationExtras.h"

#import "Shared.h"

#import "WebCoreTextRenderer.h"
#import "WebCoreTextRendererFactory.h"

// FIXME: Remove once this class gets folded in to the WebCore namespace.
using WebCore::Font;
using WebCore::FontDescription;
using WebCore::Pitch;
using WebCore::UnknownPitch;
using WebCore::FixedPitch;
using WebCore::VariablePitch;

// We know that none of the ObjC calls here will raise exceptions
// because they are all calls to WebCoreTextRenderer, which has a
// contract of not raising.
struct QFontMetricsPrivate : public Shared<QFontMetricsPrivate>, Noncopyable
{
    QFontMetricsPrivate(const FontDescription& fontDescription)
        : m_fontDescription(fontDescription), m_renderer(nil), m_pitch(UnknownPitch)
    {
        m_webCoreFont.font = nil;
    }
    ~QFontMetricsPrivate()
    {
        KWQRelease(m_renderer);
        KWQRelease(m_webCoreFont.font);
    }

    id <WebCoreTextRenderer> getRenderer()
    {
        if (!m_renderer)
            m_renderer = KWQRetain([[WebCoreTextRendererFactory sharedFactory] rendererWithFont:getWebCoreFont()]);
        return m_renderer;
    }
    
    const FontDescription& fontDescription() const { return m_fontDescription; }
    void setFontDescription(const FontDescription& fontDescription)
    {
        if (m_fontDescription == fontDescription)
            return;
        m_fontDescription = fontDescription;
        KWQRelease(m_renderer);
        m_renderer = nil;
        KWQRelease(m_webCoreFont.font);
        m_webCoreFont.font = nil;
        m_pitch = UnknownPitch;
    }

    const WebCoreFont& getWebCoreFont() const
    {
        if (!m_webCoreFont.font) {
            CREATE_FAMILY_ARRAY(fontDescription(), families);
            KWQ_BLOCK_EXCEPTIONS;
            int traits = 0;
            if (m_fontDescription.italic())
                traits |= NSItalicFontMask;
            if (m_fontDescription.weight() >= WebCore::cBoldWeight)
                traits |= NSBoldFontMask;
            m_webCoreFont = [[WebCoreTextRendererFactory sharedFactory] 
                             fontWithFamilies:families traits:traits size:m_fontDescription.computedPixelSize()];
            KWQRetain(m_webCoreFont.font);
            m_webCoreFont.forPrinter = m_fontDescription.usePrinterFont();
            KWQ_UNBLOCK_EXCEPTIONS;
        }
        return m_webCoreFont;
    }

    bool isFixedPitch() const { if (m_pitch == UnknownPitch) determinePitch(); return m_pitch == FixedPitch; };
    void determinePitch() const {
        KWQ_BLOCK_EXCEPTIONS;
        if ([[WebCoreTextRendererFactory sharedFactory] isFontFixedPitch:getWebCoreFont()])
            m_pitch = FixedPitch;
        else
            m_pitch = VariablePitch;
        KWQ_UNBLOCK_EXCEPTIONS;
    }

private:
    FontDescription m_fontDescription;
    id <WebCoreTextRenderer> m_renderer;
    mutable WebCoreFont m_webCoreFont;
    mutable Pitch m_pitch;
};

QFontMetrics::QFontMetrics()
{
}

QFontMetrics::QFontMetrics(const FontDescription& fontDescription)
    : data(new QFontMetricsPrivate(fontDescription))
{
}

QFontMetrics::QFontMetrics(const QFontMetrics &other)
    : data(other.data)
{
}

QFontMetrics::~QFontMetrics()
{
}

QFontMetrics &QFontMetrics::operator=(const QFontMetrics &other)
{
    data = other.data;
    return *this;
}

void QFontMetrics::setFontDescription(const FontDescription& fontDescription)
{
    if (!data)
        data = new QFontMetricsPrivate(fontDescription);
    else
        data->setFontDescription(fontDescription);
}

int QFontMetrics::ascent() const
{
    if (!data) {
        LOG_ERROR("called ascent on an empty QFontMetrics");
        return 0;
    }
    
    return [data->getRenderer() ascent];
}

int QFontMetrics::descent() const
{
    if (!data) {
        LOG_ERROR("called descent on an empty QFontMetrics");
        return 0;
    }
    
    return [data->getRenderer() descent];
}

int QFontMetrics::height() const
{
    // According to Qt documentation: 
    // "This is always equal to ascent()+descent()+1 (the 1 is for the base line)."
    // We DO NOT match the Qt behavior here.  This is intentional.
    return ascent() + descent();
}

int QFontMetrics::lineSpacing() const
{
    if (!data) {
        LOG_ERROR("called lineSpacing on an empty QFontMetrics");
        return 0;
    }
    return [data->getRenderer() lineSpacing];
}

float QFontMetrics::xHeight() const
{
    if (!data) {
        LOG_ERROR("called xHeight on an empty QFontMetrics");
        return 0;
    }
    return [data->getRenderer() xHeight];
}

int QFontMetrics::width(const QString &qstring, int tabWidth, int xpos, int len) const
{
    if (!data) {
        LOG_ERROR("called width on an empty QFontMetrics");
        return 0;
    }
    
    CREATE_FAMILY_ARRAY(data->fontDescription(), families);

    int length = len == -1 ? qstring.length() : len;

    WebCoreTextRun run;
    WebCoreInitializeTextRun(&run, (const UniChar *)qstring.unicode(), length, 0, length);
    
    WebCoreTextStyle style;
    WebCoreInitializeEmptyTextStyle(&style);
    style.families = families;
    style.tabWidth = tabWidth;
    style.xpos = xpos;

    return lroundf([data->getRenderer() floatWidthForRun:&run style:&style]);
}

int QFontMetrics::width(const QChar *uchars, int len, int tabWidth, int xpos) const
{
    if (!data) {
        LOG_ERROR("called width on an empty QFontMetrics");
        return 0;
    }
    
    CREATE_FAMILY_ARRAY(data->fontDescription(), families);

    WebCoreTextRun run;
    WebCoreInitializeTextRun(&run, (const UniChar *)uchars, len, 0, len);
    
    WebCoreTextStyle style;
    WebCoreInitializeEmptyTextStyle(&style);
    style.families = families;
    style.tabWidth = tabWidth;
    style.xpos = xpos;

    return lroundf([data->getRenderer() floatWidthForRun:&run style:&style]);
}

float QFontMetrics::floatWidth(const QChar *uchars, int slen, int pos, int len, int tabWidth, int xpos, int letterSpacing, int wordSpacing, bool smallCaps) const
{
    if (!data) {
        LOG_ERROR("called floatWidth on an empty QFontMetrics");
        return 0;
    }

    CREATE_FAMILY_ARRAY(data->fontDescription(), families);

    WebCoreTextRun run;
    WebCoreInitializeTextRun(&run, (const UniChar *)uchars, slen, pos, pos+len);
    
    WebCoreTextStyle style;
    WebCoreInitializeEmptyTextStyle(&style);
    style.tabWidth = tabWidth;
    style.xpos = xpos;
    style.letterSpacing = letterSpacing;
    style.wordSpacing = wordSpacing;
    style.smallCaps = smallCaps;
    style.families = families;

    return [data->getRenderer() floatWidthForRun:&run style:&style];
}

IntRect QFontMetrics::selectionRectForText(int x, int y, int h, int tabWidth, int xpos,
    const QChar *str, int len, int from, int to, int toAdd,
    bool rtl, bool visuallyOrdered, int letterSpacing, int wordSpacing, bool smallCaps) const
{
    CREATE_FAMILY_ARRAY(data->fontDescription(), families);

    if (from < 0)
        from = 0;
    if (to < 0)
        to = len;
        
    WebCoreTextRun run;
    WebCoreInitializeTextRun(&run, (const UniChar *)str, len, from, to);    
    WebCoreTextStyle style;
    WebCoreInitializeEmptyTextStyle(&style);
    style.rtl = rtl;
    style.directionalOverride = visuallyOrdered;
    style.letterSpacing = letterSpacing;
    style.wordSpacing = wordSpacing;
    style.smallCaps = smallCaps;
    style.families = families;    
    style.padding = toAdd;
    style.tabWidth = tabWidth;
    style.xpos = xpos;
    WebCoreTextGeometry geometry;
    WebCoreInitializeEmptyTextGeometry(&geometry);
    geometry.point = NSMakePoint(x, y);
    geometry.selectionY = y;
    geometry.selectionHeight = h;
    geometry.useFontMetricsForSelectionYAndHeight = false;
    return enclosingIntRect([data->getRenderer() selectionRectForRun:&run style:&style geometry:&geometry]);
}

int QFontMetrics::checkSelectionPoint(const QChar *s, int slen, int pos, int len, int toAdd, int tabWidth, int xpos, int letterSpacing, int wordSpacing, bool smallCaps, int x, bool reversed, bool dirOverride, bool includePartialGlyphs) const
{
    if (!data) {
        LOG_ERROR("called checkSelectionPoint on an empty QFontMetrics");
        return 0;
    }
    
    CREATE_FAMILY_ARRAY(data->fontDescription(), families);

    WebCoreTextRun run;
    WebCoreInitializeTextRun(&run, (const UniChar *)s, slen, pos, pos+len);
    
    WebCoreTextStyle style;
    WebCoreInitializeEmptyTextStyle(&style);
    style.letterSpacing = letterSpacing;
    style.wordSpacing = wordSpacing;
    style.smallCaps = smallCaps;
    style.families = families;
    style.padding = toAdd;
    style.tabWidth = tabWidth;
    style.xpos = xpos;
    style.rtl = reversed;
    style.directionalOverride = dirOverride;

    return [data->getRenderer() pointToOffset:&run style:&style position:x includePartialGlyphs:includePartialGlyphs];
}

IntRect QFontMetrics::boundingRect(QChar c) const
{
    return IntRect(0, 0, width(c, 0, 0), height());
}

IntRect QFontMetrics::boundingRect(const QString &qstring, int tabWidth, int xpos, int len) const
{
    return IntRect(0, 0, width(qstring, tabWidth, xpos, len), height());
}

IntRect QFontMetrics::boundingRect(int x, int y, int width, int height, int flags, const QString &str, int tabWidth, int xpos) const
{
    // FIXME: need to support word wrapping?
    return intersection(IntRect(x, y, width, height), boundingRect(str, tabWidth, xpos));
}

IntSize QFontMetrics::size(int, const QString &qstring, int tabWidth, int xpos) const
{
    return IntSize(width(qstring, tabWidth, xpos), height());
}

const WebCoreFont& QFontMetrics::getWebCoreFont() const
{
    return data->getWebCoreFont();
}

bool QFontMetrics::isFixedPitch() const
{
    return data->isFixedPitch();
}

