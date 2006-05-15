/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include "config.h"
#include "HTMLDivElement.h"

#include "CSSPropertyNames.h"
#include "CSSValueKeywords.h"
#include "HTMLNames.h"

namespace WebCore {

using namespace HTMLNames;

HTMLDivElement::HTMLDivElement(Document *doc)
    : HTMLElement(divTag, doc)
{
}

HTMLDivElement::~HTMLDivElement()
{
}

bool HTMLDivElement::mapToEntry(const QualifiedName& attrName, MappedAttributeEntry& result) const
{
    if (attrName == alignAttr) {
        result = eBlock;
        return false;
    }
    return HTMLElement::mapToEntry(attrName, result);
}

void HTMLDivElement::parseMappedAttribute(MappedAttribute *attr)
{
    if (attr->name() == alignAttr) {
        String v = attr->value();
        if (equalIgnoringCase(attr->value(), "middle") || equalIgnoringCase(attr->value(), "center"))
           addCSSProperty(attr, CSS_PROP_TEXT_ALIGN, CSS_VAL__WEBKIT_CENTER);
        else if (equalIgnoringCase(attr->value(), "left"))
            addCSSProperty(attr, CSS_PROP_TEXT_ALIGN, CSS_VAL__WEBKIT_LEFT);
        else if (equalIgnoringCase(attr->value(), "right"))
            addCSSProperty(attr, CSS_PROP_TEXT_ALIGN, CSS_VAL__WEBKIT_RIGHT);
        else
            addCSSProperty(attr, CSS_PROP_TEXT_ALIGN, v);
    } else
        HTMLElement::parseMappedAttribute(attr);
}

String HTMLDivElement::align() const
{
    return getAttribute(alignAttr);
}

void HTMLDivElement::setAlign(const String &value)
{
    setAttribute(alignAttr, value);
}

}
