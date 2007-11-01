/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
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

#ifndef WebLocalizableStrings_H
#define WebLocalizableStrings_H

#include <CoreFoundation/CoreFoundation.h>

typedef struct {
    const char *identifier;
    CFBundleRef bundle;
} WebLocalizableStringsBundle;

#ifdef __cplusplus
extern "C" {
#endif

CFStringRef WebLocalizedString(WebLocalizableStringsBundle *bundle, LPCTSTR key);
LPCTSTR WebLocalizedLPCTSTR(WebLocalizableStringsBundle *bundle, LPCTSTR key);
void SetWebLocalizedStringMainBundle(CFBundleRef bundle);

#ifdef __cplusplus
}
#endif

#ifdef FRAMEWORK_NAME

#define LOCALIZABLE_STRINGS_BUNDLE(F) LOCALIZABLE_STRINGS_BUNDLE_HELPER(F)
#define LOCALIZABLE_STRINGS_BUNDLE_HELPER(F) F ## LocalizableStringsBundle
extern WebLocalizableStringsBundle LOCALIZABLE_STRINGS_BUNDLE(FRAMEWORK_NAME);

#define UI_STRING(string, comment) WebLocalizedString(&LOCALIZABLE_STRINGS_BUNDLE(FRAMEWORK_NAME), L##string)
#define UI_STRING_KEY(string, key, comment) WebLocalizedString(&LOCALIZABLE_STRINGS_BUNDLE(FRAMEWORK_NAME), L##key)
#define LPCTSTR_UI_STRING(string, comment) WebLocalizedLPCTSTR(&LOCALIZABLE_STRINGS_BUNDLE(FRAMEWORK_NAME), L##string)
#define LPCTSTR_UI_STRING_KEY(string, key, comment) WebLocalizedLPCTSTR(&LOCALIZABLE_STRINGS_BUNDLE(FRAMEWORK_NAME), L##key)

#else

#define UI_STRING(string, comment) WebLocalizedString(0, L##string)
#define UI_STRING_KEY(string, key, comment) WebLocalizedString(0, L##key)
#define LPCTSTR_UI_STRING(string, comment) WebLocalizedLPCTSTR(0, L##string)
#define LPCTSTR_UI_STRING_KEY(string, key, comment) WebLocalizedLPCTSTR(0, L##key)

#endif

#endif
