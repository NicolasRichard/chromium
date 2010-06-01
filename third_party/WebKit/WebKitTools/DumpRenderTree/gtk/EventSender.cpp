/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Zan Dobersek <zandobersek@gmail.com>
 * Copyright (C) 2009 Holger Hans Peter Freyther
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

#include "config.h"
#include "EventSender.h"

#include "DumpRenderTree.h"

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSRetainPtr.h>
#include <JavaScriptCore/JSStringRef.h>
#include <webkit/webkitwebframe.h>
#include <webkit/webkitwebview.h>
#include <wtf/ASCIICType.h>
#include <wtf/Platform.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

// FIXME: Implement support for synthesizing drop events.

extern "C" {
    extern void webkit_web_frame_layout(WebKitWebFrame* frame);
}

static bool dragMode;
static int timeOffset = 0;

static int lastMousePositionX;
static int lastMousePositionY;
static int lastClickPositionX;
static int lastClickPositionY;
static int lastClickTimeOffset;
static int lastClickButton;
static int buttonCurrentlyDown;
static int clickCount;

struct DelayedMessage {
    GdkEvent event;
    gulong delay;
};

static DelayedMessage msgQueue[1024];

static unsigned endOfQueue;
static unsigned startOfQueue;

static const float zoomMultiplierRatio = 1.2f;

// Key event location code defined in DOM Level 3.
enum KeyLocationCode {
    DOM_KEY_LOCATION_STANDARD      = 0x00,
    DOM_KEY_LOCATION_LEFT          = 0x01,
    DOM_KEY_LOCATION_RIGHT         = 0x02,
    DOM_KEY_LOCATION_NUMPAD        = 0x03
};

static void sendOrQueueEvent(GdkEvent event);
static void dispatchEvent(GdkEvent event);
static guint getStateFlags();

static JSValueRef getDragModeCallback(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    return JSValueMakeBoolean(context, dragMode);
}

static bool setDragModeCallback(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    dragMode = JSValueToBoolean(context, value);
    return true;
}

static JSValueRef leapForwardCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount > 0) {
        msgQueue[endOfQueue].delay = JSValueToNumber(context, arguments[0], exception);
        timeOffset += msgQueue[endOfQueue].delay;
        ASSERT(!exception || !*exception);
    }

    return JSValueMakeUndefined(context);
}

bool prepareMouseButtonEvent(GdkEvent* event, int button)
{
    WebKitWebView* view = webkit_web_frame_get_web_view(mainFrame);
    if (!view)
        return false;

    memset(event, 0, sizeof(event));
    event->button.button = button;
    event->button.x = lastMousePositionX;
    event->button.y = lastMousePositionY;
    event->button.window = GTK_WIDGET(view)->window;
    event->button.device = gdk_device_get_core_pointer();
    event->button.state = getStateFlags();

    // Mouse up & down events dispatched via g_signal_emit_by_name must offset
    // their time value, so that WebKit can detect where sequences of mouse
    // clicks begin and end. This should not interfere with GDK or GTK+ event
    // processing, because the event is only seen by the widget.
    event.button.time = GDK_CURRENT_TIME + timeOffset;

    int xRoot, yRoot;
#if GTK_CHECK_VERSION(2, 17, 3)
    gdk_window_get_root_coords(GTK_WIDGET(view)->window, lastMousePositionX, lastMousePositionY, &xRoot, &yRoot);
#else
    getRootCoords(GTK_WIDGET(view), &xRoot, &yRoot);
#endif
    event->button.x_root = xRoot;
    event->button.y_root = yRoot;

    return true;
}

static JSValueRef contextClickCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    GdkEvent event;
    if (!prepareMouseButtonEvent(&event, 3))
        return JSValueMakeUndefined(context);

    event.type = GDK_BUTTON_PRESS;
    sendOrQueueEvent(event);
    event.type = GDK_BUTTON_RELEASE;
    sendOrQueueEvent(event);

    return JSValueMakeUndefined(context);
}

static void updateClickCount(int button)
{
    if (lastClickPositionX != lastMousePositionX
        || lastClickPositionY != lastMousePositionY
        || lastClickButton != button
        || timeOffset - lastClickTimeOffset >= 1)
        clickCount = 1;
    else
        clickCount++;
}

