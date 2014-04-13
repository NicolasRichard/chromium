// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "modules/screen_orientation/ScreenOrientation.h"

#include "core/frame/DOMWindow.h"
#include "core/frame/LocalFrame.h"
#include "core/frame/Screen.h"
#include "modules/screen_orientation/ScreenOrientationController.h"
#include "public/platform/Platform.h"
#include "public/platform/WebScreenOrientationType.h"

// This code assumes that WebScreenOrientationType values are included in WebScreenOrientationLockType.
#define COMPILE_ASSERT_MATCHING_ENUM(enum1, enum2) \
    COMPILE_ASSERT(static_cast<unsigned>(blink::enum1) == static_cast<unsigned>(blink::enum2), mismatching_types)
COMPILE_ASSERT_MATCHING_ENUM(WebScreenOrientationPortraitPrimary, WebScreenOrientationLockPortraitPrimary);
COMPILE_ASSERT_MATCHING_ENUM(WebScreenOrientationPortraitSecondary, WebScreenOrientationLockPortraitSecondary);
COMPILE_ASSERT_MATCHING_ENUM(WebScreenOrientationLandscapePrimary, WebScreenOrientationLockLandscapePrimary);
COMPILE_ASSERT_MATCHING_ENUM(WebScreenOrientationLandscapeSecondary, WebScreenOrientationLockLandscapeSecondary);

namespace WebCore {

struct ScreenOrientationInfo {
    const AtomicString& name;
    unsigned orientation;
};

static ScreenOrientationInfo* orientationsMap(unsigned& length)
{
    DEFINE_STATIC_LOCAL(const AtomicString, portraitPrimary, ("portrait-primary", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, portraitSecondary, ("portrait-secondary", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, landscapePrimary, ("landscape-primary", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, landscapeSecondary, ("landscape-secondary", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, any, ("any", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, portrait, ("portrait", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(const AtomicString, landscape, ("landscape", AtomicString::ConstructFromLiteral));

    static ScreenOrientationInfo orientationMap[] = {
        { portraitPrimary, blink::WebScreenOrientationLockPortraitPrimary },
        { portraitSecondary, blink::WebScreenOrientationLockPortraitSecondary },
        { landscapePrimary, blink::WebScreenOrientationLockLandscapePrimary },
        { landscapeSecondary, blink::WebScreenOrientationLockLandscapeSecondary },
        { any, blink::WebScreenOrientationLockAny },
        { portrait, blink::WebScreenOrientationLockPortrait },
        { landscape, blink::WebScreenOrientationLockLandscape }
    };
    length = WTF_ARRAY_LENGTH(orientationMap);

    return orientationMap;
}

static const AtomicString& orientationTypeToString(blink::WebScreenOrientationType orientation)
{
    unsigned length = 0;
    ScreenOrientationInfo* orientationMap = orientationsMap(length);
    for (unsigned i = 0; i < length; ++i) {
        if (static_cast<unsigned>(orientation) == orientationMap[i].orientation)
            return orientationMap[i].name;
    }

    ASSERT_NOT_REACHED();
    return nullAtom;
}

static blink::WebScreenOrientationLockType stringToOrientationLock(const AtomicString& orientationLockString)
{
    unsigned length = 0;
    ScreenOrientationInfo* orientationMap = orientationsMap(length);
    for (unsigned i = 0; i < length; ++i) {
        if (orientationMap[i].name == orientationLockString)
            return static_cast<blink::WebScreenOrientationLockType>(orientationMap[i].orientation);
    }

    ASSERT_NOT_REACHED();
    return blink::WebScreenOrientationLockDefault;
}

ScreenOrientation::ScreenOrientation(Screen& screen)
    : DOMWindowProperty(screen.frame())
    , m_orientationLockTimer(this, &ScreenOrientation::orientationLockTimerFired)
    , m_prospectiveLock(blink::WebScreenOrientationLockDefault)
{
}

void ScreenOrientation::lockOrientationAsync(blink::WebScreenOrientationLockType orientation)
{
    if (m_orientationLockTimer.isActive())
        m_orientationLockTimer.stop();

    m_prospectiveLock = orientation;
    m_orientationLockTimer.startOneShot(0, FROM_HERE);
}

void ScreenOrientation::orientationLockTimerFired(Timer<ScreenOrientation>*)
{
    if (m_prospectiveLock == blink::WebScreenOrientationLockDefault)
        blink::Platform::current()->unlockOrientation();
    else
        blink::Platform::current()->lockOrientation(m_prospectiveLock);
}

const char* ScreenOrientation::supplementName()
{
    return "ScreenOrientation";
}

Document* ScreenOrientation::document() const
{
    if (!m_associatedDOMWindow || !m_associatedDOMWindow->isCurrentlyDisplayedInFrame())
        return 0;
    ASSERT(m_associatedDOMWindow->document());
    return m_associatedDOMWindow->document();
}

ScreenOrientation& ScreenOrientation::from(Screen& screen)
{
    ScreenOrientation* supplement = static_cast<ScreenOrientation*>(WillBeHeapSupplement<Screen>::from(screen, supplementName()));
    if (!supplement) {
        supplement = new ScreenOrientation(screen);
        provideTo(screen, supplementName(), adoptPtrWillBeNoop(supplement));
    }
    return *supplement;
}

ScreenOrientation::~ScreenOrientation()
{
}

const AtomicString& ScreenOrientation::orientation(Screen& screen)
{
    ScreenOrientation& screenOrientation = ScreenOrientation::from(screen);
    if (!screenOrientation.document()) {
        // FIXME: we should try to return a better guess, like the latest known value.
        return orientationTypeToString(blink::WebScreenOrientationPortraitPrimary);
    }
    ScreenOrientationController& controller = ScreenOrientationController::from(*screenOrientation.document());
    return orientationTypeToString(controller.orientation());
}

bool ScreenOrientation::lockOrientation(Screen& screen, const AtomicString& lockString)
{
    ScreenOrientation::from(screen).lockOrientationAsync(stringToOrientationLock(lockString));
    return true;
}

void ScreenOrientation::unlockOrientation(Screen& screen)
{
    ScreenOrientation::from(screen).lockOrientationAsync(blink::WebScreenOrientationLockDefault);
}

} // namespace WebCore
