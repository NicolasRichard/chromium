// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIEW_MANAGER_PUBLIC_CPP_VIEW_MANAGER_H_
#define COMPONENTS_VIEW_MANAGER_PUBLIC_CPP_VIEW_MANAGER_H_

#include <string>

#include "components/view_manager/public/cpp/types.h"
#include "components/view_manager/public/interfaces/view_tree.mojom.h"
#include "third_party/mojo/src/mojo/public/cpp/bindings/interface_request.h"

namespace mojo {
class View;
class ViewManagerDelegate;

// Encapsulates a connection to the view manager service. A unique connection
// is made every time an app is embedded.
class ViewManager {
 public:
  virtual ~ViewManager() {}

  // The returned ViewManager instance owns itself, and is deleted when the
  // last root is destroyed or the connection to the service is broken.
  static ViewManager* Create(
      ViewManagerDelegate* delegate,
      InterfaceRequest<ViewTreeClient> request);

  // Returns the root of this connection.
  virtual View* GetRoot() = 0;

  // Returns a View known to this connection.
  virtual View* GetViewById(Id id) = 0;

  // Returns the focused view; null if focus is not yet known or another app is
  // focused.
  virtual View* GetFocusedView() = 0;

  // Creates and returns a new View (which is owned by the ViewManager). Views
  // are initially hidden, use SetVisible(true) to show.
  virtual View* CreateView() = 0;

  // Set view_tree.mojom for details.
  virtual void SetEmbedRoot() = 0;
};

}  // namespace mojo

#endif  // COMPONENTS_VIEW_MANAGER_PUBLIC_CPP_VIEW_MANAGER_H_