#if !GTK_CHECK_VERSION(2,17,3)
static void getRootCoords(GtkWidget* view, int* rootX, int* rootY)
{
    GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(view));
    int tmpX, tmpY;

    gtk_widget_translate_coordinates(view, window, lastMousePositionX, lastMousePositionY, &tmpX, &tmpY);

    gdk_window_get_origin(window->window, rootX, rootY);

    *rootX += tmpX;
    *rootY += tmpY;
}
#endif

static JSValueRef mouseDownCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    int button = 1;
    if (argumentCount == 1) {
        button = static_cast<int>(JSValueToNumber(context, arguments[0], exception)) + 1;
        g_return_val_if_fail((!exception || !*exception), JSValueMakeUndefined(context));
    }

    GdkEvent event;
    if (!prepareMouseButtonEvent(&event, button))
        return JSValueMakeUndefined(context);

    event.type = GDK_BUTTON_PRESS;
    sendOrQueueEvent(event);

    updateClickCount(event.button.button);
    if (clickCount == 2) {
        event.type = GDK_2BUTTON_PRESS;
        sendOrQueueEvent(event);
    }

    buttonCurrentlyDown = event.button.button;
    return JSValueMakeUndefined(context);
}

static guint getStateFlags()
{
    if (buttonCurrentlyDown == 1)
        return GDK_BUTTON1_MASK;
    if (buttonCurrentlyDown == 2)
        return GDK_BUTTON2_MASK;
    if (buttonCurrentlyDown == 3)
        return GDK_BUTTON3_MASK;
    return 0;
}

static JSValueRef mouseUpCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    int button = 1;
    if (argumentCount == 1) {
        button = static_cast<int>(JSValueToNumber(context, arguments[0], exception)) + 1;
        g_return_val_if_fail((!exception || !*exception), JSValueMakeUndefined(context));
    }

    GdkEvent event;
    if (!prepareMouseButtonEvent(&event, button))
        return JSValueMakeUndefined(context);

    lastClickPositionX = lastMousePositionX;
    lastClickPositionY = lastMousePositionY;
    lastClickButton = buttonCurrentlyDown;
    lastClickTimeOffset = timeOffset;
    buttonCurrentlyDown = 0;

    event.type = GDK_BUTTON_RELEASE;
    sendOrQueueEvent(event);
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseMoveToCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    WebKitWebView* view = webkit_web_frame_get_web_view(mainFrame);
    if (!view)
        return JSValueMakeUndefined(context);

    if (argumentCount < 2)
        return JSValueMakeUndefined(context);

    lastMousePositionX = (int)JSValueToNumber(context, arguments[0], exception);
    g_return_val_if_fail((!exception || !*exception), JSValueMakeUndefined(context));
    lastMousePositionY = (int)JSValueToNumber(context, arguments[1], exception);
    g_return_val_if_fail((!exception || !*exception), JSValueMakeUndefined(context));

    GdkEvent event;
    memset(&event, 0, sizeof(event));
    event.type = GDK_MOTION_NOTIFY;
    event.motion.x = lastMousePositionX;
    event.motion.y = lastMousePositionY;

    event.motion.time = GDK_CURRENT_TIME;
    event.motion.window = GTK_WIDGET(view)->window;
    event.motion.device = gdk_device_get_core_pointer();
    event.motion.state = getStateFlags();

    int xRoot, yRoot;
#if GTK_CHECK_VERSION(2,17,3)
    gdk_window_get_root_coords(GTK_WIDGET(view)->window, lastMousePositionX, lastMousePositionY, &xRoot, &yRoot);
#else
    getRootCoords(GTK_WIDGET(view), &xRoot, &yRoot);
