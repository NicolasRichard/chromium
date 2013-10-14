/*
 * Copyright (C) 2012 Google, Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
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
#include "core/frame/DOMSecurityPolicy.h"

#include "core/dom/ContextLifecycleObserver.h"
#include "core/dom/DOMStringList.h"
#include "core/dom/ExecutionContext.h"
#include "core/frame/ContentSecurityPolicy.h"
#include "wtf/text/TextPosition.h"
#include "wtf/text/WTFString.h"

namespace WebCore {

namespace {

bool isPolicyActiveInContext(ExecutionContext* context)
{
    // If the ExecutionContext has been destroyed, there's no active policy.
    if (!context)
        return false;

    return context->contentSecurityPolicy()->isActive();
}

template<bool (ContentSecurityPolicy::*allowWithType)(const String&, const String&, const KURL&, ContentSecurityPolicy::ReportingStatus) const>
bool isAllowedWithType(ExecutionContext* context, const String& type)
{
    if (!isPolicyActiveInContext(context))
        return true;

    return (context->contentSecurityPolicy()->*allowWithType)(type, type, KURL(), ContentSecurityPolicy::SuppressReport);
}

template<bool (ContentSecurityPolicy::*allowWithURL)(const KURL&, ContentSecurityPolicy::ReportingStatus) const>
bool isAllowedWithURL(ExecutionContext* context, const String& url)
{
    if (!isPolicyActiveInContext(context))
        return true;

    KURL parsedURL = context->completeURL(url);
    if (!parsedURL.isValid())
        return false; // FIXME: Figure out how to throw a JavaScript error.

    return (context->contentSecurityPolicy()->*allowWithURL)(parsedURL, ContentSecurityPolicy::SuppressReport);
}

template<bool (ContentSecurityPolicy::*allowWithContext)(const String&, const WTF::OrdinalNumber&, ContentSecurityPolicy::ReportingStatus) const>
bool isAllowed(ExecutionContext* context)
{
    if (!isPolicyActiveInContext(context))
        return true;

    return (context->contentSecurityPolicy()->*allowWithContext)(String(), WTF::OrdinalNumber::beforeFirst(), ContentSecurityPolicy::SuppressReport);
}

} // namespace

DOMSecurityPolicy::DOMSecurityPolicy(ExecutionContext* context)
    : ContextLifecycleObserver(context)
{
    ScriptWrappable::init(this);
}

DOMSecurityPolicy::~DOMSecurityPolicy()
{
}

bool DOMSecurityPolicy::isActive() const
{
    return isPolicyActiveInContext(executionContext());
}

PassRefPtr<DOMStringList> DOMSecurityPolicy::reportURIs() const
{
    RefPtr<DOMStringList> result = DOMStringList::create();

    if (isActive())
        executionContext()->contentSecurityPolicy()->gatherReportURIs(*result.get());

    return result.release();
}

bool DOMSecurityPolicy::allowsInlineScript() const
{
    return isAllowed<&ContentSecurityPolicy::allowInlineScript>(executionContext());
}

bool DOMSecurityPolicy::allowsInlineStyle() const
{
    return isAllowed<&ContentSecurityPolicy::allowInlineStyle>(executionContext());
}

bool DOMSecurityPolicy::allowsEval() const
{
    if (!isActive())
        return true;

    return executionContext()->contentSecurityPolicy()->allowEval(0, ContentSecurityPolicy::SuppressReport);
}


bool DOMSecurityPolicy::allowsConnectionTo(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowConnectToSource>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsFontFrom(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowFontFromSource>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsFormAction(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowFormAction>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsFrameFrom(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowChildFrameFromSource>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsImageFrom(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowImageFromSource>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsMediaFrom(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowMediaFromSource>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsObjectFrom(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowObjectFromSource>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsPluginType(const String& type) const
{
    return isAllowedWithType<&ContentSecurityPolicy::allowPluginType>(executionContext(), type);
}

bool DOMSecurityPolicy::allowsScriptFrom(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowScriptFromSource>(executionContext(), url);
}

bool DOMSecurityPolicy::allowsStyleFrom(const String& url) const
{
    return isAllowedWithURL<&ContentSecurityPolicy::allowStyleFromSource>(executionContext(), url);
}

} // namespace WebCore
