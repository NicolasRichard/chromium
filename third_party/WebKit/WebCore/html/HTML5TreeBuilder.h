/*
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL GOOGLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef HTML5TreeBuilder_h
#define HTML5TreeBuilder_h

#include "HTML5Lexer.h"
#include <wtf/Noncopyable.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/unicode/Unicode.h>

namespace WebCore {

class Document;
class Element;
class Frame;
class HTML5Token;
class HTMLDocument;
class LegacyHTMLTreeConstructor;
class Node;

class HTML5TreeBuilder : public Noncopyable {
public:
    HTML5TreeBuilder(HTML5Lexer*, HTMLDocument*, bool reportErrors);
    ~HTML5TreeBuilder();

    void setPaused(bool paused) { m_isPaused = paused; }
    bool isPaused() { return m_isPaused; }

    // The token really should be passed as a const& since it's never modified.
    PassRefPtr<Node> constructTreeFromToken(HTML5Token&);
    // Must be called when parser is paused before calling the parser again.
    PassRefPtr<Element> takeScriptToProcess(int& scriptStartLine);

    // Done, close any open tags, etc.
    void finished();

    static HTML5Lexer::State adjustedLexerState(HTML5Lexer::State, const AtomicString& tagName);

    // FIXME: This is a dirty, rotten hack to keep HTMLFormControlElement happy
    // until we stop using the legacy parser. DO NOT CALL THIS METHOD.
    LegacyHTMLTreeConstructor* legacyTreeConstructor() const { return m_legacyTreeConstructor.get(); }

private:
    // Represents HTML5 "insertion mode"
    // http://www.w3.org/TR/html5/syntax.html#insertion-mode
    // FIXME: Implement remainder of states.
    enum InsertionMode {
        Initial,
        AfterFrameset,
    };

    PassRefPtr<Node> passTokenToLegacyParser(HTML5Token&);
    PassRefPtr<Node> processToken(HTML5Token&, UChar currentCharacter = 0);
    
    void handleScriptStartTag();
    void handleScriptEndTag(Element*, int scriptStartLine);

    void setInsertionMode(InsertionMode value) { m_insertionMode = value; }
    InsertionMode insertionMode() const { return m_insertionMode; }

    Document* m_document; // This is only used by the m_legacyParser for now.
    bool m_reportErrors;
    bool m_isPaused;

    InsertionMode m_insertionMode;

    // HTML5 spec requires that we be able to change the state of the lexer
    // from within parser actions.
    HTML5Lexer* m_lexer;

    // We're re-using logic from the old LegacyHTMLTreeConstructor while this class is being written.
    OwnPtr<LegacyHTMLTreeConstructor> m_legacyTreeConstructor;

    // These members are intentionally duplicated as the first set is a hack
    // on top of the legacy parser which will eventually be removed.
    RefPtr<Element> m_lastScriptElement; // FIXME: Hack for <script> support on top of the old parser.
    int m_lastScriptElementStartLine; // FIXME: Hack for <script> support on top of the old parser.

    RefPtr<Element> m_scriptToProcess; // <script> tag which needs processing before resuming the parser.
    int m_scriptToProcessStartLine; // Starting line number of the script tag needing processing.
};

}

#endif