#endif
    event.motion.x_root = xRoot;
    event.motion.y_root = yRoot;

    sendOrQueueEvent(event);
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseWheelToCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    WebKitWebView* view = webkit_web_frame_get_web_view(mainFrame);
    if (!view)
        return JSValueMakeUndefined(context);

    if (argumentCount < 2)
        return JSValueMakeUndefined(context);

    int horizontal = (int)JSValueToNumber(context, arguments[0], exception);
    g_return_val_if_fail((!exception || !*exception), JSValueMakeUndefined(context));
    int vertical = (int)JSValueToNumber(context, arguments[1], exception);
    g_return_val_if_fail((!exception || !*exception), JSValueMakeUndefined(context));

    // GTK+ doesn't support multiple direction scrolls in the same event!
    g_return_val_if_fail((!vertical || !horizontal), JSValueMakeUndefined(context));

    GdkEvent event;
    event.type = GDK_SCROLL;
    event.scroll.x = lastMousePositionX;
    event.scroll.y = lastMousePositionY;
    event.scroll.time = GDK_CURRENT_TIME;
    event.scroll.window = GTK_WIDGET(view)->window;

    if (horizontal < 0)
        event.scroll.direction = GDK_SCROLL_LEFT;
    else if (horizontal > 0)
        event.scroll.direction = GDK_SCROLL_RIGHT;
    else if (vertical < 0)
        event.scroll.direction = GDK_SCROLL_UP;
    else if (vertical > 0)
        event.scroll.direction = GDK_SCROLL_DOWN;
    else
        g_assert_not_reached();

    sendOrQueueEvent(event);
    return JSValueMakeUndefined(context);
}

static JSValueRef beginDragWithFilesCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount < 1)
        return JSValueMakeUndefined(context);

    // FIXME: Implement this completely once WebCore has complete drag and drop support
    return JSValueMakeUndefined(context);
}

static void sendOrQueueEvent(GdkEvent event)
{
    // Mouse move events are queued if the previous event was queued or if a
    // delay was set up by leapForward().
    if (endOfQueue != startOfQueue || msgQueue[endOfQueue].delay) {
        msgQueue[endOfQueue++].event = event;
        replaySavedEvents();
        return;
    }

    dispatchEvent(event);
}

static void dispatchEvent(GdkEvent event)
{
    webkit_web_frame_layout(mainFrame);
    WebKitWebView* view = webkit_web_frame_get_web_view(mainFrame);
    if (!view)
        return;

    gboolean returnValue;
    if (event.type == GDK_BUTTON_PRESS || event.type == GDK_2BUTTON_PRESS)
        g_signal_emit_by_name(view, "button_press_event", &event, &returnValue);
    else if (event.type == GDK_BUTTON_RELEASE)
        g_signal_emit_by_name(view, "button_release_event", &event, &returnValue);
    else if (event.type == GDK_MOTION_NOTIFY)
        g_signal_emit_by_name(view, "motion_notify_event", &event, &returnValue);
    else if (event.type == GDK_SCROLL)
        gtk_main_do_event(&event);
}

void replaySavedEvents()
{
    // FIXME: Eventually we may need to have more sophisticated logic to
    // track drag-and-drop operations.

    // First send all the events that are ready to be sent
    while (startOfQueue < endOfQueue) {
        if (msgQueue[startOfQueue].delay) {
            g_usleep(msgQueue[startOfQueue].delay * 1000);
            msgQueue[startOfQueue].delay = 0;
        }

        dispatchEvent(msgQueue[startOfQueue++].event);
    }

    startOfQueue = 0;
    endOfQueue = 0;
}

