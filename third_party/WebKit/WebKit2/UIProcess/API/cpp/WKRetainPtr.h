/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WKRetainPtr_h
#define WKRetainPtr_h

#include <WebKit2/WebKit2.h>

namespace WebKit {

// FIXME: Consider moving these to their respective headers instead of putting
// them all here.
#define DECLARE_RETAIN_RELEASE_OVERLOADS(WKType) \
    inline void WKRetain(WKType##Ref p) { WKType##Retain(p); } \
    inline void WKRelease(WKType##Ref p) { WKType##Release(p); } \
    // end of macro

DECLARE_RETAIN_RELEASE_OVERLOADS(WKContext)
DECLARE_RETAIN_RELEASE_OVERLOADS(WKFrame)
DECLARE_RETAIN_RELEASE_OVERLOADS(WKFramePolicyListener)
DECLARE_RETAIN_RELEASE_OVERLOADS(WKPage)
DECLARE_RETAIN_RELEASE_OVERLOADS(WKPageNamespace)
DECLARE_RETAIN_RELEASE_OVERLOADS(WKPreferences)
DECLARE_RETAIN_RELEASE_OVERLOADS(WKString)
DECLARE_RETAIN_RELEASE_OVERLOADS(WKURL)

#undef DECLARE_RETAIN_RELEASE_OVERLOADS

enum WKAdoptTag { AdoptWK };

template <typename T>
class WKRetainPtr {
public:
    typedef T PtrType;

    WKRetainPtr()
        : m_ptr(0)
    {
    }

    WKRetainPtr(PtrType ptr)
        : m_ptr(ptr)
    {
        if (ptr)
            retainWKPtr(ptr);
    }

    WKRetainPtr(WKAdoptTag, PtrType ptr)
        : m_ptr(ptr)
    {
    }
    
    template <typename U>
    WKRetainPtr(const WKRetainPtr<U>& o)
        : m_ptr(o.get())
    {
        if (PtrType ptr = m_ptr)
            WKRetain(ptr);
    }
    
    WKRetainPtr(const WKRetainPtr& o)
        : m_ptr(o.m_ptr)
    {
        if (PtrType ptr = m_ptr)
            WKRetain(ptr);
    }

    ~WKRetainPtr()
    {
        if (PtrType ptr = m_ptr)
            WKRelease(ptr);
    }

    PtrType get() const { return m_ptr; }
    PtrType releaseRef() { PtrType tmp = m_ptr; m_ptr = 0; return tmp; }
    
    PtrType operator->() const { return m_ptr; }
    bool operator!() const { return !m_ptr; }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef PtrType WKRetainPtr::*UnspecifiedBoolType;
    operator UnspecifiedBoolType() const { return m_ptr ? &WKRetainPtr::m_ptr : 0; }

    WKRetainPtr& operator=(const WKRetainPtr&);
    template <typename U> WKRetainPtr& operator=(const WKRetainPtr<U>&);
    WKRetainPtr& operator=(PtrType);
    template <typename U> WKRetainPtr& operator=(U*);

    void adopt(PtrType);
    void swap(WKRetainPtr&);

private:
    PtrType m_ptr;
};

template <typename T> inline WKRetainPtr<T>& WKRetainPtr<T>::operator=(const WKRetainPtr<T>& o)
{
    PtrType optr = o.get();
    if (optr)
        WKRetain(optr);
    PtrType ptr = m_ptr;
    m_ptr = optr;
    if (ptr)
        WKRelease(ptr);
    return *this;
}

template <typename T> template <typename U> inline WKRetainPtr<T>& WKRetainPtr<T>::operator=(const WKRetainPtr<U>& o)
{
    PtrType optr = o.get();
    if (optr)
        WKRetain(optr);
    PtrType ptr = m_ptr;
    m_ptr = optr;
    if (ptr)
        WKRelease(ptr);
    return *this;
}

template <typename T> inline WKRetainPtr<T>& WKRetainPtr<T>::operator=(PtrType optr)
{
    if (optr)
        WKRetain(optr);
    PtrType ptr = m_ptr;
    m_ptr = optr;
    if (ptr)
        WKRelease(ptr);
    return *this;
}

template <typename T> inline void WKRetainPtr<T>::adopt(PtrType optr)
{
    PtrType ptr = m_ptr;
    m_ptr = optr;
    if (ptr)
        WKRelease(ptr);
}

template <typename T> template <typename U> inline WKRetainPtr<T>& WKRetainPtr<T>::operator=(U* optr)
{
    if (optr)
        WKRetain(optr);
    PtrType ptr = m_ptr;
    m_ptr = optr;
    if (ptr)
        WKRelease(ptr);
    return *this;
}

template <class T> inline void WKRetainPtr<T>::swap(WKRetainPtr<T>& o)
{
    std::swap(m_ptr, o.m_ptr);
}

template <class T> inline void swap(WKRetainPtr<T>& a, WKRetainPtr<T>& b)
{
    a.swap(b);
}

template <typename T, typename U> inline bool operator==(const WKRetainPtr<T>& a, const WKRetainPtr<U>& b)
{ 
    return a.get() == b.get(); 
}

template <typename T, typename U> inline bool operator==(const WKRetainPtr<T>& a, U* b)
{ 
    return a.get() == b; 
}

template <typename T, typename U> inline bool operator==(T* a, const WKRetainPtr<U>& b) 
{
    return a == b.get(); 
}

template <typename T, typename U> inline bool operator!=(const WKRetainPtr<T>& a, const WKRetainPtr<U>& b)
{ 
    return a.get() != b.get(); 
}

template <typename T, typename U> inline bool operator!=(const WKRetainPtr<T>& a, U* b)
{
    return a.get() != b; 
}

template <typename T, typename U> inline bool operator!=(T* a, const WKRetainPtr<U>& b)
{ 
    return a != b.get(); 
}

} // namespace WebKit

using WebKit::WKRetainPtr;

#endif // WKRetainPtr_h
