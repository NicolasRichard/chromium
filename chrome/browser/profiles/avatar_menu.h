// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PROFILES_AVATAR_MENU_H_
#define CHROME_BROWSER_PROFILES_AVATAR_MENU_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/scoped_observer.h"
#include "base/strings/string16.h"
#include "chrome/browser/profiles/profile_info_cache_observer.h"
#include "chrome/browser/profiles/profile_metrics.h"
#include "chrome/browser/ui/host_desktop.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/gfx/image/image.h"

#if defined(ENABLE_SUPERVISED_USERS)
#include "chrome/browser/supervised_user/supervised_user_service_observer.h"
#endif

class AvatarMenuActions;
class AvatarMenuObserver;
class Browser;
class Profile;
class ProfileInfoInterface;
class ProfileList;
class SupervisedUserService;

// This class represents the menu-like interface used to select profiles,
// such as the bubble that appears when the avatar icon is clicked in the
// browser window frame. This class will notify its observer when the backend
// data changes, and the view for this model should forward actions
// back to it in response to user events.
class AvatarMenu :
#if defined(ENABLE_SUPERVISED_USERS)
    public SupervisedUserServiceObserver,
#endif
    public ProfileInfoCacheObserver {
 public:
  // Represents an item in the menu.
  struct Item {
    Item(size_t menu_index, size_t profile_index, const gfx::Image& icon);
    ~Item();

    // The icon to be displayed next to the item.
    gfx::Image icon;

    // Whether or not the current browser is using this profile.
    bool active;

    // The name of this profile.
    base::string16 name;

    // A string representing the sync state of the profile.
    base::string16 sync_state;

    // Whether or not the current profile is signed in. If true, |sync_state| is
    // expected to be the email of the signed in user.
    bool signed_in;

    // Whether or not the current profile requires sign-in before use.
    bool signin_required;

    // Whether or not the current profile is a legacy supervised user profile
    // (see SupervisedUserService).
    bool legacy_supervised;

    // Whether or not the profile is associated with a child account
    // (see SupervisedUserService).
    bool child_account;

    // The index in the menu of this profile, used by views to refer to
    // profiles.
    size_t menu_index;

    // The index in the |profile_cache| for this profile.
    size_t profile_index;

    // The path of this profile.
    base::FilePath profile_path;
  };

  // Constructor. |observer| can be NULL. |browser| can be NULL and a new one
  // will be created if an action requires it.
  AvatarMenu(ProfileInfoInterface* profile_cache,
             AvatarMenuObserver* observer,
             Browser* browser);
  ~AvatarMenu() override;

  // True if avatar menu should be displayed.
  static bool ShouldShowAvatarMenu();

  // Sets |image| to the avatar corresponding to the profile at |profile_path|
  // and sets |is_rectangle| to true unless |image| is a built-in profile
  // avatar. For built-in profile avatars, returns the non-high res version.
  static void GetImageForMenuButton(const base::FilePath& profile_path,
                                    gfx::Image* image,
                                    bool* is_rectangle);

  // Compare items by name.
  static bool CompareItems(const Item* item1, const Item* item2);

  // Opens a Browser with the specified profile in response to the user
  // selecting an item. If |always_create| is true then a new window is created
  // even if a window for that profile already exists.
  void SwitchToProfile(size_t index,
                       bool always_create,
                       ProfileMetrics::ProfileOpen metric);

  // Creates a new profile.
  void AddNewProfile(ProfileMetrics::ProfileAdd type);

  // Opens the profile settings in response to clicking the edit button next to
  // an item.
  void EditProfile(size_t index);

  // Rebuilds the menu from the cache.
  void RebuildMenu();

  // Gets the number of profiles.
  size_t GetNumberOfItems() const;

  // Gets the Item at the specified index.
  const Item& GetItemAt(size_t index) const;

  // Returns the index of the active profile.
  size_t GetActiveProfileIndex();

  // Returns information about a supervised user which will be displayed in the
  // avatar menu. If the profile does not belong to a supervised user, an empty
  // string will be returned.
  base::string16 GetSupervisedUserInformation() const;

  // Returns the icon for the supervised user which will be displayed in the
  // avatar menu.
  const gfx::Image& GetSupervisedUserIcon() const;

  // This menu is also used for the always-present Mac system menubar. If the
  // last active browser changes, the menu will need to reference that browser.
  void ActiveBrowserChanged(Browser* browser);

  // Returns true if the add profile link should be shown.
  bool ShouldShowAddNewProfileLink() const;

  // Returns true if the edit profile link should be shown.
  bool ShouldShowEditProfileLink() const;

 private:
  // ProfileInfoCacheObserver:
  void OnProfileAdded(const base::FilePath& profile_path) override;
  void OnProfileWasRemoved(const base::FilePath& profile_path,
      const base::string16& profile_name) override;
  void OnProfileNameChanged(const base::FilePath& profile_path,
      const base::string16& old_profile_name) override;
  void OnProfileUserNameChanged(const base::FilePath& profile_path) override;
  void OnProfileAvatarChanged(const base::FilePath& profile_path) override;
  void OnProfileHighResAvatarLoaded(
      const base::FilePath& profile_path) override;
  void OnProfileSigninRequiredChanged(
      const base::FilePath& profile_path) override;
  void OnProfileIsOmittedChanged(const base::FilePath& profile_path) override;

#if defined(ENABLE_SUPERVISED_USERS)
  // SupervisedUserServiceObserver:
  void OnCustodianInfoChanged() override;
#endif

  // Rebuilds the menu and notifies any observers that an update occured.
  void Update();

  // The model that provides the list of menu items.
  scoped_ptr<ProfileList> profile_list_;

  // The controller for avatar menu actions.
  scoped_ptr<AvatarMenuActions> menu_actions_;

#if defined(ENABLE_SUPERVISED_USERS)
  // Observes changes to a supervised user's custodian info.
  ScopedObserver<SupervisedUserService, SupervisedUserServiceObserver>
      supervised_user_observer_;
#endif

  // The cache that provides the profile information. Weak.
  ProfileInfoInterface* profile_info_;

  // The observer of this model, which is notified of changes. Weak.
  AvatarMenuObserver* observer_;

  // Browser in which this avatar menu resides. Weak.
  Browser* browser_;

  DISALLOW_COPY_AND_ASSIGN(AvatarMenu);
};

#endif  // CHROME_BROWSER_PROFILES_AVATAR_MENU_H_
