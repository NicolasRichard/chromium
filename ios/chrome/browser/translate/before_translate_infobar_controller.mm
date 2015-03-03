// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/translate/before_translate_infobar_controller.h"

#import <UIKit/UIKit.h>

#include "base/logging.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "components/translate/core/browser/translate_infobar_delegate.h"
#include "grit/components_strings.h"
#include "ios/chrome/browser/translate/translate_infobar_tags.h"
#include "ios/public/provider/chrome/browser/chrome_browser_provider.h"
#import "ios/public/provider/chrome/browser/ui/infobar_view_delegate.h"
#import "ios/public/provider/chrome/browser/ui/infobar_view_protocol.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/image/image.h"

namespace {

CGFloat kNavigationBarHeight = 44;
CGFloat kUIPickerHeight = 216;
CGFloat kUIPickerFontSize = 26;
NSTimeInterval kPickerAnimationDurationInSeconds = 0.2;

}  // namespace

// The class is a data source and delegate to the UIPickerView that contains the
// language list.
@interface LanguagePickerController
    : UIViewController<UIPickerViewDataSource, UIPickerViewDelegate> {
  __weak translate::TranslateInfoBarDelegate* _translateInfoBarDelegate;
  NSInteger _initialRow;   // Displayed in bold font.
  NSInteger _disabledRow;  // Grayed out.
}
@end

@implementation LanguagePickerController

- (instancetype)initWithDelegate:(translate::TranslateInfoBarDelegate*)
                                     translateInfoBarDelegate
                      initialRow:(NSInteger)initialRow
                     disabledRow:(NSInteger)disabledRow {
  if ((self = [super init])) {
    _translateInfoBarDelegate = translateInfoBarDelegate;
    _initialRow = initialRow;
    _disabledRow = disabledRow;
  }
  return self;
}

#pragma mark -
#pragma mark UIPickerViewDataSource

- (NSInteger)pickerView:(UIPickerView*)pickerView
    numberOfRowsInComponent:(NSInteger)component {
  NSUInteger numRows = _translateInfoBarDelegate->num_languages();
  return numRows;
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView*)pickerView {
  return 1;
}

#pragma mark -
#pragma mark UIPickerViewDelegate

- (UIView*)pickerView:(UIPickerView*)pickerView
           viewForRow:(NSInteger)row
         forComponent:(NSInteger)component
          reusingView:(UIView*)view {
  DCHECK_EQ(0, component);
  UILabel* label = [view isKindOfClass:[UILabel class]]
                       ? (UILabel*)view
                       : [[[UILabel alloc] init] autorelease];
  [label setText:base::SysUTF16ToNSString(
                     _translateInfoBarDelegate->language_name_at(row))];
  [label setTextAlignment:NSTextAlignmentCenter];
  UIFont* font = [UIFont systemFontOfSize:kUIPickerFontSize];
  BOOL enabled = YES;
  if (row == _initialRow)
    font = [UIFont boldSystemFontOfSize:kUIPickerFontSize];
  else if (row == _disabledRow)
    enabled = NO;
  [label setFont:font];
  [label setEnabled:enabled];
  return label;
}

@end

@interface BeforeTranslateInfoBarController ()

// Action for any of the user defined buttons.
- (void)infoBarButtonDidPress:(id)sender;
// Action for any of the user defined links.
- (void)infobarLinkDidPress:(NSNumber*)tag;
// Action for the language selection "Done" button.
- (void)languageSelectionDone;
// Dismisses the language selection view.
- (void)dismissLanguageSelectionView;

@end

@implementation BeforeTranslateInfoBarController {
  __weak translate::TranslateInfoBarDelegate* _translateInfoBarDelegate;
  // A fullscreen view that catches all touch events and contains a UIPickerView
  // and a UINavigationBar.
  base::scoped_nsobject<UIView> _languageSelectionView;
  // Stores whether the user is currently choosing in the UIPickerView the
  // original language, or the target language.
  NSUInteger _languageSelectionType;
  // The language picker.
  base::scoped_nsobject<UIPickerView> _languagePicker;
  // Navigation bar associated with the picker with "Done" and "Cancel" buttons.
  base::scoped_nsobject<UINavigationBar> _navigationBar;
  // The controller of the languagePicker. Needs to be an ivar because
  // |_languagePicker| does not retain it.
  base::scoped_nsobject<LanguagePickerController> _languagePickerController;
}

#pragma mark -
#pragma mark InfoBarControllerProtocol

