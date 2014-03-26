/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2008 Apple Inc. All rights reserved.
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef UIEvent_h
#define UIEvent_h

#include "core/events/Event.h"
#include "core/events/EventDispatchMediator.h"
#include "core/frame/DOMWindow.h"

namespace WebCore {

typedef DOMWindow AbstractView;

struct UIEventInit : public EventInit {
    UIEventInit();

    RefPtrWillBeMember<AbstractView> view;
    int detail;
};

class UIEvent : public Event {
public:
    static PassRefPtr<UIEvent> create()
    {
        return adoptRef(new UIEvent);
    }
    static PassRefPtr<UIEvent> create(const AtomicString& type, bool canBubble, bool cancelable, PassRefPtrWillBeRawPtr<AbstractView> view, int detail)
    {
        return adoptRef(new UIEvent(type, canBubble, cancelable, view, detail));
    }
    static PassRefPtr<UIEvent> create(const AtomicString& type, const UIEventInit& initializer)
    {
        return adoptRef(new UIEvent(type, initializer));
    }
    virtual ~UIEvent();

    void initUIEvent(const AtomicString& type, bool canBubble, bool cancelable, PassRefPtrWillBeRawPtr<AbstractView>, int detail);

    AbstractView* view() const { return m_view.get(); }
    AbstractView* view(bool& isNull) const { isNull = !m_view; return m_view.get(); }
    int detail() const { return m_detail; }

    virtual const AtomicString& interfaceName() const OVERRIDE;
    virtual bool isUIEvent() const OVERRIDE FINAL;

    virtual int keyCode() const;
    virtual int charCode() const;

    virtual int layerX();
    virtual int layerY();

    virtual int pageX() const;
    virtual int pageY() const;

    virtual int which() const;

    virtual void trace(Visitor*) OVERRIDE;

protected:
    UIEvent();
    UIEvent(const AtomicString& type, bool canBubble, bool cancelable, PassRefPtrWillBeRawPtr<AbstractView>, int detail);
    UIEvent(const AtomicString&, const UIEventInit&);

private:
    RefPtrWillBeMember<AbstractView> m_view;
    int m_detail;
};

} // namespace WebCore

#endif // UIEvent_h
