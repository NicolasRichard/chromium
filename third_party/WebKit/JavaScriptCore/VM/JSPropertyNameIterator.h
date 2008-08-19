/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JSPropertyNameIterator_h
#define JSPropertyNameIterator_h

#include "JSCell.h"

namespace KJS {

    class Identifier;
    class JSObject;

    class JSPropertyNameIterator : public JSCell {
    public:
        static JSPropertyNameIterator* create(ExecState*, JSValue*);

        virtual ~JSPropertyNameIterator();

        virtual JSValue* toPrimitive(ExecState*, PreferredPrimitiveType) const;
        virtual bool getPrimitiveNumber(ExecState*, double&, JSValue*&);
        virtual bool toBoolean(ExecState*) const;
        virtual double toNumber(ExecState*) const;
        virtual UString toString(ExecState*) const;
        virtual JSObject* toObject(ExecState*) const;

        virtual void mark();

        JSValue* next(ExecState*);
        void invalidate();

    private:
        JSPropertyNameIterator(JSObject*, Identifier* propertyNames, size_t numProperties);

        JSObject* m_object;
        Identifier* m_propertyNames;
        Identifier* m_position;
        Identifier* m_end;
    };

} // namespace KJS

#endif // JSPropertyNameIterator_h