- (void)layoutForDelegate:(infobars::InfoBarDelegate*)delegate
                    frame:(CGRect)frame {
  _translateInfoBarDelegate = delegate->AsTranslateInfoBarDelegate();
  DCHECK(!infoBarView_);
  infoBarView_.reset([ios::GetChromeBrowserProvider()->CreateInfoBarView()
      initWithFrame:frame
           delegate:delegate_]);
  // Icon
  gfx::Image icon = _translateInfoBarDelegate->GetIcon();
  if (!icon.IsEmpty())
    [infoBarView_ addLeftIcon:icon.ToUIImage()];

  // Main text.
  [self updateInfobarLabel];

  // Close button.
  [infoBarView_ addCloseButtonWithTag:TranslateInfoBarIOSTag::BEFORE_DENY
                               target:self
                               action:@selector(infoBarButtonDidPress:)];
  // Other buttons.
  NSString* buttonAccept = l10n_util::GetNSString(IDS_TRANSLATE_INFOBAR_ACCEPT);
  NSString* buttonDeny = l10n_util::GetNSString(IDS_TRANSLATE_INFOBAR_DENY);
  [infoBarView_ addButton1:buttonAccept
                      tag1:TranslateInfoBarIOSTag::BEFORE_ACCEPT
                   button2:buttonDeny
                      tag2:TranslateInfoBarIOSTag::BEFORE_DENY
                    target:self
                    action:@selector(infoBarButtonDidPress:)];
}

- (void)updateInfobarLabel {
  NSString* originalLanguage =
      base::SysUTF16ToNSString(_translateInfoBarDelegate->language_name_at(
          _translateInfoBarDelegate->original_language_index()));
  NSString* targetLanguage =
      base::SysUTF16ToNSString(_translateInfoBarDelegate->language_name_at(
          _translateInfoBarDelegate->target_language_index()));
  base::string16 originalLanguageWithLink =
      base::SysNSStringToUTF16([[infoBarView_ class]
          stringAsLink:originalLanguage
                   tag:TranslateInfoBarIOSTag::BEFORE_SOURCE_LANGUAGE]);
  base::string16 targetLanguageWithLink =
      base::SysNSStringToUTF16([[infoBarView_ class]
          stringAsLink:targetLanguage
                   tag:TranslateInfoBarIOSTag::BEFORE_TARGET_LANGUAGE]);
  NSString* label =
      l10n_util::GetNSStringF(IDS_TRANSLATE_INFOBAR_BEFORE_MESSAGE_IOS,
                              originalLanguageWithLink, targetLanguageWithLink);
  [infoBarView_ addLabel:label
                  target:self
                  action:@selector(infobarLinkDidPress:)];
}

- (void)languageSelectionDone {
  size_t selectedRow = [_languagePicker selectedRowInComponent:0];
  if (_languageSelectionType ==
          TranslateInfoBarIOSTag::BEFORE_SOURCE_LANGUAGE &&
      selectedRow != _translateInfoBarDelegate->target_language_index()) {
    _translateInfoBarDelegate->UpdateOriginalLanguageIndex(selectedRow);
  }
  if (_languageSelectionType ==
          TranslateInfoBarIOSTag::BEFORE_TARGET_LANGUAGE &&
      selectedRow != _translateInfoBarDelegate->original_language_index()) {
    _translateInfoBarDelegate->UpdateTargetLanguageIndex(selectedRow);
  }
  [self updateInfobarLabel];
  [self dismissLanguageSelectionView];
}

- (void)dismissLanguageSelectionView {
  DCHECK_EQ(_languagePicker == nil, _navigationBar == nil);
  if (_languagePicker == nil)
    return;
  // Sets the picker's delegate and data source to nil, because the
  // |_languagePickerController| may be destroyed before the picker is hidden,
  // and even though the interactions with the picker are disabled, it might
  // still be turning and requesting data from the data source or calling the
  // delegate.
  [_languagePicker setDataSource:nil];
  [_languagePicker setDelegate:nil];
  _languagePickerController.reset();
  // Animate the picker away.
  CGRect languagePickerFrame = [_languagePicker frame];
  CGRect navigationBarFrame = [_navigationBar frame];
  const CGFloat animationHeight =
      languagePickerFrame.size.height + navigationBarFrame.size.height;
  languagePickerFrame.origin.y += animationHeight;
  navigationBarFrame.origin.y += animationHeight;
  auto blockLanguagePicker(_languagePicker);
  auto blockNavigationBar(_navigationBar);
  _languagePicker.reset();
  _navigationBar.reset();
  void (^animations)(void) = ^{
    blockLanguagePicker.get().frame = languagePickerFrame;
    blockNavigationBar.get().frame = navigationBarFrame;
  };
  auto blockSelectionView(_languageSelectionView);
  _languageSelectionView.reset();
  void (^completion)(BOOL finished) = ^(BOOL finished) {
    [blockSelectionView removeFromSuperview];
  };
  [UIView animateWithDuration:kPickerAnimationDurationInSeconds
                   animations:animations
                   completion:completion];
}

#pragma mark - Handling of User Events

- (void)infoBarButtonDidPress:(id)sender {
  // This press might have occurred after the user has already pressed a button,
  // in which case the view has been detached from the delegate and this press
  // should be ignored.
  if (!delegate_) {
    return;
  }
  if ([sender isKindOfClass:[UIButton class]]) {
    NSUInteger buttonId = static_cast<UIButton*>(sender).tag;
    DCHECK(buttonId == TranslateInfoBarIOSTag::BEFORE_ACCEPT ||
           buttonId == TranslateInfoBarIOSTag::BEFORE_DENY);
    delegate_->InfoBarButtonDidPress(buttonId);
  }
}

