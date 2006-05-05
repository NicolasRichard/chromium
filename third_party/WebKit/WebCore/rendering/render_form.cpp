/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.
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
#include "render_form.h"

#include "BrowserExtension.h"
#include "EventNames.h"
#include "FrameView.h"
#include "HTMLFormElement.h"
#include "HTMLInputElement.h"
#include "HTMLOptGroupElement.h"
#include "HTMLOptionElement.h"
#include "HTMLSelectElement.h"
#include "HTMLTextAreaElement.h"
#include "KWQComboBox.h"
#include "KWQFileButton.h"
#include "KWQLineEdit.h"
#include "KWQSlider.h"
#include "KWQTextEdit.h"
#include "PlatformMouseEvent.h"
#include "dom2_eventsimpl.h"

using namespace std;

namespace WebCore {

using namespace EventNames;
using namespace HTMLNames;

RenderFormElement::RenderFormElement(HTMLGenericFormElement *element)
    : RenderWidget(element)
{
    setInline(true);
}

RenderFormElement::~RenderFormElement()
{
}

short RenderFormElement::baselinePosition( bool f, bool isRootLineBox ) const
{
    return marginTop() + widget()->baselinePosition(m_height);
}

void RenderFormElement::setStyle(RenderStyle* s)
{
    if (canHaveIntrinsicMargins())
        addIntrinsicMarginsIfAllowed(s);

    RenderWidget::setStyle(s);

    // Do not paint a background or border for Aqua form elements
    setShouldPaintBackgroundOrBorder(false);

    m_widget->setFont(style()->font());
}

void RenderFormElement::updateFromElement()
{
    m_widget->setEnabled(!static_cast<HTMLGenericFormElement*>(node())->disabled());
}

void RenderFormElement::layout()
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    // minimum height
    m_height = 0;

    calcWidth();
    calcHeight();
    
    setNeedsLayout(false);
}

void RenderFormElement::clicked(Widget*)
{
    RenderArena* arena = ref();
    PlatformMouseEvent event; // gets "current event"
    if (node())
        static_cast<EventTargetNode*>(node())->dispatchMouseEvent(event, clickEvent, event.clickCount());
    deref(arena);
}

HorizontalAlignment RenderFormElement::textAlignment() const
{
    switch (style()->textAlign()) {
        case LEFT:
        case KHTML_LEFT:
            return AlignLeft;
        case RIGHT:
        case KHTML_RIGHT:
            return AlignRight;
        case CENTER:
        case KHTML_CENTER:
            return AlignHCenter;
        case JUSTIFY:
            // Just fall into the auto code for justify.
        case TAAUTO:
            return style()->direction() == RTL ? AlignRight : AlignLeft;
    }
    ASSERT(false); // Should never be reached.
    return AlignLeft;
}


void RenderFormElement::addIntrinsicMarginsIfAllowed(RenderStyle* _style)
{
    // Cut out the intrinsic margins completely if we end up using mini controls.
    if (_style->fontSize() < 11)
        return;
    
    // FIXME: Using width/height alone and not also dealing with min-width/max-width is flawed.
    int m = intrinsicMargin();
    if (_style->width().isIntrinsicOrAuto()) {
        if (_style->marginLeft().quirk())
            _style->setMarginLeft(Length(m, Fixed));
        if (_style->marginRight().quirk())
            _style->setMarginRight(Length(m, Fixed));
    }

    if (_style->height().isAuto()) {
        if (_style->marginTop().quirk())
            _style->setMarginTop(Length(m, Fixed));
        if (_style->marginBottom().quirk())
            _style->setMarginBottom(Length(m, Fixed));
    }
}

// -------------------------------------------------------------------------------

RenderImageButton::RenderImageButton(HTMLInputElement *element)
    : RenderImage(element)
{
    // ### support DOMActivate event when clicked
}

// -------------------------------------------------------------------------------


// -----------------------------------------------------------------------------

RenderLineEdit::RenderLineEdit(HTMLInputElement *element)
    : RenderFormElement(element), m_updating(false)
{
    QLineEdit::Type type;
    switch (element->inputType()) {
        case HTMLInputElement::PASSWORD:
            type = QLineEdit::Password;
            break;
        case HTMLInputElement::SEARCH:
            type = QLineEdit::Search;
            break;
        default:
            type = QLineEdit::Normal;
    }
    QLineEdit* edit = new QLineEdit(type);
    if (type == QLineEdit::Search)
        edit->setLiveSearch(false);
    setWidget(edit);
}

