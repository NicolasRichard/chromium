/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2006 Apple Computer, Inc.
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

#ifndef RENDER_FORM_H
#define RENDER_FORM_H

#include "RenderWidget.h"
#include "GraphicsTypes.h"
#include "RenderBlock.h"
#include "RenderImage.h"

class QListBox;

namespace WebCore {

class DocLoader;
class HTMLFormElement;
class HTMLGenericFormElement;
class HTMLInputElement;
class HTMLSelectElement;
class HTMLTextAreaElement;

// -------------------------------------------------------------------------

class RenderFormElement : public RenderWidget
{
public:
    RenderFormElement(HTMLGenericFormElement* node);
    virtual ~RenderFormElement();

    virtual const char *renderName() const { return "RenderForm"; }

    virtual bool isFormElement() const { return true; }

    // Aqua form controls never have border/padding.
    int borderTop() const { return 0; }
    int borderBottom() const { return 0; }
    int borderLeft() const { return 0; }
    int borderRight() const { return 0; }
    int paddingTop() const { return 0; }
    int paddingBottom() const { return 0; }
    int paddingLeft() const { return 0; }
    int paddingRight() const { return 0; }

    // Aqua controls use intrinsic margin values in order to leave space for focus rings
    // and to keep controls from butting up against one another.  This intrinsic margin
    // is only applied if the Web page allows the control to size intrinsically.  If the
    // Web page specifies an explicit width for a control, then we go ahead and honor that
    // precise width.  Similarly, if a Web page uses a specific margin value, we will go ahead
    // and honor that value.  
    void addIntrinsicMarginsIfAllowed(RenderStyle* _style);
    virtual bool canHaveIntrinsicMargins() const { return false; }
    int intrinsicMargin() const { return 2; }

    virtual void setStyle(RenderStyle *);
    virtual void updateFromElement();

    virtual void layout();
    virtual short baselinePosition( bool, bool ) const;

private:
    virtual void clicked(Widget*);

protected:
    virtual bool isEditable() const { return false; }

    HorizontalAlignment textAlignment() const;
};

// -------------------------------------------------------------------------

class RenderImageButton : public RenderImage
{
public:
    RenderImageButton(HTMLInputElement*);

    virtual const char *renderName() const { return "RenderImageButton"; }
    virtual bool isImageButton() const { return true; }
};

// -------------------------------------------------------------------------

class RenderLineEdit : public RenderFormElement
{
public:
    RenderLineEdit(HTMLInputElement*);

    virtual void calcMinMaxWidth();
    int calcReplacedHeight() const { return intrinsicHeight(); }
    virtual bool canHaveIntrinsicMargins() const { return true; }

    virtual const char *renderName() const { return "RenderLineEdit"; }
    virtual void updateFromElement();
    virtual void setStyle(RenderStyle *);

    int selectionStart();
    int selectionEnd();
    void setSelectionStart(int);
    void setSelectionEnd(int);
    
    bool isEdited() const;
    void setEdited(bool);
    bool isTextField() const { return true; }
    void select();
    void setSelectionRange(int, int);

public:
    void addSearchResult();

private:
    virtual void returnPressed(Widget*);
    virtual void valueChanged(Widget*);
    virtual void selectionChanged(Widget*);
    virtual void performSearch(Widget*);

    virtual bool isEditable() const { return true; }

    bool m_updating;
};

// -------------------------------------------------------------------------

class RenderFieldset : public RenderBlock
{
public:
    RenderFieldset(HTMLGenericFormElement*);

    virtual const char* renderName() const { return "RenderFieldSet"; }

    virtual RenderObject* layoutLegend(bool relayoutChildren);

    virtual void setStyle(RenderStyle*);
    
private:
    virtual void paintBoxDecorations(PaintInfo&, int tx, int ty);
    void paintBorderMinusLegend(GraphicsContext*, int tx, int ty, int w, int h, const RenderStyle *, int lx, int lw);
    RenderObject* findLegend();
};

// -------------------------------------------------------------------------

class RenderFileButton : public RenderFormElement
{
public:
    RenderFileButton(HTMLInputElement*);

    virtual const char *renderName() const { return "RenderFileButton"; }
    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    void select();

    int calcReplacedHeight() const { return intrinsicHeight(); }

    void click(bool sendMouseEvents);

protected:
    virtual bool isEditable() const { return true; }

private:
    virtual void returnPressed(Widget*);
    virtual void valueChanged(Widget*);
};

// -------------------------------------------------------------------------

class RenderLabel : public RenderFormElement
{
public:
    RenderLabel(HTMLGenericFormElement *element);

    virtual const char *renderName() const { return "RenderLabel"; }
};


// -------------------------------------------------------------------------

class RenderLegend : public RenderBlock
{
public:
    RenderLegend(HTMLGenericFormElement *element);

    virtual const char *renderName() const { return "RenderLegend"; }
};

// -------------------------------------------------------------------------

class RenderSelect : public RenderFormElement
{
public:
    RenderSelect(HTMLSelectElement*);

    virtual const char *renderName() const { return "RenderSelect"; }

    short baselinePosition( bool f, bool b ) const;
    int calcReplacedHeight() const { if (!m_useListBox) return intrinsicHeight(); return RenderFormElement::calcReplacedHeight(); }
    virtual bool canHaveIntrinsicMargins() const { return true; }

    virtual void calcMinMaxWidth();
    virtual void layout();

    void setOptionsChanged(bool _optionsChanged);

    bool selectionChanged() { return m_selectionChanged; }
    void setSelectionChanged(bool _selectionChanged) { m_selectionChanged = _selectionChanged; }
    virtual void updateFromElement();
    virtual void setStyle(RenderStyle *);

    void updateSelection();

protected:
    QListBox *createListBox();
    void setWidgetWritingDirection();

    unsigned  m_size;
    bool m_multiple;
    bool m_useListBox;
    bool m_selectionChanged;
    bool m_ignoreSelectEvents;
    bool m_optionsChanged;

private:
    virtual void valueChanged(Widget*);
    virtual void selectionChanged(Widget*);
};

// -------------------------------------------------------------------------

class RenderTextArea : public RenderFormElement
{
public:
    RenderTextArea(HTMLTextAreaElement*);

    virtual void destroy();

    virtual const char *renderName() const { return "RenderTextArea"; }
    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    virtual void setStyle(RenderStyle *);

    bool isTextArea() const { return true; }
    bool isEdited() const { return m_dirty; }
    void setEdited (bool);
    
    String text();
    String textWithHardLineBreaks();

    int selectionStart();
    int selectionEnd();
    void setSelectionStart(int);
    void setSelectionEnd(int);
    
    void select();
    void setSelectionRange(int, int);
    
    virtual bool canHaveIntrinsicMargins() const { return true; }

private:
    virtual void valueChanged(Widget*);
    virtual void selectionChanged(Widget*);
    
protected:
    virtual bool isEditable() const { return true; }

    bool m_dirty;
    bool m_updating;
};

// -------------------------------------------------------------------------

class RenderSlider : public RenderFormElement
{
public:
    RenderSlider(HTMLInputElement*);
    
    virtual const char *renderName() const { return "RenderSlider"; }
    virtual bool canHaveIntrinsicMargins() const { return true; }
    virtual void calcMinMaxWidth();
    virtual void updateFromElement();

private:
    virtual void valueChanged(Widget*);
};

} //namespace

#endif
