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

#import "KWQTextUtilities.h"

#import "KWQString.h"
#import <AppKit/NSAttributedString.h>
#import <unicode/ubrk.h>
#import <unicode/ustring.h>
#import <unicode/utypes.h>

void KWQFindWordBoundary(const QChar *chars, int len, int position, int *start, int *end)
{
    NSString *string = [[NSString alloc] initWithCharactersNoCopy:const_cast<unichar *>(reinterpret_cast<const unichar *>(chars))
        length:len freeWhenDone:NO];
    NSAttributedString *attr = [[NSAttributedString alloc] initWithString:string];
    NSRange range = [attr doubleClickAtIndex:(position >= len) ? len - 1 : position];
    [attr release];
    [string release];
    *start = range.location;
    *end = range.location + range.length;
}

int KWQFindNextWordFromIndex(const QChar *chars, int len, int position, bool forward)
{   
    NSString *string = [[NSString alloc] initWithCharactersNoCopy:const_cast<unichar *>(reinterpret_cast<const unichar *>(chars))
        length:len freeWhenDone:NO];
    NSAttributedString *attr = [[NSAttributedString alloc] initWithString:string];
    int result = [attr nextWordFromIndex:position forward:forward];
    [attr release];
    [string release];
    return result;
}

void KWQFindSentenceBoundary(const QChar *chars, int len, int position, int *start, int *end)
{
    int  startPos = 0;
    int  endPos = 0;

    const char *localeName = [[NSString localizedNameOfStringEncoding:NSASCIIStringEncoding] UTF8String];
    UErrorCode status = U_ZERO_ERROR;
    UBreakIterator *boundary = ubrk_open(UBRK_SENTENCE, localeName, const_cast<unichar *>(reinterpret_cast<const unichar *>(chars)) + position, len, &status);
    if ( boundary && U_SUCCESS(status) ) {
        startPos = ubrk_first(boundary);
        if (startPos == UBRK_DONE)
            startPos = 0;
        endPos = ubrk_next(boundary);
        if (endPos == UBRK_DONE)
            endPos = 0;
        ubrk_close(boundary);
    }

    *start = startPos;
    *end = endPos;
}

int KWQFindNextSentenceFromIndex(const QChar *chars, int len, int position, bool forward)
{
    int pos = 0;
    
    const char *localeName = [[NSString localizedNameOfStringEncoding:NSASCIIStringEncoding] UTF8String];
    UErrorCode status = U_ZERO_ERROR;
    UBreakIterator *boundary = ubrk_open(UBRK_SENTENCE, localeName, const_cast<unichar *>(reinterpret_cast<const unichar *>(chars)) + position, len, &status);
    if ( boundary && U_SUCCESS(status) ) {
        int firstpos = ubrk_first(boundary);
        if (forward) {
            pos = ubrk_next(boundary);
            if (pos == firstpos)
                pos = ubrk_next(boundary);
        } else {
            pos = ubrk_previous(boundary);
            if (pos == firstpos)
                pos = ubrk_previous(boundary);
        }
        ubrk_close(boundary);
    }

    if (pos == UBRK_DONE)
        pos = 0;
        
    return pos;
}