void RenderLineEdit::selectionChanged(Widget*)
{
    // We only want to call onselect if there actually is a selection
    QLineEdit* w = static_cast<QLineEdit*>(m_widget);
    if (w->hasSelectedText())
        static_cast<HTMLGenericFormElement*>(node())->onSelect();
}

void RenderLineEdit::returnPressed(Widget*)
{
    // Emit onChange if necessary
    // Works but might not be enough, dirk said he had another solution at
    // hand (can't remember which) - David
    if (isTextField() && isEdited()) {
        static_cast<HTMLGenericFormElement*>(node())->onChange();
        setEdited(false);
    }

    if (HTMLFormElement* fe = static_cast<HTMLGenericFormElement*>(node())->form())
        fe->submitClick();
}

void RenderLineEdit::performSearch(Widget*)
{
    // Fire the "search" DOM event.
    static_cast<EventTargetNode*>(node())->dispatchHTMLEvent(searchEvent, true, false);
}

void RenderLineEdit::addSearchResult()
{
    if (widget())
        static_cast<QLineEdit*>(widget())->addSearchResult();
}

void RenderLineEdit::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

    // Let the widget tell us how big it wants to be.
    m_updating = true;
    int size = static_cast<HTMLInputElement*>(node())->size();
    IntSize s(static_cast<QLineEdit*>(widget())->sizeForCharacterWidth(size > 0 ? size : 20));
    m_updating = false;

    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());

    RenderFormElement::calcMinMaxWidth();
}

void RenderLineEdit::setStyle(RenderStyle *s)
{
    RenderFormElement::setStyle(s);

    QLineEdit* w = static_cast<QLineEdit*>(widget());
    w->setAlignment(textAlignment());
    w->setWritingDirection(style()->direction() == RTL ? RTL : LTR);
}

void RenderLineEdit::updateFromElement()
{
    HTMLInputElement* e = static_cast<HTMLInputElement*>(node());
    QLineEdit* w = static_cast<QLineEdit*>(widget());
    
    int ml = e->maxLength();
    if (ml <= 0 || ml > 1024)
        ml = 1024;
    if (w->maxLength() != ml)
        w->setMaxLength(ml);

    if (!e->valueMatchesRenderer()) {
        String widgetText = w->text();
        String newText = e->value();
        newText.replace(QChar('\\'), backslashAsCurrencySymbol());
        if (widgetText != newText) {
            int pos = w->cursorPosition();

            m_updating = true;
            w->setText(newText);
            m_updating = false;
            
            w->setEdited( false );

            w->setCursorPosition(pos);
        }
        e->setValueMatchesRenderer();
    }

    w->setReadOnly(e->isReadOnlyControl());
    
    // Handle updating the search attributes.
    w->setPlaceholderString(e->getAttribute(placeholderAttr).deprecatedString());
    if (w->type() == QLineEdit::Search) {
        w->setLiveSearch(!e->getAttribute(incrementalAttr).isNull());
        w->setAutoSaveName(e->getAttribute(autosaveAttr));
        w->setMaxResults(e->maxResults());
    }

    w->setColors(style()->backgroundColor(), style()->color());

    RenderFormElement::updateFromElement();
}

void RenderLineEdit::valueChanged(Widget*)
{
    if (m_updating) // Don't alter the value if we are in the middle of initing the control, since
        return;     // we are getting the value from the DOM and it's not user input.

    String newText = static_cast<QLineEdit*>(widget())->text();

    // A null string value is used to indicate that the form control has not altered the original
    // default value.  That means that we should never use the null string value when the user
    // empties a textfield, but should always force an empty textfield to use the empty string.
    if (newText.isNull())
        newText = "";

    newText.replace(backslashAsCurrencySymbol(), '\\');
    static_cast<HTMLInputElement*>(node())->setValueFromRenderer(newText);
}

int RenderLineEdit::selectionStart()
{
    QLineEdit *lineEdit = static_cast<QLineEdit *>(m_widget);
    int start = lineEdit->selectionStart();
    if (start == -1)
        start = lineEdit->cursorPosition();
    return start;
}

int RenderLineEdit::selectionEnd()
{
    QLineEdit *lineEdit = static_cast<QLineEdit *>(m_widget);
    int start = lineEdit->selectionStart();
    if (start == -1)
        return lineEdit->cursorPosition();
    return start + (int)lineEdit->selectedText().length();
}

