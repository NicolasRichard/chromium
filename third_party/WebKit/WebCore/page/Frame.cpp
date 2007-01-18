/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *                     2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.
 * Copyright (C) 2005 Alexey Proskuryakov <ap@nypop.com>
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
 */

#include "config.h"
#include "Frame.h"
#include "FramePrivate.h"

#include "ApplyStyleCommand.h"
#include "BeforeUnloadEvent.h"
#include "Chrome.h"
#include "CSSComputedStyleDeclaration.h"
#include "CSSProperty.h"
#include "CSSPropertyNames.h"
#include "Cache.h"
#include "CachedCSSStyleSheet.h"
#include "DOMWindow.h"
#include "DocLoader.h"
#include "DocumentType.h"
#include "EditingText.h"
#include "Event.h"
#include "EventNames.h"
#include "FloatRect.h"
#include "Frame.h"
#include "FrameLoadRequest.h"
#include "FrameLoader.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLFormElement.h"
#include "HTMLFrameElementBase.h"
#include "HTMLGenericFormElement.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "HTMLObjectElement.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "IconDatabase.h"
#include "IconLoader.h"
#include "ImageDocument.h"
#include "IndentOutdentCommand.h"
#include "MediaFeatureNames.h"
#include "MouseEventWithHitTestResults.h"
#include "NodeList.h"
#include "Page.h"
#include "PlatformScrollBar.h"
#include "RenderListBox.h"
#include "RenderObject.h"
#include "RenderPart.h"
#include "RenderTextControl.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "SegmentedString.h"
#include "TextIterator.h"
#include "TextResourceDecoder.h"
#include "TypingCommand.h"
#include "XMLTokenizer.h"
#include "cssstyleselector.h"
#include "htmlediting.h"
#include "kjs_proxy.h"
#include "kjs_window.h"
#include "markup.h"
#include "visible_units.h"
#include "xmlhttprequest.h"
#include <math.h>
#include <sys/types.h>
#include <wtf/Platform.h>

#if PLATFORM(MAC)
#include "FrameMac.h"
#endif

#if !PLATFORM(WIN_OS)
#include <unistd.h>
#endif

#ifdef SVG_SUPPORT
#include "SVGNames.h"
#include "XLinkNames.h"
#include "XMLNames.h"
#include "SVGDocument.h"
#include "SVGDocumentExtensions.h"
#endif

using namespace std;

using KJS::JSLock;
using KJS::JSValue;
using KJS::Location;
using KJS::PausedTimeouts;
using KJS::SavedProperties;
using KJS::SavedBuiltins;
using KJS::UString;
using KJS::Window;

