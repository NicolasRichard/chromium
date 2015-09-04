// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/browser_window_command_handler.h"

#include "base/logging.h"
#import "base/mac/foundation_util.h"
#include "chrome/app/chrome_command_ids.h"
#import "chrome/browser/app_controller_mac.h"
#include "chrome/browser/fullscreen.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/bookmarks/bookmark_utils.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#import "chrome/browser/ui/cocoa/browser_window_controller_private.h"
#include "chrome/browser/ui/toolbar/encoding_menu_controller.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/web_contents.h"
#import "ui/base/cocoa/cocoa_base_utils.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/l10n_util_mac.h"

namespace {

// Update a toggle state for an item if modified. The item may be an NSMenuItem
// or NSButton. Called by -validateUserInterfaceItem:.
void UpdateToggleStateWithTag(NSInteger tag, id item, NSWindow* window) {
  if (![item respondsToSelector:@selector(state)] ||
      ![item respondsToSelector:@selector(setState:)])
    return;

  Browser* browser = chrome::FindBrowserWithWindow(window);
  DCHECK(browser);

  // On Windows this logic happens in bookmark_bar_view.cc. This simply updates
  // the menu item; it does not display the bookmark bar itself.
  if (tag == IDC_SHOW_BOOKMARK_BAR) {
    bool toggled = browser->window()->IsBookmarkBarVisible();
    NSInteger oldState = [item state];
    NSInteger newState = toggled ? NSOnState : NSOffState;
    if (oldState != newState)
      [item setState:newState];
    return;
  }

  // Update the checked/unchecked state of items in the encoding menu.
  // On Windows, this logic is part of |EncodingMenuModel| in
  // browser/ui/views/toolbar_view.h.
  EncodingMenuController encoding_controller;
  if (!encoding_controller.DoesCommandBelongToEncodingMenu(tag))
    return;

  Profile* profile = browser->profile();
  DCHECK(profile);
  content::WebContents* current_tab =
      browser->tab_strip_model()->GetActiveWebContents();
  if (!current_tab)
    return;

  const std::string encoding = current_tab->GetEncoding();

  bool toggled = encoding_controller.IsItemChecked(profile, encoding, tag);
  NSInteger oldState = [item state];
  NSInteger newState = toggled ? NSOnState : NSOffState;
  if (oldState != newState)
    [item setState:newState];
}

// Get the text for the "Enter/Exit Fullscreen/Presentation Mode" menu item.
// TODO(jackhou): Remove the dependency on BrowserWindowController(Private).
NSString* GetTitleForFullscreenMenuItem(Browser* browser) {
  NSWindow* ns_window = browser->window()->GetNativeWindow();
  if (BrowserWindowController* controller = [ns_window windowController]) {
    if (!chrome::mac::SupportsSystemFullscreen()) {
      return l10n_util::GetNSString([controller inPresentationMode]
                                        ? IDS_EXIT_PRESENTATION_MAC
                                        : IDS_ENTER_PRESENTATION_MAC);
    }

    return l10n_util::GetNSString([controller isInAppKitFullscreen]
                                      ? IDS_EXIT_FULLSCREEN_MAC
                                      : IDS_ENTER_FULLSCREEN_MAC);
  }

  return l10n_util::GetNSString(browser->window()->IsFullscreen()
                                    ? IDS_EXIT_FULLSCREEN_MAC
                                    : IDS_ENTER_FULLSCREEN_MAC);
}

// Identify the actual Browser to which the command should be dispatched. It
// might belong to a background window, yet another dispatcher gets it because
// it is the foreground window's dispatcher and thus in the responder chain.
// Some senders don't have this problem (for example, menus only operate on the
// foreground window), so this is only an issue for senders that are part of
// windows.
Browser* FindBrowserForSender(id sender, NSWindow* window) {
  NSWindow* targetWindow = window;
  if ([sender respondsToSelector:@selector(window)])
    targetWindow = [sender window];
  Browser* browser = chrome::FindBrowserWithWindow(targetWindow);
  DCHECK(browser);
  return browser;
}

}  // namespace

