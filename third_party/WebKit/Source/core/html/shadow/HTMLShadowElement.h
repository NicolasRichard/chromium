/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#ifndef HTMLShadowElement_h
#define HTMLShadowElement_h

#include "core/dom/shadow/InsertionPoint.h"
#include "wtf/Forward.h"

namespace WebCore {

class HTMLShadowElement FINAL : public InsertionPoint {
public:
    static PassRefPtr<HTMLShadowElement> create(const QualifiedName&, Document&);

    virtual ~HTMLShadowElement();

    ShadowRoot* olderShadowRoot();

private:
    HTMLShadowElement(const QualifiedName&, Document&);
    virtual InsertionNotificationRequest insertedInto(ContainerNode* insertionPoint) OVERRIDE;
};

inline bool isHTMLShadowElement(const Node* node)
{
    ASSERT(node);
    return node->hasTagName(HTMLNames::shadowTag);
}

inline HTMLShadowElement* toHTMLShadowElement(Node* node)
{
    ASSERT_WITH_SECURITY_IMPLICATION(!node || isHTMLShadowElement(node));
    return static_cast<HTMLShadowElement*>(node);
}

inline const HTMLShadowElement* toHTMLShadowElement(const Node* node)
{
    ASSERT_WITH_SECURITY_IMPLICATION(!node || isHTMLShadowElement(node));
    return static_cast<const HTMLShadowElement*>(node);
}

inline bool isActiveShadowInsertionPoint(const Node* node)
{
    return isHTMLShadowElement(node) && toHTMLShadowElement(node)->isActive();
}

} // namespace WebCore

#endif // HTMLShadowElement_h