static JSValueRef keyDownCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount < 1)
        return JSValueMakeUndefined(context);

    static const JSStringRef lengthProperty = JSStringCreateWithUTF8CString("length");

    webkit_web_frame_layout(mainFrame);

    // handle modifier keys.
    int state = 0;
    if (argumentCount > 1) {
        JSObjectRef modifiersArray = JSValueToObject(context, arguments[1], exception);
        if (modifiersArray) {
            for (int i = 0; i < JSValueToNumber(context, JSObjectGetProperty(context, modifiersArray, lengthProperty, 0), 0); ++i) {
                JSValueRef value = JSObjectGetPropertyAtIndex(context, modifiersArray, i, 0);
                JSStringRef string = JSValueToStringCopy(context, value, 0);
                if (JSStringIsEqualToUTF8CString(string, "ctrlKey"))
                    state |= GDK_CONTROL_MASK;
                else if (JSStringIsEqualToUTF8CString(string, "shiftKey"))
                    state |= GDK_SHIFT_MASK;
                else if (JSStringIsEqualToUTF8CString(string, "altKey"))
                    state |= GDK_MOD1_MASK;

                JSStringRelease(string);
            }
        }
    }

    // handle location argument.
    int location = DOM_KEY_LOCATION_STANDARD;
    if (argumentCount > 2)
        location = (int)JSValueToNumber(context, arguments[2], exception);

    JSStringRef character = JSValueToStringCopy(context, arguments[0], exception);
    g_return_val_if_fail((!exception || !*exception), JSValueMakeUndefined(context));
    int gdkKeySym = GDK_VoidSymbol;
    if (location == DOM_KEY_LOCATION_NUMPAD) {
        if (JSStringIsEqualToUTF8CString(character, "leftArrow"))
            gdkKeySym = GDK_KP_Left;
        else if (JSStringIsEqualToUTF8CString(character, "rightArrow"))
            gdkKeySym = GDK_KP_Right;
        else if (JSStringIsEqualToUTF8CString(character, "upArrow"))
            gdkKeySym = GDK_KP_Up;
        else if (JSStringIsEqualToUTF8CString(character, "downArrow"))
            gdkKeySym = GDK_KP_Down;
        else if (JSStringIsEqualToUTF8CString(character, "pageUp"))
            gdkKeySym = GDK_KP_Page_Up;
        else if (JSStringIsEqualToUTF8CString(character, "pageDown"))
            gdkKeySym = GDK_KP_Page_Down;
        else if (JSStringIsEqualToUTF8CString(character, "home"))
            gdkKeySym = GDK_KP_Home;
        else if (JSStringIsEqualToUTF8CString(character, "end"))
            gdkKeySym = GDK_KP_End;
        else
            // Assume we only get arrow/pgUp/pgDn/home/end keys with
            // location=NUMPAD for now.
            g_assert_not_reached();
    } else {
        if (JSStringIsEqualToUTF8CString(character, "leftArrow"))
            gdkKeySym = GDK_Left;
        else if (JSStringIsEqualToUTF8CString(character, "rightArrow"))
            gdkKeySym = GDK_Right;
        else if (JSStringIsEqualToUTF8CString(character, "upArrow"))
            gdkKeySym = GDK_Up;
        else if (JSStringIsEqualToUTF8CString(character, "downArrow"))
            gdkKeySym = GDK_Down;
        else if (JSStringIsEqualToUTF8CString(character, "pageUp"))
            gdkKeySym = GDK_Page_Up;
        else if (JSStringIsEqualToUTF8CString(character, "pageDown"))
            gdkKeySym = GDK_Page_Down;
        else if (JSStringIsEqualToUTF8CString(character, "home"))
            gdkKeySym = GDK_Home;
        else if (JSStringIsEqualToUTF8CString(character, "end"))
            gdkKeySym = GDK_End;
        else if (JSStringIsEqualToUTF8CString(character, "delete"))
            gdkKeySym = GDK_BackSpace;
        else if (JSStringIsEqualToUTF8CString(character, "F1"))
            gdkKeySym = GDK_F1;
        else if (JSStringIsEqualToUTF8CString(character, "F2"))
            gdkKeySym = GDK_F2;
        else if (JSStringIsEqualToUTF8CString(character, "F3"))
            gdkKeySym = GDK_F3;
        else if (JSStringIsEqualToUTF8CString(character, "F4"))
            gdkKeySym = GDK_F4;
        else if (JSStringIsEqualToUTF8CString(character, "F5"))
            gdkKeySym = GDK_F5;
        else if (JSStringIsEqualToUTF8CString(character, "F6"))
            gdkKeySym = GDK_F6;
        else if (JSStringIsEqualToUTF8CString(character, "F7"))
            gdkKeySym = GDK_F7;
        else if (JSStringIsEqualToUTF8CString(character, "F8"))
            gdkKeySym = GDK_F8;
        else if (JSStringIsEqualToUTF8CString(character, "F9"))
            gdkKeySym = GDK_F9;
        else if (JSStringIsEqualToUTF8CString(character, "F10"))
            gdkKeySym = GDK_F10;
        else if (JSStringIsEqualToUTF8CString(character, "F11"))
            gdkKeySym = GDK_F11;
        else if (JSStringIsEqualToUTF8CString(character, "F12"))
            gdkKeySym = GDK_F12;
        else {
            int charCode = JSStringGetCharactersPtr(character)[0];
            if (charCode == '\n' || charCode == '\r')
                gdkKeySym = GDK_Return;
            else if (charCode == '\t')
                gdkKeySym = GDK_Tab;
            else if (charCode == '\x8')
                gdkKeySym = GDK_BackSpace;
            else {
                gdkKeySym = gdk_unicode_to_keyval(charCode);
                if (WTF::isASCIIUpper(charCode))
                    state |= GDK_SHIFT_MASK;
            }
        }
    }
    JSStringRelease(character);

    WebKitWebView* view = webkit_web_frame_get_web_view(mainFrame);
    if (!view)
        return JSValueMakeUndefined(context);

    // create and send the event
    GdkEvent event;
    memset(&event, 0, sizeof(event));
    event.key.keyval = gdkKeySym;
    event.key.state = state;
    event.key.window = GTK_WIDGET(view)->window;

    // When synthesizing an event, an invalid hardware_keycode value
    // can cause it to be badly processed by Gtk+.
    GdkKeymapKey* keys;
    gint n_keys;
    if (gdk_keymap_get_entries_for_keyval(gdk_keymap_get_default(), gdkKeySym, &keys, &n_keys)) {
        event.key.hardware_keycode = keys[0].keycode;
        g_free(keys);
    }

    gboolean return_val;
    event.key.type = GDK_KEY_PRESS;

    g_signal_emit_by_name(view, "key-press-event", &event.key, &return_val);

    event.key.type = GDK_KEY_RELEASE;
    g_signal_emit_by_name(view, "key-release-event", &event.key, &return_val);

    return JSValueMakeUndefined(context);
}