void RenderLineEdit::setSelectionStart(int start)
{
    int realStart = max(start, 0);
    int length = max(selectionEnd() - realStart, 0);
    static_cast<QLineEdit *>(m_widget)->setSelection(realStart, length);
}

void RenderLineEdit::setSelectionEnd(int end)
{
    int start = selectionStart();
    int realEnd = max(end, 0);
    int length = realEnd - start;
    if (length < 0) {
        start = realEnd;
        length = 0;
    }
    static_cast<QLineEdit *>(m_widget)->setSelection(start, length);
}

void RenderLineEdit::select()
{
    static_cast<QLineEdit*>(m_widget)->selectAll();
}

bool RenderLineEdit::isEdited() const
{
    return static_cast<QLineEdit*>(m_widget)->edited();
}
void RenderLineEdit::setEdited(bool x)
{
    static_cast<QLineEdit*>(m_widget)->setEdited(x);
}

void RenderLineEdit::setSelectionRange(int start, int end)
{
    int realStart = max(start, 0);
    int length = max(end - realStart, 0);
    static_cast<QLineEdit *>(m_widget)->setSelection(realStart, length);
}

// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(HTMLGenericFormElement *element)
: RenderBlock(element)
{
}

RenderObject* RenderFieldset::layoutLegend(bool relayoutChildren)
{
    RenderObject* legend = findLegend();
    if (legend) {
        if (relayoutChildren)
            legend->setNeedsLayout(true);
        legend->layoutIfNeeded();

        int xPos = borderLeft() + paddingLeft() + legend->marginLeft();
        if (style()->direction() == RTL)
            xPos = m_width - paddingRight() - borderRight() - legend->width() - legend->marginRight();
        int b = borderTop();
        int h = legend->height();
        legend->setPos(xPos, max((b-h)/2, 0));
        m_height = max(b,h) + paddingTop();
    }
    return legend;
}

RenderObject* RenderFieldset::findLegend()
{
    for (RenderObject* legend = firstChild(); legend; legend = legend->nextSibling()) {
      if (!legend->isFloatingOrPositioned() && legend->element() &&
          legend->element()->hasTagName(legendTag))
        return legend;
    }
    return 0;
}

void RenderFieldset::paintBoxDecorations(PaintInfo& i, int _tx, int _ty)
{
    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    RenderObject* legend = findLegend();
    if (!legend)
        return RenderBlock::paintBoxDecorations(i, _tx, _ty);

    int yOff = (legend->yPos() > 0) ? 0 : (legend->height()-borderTop())/2;
    h -= yOff;
    _ty += yOff - borderTopExtra();

    int my = max(_ty, i.r.y());
    int end = min(i.r.bottom(),  _ty + h);
    int mh = end - my;

    paintBackground(i.p, style()->backgroundColor(), style()->backgroundLayers(), my, mh, _tx, _ty, w, h);

    if (style()->hasBorder())
        paintBorderMinusLegend(i.p, _tx, _ty, w, h, style(), legend->xPos(), legend->width());
}