namespace WebCore {

using namespace EventNames;
using namespace HTMLNames;

const double caretBlinkFrequency = 0.5;

class UserStyleSheetLoader : public CachedResourceClient {
public:
    UserStyleSheetLoader(Frame* frame, const String& url, DocLoader* docLoader)
        : m_frame(frame)
        , m_cachedSheet(docLoader->requestCSSStyleSheet(url, ""))
    {
        m_cachedSheet->ref(this);
    }
    ~UserStyleSheetLoader()
    {
        m_cachedSheet->deref(this);
    }
private:
    virtual void setCSSStyleSheet(const String& /*URL*/, const String& /*charset*/, const String& sheet)
    {
        m_frame->setUserStyleSheet(sheet);
    }
    Frame* m_frame;
    CachedCSSStyleSheet* m_cachedSheet;
};

#ifndef NDEBUG
struct FrameCounter { 
    static int count; 
    ~FrameCounter() { if (count != 0) fprintf(stderr, "LEAK: %d Frame\n", count); }
};
int FrameCounter::count = 0;
static FrameCounter frameCounter;
#endif

static inline Frame* parentFromOwnerElement(HTMLFrameOwnerElement* ownerElement)
{
    if (!ownerElement)
        return 0;
    return ownerElement->document()->frame();
}

Frame::Frame(Page* page, HTMLFrameOwnerElement* ownerElement, FrameLoaderClient* frameLoaderClient) 
    : d(new FramePrivate(page, parentFromOwnerElement(ownerElement), this, ownerElement, frameLoaderClient))
{
    AtomicString::init();
    EventNames::init();
    HTMLNames::init();
    QualifiedName::init();
    MediaFeatureNames::init();

#ifdef SVG_SUPPORT
    SVGNames::init();
    XLinkNames::init();
    XMLNames::init();
#endif

    if (!ownerElement)
        page->setMainFrame(this);
    else {
        // FIXME: Frames were originally created with a refcount of 1.
        // Leave this ref call here until we can straighten that out.
        ref();
        page->incrementFrameCount();
        ownerElement->m_contentFrame = this;
    }

#ifndef NDEBUG
    ++FrameCounter::count;
#endif
}

Frame::~Frame()
{
    // FIXME: We should not be doing all this work inside the destructor

    ASSERT(!d->m_lifeSupportTimer.isActive());

#ifndef NDEBUG
    --FrameCounter::count;
#endif

    if (d->m_jscript && d->m_jscript->haveInterpreter())
        if (Window* w = Window::retrieveWindow(this)) {
            w->disconnectFrame();
            // Must clear the window pointer, otherwise we will not
            // garbage-collect collect the window (inside the call to
            // delete d below).
            w = 0;
        }

    disconnectOwnerElement();
    
    if (d->m_domWindow)
        d->m_domWindow->disconnectFrame();
            
    if (d->m_view) {
        d->m_view->hide();
        d->m_view->m_frame = 0;
    }
  
    ASSERT(!d->m_lifeSupportTimer.isActive());

    delete d->m_userStyleSheetLoader;
    delete d;
    d = 0;
}

FrameLoader* Frame::loader() const
{
    return d->m_loader;
}

FrameView* Frame::view() const
{
    return d->m_view.get();
}

void Frame::setView(FrameView* view)
{
    // Detach the document now, so any onUnload handlers get run - if
    // we wait until the view is destroyed, then things won't be
    // hooked up enough for some JavaScript calls to work.
    if (d->m_doc && view == 0)
        d->m_doc->detach();

    d->m_view = view;

#ifdef MULTIPLE_FORM_SUBMISSION_PROTECTION
    // Only one form submission is allowed per view of a part.
    // Since this part may be getting reused as a result of being
    // pulled from the back/forward cache, reset this flag.
    loader()->resetMultipleFormSubmissionProtection();
#endif
}

KJSProxy *Frame::scriptProxy()
{
    if (!settings()->isJavaScriptEnabled())
        return 0;

    if (!d->m_jscript)
        d->m_jscript = new KJSProxy(this);

    return d->m_jscript;
}

Document *Frame::document() const
{
    if (d)
        return d->m_doc.get();
    return 0;
}

void Frame::setDocument(Document* newDoc)
{
    if (d) {
        if (d->m_doc)
            d->m_doc->detach();
        d->m_doc = newDoc;
        if (newDoc)
            newDoc->attach();
    }
}

const Settings *Frame::settings() const
{
  return d->m_settings;
}

void Frame::setUserStyleSheetLocation(const KURL& url)
{
    delete d->m_userStyleSheetLoader;
    d->m_userStyleSheetLoader = 0;
    if (d->m_doc && d->m_doc->docLoader())
        d->m_userStyleSheetLoader = new UserStyleSheetLoader(this, url.url(), d->m_doc->docLoader());
}

void Frame::setUserStyleSheet(const String& styleSheet)
{
    delete d->m_userStyleSheetLoader;
    d->m_userStyleSheetLoader = 0;
    if (d->m_doc)
        d->m_doc->setUserStyleSheet(styleSheet);
}

String Frame::selectedText() const
{
    return plainText(selectionController()->toRange().get());
}

SelectionController* Frame::selectionController() const
{
    return &d->m_selectionController;
}

Editor* Frame::editor() const
{
    return &d->m_editor;
}

CommandByName* Frame::command() const
{
    return &d->m_command;
}

TextGranularity Frame::selectionGranularity() const
{
    return d->m_selectionGranularity;
}

void Frame::setSelectionGranularity(TextGranularity granularity) const
{
    d->m_selectionGranularity = granularity;
}

SelectionController* Frame::dragCaretController() const
{
    return d->m_page->dragCaretController();
}

const Selection& Frame::mark() const
{
    return d->m_mark;
}

void Frame::setMark(const Selection& s)
{
    ASSERT(!s.base().node() || s.base().node()->document() == document());
    ASSERT(!s.extent().node() || s.extent().node()->document() == document());
    ASSERT(!s.start().node() || s.start().node()->document() == document());
    ASSERT(!s.end().node() || s.end().node()->document() == document());

    d->m_mark = s;
}

void Frame::notifyRendererOfSelectionChange(bool userTriggered)
{
    RenderObject* renderer = 0;
    if (selectionController()->rootEditableElement())
        renderer = selectionController()->rootEditableElement()->shadowAncestorNode()->renderer();

    // If the current selection is in a textfield or textarea, notify the renderer that the selection has changed
    if (renderer && (renderer->isTextArea() || renderer->isTextField()))
        static_cast<RenderTextControl*>(renderer)->selectionChanged(userTriggered);
}

void Frame::invalidateSelection()
{
    selectionController()->setNeedsLayout();
    selectionLayoutChanged();
}

void Frame::setCaretVisible(bool flag)
{
    if (d->m_caretVisible == flag)
        return;
    clearCaretRectIfNeeded();
    if (flag)
        setFocusedNodeIfNeeded();
    d->m_caretVisible = flag;
    selectionLayoutChanged();
}


void Frame::clearCaretRectIfNeeded()
{
    if (d->m_caretPaint) {
        d->m_caretPaint = false;
        selectionController()->invalidateCaretRect();
    }        
}

// Helper function that tells whether a particular node is an element that has an entire
// Frame and FrameView, a <frame>, <iframe>, or <object>.
static bool isFrameElement(const Node *n)
{
    if (!n)
        return false;
    RenderObject *renderer = n->renderer();
    if (!renderer || !renderer->isWidget())
        return false;
    Widget* widget = static_cast<RenderWidget*>(renderer)->widget();
    return widget && widget->isFrameView();
}

void Frame::setFocusedNodeIfNeeded()
{
    if (!document() || selectionController()->isNone() || !d->m_isActive)
        return;

    Node* target = selectionController()->rootEditableElement();
    if (target) {
        RenderObject* renderer = target->renderer();

        // Walk up the render tree to search for a node to focus.
        // Walking up the DOM tree wouldn't work for shadow trees, like those behind the engine-based text fields.
        while (renderer) {
            // We don't want to set focus on a subframe when selecting in a parent frame,
            // so add the !isFrameElement check here. There's probably a better way to make this
            // work in the long term, but this is the safest fix at this time.
            if (target && target->isMouseFocusable() && !isFrameElement(target)) {
                document()->setFocusedNode(target);
                return;
            }
            renderer = renderer->parent();
            if (renderer)
                target = renderer->element();
        }
        document()->setFocusedNode(0);
    }
}

void Frame::selectionLayoutChanged()
{
    bool caretRectChanged = selectionController()->recomputeCaretRect();

    bool shouldBlink = d->m_caretVisible
        && selectionController()->isCaret() && selectionController()->isContentEditable();

    // If the caret moved, stop the blink timer so we can restart with a
    // black caret in the new location.
    if (caretRectChanged || !shouldBlink)
        d->m_caretBlinkTimer.stop();

    // Start blinking with a black caret. Be sure not to restart if we're
    // already blinking in the right location.
    if (shouldBlink && !d->m_caretBlinkTimer.isActive()) {
        d->m_caretBlinkTimer.startRepeating(caretBlinkFrequency);
        d->m_caretPaint = true;
    }

    if (d->m_doc)
        d->m_doc->updateSelection();
}

void Frame::setXPosForVerticalArrowNavigation(int x)
{
    d->m_xPosForVerticalArrowNavigation = x;
}

int Frame::xPosForVerticalArrowNavigation() const
{
    return d->m_xPosForVerticalArrowNavigation;
}

void Frame::caretBlinkTimerFired(Timer<Frame>*)
{
    ASSERT(d->m_caretVisible);
    ASSERT(selectionController()->isCaret());
    bool caretPaint = d->m_caretPaint;
    if (selectionController()->isCaretBlinkingSuspended() && caretPaint)
        return;
    d->m_caretPaint = !caretPaint;
    selectionController()->invalidateCaretRect();
}

void Frame::paintCaret(GraphicsContext* p, const IntRect& rect) const
{
    if (d->m_caretPaint && d->m_caretVisible)
        selectionController()->paintCaret(p, rect);
}

void Frame::paintDragCaret(GraphicsContext* p, const IntRect& rect) const
{
    SelectionController* dragCaretController = d->m_page->dragCaretController();
    assert(dragCaretController->selection().isCaret());
    if (dragCaretController->selection().start().node()->document()->frame() == this)
        dragCaretController->paintCaret(p, rect);
}

int Frame::zoomFactor() const
{
  return d->m_zoomFactor;
}

void Frame::setZoomFactor(int percent)
{  
  if (d->m_zoomFactor == percent)
      return;

  d->m_zoomFactor = percent;

  if (d->m_doc)
      d->m_doc->recalcStyle(Node::Force);

  for (Frame* child = tree()->firstChild(); child; child = child->tree()->nextSibling())
      child->setZoomFactor(d->m_zoomFactor);

  if (d->m_doc && d->m_doc->renderer() && d->m_doc->renderer()->needsLayout())
      view()->layout();
}

void Frame::setJSStatusBarText(const String& text)
{
    d->m_kjsStatusBarText = text;
    setStatusBarText(d->m_kjsStatusBarText);
}

void Frame::setJSDefaultStatusBarText(const String& text)
{
    d->m_kjsDefaultStatusBarText = text;
    setStatusBarText(d->m_kjsDefaultStatusBarText);
}

String Frame::jsStatusBarText() const
{
    return d->m_kjsStatusBarText;
}

String Frame::jsDefaultStatusBarText() const
{
   return d->m_kjsDefaultStatusBarText;
}

void Frame::reparseConfiguration()
{
    if (d->m_doc)
        d->m_doc->docLoader()->setAutoLoadImages(d->m_settings->loadsImagesAutomatically());
        
    const KURL& userStyleSheetLocation = d->m_settings->userStyleSheetLocation();
    if (!userStyleSheetLocation.isEmpty())
        setUserStyleSheetLocation(userStyleSheetLocation);
    else
        setUserStyleSheet(String());

    // FIXME: It's not entirely clear why the following is needed.
    // The document automatically does this as required when you set the style sheet.
    // But we had problems when this code was removed. Details are in
    // <http://bugs.webkit.org/show_bug.cgi?id=8079>.
    if (d->m_doc)
        d->m_doc->updateStyleSelector();
}

bool Frame::shouldChangeSelection(const Selection& newSelection) const
{
    return shouldChangeSelection(selectionController()->selection(), newSelection, newSelection.affinity(), false);
}

bool Frame::shouldDeleteSelection(const Selection& newSelection) const
{
    return true;
}

bool Frame::isContentEditable() const 
{
    if (!d->m_doc)
        return false;
    return d->m_doc->inDesignMode();
}

void Frame::textFieldDidBeginEditing(Element* input)
{
}

void Frame::textFieldDidEndEditing(Element* input)
{
}

void Frame::textDidChangeInTextField(Element* input)
{
}

bool Frame::doTextFieldCommandFromEvent(Element*, KeyboardEvent*)
{
    return false;
}

void Frame::textWillBeDeletedInTextField(Element* input)
{
}

void Frame::textDidChangeInTextArea(Element* input)
{
}

CSSMutableStyleDeclaration *Frame::typingStyle() const
{
    return d->m_typingStyle.get();
}

void Frame::setTypingStyle(CSSMutableStyleDeclaration *style)
{
    d->m_typingStyle = style;
}

void Frame::clearTypingStyle()
{
    d->m_typingStyle = 0;
}

void Frame::copyToPasteboard()
{
    issueCopyCommand();
}

void Frame::cutToPasteboard()
{
    issueCutCommand();
}

void Frame::pasteFromPasteboard()
{
    issuePasteCommand();
}

void Frame::pasteAndMatchStyle()
{
    issuePasteAndMatchStyleCommand();
}

void Frame::transpose()
{
    issueTransposeCommand();
}

void Frame::computeAndSetTypingStyle(CSSStyleDeclaration *style, EditAction editingAction)
{
    if (!style || style->length() == 0) {
        clearTypingStyle();
        return;
    }

    // Calculate the current typing style.
    RefPtr<CSSMutableStyleDeclaration> mutableStyle = style->makeMutable();
    if (typingStyle()) {
        typingStyle()->merge(mutableStyle.get());
        mutableStyle = typingStyle();
    }

    Node *node = selectionController()->selection().visibleStart().deepEquivalent().node();
    CSSComputedStyleDeclaration computedStyle(node);
    computedStyle.diff(mutableStyle.get());
    
    // Handle block styles, substracting these from the typing style.
    RefPtr<CSSMutableStyleDeclaration> blockStyle = mutableStyle->copyBlockProperties();
    blockStyle->diff(mutableStyle.get());
    if (document() && blockStyle->length() > 0)
        applyCommand(new ApplyStyleCommand(document(), blockStyle.get(), editingAction));
    
    // Set the remaining style as the typing style.
    d->m_typingStyle = mutableStyle.release();
}

static void updateState(CSSMutableStyleDeclaration *desiredStyle, CSSComputedStyleDeclaration *computedStyle, bool& atStart, Frame::TriState& state)
{
    DeprecatedValueListConstIterator<CSSProperty> end;
    for (DeprecatedValueListConstIterator<CSSProperty> it = desiredStyle->valuesIterator(); it != end; ++it) {
        int propertyID = (*it).id();
        String desiredProperty = desiredStyle->getPropertyValue(propertyID);
        String computedProperty = computedStyle->getPropertyValue(propertyID);
        Frame::TriState propertyState = equalIgnoringCase(desiredProperty, computedProperty)
            ? Frame::trueTriState : Frame::falseTriState;
        if (atStart) {
            state = propertyState;
            atStart = false;
        } else if (state != propertyState) {
            state = Frame::mixedTriState;
            break;
        }
    }
}

Frame::TriState Frame::selectionHasStyle(CSSStyleDeclaration *style) const
{
    bool atStart = true;
    TriState state = falseTriState;

    RefPtr<CSSMutableStyleDeclaration> mutableStyle = style->makeMutable();

    if (!selectionController()->isRange()) {
        Node* nodeToRemove;
        RefPtr<CSSComputedStyleDeclaration> selectionStyle = selectionComputedStyle(nodeToRemove);
        if (!selectionStyle)
            return falseTriState;
        updateState(mutableStyle.get(), selectionStyle.get(), atStart, state);
        if (nodeToRemove) {
            ExceptionCode ec = 0;
            nodeToRemove->remove(ec);
            assert(ec == 0);
        }
    } else {
        for (Node* node = selectionController()->start().node(); node; node = node->traverseNextNode()) {
            RefPtr<CSSComputedStyleDeclaration> computedStyle = new CSSComputedStyleDeclaration(node);
            if (computedStyle)
                updateState(mutableStyle.get(), computedStyle.get(), atStart, state);
            if (state == mixedTriState)
                break;
            if (node == selectionController()->end().node())
                break;
        }
    }

    return state;
}

String Frame::selectionStartStylePropertyValue(int stylePropertyID) const
{
    Node *nodeToRemove;
    RefPtr<CSSStyleDeclaration> selectionStyle = selectionComputedStyle(nodeToRemove);
    if (!selectionStyle)
        return String();

    String value = selectionStyle->getPropertyValue(stylePropertyID);

    if (nodeToRemove) {
        ExceptionCode ec = 0;
        nodeToRemove->remove(ec);
        assert(ec == 0);
    }

    return value;
}

CSSComputedStyleDeclaration *Frame::selectionComputedStyle(Node *&nodeToRemove) const
{
    nodeToRemove = 0;

    if (!document())
        return 0;

    if (selectionController()->isNone())
        return 0;

    RefPtr<Range> range(selectionController()->toRange());
    Position pos = range->editingStartPosition();

    Element *elem = pos.element();
    if (!elem)
        return 0;
    
    RefPtr<Element> styleElement = elem;
    ExceptionCode ec = 0;

    if (d->m_typingStyle) {
        styleElement = document()->createElementNS(xhtmlNamespaceURI, "span", ec);
        ASSERT(ec == 0);

        styleElement->setAttribute(styleAttr, d->m_typingStyle->cssText().impl(), ec);
        ASSERT(ec == 0);
        
        styleElement->appendChild(document()->createEditingTextNode(""), ec);
        ASSERT(ec == 0);

        if (elem->renderer() && elem->renderer()->canHaveChildren()) {
            elem->appendChild(styleElement, ec);
        } else {
            Node *parent = elem->parent();
            Node *next = elem->nextSibling();

            if (next) {
                parent->insertBefore(styleElement, next, ec);
            } else {
                parent->appendChild(styleElement, ec);
            }
        }
        assert(ec == 0);

        nodeToRemove = styleElement.get();
    }

    return new CSSComputedStyleDeclaration(styleElement);
}

void Frame::applyEditingStyleToBodyElement() const
{
    if (!d->m_doc)
        return;
        
    RefPtr<NodeList> list = d->m_doc->getElementsByTagName("body");
    unsigned len = list->length();
    for (unsigned i = 0; i < len; i++) {
        applyEditingStyleToElement(static_cast<Element*>(list->item(i)));    
    }
}

void Frame::removeEditingStyleFromBodyElement() const
{
    if (!d->m_doc)
        return;
        
    RefPtr<NodeList> list = d->m_doc->getElementsByTagName("body");
    unsigned len = list->length();
    for (unsigned i = 0; i < len; i++) {
        removeEditingStyleFromElement(static_cast<Element*>(list->item(i)));    
    }
}

void Frame::applyEditingStyleToElement(Element* element) const
{
    if (!element)
        return;

    CSSStyleDeclaration* style = element->style();
    ASSERT(style);

    ExceptionCode ec = 0;
    style->setProperty(CSS_PROP_WORD_WRAP, "break-word", false, ec);
    ASSERT(ec == 0);
    style->setProperty(CSS_PROP__WEBKIT_NBSP_MODE, "space", false, ec);
    ASSERT(ec == 0);
    style->setProperty(CSS_PROP__WEBKIT_LINE_BREAK, "after-white-space", false, ec);
    ASSERT(ec == 0);
}

void Frame::removeEditingStyleFromElement(Element*) const
{
}

bool Frame::isCharacterSmartReplaceExempt(UChar, bool)
{
    // no smart replace
    return true;
}

#ifndef NDEBUG
static HashSet<Frame*> lifeSupportSet;
#endif

void Frame::endAllLifeSupport()
{
#ifndef NDEBUG
    HashSet<Frame*> lifeSupportCopy = lifeSupportSet;
    HashSet<Frame*>::iterator end = lifeSupportCopy.end();
    for (HashSet<Frame*>::iterator it = lifeSupportCopy.begin(); it != end; ++it)
        (*it)->endLifeSupport();
#endif
}

void Frame::keepAlive()
{
    if (d->m_lifeSupportTimer.isActive())
        return;
    ref();
#ifndef NDEBUG
    lifeSupportSet.add(this);
#endif
    d->m_lifeSupportTimer.startOneShot(0);
}

void Frame::endLifeSupport()
{
    if (!d->m_lifeSupportTimer.isActive())
        return;
    d->m_lifeSupportTimer.stop();
#ifndef NDEBUG
    lifeSupportSet.remove(this);
#endif
    deref();
}

void Frame::lifeSupportTimerFired(Timer<Frame>*)
{
#ifndef NDEBUG
    lifeSupportSet.remove(this);
#endif
    deref();
}

void Frame::setSettings(Settings *settings)
{
    d->m_settings = settings;
}

RenderObject *Frame::renderer() const
{
    Document *doc = document();
    return doc ? doc->renderer() : 0;
}

HTMLFrameOwnerElement* Frame::ownerElement() const
{
    return d->m_ownerElement;
}

RenderPart* Frame::ownerRenderer()
{
    HTMLFrameOwnerElement* ownerElement = d->m_ownerElement;
    if (!ownerElement)
        return 0;
    return static_cast<RenderPart*>(ownerElement->renderer());
}

IntRect Frame::selectionRect() const
{
    RenderView *root = static_cast<RenderView*>(renderer());
    if (!root)
        return IntRect();

    return root->selectionRect();
}

// returns FloatRect because going through IntRect would truncate any floats
FloatRect Frame::visibleSelectionRect() const
{
    if (!d->m_view)
        return FloatRect();
    
    return intersection(selectionRect(), d->m_view->visibleContentRect());
}

bool Frame::isFrameSet() const
{
    Document* document = d->m_doc.get();
    if (!document || !document->isHTMLDocument())
        return false;
    Node *body = static_cast<HTMLDocument*>(document)->body();
    return body && body->renderer() && body->hasTagName(framesetTag);
}

// Scans logically forward from "start", including any child frames
static HTMLFormElement *scanForForm(Node *start)
{
    Node *n;
    for (n = start; n; n = n->traverseNextNode()) {
        if (n->hasTagName(formTag))
            return static_cast<HTMLFormElement*>(n);
        else if (n->isHTMLElement() && static_cast<HTMLElement*>(n)->isGenericFormElement())
            return static_cast<HTMLGenericFormElement*>(n)->form();
        else if (n->hasTagName(frameTag) || n->hasTagName(iframeTag)) {
            Node *childDoc = static_cast<HTMLFrameElementBase*>(n)->contentDocument();
            if (HTMLFormElement *frameResult = scanForForm(childDoc))
                return frameResult;
        }
    }
    return 0;
}

// We look for either the form containing the current focus, or for one immediately after it
HTMLFormElement *Frame::currentForm() const
{
    // start looking either at the active (first responder) node, or where the selection is
    Node *start = d->m_doc ? d->m_doc->focusedNode() : 0;
    if (!start)
        start = selectionController()->start().node();
    
    // try walking up the node tree to find a form element
    Node *n;
    for (n = start; n; n = n->parentNode()) {
        if (n->hasTagName(formTag))
            return static_cast<HTMLFormElement*>(n);
        else if (n->isHTMLElement()
                   && static_cast<HTMLElement*>(n)->isGenericFormElement())
            return static_cast<HTMLGenericFormElement*>(n)->form();
    }
    
    // try walking forward in the node tree to find a form element
    return start ? scanForForm(start) : 0;
}

// FIXME: should this go in SelectionController?
void Frame::revealSelection(const RenderLayer::ScrollAlignment& alignment) const
{
    IntRect rect;
    
    switch (selectionController()->state()) {
        case Selection::NONE:
            return;
            
        case Selection::CARET:
            rect = selectionController()->caretRect();
            break;
            
        case Selection::RANGE:
            rect = selectionRect();
            break;
    }

    Position start = selectionController()->start();

    ASSERT(start.node());
    if (start.node() && start.node()->renderer()) {
        // FIXME: This code only handles scrolling the startContainer's layer, but
        // the selection rect could intersect more than just that. 
        // See <rdar://problem/4799899>.
        if (RenderLayer *layer = start.node()->renderer()->enclosingLayer())
            layer->scrollRectToVisible(rect, alignment, alignment);
    }
}

void Frame::revealCaret(const RenderLayer::ScrollAlignment& alignment) const
{
    if (selectionController()->isNone())
        return;

    Position extent = selectionController()->extent();
    if (extent.node() && extent.node()->renderer()) {
        IntRect extentRect = VisiblePosition(extent).caretRect();
        RenderLayer* layer = extent.node()->renderer()->enclosingLayer();
        if (layer)
            layer->scrollRectToVisible(extentRect, alignment, alignment);
    }
}

// FIXME: why is this here instead of on the FrameView?
void Frame::paint(GraphicsContext* p, const IntRect& rect)
{
#ifndef NDEBUG
    bool fillWithRed;
    if (!document() || document()->printing())
        fillWithRed = false; // Printing, don't fill with red (can't remember why).
    else if (document()->ownerElement())
        fillWithRed = false; // Subframe, don't fill with red.
    else if (view() && view()->isTransparent())
        fillWithRed = false; // Transparent, don't fill with red.
    else if (d->m_paintRestriction == PaintRestrictionSelectionOnly || d->m_paintRestriction == PaintRestrictionSelectionOnlyWhiteText)
        fillWithRed = false; // Selections are transparent, don't fill with red.
    else if (d->m_elementToDraw)
        fillWithRed = false; // Element images are transparent, don't fill with red.
    else
        fillWithRed = true;
    
    if (fillWithRed)
        p->fillRect(rect, Color(0xFF, 0, 0));
#endif
    
    if (renderer()) {
        // d->m_elementToDraw is used to draw only one element
        RenderObject *eltRenderer = d->m_elementToDraw ? d->m_elementToDraw->renderer() : 0;
        if (d->m_paintRestriction == PaintRestrictionNone)
            renderer()->document()->invalidateRenderedRectsForMarkersInRect(rect);
        renderer()->layer()->paint(p, rect, d->m_paintRestriction, eltRenderer);

#if PLATFORM(MAC)
        // Regions may have changed as a result of the visibility/z-index of element changing.
        if (renderer()->document()->dashboardRegionsDirty())
            renderer()->view()->frameView()->updateDashboardRegions();
#endif
    } else
        LOG_ERROR("called Frame::paint with nil renderer");
}

void Frame::adjustPageHeight(float *newBottom, float oldTop, float oldBottom, float bottomLimit)
{
    RenderView *root = static_cast<RenderView*>(document()->renderer());
    if (root) {
        // Use a context with painting disabled.
        GraphicsContext context((PlatformGraphicsContext*)0);
        root->setTruncatedAt((int)floorf(oldBottom));
        IntRect dirtyRect(0, (int)floorf(oldTop), root->docWidth(), (int)ceilf(oldBottom - oldTop));
        root->layer()->paint(&context, dirtyRect);
        *newBottom = root->bestTruncatedAt();
        if (*newBottom == 0)
            *newBottom = oldBottom;
    } else
        *newBottom = oldBottom;
}

Frame* Frame::frameForWidget(const Widget* widget)
{
    ASSERT_ARG(widget, widget);

    if (RenderWidget* renderer = RenderWidget::find(widget))
        if (Node* node = renderer->node())
            return node->document()->frame();

    // Assume all widgets are either a FrameView or owned by a RenderWidget.
    // FIXME: That assumption is not right for scroll bars!
    ASSERT(widget->isFrameView());
    return static_cast<const FrameView*>(widget)->frame();
}

void Frame::forceLayout()
{
    FrameView *v = d->m_view.get();
    if (v) {
        v->layout(false);
        // We cannot unschedule a pending relayout, since the force can be called with
        // a tiny rectangle from a drawRect update.  By unscheduling we in effect
        // "validate" and stop the necessary full repaint from occurring.  Basically any basic
        // append/remove DHTML is broken by this call.  For now, I have removed the optimization
        // until we have a better invalidation stategy. -dwh
        //v->unscheduleRelayout();
    }
}

void Frame::forceLayoutWithPageWidthRange(float minPageWidth, float maxPageWidth)
{
    // Dumping externalRepresentation(m_frame->renderer()).ascii() is a good trick to see
    // the state of things before and after the layout
    RenderView *root = static_cast<RenderView*>(document()->renderer());
    if (root) {
        // This magic is basically copied from khtmlview::print
        int pageW = (int)ceilf(minPageWidth);
        root->setWidth(pageW);
        root->setNeedsLayoutAndMinMaxRecalc();
        forceLayout();
        
        // If we don't fit in the minimum page width, we'll lay out again. If we don't fit in the
        // maximum page width, we will lay out to the maximum page width and clip extra content.
        // FIXME: We are assuming a shrink-to-fit printing implementation.  A cropping
        // implementation should not do this!
        int rightmostPos = root->rightmostPosition();
        if (rightmostPos > minPageWidth) {
            pageW = min(rightmostPos, (int)ceilf(maxPageWidth));
            root->setWidth(pageW);
            root->setNeedsLayoutAndMinMaxRecalc();
            forceLayout();
        }
    }
}

void Frame::sendResizeEvent()
{
    if (Document* doc = document())
        doc->dispatchWindowEvent(EventNames::resizeEvent, false, false);
}

void Frame::sendScrollEvent()
{
    FrameView *v = d->m_view.get();
    if (v) {
        Document *doc = document();
        if (!doc)
            return;
        doc->dispatchHTMLEvent(scrollEvent, true, false);
    }
}

void Frame::clearTimers(FrameView *view)
{
    if (view) {
        view->unscheduleRelayout();
        if (view->frame()) {
            Document* document = view->frame()->document();
            if (document && document->renderer() && document->renderer()->layer())
                document->renderer()->layer()->suspendMarquees();
        }
    }
}

void Frame::clearTimers()
{
    clearTimers(d->m_view.get());
}

RenderStyle *Frame::styleForSelectionStart(Node *&nodeToRemove) const
{
    nodeToRemove = 0;
    
    if (!document())
        return 0;
    if (selectionController()->isNone())
        return 0;
    
    Position pos = selectionController()->selection().visibleStart().deepEquivalent();
    if (!pos.isCandidate())
        return 0;
    Node *node = pos.node();
    if (!node)
        return 0;
    
    if (!d->m_typingStyle)
        return node->renderer()->style();
    
    ExceptionCode ec = 0;
    RefPtr<Element> styleElement = document()->createElementNS(xhtmlNamespaceURI, "span", ec);
    ASSERT(ec == 0);
    
    styleElement->setAttribute(styleAttr, d->m_typingStyle->cssText().impl(), ec);
    ASSERT(ec == 0);
    
    styleElement->appendChild(document()->createEditingTextNode(""), ec);
    ASSERT(ec == 0);
    
    node->parentNode()->appendChild(styleElement, ec);
    ASSERT(ec == 0);
    
    nodeToRemove = styleElement.get();    
    return styleElement->renderer()->style();
}

void Frame::setSelectionFromNone()
{
    // Put a caret inside the body if the entire frame is editable (either the 
    // entire WebView is editable or designMode is on for this document).
    Document *doc = document();
    if (!doc || !selectionController()->isNone() || !isContentEditable())
        return;
        
    Node* node = doc->documentElement();
    while (node && !node->hasTagName(bodyTag))
        node = node->traverseNextNode();
    if (node)
        selectionController()->setSelection(Selection(Position(node, 0), DOWNSTREAM));
}

bool Frame::isActive() const
{
    return d->m_isActive;
}

void Frame::setIsActive(bool flag)
{
    if (d->m_isActive == flag)
        return;
    
    d->m_isActive = flag;

    // This method does the job of updating the view based on whether the view is "active".
    // This involves three kinds of drawing updates:

    // 1. The background color used to draw behind selected content (active | inactive color)
    if (d->m_view)
        d->m_view->updateContents(enclosingIntRect(visibleSelectionRect()));

    // 2. Caret blinking (blinks | does not blink)
    if (flag)
        setSelectionFromNone();
    setCaretVisible(flag);
    
    // 3. The drawing of a focus ring around links in web pages.
    Document *doc = document();
    if (doc) {
        Node *node = doc->focusedNode();
        if (node) {
            node->setChanged();
            if (node->renderer() && node->renderer()->style()->hasAppearance())
                theme()->stateChanged(node->renderer(), FocusState);
        }
    }
    
    // 4. Changing the tint of controls from clear to aqua/graphite and vice versa.  We
    // do a "fake" paint.  When the theme gets a paint call, it can then do an invalidate.  This is only
    // done if the theme supports control tinting.
    if (doc && d->m_view && theme()->supportsControlTints() && renderer()) {
        doc->updateLayout(); // Ensure layout is up to date.
        IntRect visibleRect(enclosingIntRect(d->m_view->visibleContentRect()));
        GraphicsContext context((PlatformGraphicsContext*)0);
        context.setUpdatingControlTints(true);
        paint(&context, visibleRect);
    }
   
    // 5. Enable or disable secure keyboard entry
    if ((flag && !isSecureKeyboardEntry() && doc && doc->focusedNode() && doc->focusedNode()->hasTagName(inputTag) && 
            static_cast<HTMLInputElement*>(doc->focusedNode())->inputType() == HTMLInputElement::PASSWORD) ||
        (!flag && isSecureKeyboardEntry()))
            setSecureKeyboardEntry(flag);
}

void Frame::setWindowHasFocus(bool flag)
{
    if (d->m_windowHasFocus == flag)
        return;
    d->m_windowHasFocus = flag;
    
    if (Document *doc = document())
        doc->dispatchWindowEvent(flag ? focusEvent : blurEvent, false, false);
}

bool Frame::inViewSourceMode() const
{
    return d->m_inViewSourceMode;
}

void Frame::setInViewSourceMode(bool mode) const
{
    d->m_inViewSourceMode = mode;
}
  
UChar Frame::backslashAsCurrencySymbol() const
{
    Document *doc = document();
    if (!doc)
        return '\\';
    TextResourceDecoder *decoder = doc->decoder();
    if (!decoder)
        return '\\';

    return decoder->encoding().backslashAsCurrencySymbol();
}

bool Frame::markedTextUsesUnderlines() const
{
    return d->m_markedTextUsesUnderlines;
}

const Vector<MarkedTextUnderline>& Frame::markedTextUnderlines() const
{
    return d->m_markedTextUnderlines;
}

static bool isInShadowTree(Node* node)
{
    for (Node* n = node; n; n = n->parentNode())
        if (n->isShadowNode())
            return true;
    return false;
}

// Searches from the beginning of the document if nothing is selected.
bool Frame::findString(const String& target, bool forward, bool caseFlag, bool wrapFlag, bool startInSelection)
{
    if (target.isEmpty())
        return false;
    
    // Start from an edge of the selection, if there's a selection that's not in shadow content. Which edge
    // is used depends on whether we're searching forward or backward, and whether startInSelection is set.
    RefPtr<Range> searchRange(rangeOfContents(document()));
    Selection selection(selectionController()->selection());
    Node* selectionBaseNode = selection.base().node();
    
    // FIXME 3099526: We don't search in the shadow trees (e.g. text fields and textareas), though we'd like to
    // someday. If we don't explicitly skip them here, we'll miss hits in the regular content.
    bool selectionIsInMainContent = selectionBaseNode && !isInShadowTree(selectionBaseNode);

    if (selectionIsInMainContent) {
        if (forward)
            setStart(searchRange.get(), startInSelection ? selection.visibleStart() : selection.visibleEnd());
        else
            setEnd(searchRange.get(), startInSelection ? selection.visibleEnd() : selection.visibleStart());
    }
    RefPtr<Range> resultRange(findPlainText(searchRange.get(), target, forward, caseFlag));
    // If we started in the selection and the found range exactly matches the existing selection, find again.
    // Build a selection with the found range to remove collapsed whitespace.
    // Compare ranges instead of selection objects to ignore the way that the current selection was made.
    if (startInSelection && selectionIsInMainContent && *Selection(resultRange.get()).toRange() == *selection.toRange()) {
        searchRange = rangeOfContents(document());
        if (forward)
            setStart(searchRange.get(), selection.visibleEnd());
        else
            setEnd(searchRange.get(), selection.visibleStart());
        resultRange = findPlainText(searchRange.get(), target, forward, caseFlag);
    }
    
    int exception = 0;
    
    // If we didn't find anything and we're wrapping, search again in the entire document (this will
    // redundantly re-search the area already searched in some cases).
    if (resultRange->collapsed(exception) && wrapFlag) {
        searchRange = rangeOfContents(document());
        resultRange = findPlainText(searchRange.get(), target, forward, caseFlag);
        // We used to return false here if we ended up with the same range that we started with
        // (e.g., the selection was already the only instance of this text). But we decided that
        // this should be a success case instead, so we'll just fall through in that case.
    }

    if (resultRange->collapsed(exception))
        return false;

    selectionController()->setSelection(Selection(resultRange.get(), DOWNSTREAM));
    revealSelection();
    return true;
}

unsigned Frame::markAllMatchesForText(const String& target, bool caseFlag, unsigned limit)
{
    if (target.isEmpty())
        return 0;
    
    RefPtr<Range> searchRange(rangeOfContents(document()));
    
    int exception = 0;
    unsigned matchCount = 0;
    do {
        RefPtr<Range> resultRange(findPlainText(searchRange.get(), target, true, caseFlag));
        if (resultRange->collapsed(exception))
            break;
        
        // A non-collapsed result range can in some funky whitespace cases still not
        // advance the range's start position (4509328). Break to avoid infinite loop.
        VisiblePosition newStart = endVisiblePosition(resultRange.get(), DOWNSTREAM);
        if (newStart == startVisiblePosition(searchRange.get(), DOWNSTREAM))
            break;

        ++matchCount;
        
        document()->addMarker(resultRange.get(), DocumentMarker::TextMatch);        
        
        // Stop looking if we hit the specified limit. A limit of 0 means no limit.
        if (limit > 0 && matchCount >= limit)
            break;
        
        setStart(searchRange.get(), newStart);
    } while (true);
    
    // Do a "fake" paint in order to execute the code that computes the rendered rect for 
    // each text match.
    Document* doc = document();
    if (doc && d->m_view && renderer()) {
        doc->updateLayout(); // Ensure layout is up to date.
        IntRect visibleRect(enclosingIntRect(d->m_view->visibleContentRect()));
        GraphicsContext context((PlatformGraphicsContext*)0);
        context.setPaintingDisabled(true);
        paint(&context, visibleRect);
    }
    
    return matchCount;
}

bool Frame::markedTextMatchesAreHighlighted() const
{
    return d->m_highlightTextMatches;
}

void Frame::setMarkedTextMatchesAreHighlighted(bool flag)
{
    if (flag == d->m_highlightTextMatches)
        return;
    
    d->m_highlightTextMatches = flag;
    document()->repaintMarkers(DocumentMarker::TextMatch);
}

FrameTree* Frame::tree() const
{
    return &d->m_treeNode;
}

DOMWindow* Frame::domWindow() const
{
    if (!d->m_domWindow)
        d->m_domWindow = new DOMWindow(const_cast<Frame*>(this));

    return d->m_domWindow.get();
}

Page* Frame::page() const
{
    return d->m_page;
}

EventHandler* Frame::eventHandler() const
{
    return &d->m_eventHandler;
}

void Frame::pageDestroyed()
{
    d->m_page = 0;

    // This will stop any JS timers
    if (d->m_jscript && d->m_jscript->haveInterpreter())
        if (Window* w = Window::retrieveWindow(this))
            w->disconnectFrame();
}

void Frame::setStatusBarText(const String&)
{
}

void Frame::disconnectOwnerElement()
{
    if (d->m_ownerElement) {
        d->m_ownerElement->m_contentFrame = 0;
        if (d->m_page)
            d->m_page->decrementFrameCount();
    }
    d->m_ownerElement = 0;
}

String Frame::documentTypeString() const
{
    if (Document *doc = document())
        if (DocumentType *doctype = doc->realDocType())
            return doctype->toString();

    return String();
}

bool Frame::prohibitsScrolling() const
{
    return d->m_prohibitsScrolling;
}

void Frame::setProhibitsScrolling(bool prohibit)
{
    d->m_prohibitsScrolling = prohibit;
}

bool Frame::shouldClose()
{
    Chrome* chrome = page() ? page()->chrome() : 0;
    if (!chrome || !chrome->canRunBeforeUnloadConfirmPanel())
        return true;

    RefPtr<Document> doc = document();
    if (!doc)
        return true;
    HTMLElement* body = doc->body();
    if (!body)
        return true;

    RefPtr<BeforeUnloadEvent> beforeUnloadEvent = new BeforeUnloadEvent;
    beforeUnloadEvent->setTarget(doc);
    doc->handleWindowEvent(beforeUnloadEvent.get(), false);

    if (!beforeUnloadEvent->defaultPrevented() && doc)
        doc->defaultEventHandler(beforeUnloadEvent.get());
    if (beforeUnloadEvent->result().isNull())
        return true;

    String text = beforeUnloadEvent->result();
    text.replace('\\', backslashAsCurrencySymbol());

    return chrome->runBeforeUnloadConfirmPanel(text, this);
}

void Frame::scheduleClose()
{
    if (!shouldClose())
        return;

    Chrome* chrome = page() ? page()->chrome() : 0;
    if (chrome)
        chrome->closeWindowSoon();
}

FramePrivate::FramePrivate(Page* page, Frame* parent, Frame* thisFrame, HTMLFrameOwnerElement* ownerElement, FrameLoaderClient* frameLoaderClient)
    : m_page(page)
    , m_treeNode(thisFrame, parent)
    , m_ownerElement(ownerElement)
    , m_jscript(0)
    , m_settings(0)
    , m_zoomFactor(parent ? parent->d->m_zoomFactor : 100)
    , m_selectionController(thisFrame)
    , m_caretBlinkTimer(thisFrame, &Frame::caretBlinkTimerFired)
    , m_editor(thisFrame)
    , m_command(thisFrame)
    , m_eventHandler(thisFrame)
    , m_caretVisible(false)
    , m_caretPaint(true)
    , m_isActive(false)
    , m_lifeSupportTimer(thisFrame, &Frame::lifeSupportTimerFired)
    , m_loader(new FrameLoader(thisFrame, frameLoaderClient))
    , m_userStyleSheetLoader(0)
    , m_paintRestriction(PaintRestrictionNone)
    , m_markedTextUsesUnderlines(false)
    , m_highlightTextMatches(false)
    , m_windowHasFocus(false)
    , m_inViewSourceMode(false)
    , frameCount(0)
    , m_prohibitsScrolling(false)
{
}

FramePrivate::~FramePrivate()
{
    delete m_jscript;
    delete m_loader;
}

} // namespace WebCore
