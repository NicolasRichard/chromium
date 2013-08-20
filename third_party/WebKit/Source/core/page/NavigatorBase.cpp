/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
 *
 */

#include "config.h"
#include "core/page/NavigatorBase.h"

#include "wtf/text/WTFString.h"

#ifndef WEBCORE_NAVIGATOR_PRODUCT
#define WEBCORE_NAVIGATOR_PRODUCT "Gecko"
#endif // ifndef WEBCORE_NAVIGATOR_PRODUCT

#ifndef WEBCORE_NAVIGATOR_PRODUCT_SUB
#define WEBCORE_NAVIGATOR_PRODUCT_SUB "20030107"
#endif // ifndef WEBCORE_NAVIGATOR_PRODUCT_SUB

#ifndef WEBCORE_NAVIGATOR_VENDOR
#define WEBCORE_NAVIGATOR_VENDOR "Apple Computer, Inc."
#endif // ifndef WEBCORE_NAVIGATOR_VENDOR

#ifndef WEBCORE_NAVIGATOR_VENDOR_SUB
#define WEBCORE_NAVIGATOR_VENDOR_SUB ""
#endif // ifndef WEBCORE_NAVIGATOR_VENDOR_SUB


namespace WebCore {

NavigatorBase::~NavigatorBase()
{
}

String NavigatorBase::appCodeName() const
{
    return "Mozilla";
}

String NavigatorBase::product() const
{
    return WEBCORE_NAVIGATOR_PRODUCT;
}

String NavigatorBase::productSub() const
{
    return WEBCORE_NAVIGATOR_PRODUCT_SUB;
}

String NavigatorBase::vendor() const
{
    return WEBCORE_NAVIGATOR_VENDOR;
}

String NavigatorBase::vendorSub() const
{
    return WEBCORE_NAVIGATOR_VENDOR_SUB;
}

} // namespace WebCore