void RenderFieldset::paintBorderMinusLegend(GraphicsContext* p, int _tx, int _ty, int w, int h,
                                            const RenderStyle* style, int lx, int lw)
{

    const Color& tc = style->borderTopColor();
    const Color& bc = style->borderBottomColor();

    EBorderStyle ts = style->borderTopStyle();
    EBorderStyle bs = style->borderBottomStyle();
    EBorderStyle ls = style->borderLeftStyle();
    EBorderStyle rs = style->borderRightStyle();

    bool render_t = ts > BHIDDEN;
    bool render_l = ls > BHIDDEN;
    bool render_r = rs > BHIDDEN;
    bool render_b = bs > BHIDDEN;

    if(render_t) {
        drawBorder(p, _tx, _ty, _tx + lx, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0), 0);
        drawBorder(p, _tx+lx+lw, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   0, (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));
    }

    if(render_b)
        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(), bs,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0),
                   (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));

    if(render_l)
    {
        const Color& lc = style->borderLeftColor();

        bool ignore_top =
            (tc == lc) &&
            (ls >= OUTSET) &&
            (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

        bool ignore_bottom =
            (bc == lc) &&
            (ls >= OUTSET) &&
            (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx, _ty, _tx + style->borderLeftWidth(), _ty + h, BSLeft, lc, style->color(), ls,
                   ignore_top?0:style->borderTopWidth(),
                   ignore_bottom?0:style->borderBottomWidth());
    }

    if(render_r)
    {
        const Color& rc = style->borderRightColor();

        bool ignore_top =
            (tc == rc) &&
            (rs >= DOTTED || rs == INSET) &&
            (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

        bool ignore_bottom =
            (bc == rc) &&
            (rs >= DOTTED || rs == INSET) &&
            (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx + w - style->borderRightWidth(), _ty, _tx + w, _ty + h, BSRight, rc, style->color(), rs,
                   ignore_top?0:style->borderTopWidth(),
                   ignore_bottom?0:style->borderBottomWidth());
    }
}

void RenderFieldset::setStyle(RenderStyle* _style)
{
    RenderBlock::setStyle(_style);

    // WinIE renders fieldsets with display:inline like they're inline-blocks.  For us,
    // an inline-block is just a block element with replaced set to true and inline set
    // to true.  Ensure that if we ended up being inline that we set our replaced flag
    // so that we're treated like an inline-block.
    if (isInline())
        setReplaced(true);
}    
    
// -------------------------------------------------------------------------

RenderFileButton::RenderFileButton(HTMLInputElement *element)
    : RenderFormElement(element)
{
    setWidget(new KWQFileButton(view()->frame()));
}

void RenderFileButton::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

    // Let the widget tell us how big it wants to be.
    int size = static_cast<HTMLInputElement*>(node())->size();
    IntSize s(static_cast<KWQFileButton *>(widget())->sizeForCharacterWidth(size > 0 ? size : 20));

    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());

    RenderFormElement::calcMinMaxWidth();
}

void RenderFileButton::updateFromElement()
{
    static_cast<KWQFileButton*>(widget())->setFilename(
        static_cast<HTMLInputElement*>(node())->value().deprecatedString());

    RenderFormElement::updateFromElement();
}

void RenderFileButton::returnPressed(Widget*)
{
    if (static_cast<HTMLInputElement*>(node())->form())
        static_cast<HTMLInputElement*>(node())->form()->prepareSubmit();
}

void RenderFileButton::valueChanged(Widget*)
{
    static_cast<HTMLInputElement*>(node())->setValueFromRenderer(static_cast<KWQFileButton*>(widget())->filename());
    static_cast<HTMLInputElement*>(node())->onChange();
}

void RenderFileButton::select()
{
}


void RenderFileButton::click(bool sendMouseEvents)
{
    static_cast<KWQFileButton *>(widget())->click(sendMouseEvents);
}


// -------------------------------------------------------------------------

RenderLabel::RenderLabel(HTMLGenericFormElement *element)
    : RenderFormElement(element)
{

}

// -------------------------------------------------------------------------

RenderLegend::RenderLegend(HTMLGenericFormElement *element)
    : RenderBlock(element)
{
}

// -------------------------------------------------------------------------

RenderSelect::RenderSelect(HTMLSelectElement *element)
    : RenderFormElement(element)
{
    m_ignoreSelectEvents = false;
    m_multiple = element->multiple();
    m_size = element->size();
    m_useListBox = (m_multiple || m_size > 1);
    m_selectionChanged = true;
    m_optionsChanged = true;

    if (m_useListBox)
        setWidget(createListBox());
    else
        setWidget(new QComboBox);
}


void RenderSelect::setWidgetWritingDirection()
{
    TextDirection d = style()->direction() == RTL ? RTL : LTR;
    if (m_useListBox)
        static_cast<QListBox *>(m_widget)->setWritingDirection(d);
    else
        static_cast<QComboBox *>(m_widget)->setWritingDirection(d);
}

void RenderSelect::setStyle(RenderStyle *s)
{
    RenderFormElement::setStyle(s);
    setWidgetWritingDirection();
}


