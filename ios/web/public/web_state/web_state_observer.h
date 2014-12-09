// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_PUBLIC_WEB_STATE_WEB_STATE_OBSERVER_H_
#define IOS_WEB_PUBLIC_WEB_STATE_WEB_STATE_OBSERVER_H_

#include "base/macros.h"

namespace web {

struct LoadCommittedDetails;
class WebState;
class WebStateImpl;

// An observer API implemented by classes which are interested in various page
// load events from WebState.
class WebStateObserver {
 public:
  // Returns the web state associated with this observer.
  WebState* web_state() const { return web_state_; }

  // This method is invoked when a new non-pending navigation item is created.
  // This corresponds to one NavigationManager item being created
  // (in the case of new navigations) or renavigated to (for back/forward
  // navigations).
  virtual void NavigationItemCommitted(
      const LoadCommittedDetails& load_details) {};

  // Called when the current page is loaded.
  virtual void PageLoaded() {};

  // Called on URL hash change events.
  virtual void URLHashChanged() {};

  // Called on history state change events.
  virtual void HistoryStateChanged() {};

  // Invoked when the WebState is being destroyed. Gives subclasses a chance
  // to cleanup.
  virtual void WebStateDestroyed() {}

 protected:
  // Use this constructor when the object is tied to a single WebState for
  // its entire lifetime.
  explicit WebStateObserver(WebState* web_state);

  // Use this constructor when the object wants to observe a WebState for
  // part of its lifetime.  It can then call Observe() to start and stop
  // observing.
  WebStateObserver();

  virtual ~WebStateObserver();

  // Start observing a different WebState; used with the default constructor.
  void Observe(WebState* web_state);

 private:
  friend class WebStateImpl;

  // Stops observing the current web state.
  void ResetWebState();

  WebState* web_state_;

  DISALLOW_COPY_AND_ASSIGN(WebStateObserver);
};

}  // namespace web

#endif  // IOS_WEB_PUBLIC_WEB_STATE_WEB_STATE_OBSERVER_H_
