/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "V8CSSRule.h"

#include "V8CSSCharsetRule.h"
#include "V8CSSFontFaceRule.h"
#include "V8CSSHostRule.h"
#include "V8CSSImportRule.h"
#include "V8CSSMediaRule.h"
#include "V8CSSPageRule.h"
#include "V8CSSStyleRule.h"
#include "V8CSSSupportsRule.h"
#include "V8CSSViewportRule.h"
#include "V8WebKitCSSFilterRule.h"
#include "V8WebKitCSSKeyframeRule.h"
#include "V8WebKitCSSKeyframesRule.h"
#include "V8WebKitCSSRegionRule.h"

namespace WebCore {

v8::Handle<v8::Object> wrap(CSSRule* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    switch (impl->type()) {
    case CSSRule::UNKNOWN_RULE:
        // CSSUnknownRule.idl is explicitly excluded as it doesn't add anything
        // over CSSRule.idl (see bindings/derived_sources.gyp: 'idl_files').
        // -> Use the base class wrapper here.
        return V8CSSRule::createWrapper(impl, creationContext, isolate);
    case CSSRule::STYLE_RULE:
        return wrap(static_cast<CSSStyleRule*>(impl), creationContext, isolate);
    case CSSRule::CHARSET_RULE:
        return wrap(static_cast<CSSCharsetRule*>(impl), creationContext, isolate);
    case CSSRule::IMPORT_RULE:
        return wrap(static_cast<CSSImportRule*>(impl), creationContext, isolate);
    case CSSRule::MEDIA_RULE:
        return wrap(static_cast<CSSMediaRule*>(impl), creationContext, isolate);
    case CSSRule::FONT_FACE_RULE:
        return wrap(static_cast<CSSFontFaceRule*>(impl), creationContext, isolate);
    case CSSRule::PAGE_RULE:
        return wrap(static_cast<CSSPageRule*>(impl), creationContext, isolate);
    case CSSRule::WEBKIT_KEYFRAME_RULE:
        return wrap(static_cast<CSSKeyframeRule*>(impl), creationContext, isolate);
    case CSSRule::WEBKIT_KEYFRAMES_RULE:
        return wrap(static_cast<CSSKeyframesRule*>(impl), creationContext, isolate);
    case CSSRule::SUPPORTS_RULE:
        return wrap(static_cast<CSSSupportsRule*>(impl), creationContext, isolate);
    case CSSRule::WEBKIT_VIEWPORT_RULE:
        return wrap(static_cast<CSSViewportRule*>(impl), creationContext, isolate);
    case CSSRule::WEBKIT_REGION_RULE:
        return wrap(static_cast<CSSRegionRule*>(impl), creationContext, isolate);
    case CSSRule::HOST_RULE:
        return wrap(static_cast<CSSHostRule*>(impl), creationContext, isolate);
    case CSSRule::WEBKIT_FILTER_RULE:
        return wrap(static_cast<CSSFilterRule*>(impl), creationContext, isolate);
    }
    return V8CSSRule::createWrapper(impl, creationContext, isolate);
}

} // namespace WebCore