- (void)infobarLinkDidPress:(NSNumber*)tag {
  DCHECK([tag isKindOfClass:[NSNumber class]]);
  _languageSelectionType = [tag unsignedIntegerValue];
  DCHECK(_languageSelectionType ==
             TranslateInfoBarIOSTag::BEFORE_SOURCE_LANGUAGE ||
         _languageSelectionType ==
             TranslateInfoBarIOSTag::BEFORE_TARGET_LANGUAGE);
  if (_languagePicker != nil)
    return;  // The UIPickerView is already up.

  // Creates and adds the view containing the UIPickerView and the
  // UINavigationBar.
  UIView* parentView =
      [[UIApplication sharedApplication] keyWindow].rootViewController.view;
  // Convert the parent frame to handle device rotation.
  CGRect parentFrame =
      CGRectApplyAffineTransform([parentView frame], [parentView transform]);
  const CGFloat totalPickerHeight = kUIPickerHeight + kNavigationBarHeight;
  CGRect languageSelectionViewFrame =
      CGRectMake(0, parentFrame.size.height - totalPickerHeight,
                 parentFrame.size.width, totalPickerHeight);
  _languageSelectionView.reset(
      [[UIView alloc] initWithFrame:languageSelectionViewFrame]);
  [_languageSelectionView
      setAutoresizingMask:UIViewAutoresizingFlexibleWidth |
                          UIViewAutoresizingFlexibleTopMargin];
  [parentView addSubview:_languageSelectionView];

  // Creates the navigation bar and its buttons.
  CGRect finalPickerFrame = CGRectMake(
      0, [_languageSelectionView frame].size.height - kUIPickerHeight,
      [_languageSelectionView frame].size.width, kUIPickerHeight);
  CGRect finalNavigationBarFrame = CGRectMake(
      0, 0, [_languageSelectionView frame].size.width, kNavigationBarHeight);
  // The language picker animates from the bottom of the screen.
  CGRect initialPickerFrame = finalPickerFrame;
  initialPickerFrame.origin.y += totalPickerHeight;
  CGRect initialNavigationBarFrame = finalNavigationBarFrame;
  initialNavigationBarFrame.origin.y += totalPickerHeight;

  _navigationBar.reset(
      [[UINavigationBar alloc] initWithFrame:initialNavigationBarFrame]);
  const UIViewAutoresizing resizingMask =
      UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleTopMargin;
  [_navigationBar setAutoresizingMask:resizingMask];
  base::scoped_nsobject<UIBarButtonItem> doneButton([[UIBarButtonItem alloc]
      initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                           target:self
                           action:@selector(languageSelectionDone)]);
  base::scoped_nsobject<UIBarButtonItem> cancelButton([[UIBarButtonItem alloc]
      initWithBarButtonSystemItem:UIBarButtonSystemItemCancel
                           target:self
                           action:@selector(dismissLanguageSelectionView)]);
  base::scoped_nsobject<UINavigationItem> item(
      [[UINavigationItem alloc] initWithTitle:nil]);
  [item setRightBarButtonItem:doneButton];
  [item setLeftBarButtonItem:cancelButton];
  [item setHidesBackButton:YES];
  [_navigationBar pushNavigationItem:item animated:NO];

  // Creates the PickerView and its controller.
  NSInteger selectedRow;
  NSInteger disabledRow;
  if (_languageSelectionType ==
      TranslateInfoBarIOSTag::BEFORE_SOURCE_LANGUAGE) {
    selectedRow = _translateInfoBarDelegate->original_language_index();
    disabledRow = _translateInfoBarDelegate->target_language_index();
  } else {
    selectedRow = _translateInfoBarDelegate->target_language_index();
    disabledRow = _translateInfoBarDelegate->original_language_index();
  }
  _languagePickerController.reset([[LanguagePickerController alloc]
      initWithDelegate:_translateInfoBarDelegate
            initialRow:selectedRow
           disabledRow:disabledRow]);
  _languagePicker.reset(
      [[UIPickerView alloc] initWithFrame:initialPickerFrame]);
  [_languagePicker setAutoresizingMask:resizingMask];
  [_languagePicker setShowsSelectionIndicator:YES];
  [_languagePicker setDataSource:_languagePickerController];
  [_languagePicker setDelegate:_languagePickerController];
  [_languagePicker setShowsSelectionIndicator:YES];
  [_languagePicker setBackgroundColor:[infoBarView_ backgroundColor]];
  [_languagePicker selectRow:selectedRow inComponent:0 animated:NO];

  auto blockLanguagePicker(_languagePicker);
  auto blockNavigationBar(_navigationBar);
  [UIView animateWithDuration:kPickerAnimationDurationInSeconds
                   animations:^{
                     blockLanguagePicker.get().frame = finalPickerFrame;
                     blockNavigationBar.get().frame = finalNavigationBarFrame;
                   }];

  // Add the subviews.
  [_languageSelectionView addSubview:_languagePicker];
  [_languageSelectionView addSubview:_navigationBar];
}

- (void)removeView {
  [super removeView];
  [self dismissLanguageSelectionView];
}

- (void)detachView {
  [super detachView];
  [self dismissLanguageSelectionView];
}

@end
