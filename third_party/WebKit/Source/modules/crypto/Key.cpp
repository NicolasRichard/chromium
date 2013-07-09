/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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
#include "modules/crypto/Key.h"

#include "modules/crypto/Algorithm.h"

namespace WebCore {

namespace {

const char* keyTypeToString(WebKit::WebCryptoKeyType type)
{
    switch (type) {
    case WebKit::WebCryptoKeyTypeSecret:
        return "secret";
    case WebKit::WebCryptoKeyTypePublic:
        return "public";
    case WebKit::WebCryptoKeyTypePrivate:
        return "private";
    }
    ASSERT_NOT_REACHED();
    return 0;
}

const char* keyUsageToString(WebKit::WebCryptoKeyUsage usage)
{
    switch (usage) {
    case WebKit::WebCryptoKeyUsageEncrypt:
        return "encrypt";
    case WebKit::WebCryptoKeyUsageDecrypt:
        return "decrypt";
    case WebKit::WebCryptoKeyUsageSign:
        return "sign";
    case WebKit::WebCryptoKeyUsageVerify:
        return "verify";
    case WebKit::WebCryptoKeyUsageDeriveKey:
        return "deriveKey";
    case WebKit::WebCryptoKeyUsageWrapKey:
        return "wrapKey";
    case WebKit::WebCryptoKeyUsageUnwrapKey:
        return "unwrapKey";
    case WebKit::EndOfWebCryptoKeyUsage:
        break;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

} // namespace

Key::Key(const WebKit::WebCryptoKey& key)
    : m_key(key)
{
    ScriptWrappable::init(this);
}

String Key::type() const
{
    return ASCIILiteral(keyTypeToString(m_key.type()));
}

bool Key::extractable() const
{
    return m_key.extractable();
}

Algorithm* Key::algorithm()
{
    if (!m_algorithm)
        m_algorithm = Algorithm::create(m_key.algorithm());
    return m_algorithm.get();
}

// FIXME: This creates a new javascript array each time. What should happen
//        instead is return the same (immutable) array. (Javascript callers can
//        distinguish this by doing an == test on the arrays and seeing they are
//        different).
Vector<String> Key::usages() const
{
    Vector<String> result;

    // The WebCryptoKeyUsage values are consecutive powers of 2. Test each one in order.
    for (int i = 0; (1 << i) < WebKit::EndOfWebCryptoKeyUsage; ++i) {
        WebKit::WebCryptoKeyUsage usage = static_cast<WebKit::WebCryptoKeyUsage>(1 << i);
        if (m_key.usages() & usage)
            result.append(ASCIILiteral(keyUsageToString(usage)));
    }
    return result;
}

} // namespace WebCore
