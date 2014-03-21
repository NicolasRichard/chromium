/*
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#ifndef SVGLengthListTearOff_h
#define SVGLengthListTearOff_h

#include "core/svg/SVGLengthList.h"
#include "core/svg/properties/SVGListPropertyTearOffHelper.h"

namespace WebCore {

class SVGLengthListTearOff FINAL :
    public SVGListPropertyTearOffHelper<SVGLengthListTearOff, SVGLengthList>,
    public ScriptWrappable {
public:
    static PassRefPtr<SVGLengthListTearOff> create(PassRefPtr<SVGLengthList> target, SVGElement* contextElement, PropertyIsAnimValType propertyIsAnimVal, const QualifiedName& attributeName = nullQName())
    {
        return adoptRef(new SVGLengthListTearOff(target, contextElement, propertyIsAnimVal, attributeName));
    }

private:
    SVGLengthListTearOff(PassRefPtr<SVGLengthList> target, SVGElement* contextElement, PropertyIsAnimValType propertyIsAnimVal, const QualifiedName& attributeName = nullQName())
        : SVGListPropertyTearOffHelper<SVGLengthListTearOff, SVGLengthList>(target, contextElement, propertyIsAnimVal, attributeName)
    {
        ScriptWrappable::init(this);
    }
};

} // namespace WebCore

#endif // SVGLengthListTearOff_h_
