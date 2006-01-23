/*
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

#ifndef DOM_TextImpl_h
#define DOM_TextImpl_h

#include "NodeImpl.h"

namespace WebCore {

class CharacterDataImpl : public NodeImpl
{
public:
    CharacterDataImpl(DocumentImpl*, const DOMString &_text);
    CharacterDataImpl(DocumentImpl*);
    virtual ~CharacterDataImpl();

    // DOM methods & attributes for CharacterData

    virtual DOMString data() const;
    virtual void setData( const DOMString &_data, int &exceptioncode );
    virtual unsigned length (  ) const;
    virtual DOMString substringData ( const unsigned offset, const unsigned count, int &exceptioncode );
    virtual void appendData ( const DOMString &arg, int &exceptioncode );
    virtual void insertData ( const unsigned offset, const DOMString &arg, int &exceptioncode );
    virtual void deleteData ( const unsigned offset, const unsigned count, int &exceptioncode );
    virtual void replaceData ( const unsigned offset, const unsigned count, const DOMString &arg, int &exceptioncode );

    bool containsOnlyWhitespace() const;
    bool containsOnlyWhitespace(unsigned int from, unsigned int len) const;
    
    // DOM methods overridden from  parent classes

    virtual DOMString nodeValue() const;
    virtual void setNodeValue( const DOMString &_nodeValue, int &exceptioncode );

    // Other methods (not part of DOM)

    DOMStringImpl *string() { return str; }
    virtual void checkCharDataOperation( const unsigned offset, int &exceptioncode );

    virtual int maxOffset() const;
    virtual int caretMinOffset() const;
    virtual int caretMaxOffset() const;
    virtual unsigned caretMaxRenderedOffset() const;

    virtual bool rendererIsNeeded(RenderStyle *);
    
#ifndef NDEBUG
    virtual void dump(QTextStream *stream, QString ind = "") const;
#endif

protected:
    // note: since DOMStrings are shared, str should always be copied when making
    // a change or returning a string
    DOMStringImpl *str;

    void dispatchModifiedEvent(DOMStringImpl *prevValue);
};

// ----------------------------------------------------------------------------

class CommentImpl : public CharacterDataImpl
{
public:
    CommentImpl(DocumentImpl*, const DOMString &_text);
    CommentImpl(DocumentImpl*);
    virtual ~CommentImpl();

    // DOM methods overridden from  parent classes
    const AtomicString& localName() const;
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual PassRefPtr<NodeImpl> cloneNode(bool deep);

    // Other methods (not part of DOM)
    virtual bool isCommentNode() const { return true; }
    virtual bool childTypeAllowed( unsigned short type );

    virtual DOMString toString() const;
};

// ----------------------------------------------------------------------------

class TextImpl : public CharacterDataImpl
{
public:
    TextImpl(DocumentImpl *impl, const DOMString &_text);
    TextImpl(DocumentImpl *impl);
    virtual ~TextImpl();

    // DOM methods & attributes for CharacterData

    TextImpl *splitText ( const unsigned offset, int &exceptioncode );

    // DOM methods overridden from  parent classes
    const AtomicString& localName() const;
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual PassRefPtr<NodeImpl> cloneNode(bool deep);

    // Other methods (not part of DOM)

    virtual bool isTextNode() const { return true; }
    virtual void attach();
    virtual bool rendererIsNeeded(RenderStyle *);
    virtual RenderObject *createRenderer(RenderArena *, RenderStyle *);
    virtual void recalcStyle( StyleChange = NoChange );
    virtual bool childTypeAllowed( unsigned short type );

    virtual DOMString toString() const;

#ifndef NDEBUG
    virtual void formatForDebugger(char *buffer, unsigned length) const;
#endif

protected:
    virtual TextImpl* createNew(DOMStringImpl*);
};

// ----------------------------------------------------------------------------

class CDATASectionImpl : public TextImpl
{
// ### should these have id==ID_TEXT
public:
    CDATASectionImpl(DocumentImpl *impl, const DOMString &_text);
    CDATASectionImpl(DocumentImpl *impl);
    virtual ~CDATASectionImpl();

    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual PassRefPtr<NodeImpl> cloneNode(bool deep);
    virtual bool childTypeAllowed( unsigned short type );
    virtual DOMString toString() const;

protected:
    virtual TextImpl* createNew(DOMStringImpl*);
};

// ----------------------------------------------------------------------------

class EditingTextImpl : public TextImpl
{
public:
    EditingTextImpl(DocumentImpl *impl, const DOMString &text);
    EditingTextImpl(DocumentImpl *impl);
    virtual ~EditingTextImpl();

    virtual bool rendererIsNeeded(RenderStyle *);
};

} //namespace

#endif
