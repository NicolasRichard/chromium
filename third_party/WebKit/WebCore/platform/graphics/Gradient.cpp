/*
 * Copyright (C) 2006, 2007, 2008 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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
#include "Gradient.h"

#include "CSSParser.h"

namespace WebCore {

Gradient::Gradient(const FloatPoint& p0, const FloatPoint& p1)
    : m_radial(false)
    , m_p0(p0)
    , m_p1(p1)
    , m_r0(0)
    , m_r1(0)
    , m_stopsSorted(false)
    , m_lastStop(0)
{
    platformInit();
}

Gradient::Gradient(const FloatPoint& p0, float r0, const FloatPoint& p1, float r1)
    : m_radial(true)
    , m_p0(p0)
    , m_p1(p1)
    , m_r0(r0)
    , m_r1(r1)
    , m_stopsSorted(false)
    , m_lastStop(0)
{
    platformInit();
}

Gradient::~Gradient()
{
    platformDestroy();
}

void Gradient::addColorStop(float value, const String& color)
{
    RGBA32 rgba = 0; // default is transparant black
    CSSParser::parseColor(rgba, color);
    m_stops.append(ColorStop(value,
        ((rgba >> 16) & 0xFF) / 255.0f,
        ((rgba >> 8) & 0xFF) / 255.0f,
        (rgba & 0xFF) / 255.0f,
        ((rgba >> 24) & 0xFF) / 255.0f));

    m_stopsSorted = false;

    platformDestroy();
}

void Gradient::addColorStop(float value, const Color& color)
{
    m_stops.append(ColorStop(value,
        color.red() / 255.0f,
        color.green() / 255.0f,
        color.blue() / 255.0f,
        color.alpha() / 255.0f));

    m_stopsSorted = false;

    platformDestroy();
}

static inline bool compareStops(const Gradient::ColorStop &a, const Gradient::ColorStop &b)
{
    return a.stop < b.stop;
}

void Gradient::getColor(float value, float* r, float* g, float* b, float* a) const
{
    ASSERT(value >= 0);
    ASSERT(value <= 1);

    if (m_stops.isEmpty()) {
        *r = 0;
        *g = 0;
        *b = 0;
        *a = 0;
        return;
    }
    if (!m_stopsSorted) {
        if (m_stops.size())
            std::stable_sort(m_stops.begin(), m_stops.end(), compareStops);
        m_stopsSorted = true;
    }
    if (value <= 0 || value <= m_stops.first().stop) {
        *r = m_stops.first().red;
        *g = m_stops.first().green;
        *b = m_stops.first().blue;
        *a = m_stops.first().alpha;
        return;
    }
    if (value >= 1 || value >= m_stops.last().stop) {
        *r = m_stops.last().red;
        *g = m_stops.last().green;
        *b = m_stops.last().blue;
        *a = m_stops.last().alpha;
        return;
    }

    // Find stop before and stop after and interpolate.
    int stop = findStop(value);
    const ColorStop& lastStop = m_stops[stop];    
    const ColorStop& nextStop = m_stops[stop + 1];
    float stopFraction = (value - lastStop.stop) / (nextStop.stop - lastStop.stop);
    *r = lastStop.red + (nextStop.red - lastStop.red) * stopFraction;
    *g = lastStop.green + (nextStop.green - lastStop.green) * stopFraction;
    *b = lastStop.blue + (nextStop.blue - lastStop.blue) * stopFraction;
    *a = lastStop.alpha + (nextStop.alpha - lastStop.alpha) * stopFraction;
}

int Gradient::findStop(float value) const
{
    ASSERT(value >= 0);
    ASSERT(value <= 1);
    ASSERT(m_stopsSorted);

    int numStops = m_stops.size();
    ASSERT(numStops >= 2);
    ASSERT(m_lastStop < numStops - 1);

    int i = m_lastStop;
    if (value < m_stops[i].stop)
        i = 1;
    else
        i = m_lastStop + 1;

    for (; i < numStops - 1; ++i)
        if (value < m_stops[i].stop)
            break;

    m_lastStop = i - 1;
    return m_lastStop;
}

} //namespace
