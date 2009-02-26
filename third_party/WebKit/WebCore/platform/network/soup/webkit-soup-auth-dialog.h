/*
 * Copyright (C) 2009 Igalia S.L., Author: Xan Lopez <xlopez@igalia.com>
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
 */

#include <gtk/gtk.h>

#ifndef WEBKIT_SOUP_AUTH_DIALOG_H
#define WEBKIT_SOUP_AUTH_DIALOG_H 1

G_BEGIN_DECLS

#define WEBKIT_TYPE_SOUP_AUTH_DIALOG            (webkit_soup_auth_dialog_get_type ())
#define WEBKIT_SOUP_AUTH_DIALOG(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), WEBKIT_TYPE_SOUP_AUTH_DIALOG, WebKitSoupAuthDialog))
#define WEBKIT_SOUP_AUTH_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), WEBKIT_TYPE_SOUP_AUTH_DIALOG, WebKitSoupAuthDialog))
#define WEBKIT_IS_SOUP_AUTH_DIALOG(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), WEBKIT_TYPE_SOUP_AUTH_DIALOG))
#define WEBKIT_IS_SOUP_AUTH_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEBKIT_TYPE_SOUP_AUTH_DIALOG))
#define WEBKIT_SOUP_AUTH_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), WEBKIT_TYPE_SOUP_AUTH_DIALOG, WebKitSoupAuthDialog))

typedef struct {
    GObject parent_instance;
} WebKitSoupAuthDialog;

typedef struct {
    GObjectClass parent_class;

    GtkWidget* (*current_toplevel) (WebKitSoupAuthDialog* feature, SoupMessage* message);
} WebKitSoupAuthDialogClass;

GType webkit_soup_auth_dialog_get_type (void);

G_END_DECLS

#endif /* WEBKIT_SOUP_AUTH_DIALOG_H */
