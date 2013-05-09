/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012 Apple Inc. All rights reserved.
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

#ifndef KURL_h
#define KURL_h

#include "core/platform/KURLGooglePrivate.h"
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/text/WTFString.h>

#if USE(CF)
typedef const struct __CFURL* CFURLRef;
#endif

namespace WTF{
class TextEncoding;
}

namespace WebCore {

struct KURLHash;

enum ParsedURLStringTag { ParsedURLString };

class KURL {
public:
    // Generates a URL which contains a null string.
    KURL() { invalidate(); }

    // The argument is an absolute URL string. The string is assumed to be output of KURL::string() called on a valid
    // KURL object, or indiscernible from such.
    // It is usually best to avoid repeatedly parsing a string, unless memory saving outweigh the possible slow-downs.
    KURL(ParsedURLStringTag, const String&);
    explicit KURL(WTF::HashTableDeletedValueType) : m_url(WTF::HashTableDeletedValue) { }

    bool isHashTableDeletedValue() const { return string().isHashTableDeletedValue(); }

    // Resolves the relative URL with the given base URL. If provided, the
    // TextEncoding is used to encode non-ASCII characers. The base URL can be
    // null or empty, in which case the relative URL will be interpreted as
    // absolute.
    // FIXME: If the base URL is invalid, this always creates an invalid
    // URL. Instead I think it would be better to treat all invalid base URLs
    // the same way we treate null and empty base URLs.
    KURL(const KURL& base, const String& relative);
    KURL(const KURL& base, const String& relative, const WTF::TextEncoding&);

    // For conversions from other structures that have already parsed and
    // canonicalized the URL. The input must be exactly what KURL would have
    // done with the same input.
    KURL(const CString& canonicalSpec, const url_parse::Parsed&, bool isValid);

    String strippedForUseAsReferrer() const;

    // FIXME: The above functions should be harmonized so that passing a
    // base of null or the empty string gives the same result as the
    // standard String constructor.

    // Makes a deep copy. Helpful only if you need to use a KURL on another
    // thread.  Since the underlying StringImpl objects are immutable, there's
    // no other reason to ever prefer copy() over plain old assignment.
    KURL copy() const;

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    // Returns true if this URL has a path. Note that "http://foo.com/" has a
    // path of "/", so this function will return true. Only invalid or
    // non-hierarchical (like "javascript:") URLs will have no path.
    bool hasPath() const;

    // Returns true if you can set the host and port for the URL.
    // Non-hierarchical URLs don't have a host and port.
    bool canSetHostOrPort() const { return isHierarchical(); }

    bool canSetPathname() const { return isHierarchical(); }
    bool isHierarchical() const;

    const String& string() const { return m_url.string(); }

    String elidedString() const;

    String protocol() const;
    String host() const;
    unsigned short port() const;
    bool hasPort() const;
    String user() const;
    String pass() const;
    String path() const;
    String lastPathComponent() const;
    String query() const;
    String fragmentIdentifier() const;
    bool hasFragmentIdentifier() const;

    String baseAsString() const;

    // Returns true if the current URL's protocol is the same as the null-
    // terminated ASCII argument. The argument must be lower-case.
    bool protocolIs(const char*) const;
    bool protocolIsData() const { return protocolIs("data"); }
    bool protocolIsInHTTPFamily() const;
    bool isLocalFile() const;
    bool isBlankURL() const;

    bool setProtocol(const String&);
    void setHost(const String&);

    void removePort();
    void setPort(unsigned short);

    // Input is like "foo.com" or "foo.com:8000".
    void setHostAndPort(const String&);

    void setUser(const String&);
    void setPass(const String&);

    // If you pass an empty path for HTTP or HTTPS URLs, the resulting path
    // will be "/".
    void setPath(const String&);

    // The query may begin with a question mark, or, if not, one will be added
    // for you. Setting the query to the empty string will leave a "?" in the
    // URL (with nothing after it). To clear the query, pass a null string.
    void setQuery(const String&);

    void setFragmentIdentifier(const String&);
    void removeFragmentIdentifier();

    friend bool equalIgnoringFragmentIdentifier(const KURL&, const KURL&);

    friend bool protocolHostAndPortAreEqual(const KURL&, const KURL&);

    unsigned hostStart() const;
    unsigned hostEnd() const;

    unsigned pathStart() const;
    unsigned pathEnd() const;
    unsigned pathAfterLastSlash() const;

    operator const String&() const { return string(); }

#if USE(CF)
    KURL(CFURLRef);
    CFURLRef createCFURL() const;
#endif

#ifdef __OBJC__
    operator NSString*() const { return string(); }
#endif

    // Getters for the parsed structure and its corresponding 8-bit string.
    const url_parse::Parsed& parsed() const { return m_url.m_parsed; }
    const CString& utf8String() const { return m_url.utf8String(); }

    const KURL* innerURL() const { return m_url.innerURL(); }

#ifndef NDEBUG
    void print() const;
#endif

    void reportMemoryUsage(MemoryObjectInfo*) const;
    bool isSafeToSendToAnotherThread() const;

private:
    void invalidate();
    static bool protocolIs(const String&, const char*);

    friend class KURLGooglePrivate;
    KURLGooglePrivate m_url;
};

bool operator==(const KURL&, const KURL&);
bool operator==(const KURL&, const String&);
bool operator==(const String&, const KURL&);
bool operator!=(const KURL&, const KURL&);
bool operator!=(const KURL&, const String&);
bool operator!=(const String&, const KURL&);

bool equalIgnoringFragmentIdentifier(const KURL&, const KURL&);
bool protocolHostAndPortAreEqual(const KURL&, const KURL&);

const KURL& blankURL();

// Functions to do URL operations on strings.
// These are operations that aren't faster on a parsed URL.
// These are also different from the KURL functions in that they don't require the string to be a valid and parsable URL.
// This is especially important because valid javascript URLs are not necessarily considered valid by KURL.

bool protocolIs(const String& url, const char* protocol);
bool protocolIsJavaScript(const String& url);

bool isDefaultPortForProtocol(unsigned short port, const String& protocol);
bool portAllowed(const KURL&); // Blacklist ports that should never be used for Web resources.

bool isValidProtocol(const String&);

// Unescapes the given string using URL escaping rules, given an optional
// encoding (defaulting to UTF-8 otherwise). DANGER: If the URL has "%00"
// in it, the resulting string will have embedded null characters!
String decodeURLEscapeSequences(const String&);
String decodeURLEscapeSequences(const String&, const WTF::TextEncoding&);

String encodeWithURLEscapeSequences(const String&);

// Inlines.

inline bool operator==(const KURL& a, const KURL& b)
{
    return a.string() == b.string();
}

inline bool operator==(const KURL& a, const String& b)
{
    return a.string() == b;
}

inline bool operator==(const String& a, const KURL& b)
{
    return a == b.string();
}

inline bool operator!=(const KURL& a, const KURL& b)
{
    return a.string() != b.string();
}

inline bool operator!=(const KURL& a, const String& b)
{
    return a.string() != b;
}

inline bool operator!=(const String& a, const KURL& b)
{
    return a != b.string();
}

} // namespace WebCore

namespace WTF {

    // KURLHash is the default hash for String
    template<typename T> struct DefaultHash;
    template<> struct DefaultHash<WebCore::KURL> {
        typedef WebCore::KURLHash Hash;
    };

} // namespace WTF

#endif // KURL_h
