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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(__OBJC__)

#if defined(BUILDING_ON_TIGER) || defined(BUILDING_ON_LEOPARD)
#define DELEGATES_DECLARED_AS_FORMAL_PROTOCOLS 0
#else
#import <Foundation/NSPrivateDecls.h>
#define DELEGATES_DECLARED_AS_FORMAL_PROTOCOLS defined(__COCOA_FORMAL_PROTOCOLS__)
#endif

#if !DELEGATES_DECLARED_AS_FORMAL_PROTOCOLS

#define EMPTY_PROTOCOL(NAME) \
@protocol NAME <NSObject> \
@end

EMPTY_PROTOCOL(NSTableViewDataSource)
EMPTY_PROTOCOL(NSTableViewDelegate)
EMPTY_PROTOCOL(NSWindowDelegate)

#undef EMPTY_PROTOCOL

#endif /* !DELEGATES_DECLARED_AS_FORMAL_PROTOCOLS */

#endif /* defined(__OBJC__) */