static void zoomIn(gboolean fullContentsZoom)
{
    WebKitWebView* view = webkit_web_frame_get_web_view(mainFrame);
    if (!view)
        return;

    webkit_web_view_set_full_content_zoom(view, fullContentsZoom);
    gfloat currentZoom = webkit_web_view_get_zoom_level(view);
    webkit_web_view_set_zoom_level(view, currentZoom * zoomMultiplierRatio);
}

static void zoomOut(gboolean fullContentsZoom)
{
    WebKitWebView* view = webkit_web_frame_get_web_view(mainFrame);
    if (!view)
        return;

    webkit_web_view_set_full_content_zoom(view, fullContentsZoom);
    gfloat currentZoom = webkit_web_view_get_zoom_level(view);
    webkit_web_view_set_zoom_level(view, currentZoom / zoomMultiplierRatio);
}

static JSValueRef textZoomInCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    zoomIn(FALSE);
    return JSValueMakeUndefined(context);
}

static JSValueRef textZoomOutCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    zoomOut(FALSE);
    return JSValueMakeUndefined(context);
}

static JSValueRef zoomPageInCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    zoomIn(TRUE);
    return JSValueMakeUndefined(context);
}

static JSValueRef zoomPageOutCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    zoomOut(TRUE);
    return JSValueMakeUndefined(context);
}

static JSStaticFunction staticFunctions[] = {
    { "mouseWheelTo", mouseWheelToCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "contextClick", contextClickCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "mouseDown", mouseDownCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "mouseUp", mouseUpCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "mouseMoveTo", mouseMoveToCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "beginDragWithFiles", beginDragWithFilesCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "leapForward", leapForwardCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "keyDown", keyDownCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "textZoomIn", textZoomInCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "textZoomOut", textZoomOutCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "zoomPageIn", zoomPageInCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "zoomPageOut", zoomPageOutCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { 0, 0, 0 }
};

static JSStaticValue staticValues[] = {
    { "dragMode", getDragModeCallback, setDragModeCallback, kJSPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

static JSClassRef getClass(JSContextRef context)
{
    static JSClassRef eventSenderClass = 0;

    if (!eventSenderClass) {
        JSClassDefinition classDefinition = {
                0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        classDefinition.staticFunctions = staticFunctions;
        classDefinition.staticValues = staticValues;

        eventSenderClass = JSClassCreate(&classDefinition);
    }

    return eventSenderClass;
}

JSObjectRef makeEventSender(JSContextRef context, bool isTopFrame)
{
    if (isTopFrame) {
        dragMode = true;

        // Fly forward in time one second when the main frame loads. This will
        // ensure that when a test begins clicking in the same location as
        // a previous test, those clicks won't be interpreted as continuations
        // of the previous test's click sequences.
        timeOffset += 1000;

        lastMousePositionX = lastMousePositionY = 0;
        lastClickPositionX = lastClickPositionY = 0;
        lastClickTimeOffset = 0;
        lastClickButton = 0;
        buttonCurrentlyDown = 0;
        clickCount = 0;

        endOfQueue = 0;
        startOfQueue = 0;
    }

    return JSObjectMake(context, getClass(context), 0);
}