void RenderSelect::updateFromElement()
{
    m_ignoreSelectEvents = true;

    // change widget type
    bool oldMultiple = m_multiple;
    unsigned oldSize = m_size;
    bool oldListbox = m_useListBox;

    m_multiple = static_cast<HTMLSelectElement*>(node())->multiple();
    m_size = static_cast<HTMLSelectElement*>(node())->size();
    m_useListBox = (m_multiple || m_size > 1);

    if (oldMultiple != m_multiple || oldSize != m_size) {
        if (m_useListBox != oldListbox) {
            // type of select has changed
            delete m_widget;

            if (m_useListBox)
                setWidget(createListBox());
            else
                setWidget(new QComboBox);
            setWidgetWritingDirection();
        }

        if (m_useListBox && oldMultiple != m_multiple) {
            static_cast<QListBox*>(m_widget)->setSelectionMode(m_multiple ? QListBox::Extended : QListBox::Single);
        }
        m_selectionChanged = true;
        m_optionsChanged = true;
    }

    // update contents listbox/combobox based on options in m_element
    if ( m_optionsChanged ) {
        if (static_cast<HTMLSelectElement*>(node())->m_recalcListItems)
            static_cast<HTMLSelectElement*>(node())->recalcListItems();
        DeprecatedArray<HTMLElement*> listItems = static_cast<HTMLSelectElement*>(node())->listItems();
        int listIndex;

        if (m_useListBox)
            static_cast<QListBox*>(m_widget)->clear();
        else
            static_cast<QComboBox*>(m_widget)->clear();

        bool groupEnabled = true;
        for (listIndex = 0; listIndex < int(listItems.size()); listIndex++) {
            if (listItems[listIndex]->hasTagName(optgroupTag)) {
                HTMLOptGroupElement *optgroupElement = static_cast<HTMLOptGroupElement*>(listItems[listIndex]);
                DeprecatedString label = optgroupElement->getAttribute(labelAttr).deprecatedString();
                label.replace(QChar('\\'), backslashAsCurrencySymbol());
                
                // In WinIE, an optgroup can't start or end with whitespace (other than the indent
                // we give it).  We match this behavior.
                label = label.stripWhiteSpace();
                // We want to collapse our whitespace too.  This will match other browsers.
                label = label.simplifyWhiteSpace();

                groupEnabled = optgroupElement->isEnabled();
                
                if (m_useListBox)
                    static_cast<QListBox*>(m_widget)->appendGroupLabel(label, groupEnabled);
                else
                    static_cast<QComboBox*>(m_widget)->appendGroupLabel(label);
            }
            else if (listItems[listIndex]->hasTagName(optionTag)) {
                HTMLOptionElement *optionElement = static_cast<HTMLOptionElement*>(listItems[listIndex]);
                DeprecatedString itemText;
                if (optionElement->hasAttribute(labelAttr))
                    itemText = optionElement->getAttribute(labelAttr).deprecatedString();
                else
                    itemText = optionElement->text().deprecatedString();
                
                itemText.replace(QChar('\\'), backslashAsCurrencySymbol());

                // In WinIE, leading and trailing whitespace is ignored in options. We match this behavior.
                itemText = itemText.stripWhiteSpace();
                // We want to collapse our whitespace too.  This will match other browsers.
                itemText = itemText.simplifyWhiteSpace();
                
                if (listItems[listIndex]->parentNode()->hasTagName(optgroupTag))
                    itemText.prepend("    ");

                if (m_useListBox)
                    static_cast<QListBox*>(m_widget)->appendItem(itemText, groupEnabled && optionElement->isEnabled());
                else
                    static_cast<QComboBox*>(m_widget)->appendItem(itemText, groupEnabled && optionElement->isEnabled());
            }
            else if (listItems[listIndex]->hasTagName(hrTag)) {
                if (!m_useListBox)
                    static_cast<QComboBox*>(m_widget)->appendSeparator();
            }
            else
                KHTMLAssert(false);
            m_selectionChanged = true;
        }
        if (m_useListBox)
            static_cast<QListBox*>(m_widget)->doneAppendingItems();
        setNeedsLayoutAndMinMaxRecalc();
        m_optionsChanged = false;
    }

    // update selection
    if (m_selectionChanged) {
        updateSelection();
    }

    m_ignoreSelectEvents = false;

    RenderFormElement::updateFromElement();
}


short RenderSelect::baselinePosition( bool f, bool isRootLineBox ) const
{
    if (m_useListBox) {
        // FIXME: Should get the hardcoded constant of 7 by calling a QListBox function,
        // as we do for other widget classes.
        return RenderWidget::baselinePosition( f, isRootLineBox ) - 7;
    }
    return RenderFormElement::baselinePosition( f, isRootLineBox );
}


void RenderSelect::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    if (m_optionsChanged)
        updateFromElement();

    // ### ugly HACK FIXME!!!
    setMinMaxKnown();
    layoutIfNeeded();
    setNeedsLayoutAndMinMaxRecalc();
    // ### end FIXME

    RenderFormElement::calcMinMaxWidth();
}