@implementation BrowserWindowCommandHandler

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)item
                           window:(NSWindow*)window {
  if ([item action] != @selector(commandDispatch:) &&
      [item action] != @selector(commandDispatchUsingKeyModifiers:)) {
    // NSWindow should only forward the above selectors here. All other
    // selectors must be handled by the default -[NSWindow
    // validateUserInterfaceItem:window:].
    NOTREACHED();
    // By default, interface items are enabled if the object in the responder
    // chain that implements the action does not implement
    // -validateUserInterfaceItem. Since we only care about -commandDispatch,
    // return YES for all other actions.
    return YES;
  }

  Browser* browser = chrome::FindBrowserWithWindow(window);
  DCHECK(browser);
  NSInteger tag = [item tag];
  if (!chrome::SupportsCommand(browser, tag))
    return NO;

  // Generate return value (enabled state).
  BOOL enable = chrome::IsCommandEnabled(browser, tag);
  switch (tag) {
    case IDC_CLOSE_TAB:
      // Disable "close tab" if the receiving window is not tabbed.
      // We simply check whether the item has a keyboard shortcut set here;
      // app_controller_mac.mm actually determines whether the item should
      // be enabled.
      if (NSMenuItem* menuItem = base::mac::ObjCCast<NSMenuItem>(item))
        enable &= !![[menuItem keyEquivalent] length];
      break;
    case IDC_FULLSCREEN: {
      if (NSMenuItem* menuItem = base::mac::ObjCCast<NSMenuItem>(item)) {
        if (chrome::mac::SupportsSystemFullscreen())
          [menuItem setTitle:GetTitleForFullscreenMenuItem(browser)];
        else
          [menuItem setHidden:YES];
      }
      break;
    }
    case IDC_PRESENTATION_MODE: {
      if (NSMenuItem* menuItem = base::mac::ObjCCast<NSMenuItem>(item)) {
        [menuItem setTitle:GetTitleForFullscreenMenuItem(browser)];

        if (chrome::mac::SupportsSystemFullscreen())
          [menuItem setAlternate:YES];
      }
      break;
    }
    case IDC_SHOW_SIGNIN: {
      Profile* original_profile = browser->profile()->GetOriginalProfile();
      [AppController updateSigninItem:item
                           shouldShow:enable
                       currentProfile:original_profile];
      break;
    }
    case IDC_BOOKMARK_PAGE: {
      // Extensions have the ability to hide the bookmark page menu item.
      // This only affects the bookmark page menu item under the main menu.
      // The bookmark page menu item under the wrench menu has its
      // visibility controlled by WrenchMenuModel.
      bool shouldHide =
          chrome::ShouldRemoveBookmarkThisPageUI(browser->profile());
      NSMenuItem* menuItem = base::mac::ObjCCast<NSMenuItem>(item);
      [menuItem setHidden:shouldHide];
      break;
    }
    case IDC_BOOKMARK_ALL_TABS: {
      // Extensions have the ability to hide the bookmark all tabs menu
      // item.  This only affects the bookmark page menu item under the main
      // menu.  The bookmark page menu item under the wrench menu has its
      // visibility controlled by WrenchMenuModel.
      bool shouldHide =
          chrome::ShouldRemoveBookmarkOpenPagesUI(browser->profile());
      NSMenuItem* menuItem = base::mac::ObjCCast<NSMenuItem>(item);
      [menuItem setHidden:shouldHide];
      break;
    }
    default:
      // Special handling for the contents of the Text Encoding submenu. On
      // Mac OS, instead of enabling/disabling the top-level menu item, we
      // enable/disable the submenu's contents (per Apple's HIG).
      EncodingMenuController encoding_controller;
      if (encoding_controller.DoesCommandBelongToEncodingMenu(tag))
        enable &= chrome::IsCommandEnabled(browser, IDC_ENCODING_MENU);
  }

  // If the item is toggleable, find its toggle state and
  // try to update it.  This is a little awkward, but the alternative is
  // to check after a commandDispatch, which seems worse.
  UpdateToggleStateWithTag(tag, item, window);

  return enable;
}

- (void)commandDispatch:(id)sender window:(NSWindow*)window {
  DCHECK(sender);

  // When system fullscreen is available, it supercedes presentation mode.
  int command = [sender tag];
  if (command == IDC_PRESENTATION_MODE &&
      chrome::mac::SupportsSystemFullscreen())
    command = IDC_FULLSCREEN;

  chrome::ExecuteCommand(FindBrowserForSender(sender, window), command);
}

- (void)commandDispatchUsingKeyModifiers:(id)sender window:(NSWindow*)window {
  DCHECK(sender);

  if (![sender isEnabled]) {
    // This code is reachable e.g. if the user mashes the back button, queuing
    // up a bunch of events before the button's enabled state is updated:
    // http://crbug.com/63254
    return;
  }

  NSInteger command = [sender tag];
  NSUInteger modifierFlags = [[NSApp currentEvent] modifierFlags];
  if ((command == IDC_RELOAD) &&
      (modifierFlags & (NSShiftKeyMask | NSControlKeyMask))) {
    command = IDC_RELOAD_IGNORING_CACHE;
    // Mask off Shift and Control so they don't affect the disposition below.
    modifierFlags &= ~(NSShiftKeyMask | NSControlKeyMask);
  }
  if (![[sender window] isMainWindow]) {
    // Remove the command key from the flags, it means "keep the window in
    // the background" in this case.
    modifierFlags &= ~NSCommandKeyMask;
  }
  chrome::ExecuteCommandWithDisposition(
      FindBrowserForSender(sender, window), command,
      ui::WindowOpenDispositionFromNSEventWithFlags([NSApp currentEvent],
                                                    modifierFlags));
}

@end
