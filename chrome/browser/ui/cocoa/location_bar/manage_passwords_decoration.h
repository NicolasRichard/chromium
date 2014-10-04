// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_COCOA_LOCATION_BAR_MANAGE_PASSWORDS_DECORATION_H_
#define CHROME_BROWSER_UI_COCOA_LOCATION_BAR_MANAGE_PASSWORDS_DECORATION_H_

#import <Cocoa/Cocoa.h>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/ui/cocoa/location_bar/image_decoration.h"
#include "chrome/browser/ui/passwords/manage_passwords_icon.h"

class CommandUpdater;
class LocationBarViewMac;
class ManagePasswordsDecoration;

// Cocoa implementation of ManagePasswordsIcon that delegates to
// ManagePasswordsDecoration.
class ManagePasswordsIconCocoa : public ManagePasswordsIcon {
 public:
  ManagePasswordsIconCocoa(ManagePasswordsDecoration* decoration);
  virtual ~ManagePasswordsIconCocoa();
  virtual void UpdateVisibleUI() override;

  int icon_id() { return icon_id_; }
  int tooltip_text_id() { return tooltip_text_id_; }

 private:
  ManagePasswordsDecoration* decoration_;  // weak, owns us
};

// Manage passwords icon on the right side of the field. This appears when
// password management is available on the current page.
class ManagePasswordsDecoration : public ImageDecoration {
 public:
  explicit ManagePasswordsDecoration(CommandUpdater* command_updater,
                                     LocationBarViewMac* location_bar);
  virtual ~ManagePasswordsDecoration();

  // Implement |LocationBarDecoration|
  virtual bool AcceptsMousePress() override;
  virtual bool OnMousePressed(NSRect frame, NSPoint location) override;
  virtual NSString* GetToolTip() override;
  virtual NSPoint GetBubblePointInFrame(NSRect frame) override;

  // Updates the decoration according to icon state changes.
  void UpdateVisibleUI();

  // Accessor for the platform-independent interface.
  ManagePasswordsIconCocoa* icon() { return icon_.get(); }

 private:
  // Triggers a redraw after a state change.
  void OnChange();

  // Updates child view states.
  void UpdateUIState();

  // Shows the manage passwords bubble.
  CommandUpdater* command_updater_;  // Weak, owned by Browser.

  // Displays all the decorations.
  LocationBarViewMac* location_bar_;  // Weak, owns us.

  // The platform-independent interface.
  scoped_ptr<ManagePasswordsIconCocoa> icon_;

  DISALLOW_COPY_AND_ASSIGN(ManagePasswordsDecoration);
};

#endif  // CHROME_BROWSER_UI_COCOA_LOCATION_BAR_MANAGE_PASSWORDS_DECORATION_H_