void RenderSelect::layout( )
{
    KHTMLAssert(needsLayout());
    KHTMLAssert(minMaxKnown());

    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    // calculate size
    if(m_useListBox) {
        QListBox* w = static_cast<QListBox*>(m_widget);


        int size = m_size;
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to min(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO min(number of elements, 10)
        // so I did that ;-)
        if(size < 1)
            size = min(static_cast<QListBox*>(m_widget)->count(), 10U);

        // Let the widget tell us how big it wants to be.
        IntSize s(w->sizeForNumberOfLines(size));
        setIntrinsicWidth( s.width() );
        setIntrinsicHeight( s.height() );
    }
    else {
        IntSize s(m_widget->sizeHint());
        setIntrinsicWidth( s.width() );
        setIntrinsicHeight( s.height() );
    }

    RenderFormElement::layout();

    // and now disable the widget in case there is no <option> given
    DeprecatedArray<HTMLElement*> listItems = static_cast<HTMLSelectElement*>(node())->listItems();

    bool foundOption = false;
    for (unsigned i = 0; i < listItems.size() && !foundOption; i++)
        foundOption = (listItems[i]->hasTagName(optionTag));

    m_widget->setEnabled(foundOption && ! static_cast<HTMLSelectElement*>(node())->disabled());
}

void RenderSelect::valueChanged(Widget*)
{
    if (m_ignoreSelectEvents)
        return;

    KHTMLAssert(!m_useListBox);

    int index = static_cast<QComboBox*>(m_widget)->currentItem();

    DeprecatedArray<HTMLElement*> listItems = static_cast<HTMLSelectElement*>(node())->listItems();
    if (index >= 0 && index < (int)listItems.size()) {
        bool found = listItems[index]->hasTagName(optionTag);
        if (!found) {
            // this one is not selectable,  we need to find an option element
            while ((unsigned) index < listItems.size()) {
                if (listItems[index]->hasTagName(optionTag)) {
                    found = true;
                    break;
                }
                ++index;
            }

            if (!found) {
                while (index >= 0) {
                    if (listItems[index]->hasTagName(optionTag)) {
                        found = true;
                        break;
                    }
                    --index;
                }
            }
        }

        if (found) {
            if ( index != static_cast<QComboBox*>(m_widget)->currentItem() )
                static_cast<QComboBox*>(m_widget)->setCurrentItem( index );

            for (unsigned i = 0; i < listItems.size(); ++i)
                if (listItems[i]->hasTagName(optionTag) && i != (unsigned int) index)
                    static_cast<HTMLOptionElement*>( listItems[i] )->m_selected = false;

            static_cast<HTMLOptionElement*>(listItems[index])->m_selected = true;
        }
    }

    static_cast<HTMLSelectElement*>(node())->onChange();
}


void RenderSelect::selectionChanged(Widget*)
{
    if (m_ignoreSelectEvents)
        return;

    // don't use listItems() here as we have to avoid recalculations - changing the
    // option list will make use update options not in the way the user expects them
    DeprecatedArray<HTMLElement*> listItems = static_cast<HTMLSelectElement*>(node())->m_listItems;
    int j = 0;
    for (unsigned i = 0; i < listItems.count(); i++) {
        // don't use setSelected() here because it will cause us to be called
        // again with updateSelection.
        if (listItems[i]->hasTagName(optionTag))
            static_cast<HTMLOptionElement*>( listItems[i] )
                ->m_selected = static_cast<QListBox*>( m_widget )->isSelected( j );
        if (listItems[i]->hasTagName(optionTag) || listItems[i]->hasTagName(optgroupTag))
            ++j;
    }
    static_cast<HTMLSelectElement*>(node())->onChange();
}


void RenderSelect::setOptionsChanged(bool _optionsChanged)
{
    m_optionsChanged = _optionsChanged;
}

QListBox* RenderSelect::createListBox()
{
    QListBox *lb = new QListBox();
    lb->setSelectionMode(m_multiple ? QListBox::Extended : QListBox::Single);
    m_ignoreSelectEvents = false;
    return lb;
}

void RenderSelect::updateSelection()
{
    DeprecatedArray<HTMLElement*> listItems = static_cast<HTMLSelectElement*>(node())->listItems();
    int i;
    if (m_useListBox) {
        // if multi-select, we select only the new selected index
        QListBox *listBox = static_cast<QListBox*>(m_widget);
        int j = 0;
        for (i = 0; i < int(listItems.size()); i++) {
            listBox->setSelected(j, listItems[i]->hasTagName(optionTag) &&
                                static_cast<HTMLOptionElement*>(listItems[i])->selected());
            if (listItems[i]->hasTagName(optionTag) || listItems[i]->hasTagName(optgroupTag))
                ++j;
            
        }
    }
    else {
        bool found = false;
        unsigned firstOption = listItems.size();
        i = listItems.size();
        while (i--)
            if (listItems[i]->hasTagName(optionTag)) {
                if (found)
                    static_cast<HTMLOptionElement*>(listItems[i])->m_selected = false;
                else if (static_cast<HTMLOptionElement*>(listItems[i])->selected()) {
                    static_cast<QComboBox*>( m_widget )->setCurrentItem(i);
                    found = true;
                }
                firstOption = i;
            }

        ASSERT(firstOption == listItems.size() || found);
    }

    m_selectionChanged = false;
}


// -------------------------------------------------------------------------


// -------------------------------------------------------------------------

RenderTextArea::RenderTextArea(HTMLTextAreaElement *element)
    : RenderFormElement(element), m_dirty(false), m_updating(false)
{
    QTextEdit *edit = new QTextEdit(view());

    if (element->wrap() != HTMLTextAreaElement::ta_NoWrap)
        edit->setWordWrap(QTextEdit::WidgetWidth);
    else
        edit->setWordWrap(QTextEdit::NoWrap);

    setWidget(edit);
}

void RenderTextArea::destroy()
{
    static_cast<HTMLTextAreaElement*>(node())->rendererWillBeDestroyed();
    RenderFormElement::destroy();
}

void RenderTextArea::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QTextEdit* w = static_cast<QTextEdit*>(m_widget);
    IntSize size(w->sizeWithColumnsAndRows(
        max(static_cast<HTMLTextAreaElement*>(node())->cols(), 1),
        max(static_cast<HTMLTextAreaElement*>(node())->rows(), 1)));

    setIntrinsicWidth( size.width() );
    setIntrinsicHeight( size.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderTextArea::setStyle(RenderStyle *s)
{
    RenderFormElement::setStyle(s);

    QTextEdit* w = static_cast<QTextEdit*>(m_widget);
    w->setAlignment(textAlignment());
    w->setLineHeight(RenderObject::lineHeight(true));

    w->setWritingDirection(style()->direction() == RTL ? RTL : LTR);

    ScrollBarMode scrollMode = ScrollBarAuto;
    switch (style()->overflow()) {
        case OAUTO:
        case OMARQUEE: // makes no sense, map to auto
        case OOVERLAY: // not implemented for text, map to auto
        case OVISIBLE:
            break;
        case OHIDDEN:
            scrollMode = ScrollBarAlwaysOff;
            break;
        case OSCROLL:
            scrollMode = ScrollBarAlwaysOn;
            break;
    }
    ScrollBarMode horizontalScrollMode = scrollMode;
    if (static_cast<HTMLTextAreaElement*>(node())->wrap() != HTMLTextAreaElement::ta_NoWrap)
        horizontalScrollMode = ScrollBarAlwaysOff;

    w->setScrollBarModes(horizontalScrollMode, scrollMode);
}

void RenderTextArea::setEdited(bool x)
{
    m_dirty = x;
}

void RenderTextArea::updateFromElement()
{
    HTMLTextAreaElement* e = static_cast<HTMLTextAreaElement*>(node());
    QTextEdit* w = static_cast<QTextEdit*>(m_widget);

    w->setReadOnly(e->isReadOnlyControl());
    w->setDisabled(e->disabled());

    String widgetText = text();
    String text = e->value();
    text.replace(QChar('\\'), backslashAsCurrencySymbol());
    if (widgetText != text) {
        int line, col;
        w->getCursorPosition( &line, &col );
        m_updating = true;
        w->setText(text);
        m_updating = false;
        w->setCursorPosition( line, col );
    }
    m_dirty = false;

    w->setColors(style()->backgroundColor(), style()->color());

    RenderFormElement::updateFromElement();
}

String RenderTextArea::text()
{
    String txt = static_cast<QTextEdit*>(m_widget)->text();
    return txt.replace(backslashAsCurrencySymbol(), QChar('\\'));
}

String RenderTextArea::textWithHardLineBreaks()
{
    String txt = static_cast<QTextEdit*>(m_widget)->textWithHardLineBreaks();
    return txt.replace(backslashAsCurrencySymbol(), QChar('\\'));
}

void RenderTextArea::valueChanged(Widget*)
{
    if (m_updating)
        return;
    static_cast<HTMLTextAreaElement*>(node())->invalidateValue();
    m_dirty = true;
}

int RenderTextArea::selectionStart()
{
    QTextEdit *textEdit = static_cast<QTextEdit *>(m_widget);
    return textEdit->selectionStart();
}

int RenderTextArea::selectionEnd()
{
    QTextEdit *textEdit = static_cast<QTextEdit *>(m_widget);
    return textEdit->selectionEnd();
}

void RenderTextArea::setSelectionStart(int start)
{
    QTextEdit *textEdit = static_cast<QTextEdit *>(m_widget);
    textEdit->setSelectionStart(start);
}

void RenderTextArea::setSelectionEnd(int end)
{
    QTextEdit *textEdit = static_cast<QTextEdit *>(m_widget);
    textEdit->setSelectionEnd(end);
}

void RenderTextArea::select()
{
    static_cast<QTextEdit *>(m_widget)->selectAll();
}

void RenderTextArea::setSelectionRange(int start, int end)
{
    QTextEdit *textEdit = static_cast<QTextEdit *>(m_widget);
    textEdit->setSelectionRange(start, end-start);
}

void RenderTextArea::selectionChanged(Widget*)
{
    QTextEdit* w = static_cast<QTextEdit*>(m_widget);

    // We only want to call onselect if there actually is a selection
    if (!w->hasSelectedText())
        return;
    
    static_cast<HTMLTextAreaElement*>(node())->onSelect();
}

// ---------------------------------------------------------------------------

RenderSlider::RenderSlider(HTMLInputElement* element)
    : RenderFormElement(element)
{
    setWidget(new QSlider);
}

void RenderSlider::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());
    
    // Let the widget tell us how big it wants to be.
    IntSize s(widget()->sizeHint());
    bool widthSet = !style()->width().isAuto();
    bool heightSet = !style()->height().isAuto();
    if (heightSet && !widthSet) {
        // Flip the intrinsic dimensions.
        int barLength = s.width();
        s = IntSize(s.height(), barLength);
    }
    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
    
    RenderFormElement::calcMinMaxWidth();
}

