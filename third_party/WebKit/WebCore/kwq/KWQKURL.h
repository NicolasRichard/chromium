/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
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

#ifndef KURL_H_
#define KURL_H_

#include <qstring.h>
#include <qvaluelist.h>
#include <KWQRefPtr.h>

#ifdef __OBJC__
@class NSURL;
#else
class NSURL;
#endif

class KURL {
public:
    KURL();
    KURL(const char *, int encoding_hint=0);
    KURL(const KURL &, const QString &);
    KURL(const QString &, int encoding_hint=0);
    
    inline bool isEmpty() const { return urlString.isEmpty(); } 
    inline bool isMalformed() const { return !m_isValid; }
    inline bool isValid() const { return m_isValid; }
    bool hasPath() const;

    inline QString url() const { return urlString; }
    QString protocol() const;
    QString host() const;
    unsigned short int port() const;
    QString pass() const;
    QString user() const;
    QString ref() const;
    QString query() const;
    QString path() const;
    QString htmlRef() const;
    QString encodedHtmlRef() const;

    void setProtocol(const QString &);
    void setHost(const QString &);
    void setPort(unsigned short int);
    void setRef(const QString &);
    void setQuery(const QString &, int encoding_hint=0);
    void setPath(const QString &);

    QString prettyURL(int trailing=0) const;
    
    NSURL *getNSURL() const;

    static QString decode_string(const QString &urlString);
    
    friend bool operator==(const KURL &, const KURL &);

private:
    void parse(const char *url);

    QString urlString;
    bool m_isValid;
    int schemeEndPos;
    int userStartPos;
    int userEndPos;
    int passwordEndPos;
    int hostEndPos;
    int portEndPos;
    int pathEndPos;
    int queryEndPos;
    int fragmentEndPos;
};

#endif
