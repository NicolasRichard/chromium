/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "modules/device_orientation/DeviceRotationRate.h"

namespace WebCore {

DEFINE_GC_INFO(DeviceRotationRate);

DeviceRotationRate::DeviceRotationRate(PassRefPtrWillBeRawPtr<DeviceMotionData::RotationRate> rotationRate)
    : m_rotationRate(rotationRate)
{
    ScriptWrappable::init(this);
}

void DeviceRotationRate::trace(Visitor* visitor)
{
    visitor->trace(m_rotationRate);
}

double DeviceRotationRate::alpha(bool& isNull) const
{
    if (m_rotationRate->canProvideAlpha())
        return m_rotationRate->alpha();

    isNull = true;
    return 0;
}

double DeviceRotationRate::beta(bool& isNull) const
{
    if (m_rotationRate->canProvideBeta())
        return m_rotationRate->beta();

    isNull = true;
    return 0;
}

double DeviceRotationRate::gamma(bool& isNull) const
{
    if (m_rotationRate->canProvideGamma())
        return m_rotationRate->gamma();

    isNull = true;
    return 0;
}

} // namespace WebCore