void RenderSlider::updateFromElement()
{
    String value = static_cast<HTMLInputElement*>(node())->value();
    const AtomicString& minStr = static_cast<HTMLInputElement*>(node())->getAttribute(minAttr);
    const AtomicString& maxStr = static_cast<HTMLInputElement*>(node())->getAttribute(maxAttr);
    const AtomicString& precision = static_cast<HTMLInputElement*>(node())->getAttribute(precisionAttr);
    
    double minVal = minStr.isNull() ? 0.0 : minStr.deprecatedString().toDouble();
    double maxVal = maxStr.isNull() ? 100.0 : maxStr.deprecatedString().toDouble();
    minVal = min(minVal, maxVal); // Make sure the range is sane.
    
    double val = value.isNull() ? (maxVal + minVal)/2.0 : value.deprecatedString().toDouble();
    val = max(minVal, min(val, maxVal)); // Make sure val is within min/max.
    
    // Force integer value if not float.
    if (!equalIgnoringCase(precision, "float"))
        val = (int)(val + 0.5);

    static_cast<HTMLInputElement*>(node())->setValue(String::number(val));

    QSlider* slider = (QSlider*)widget();
     
    slider->setMinValue(minVal);
    slider->setMaxValue(maxVal);
    slider->setValue(val);

    RenderFormElement::updateFromElement();
}

void RenderSlider::valueChanged(Widget*)
{
    QSlider* slider = static_cast<QSlider*>(widget());

    double val = slider->value();
    const AtomicString& precision = static_cast<HTMLInputElement*>(node())->getAttribute(precisionAttr);

    // Force integer value if not float.
    if (!equalIgnoringCase(precision, "float"))
        val = (int)(val + 0.5);

    static_cast<HTMLInputElement*>(node())->setValue(String::number(val));
    
    // Fire the "input" DOM event.
    static_cast<HTMLInputElement*>(node())->dispatchHTMLEvent(inputEvent, true, false);
}

}
