/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef WebKitDOMTestCallback_h
#define WebKitDOMTestCallback_h

#if ENABLE(DATABASE)

#include "webkit/webkitdomdefines.h"
#include <glib-object.h>
#include <webkit/webkitdefines.h>
#include "webkit/WebKitDOMObject.h"


G_BEGIN_DECLS
#define WEBKIT_TYPE_DOM_TEST_CALLBACK            (webkit_dom_test_callback_get_type())
#define WEBKIT_DOM_TEST_CALLBACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WEBKIT_TYPE_DOM_TEST_CALLBACK, WebKitDOMTestCallback))
#define WEBKIT_DOM_TEST_CALLBACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  WEBKIT_TYPE_DOM_TEST_CALLBACK, WebKitDOMTestCallbackClass)
#define WEBKIT_DOM_IS_TEST_CALLBACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WEBKIT_TYPE_DOM_TEST_CALLBACK))
#define WEBKIT_DOM_IS_TEST_CALLBACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  WEBKIT_TYPE_DOM_TEST_CALLBACK))
#define WEBKIT_DOM_TEST_CALLBACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  WEBKIT_TYPE_DOM_TEST_CALLBACK, WebKitDOMTestCallbackClass))

struct _WebKitDOMTestCallback {
    WebKitDOMObject parent_instance;
};

struct _WebKitDOMTestCallbackClass {
    WebKitDOMObjectClass parent_class;
};

WEBKIT_API GType
webkit_dom_test_callback_get_type (void);

WEBKIT_API gboolean
webkit_dom_test_callback_callback_with_class1param(WebKitDOMTestCallback* self, WebKitDOMClass1*  class1param);

WEBKIT_API gboolean
webkit_dom_test_callback_callback_with_class2param(WebKitDOMTestCallback* self, WebKitDOMClass2*  class2param, gchar*  str_arg);

WEBKIT_API glong
webkit_dom_test_callback_callback_with_non_bool_return_type(WebKitDOMTestCallback* self, WebKitDOMClass3*  class3param);

G_END_DECLS

#endif /* ENABLE(DATABASE) */

#endif /* WebKitDOMTestCallback_h */
