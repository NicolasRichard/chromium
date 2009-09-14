/*
 * Copyright (C) 2003, 2006, 2008, 2009 Apple Inc. All rights reserved.
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

#ifndef CString_h
#define CString_h

#include "SharedBuffer.h"

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>

namespace WebCore {

    class CStringBuffer : public RefCounted<CStringBuffer> {
    public:
        const char* data() { return m_vector.data(); }
        size_t length() { return m_vector.size(); }
        
    private:
        friend class CString;

        static PassRefPtr<CStringBuffer> create(unsigned length) { return adoptRef(new CStringBuffer(length)); }
        static PassRefPtr<CStringBuffer> create(const Vector<char>& vectorToAdopt) { return adoptRef(new CStringBuffer(vectorToAdopt)); }
        CStringBuffer(unsigned length) : m_vector(length) { }
        CStringBuffer(const Vector<char>& vectorToAdopt) : m_vector(vectorToAdopt) { }
        char* mutableData() { return m_vector.data(); }

        PassRefPtr<CStringBuffer> base64Encode();
        Vector<char> m_vector;
    };

    // A container for a null-terminated char array supporting copy-on-write
    // assignment.  The contained char array may be null.
    class CString {
    public:
        CString() { }
        CString(const char*);
        CString(const char*, unsigned length);
        CString(CStringBuffer* buffer) : m_buffer(buffer) { }
        static CString newUninitialized(size_t length, char*& characterBuffer);

        CString base64Encode();
        
        const char* data() const;
        char* mutableData();
        unsigned length() const;

        bool isNull() const { return !m_buffer; }

        CStringBuffer* buffer() const { return m_buffer.get(); }

    private:
        void copyBufferIfNeeded();
        void init(const char*, unsigned length);
        RefPtr<CStringBuffer> m_buffer;
    };

    bool operator==(const CString& a, const CString& b);
    inline bool operator!=(const CString& a, const CString& b) { return !(a == b); }

} // namespace WebCore

#endif // CString_h
