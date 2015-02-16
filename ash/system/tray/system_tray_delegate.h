// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SYSTEM_TRAY_SYSTEM_TRAY_DELEGATE_H_
#define ASH_SYSTEM_TRAY_SYSTEM_TRAY_DELEGATE_H_

#include <string>
#include <vector>

#include "ash/ash_export.h"
#include "ash/system/user/login_status.h"
#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/i18n/time_formatting.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "ui/gfx/image/image_skia.h"

namespace base {
class TimeDelta;
class TimeTicks;
}

namespace ash {

class CustodianInfoTrayObserver;
class ShutdownPolicyObserver;

struct ASH_EXPORT NetworkIconInfo {
  NetworkIconInfo();
  ~NetworkIconInfo();

  bool highlight() const { return connected || connecting; }

  bool connecting;
  bool connected;
  bool tray_icon_visible;
  gfx::ImageSkia image;
  base::string16 name;
  base::string16 description;
  std::string service_path;
  bool is_cellular;
};

struct ASH_EXPORT BluetoothDeviceInfo {
  BluetoothDeviceInfo();
  ~BluetoothDeviceInfo();

  std::string address;
  base::string16 display_name;
  bool connected;
  bool connecting;
  bool paired;
};

using BluetoothDeviceList = std::vector<BluetoothDeviceInfo>;

struct ASH_EXPORT IMEPropertyInfo {
  IMEPropertyInfo();
  ~IMEPropertyInfo();

  bool selected;
  std::string key;
  base::string16 name;
};

using IMEPropertyInfoList = std::vector<IMEPropertyInfo>;

struct ASH_EXPORT IMEInfo {
  IMEInfo();
  ~IMEInfo();

  bool selected;
  bool third_party;
  std::string id;
  base::string16 name;
  base::string16 medium_name;
  base::string16 short_name;
};

struct ASH_EXPORT UpdateInfo {
  enum UpdateSeverity {
    UPDATE_NORMAL,
    UPDATE_LOW_GREEN,
    UPDATE_HIGH_ORANGE,
    UPDATE_SEVERE_RED,
  };

  UpdateInfo();
  ~UpdateInfo();

  UpdateSeverity severity;
  bool update_required;
  bool factory_reset_required;
};

using IMEInfoList = std::vector<IMEInfo>;

class NetworkingConfigDelegate;
class VolumeControlDelegate;

using RebootOnShutdownCallback = base::Callback<void(bool)>;

namespace tray {
class UserAccountsDelegate;
}  // namespace tray

class ASH_EXPORT SystemTrayDelegate {
 public:
  virtual ~SystemTrayDelegate() {}

  // Called after SystemTray has been instantiated.
  virtual void Initialize() = 0;

  // Called before SystemTray is destroyed.
  virtual void Shutdown() = 0;

  // Returns true if system tray should be visible on startup.
  virtual bool GetTrayVisibilityOnStartup() = 0;

  // Gets information about the active user.
  virtual user::LoginStatus GetUserLoginStatus() const = 0;

  // Shows UI for changing user's profile picture.
  virtual void ChangeProfilePicture() = 0;

  // Returns the domain that manages the device, if it is enterprise-enrolled.
  virtual const std::string GetEnterpriseDomain() const = 0;

  // Returns notification for enterprise enrolled devices.
  virtual const base::string16 GetEnterpriseMessage() const = 0;

  // Returns the display email of the user that manages the current supervised
  // user.
  virtual const std::string GetSupervisedUserManager() const = 0;

  // Returns the name of the user that manages the current supervised user.
  virtual const base::string16 GetSupervisedUserManagerName() const = 0;

  // Returns the notification for supervised users.
  virtual const base::string16 GetSupervisedUserMessage() const = 0;

  // Returns true if the current user is supervised: has legacy supervised
  // account or kid account.
  virtual bool IsUserSupervised() const = 0;

  // Returns true if the current user is child.
  // TODO(merkulova): remove on FakeUserManager componentization.
  // crbug.com/443119
  virtual bool IsUserChild() const = 0;

  // Fills |info| structure with current update info.
  virtual void GetSystemUpdateInfo(UpdateInfo* info) const = 0;

  // Returns the desired hour clock type.
  virtual base::HourClockType GetHourClockType() const = 0;

  // Shows settings.
  virtual void ShowSettings() = 0;

  // Returns true if settings menu item should appear.
  virtual bool ShouldShowSettings() = 0;

  // Shows the settings related to date, timezone etc.
  virtual void ShowDateSettings() = 0;

  // Shows the dialog to set system time, date, and timezone.
  virtual void ShowSetTimeDialog() = 0;

  // Shows the settings related to network. If |service_path| is not empty,
  // show the settings for that network.
  virtual void ShowNetworkSettings(const std::string& service_path) = 0;

  // Shows the settings related to bluetooth.
  virtual void ShowBluetoothSettings() = 0;

  // Shows settings related to multiple displays.
  virtual void ShowDisplaySettings() = 0;

  // Shows the page that lets you disable performance tracing.
  virtual void ShowChromeSlow() = 0;

  // Returns true if the notification for the display configuration change
  // should appear.
  virtual bool ShouldShowDisplayNotification() = 0;

  // Shows settings related to input methods.
  virtual void ShowIMESettings() = 0;

  // Shows help.
  virtual void ShowHelp() = 0;

  // Show accessilibity help.
  virtual void ShowAccessibilityHelp() = 0;

  // Show the settings related to accessilibity.
  virtual void ShowAccessibilitySettings() = 0;

  // Shows more information about public account mode.
  virtual void ShowPublicAccountInfo() = 0;

  // Shows information about enterprise enrolled devices.
  virtual void ShowEnterpriseInfo() = 0;

  // Shows information about supervised users.
  virtual void ShowSupervisedUserInfo() = 0;

  // Shows login UI to add other users to this session.
  virtual void ShowUserLogin() = 0;

  // Attempts to sign out the user.
  virtual void SignOut() = 0;

  // Attempts to lock the screen.
  virtual void RequestLockScreen() = 0;

  // Attempts to restart the system for update.
  virtual void RequestRestartForUpdate() = 0;

  // Returns a list of available bluetooth devices.
  virtual void GetAvailableBluetoothDevices(BluetoothDeviceList* devices) = 0;

  // Requests bluetooth start discovering devices.
  virtual void BluetoothStartDiscovering() = 0;

  // Requests bluetooth stop discovering devices.
  virtual void BluetoothStopDiscovering() = 0;

  // Connect to a specific bluetooth device.
  virtual void ConnectToBluetoothDevice(const std::string& address) = 0;

  // Returns true if bluetooth adapter is discovering bluetooth devices.
  virtual bool IsBluetoothDiscovering() = 0;

  // Returns the currently selected IME.
  virtual void GetCurrentIME(IMEInfo* info) = 0;

  // Returns a list of availble IMEs.
  virtual void GetAvailableIMEList(IMEInfoList* list) = 0;

  // Returns a list of properties for the currently selected IME.
  virtual void GetCurrentIMEProperties(IMEPropertyInfoList* list) = 0;

  // Switches to the selected input method.
  virtual void SwitchIME(const std::string& ime_id) = 0;

  // Activates an IME property.
  virtual void ActivateIMEProperty(const std::string& key) = 0;

  // Shows UI to manage bluetooth devices.
  virtual void ManageBluetoothDevices() = 0;

  // Toggles bluetooth.
  virtual void ToggleBluetooth() = 0;

  // Shows UI to connect to an unlisted network of type |type|. On Chrome OS
  // |type| corresponds to a Shill network type.
  virtual void ShowOtherNetworkDialog(const std::string& type) = 0;

  // Returns whether bluetooth capability is available.
  virtual bool GetBluetoothAvailable() = 0;

  // Returns whether bluetooth is enabled.
  virtual bool GetBluetoothEnabled() = 0;

  // Returns whether the delegate has initiated a bluetooth discovery session.
  virtual bool GetBluetoothDiscovering() = 0;

  // Shows UI for changing proxy settings.
  virtual void ChangeProxySettings() = 0;

  // Returns NetworkingConfigDelegate.
  virtual NetworkingConfigDelegate* GetNetworkingConfigDelegate() const = 0;

  // Returns VolumeControlDelegate.
  virtual VolumeControlDelegate* GetVolumeControlDelegate() const = 0;

  // Sets VolumeControlDelegate.
  virtual void SetVolumeControlDelegate(
      scoped_ptr<VolumeControlDelegate> delegate) = 0;

  // Retrieves the session start time. Returns |false| if the time is not set.
  virtual bool GetSessionStartTime(base::TimeTicks* session_start_time) = 0;

  // Retrieves the session length limit. Returns |false| if no limit is set.
  virtual bool GetSessionLengthLimit(base::TimeDelta* session_length_limit) = 0;

  // Get the system tray menu size in pixels (dependent on the language).
  virtual int GetSystemTrayMenuWidth() = 0;

  // The active user has been changed. This will be called when the UI is ready
  // to be switched to the new user.
  // Note: This will happen after SessionStateObserver::ActiveUserChanged fires.
  virtual void ActiveUserWasChanged() = 0;

  // Returns true when the Search key is configured to be treated as Caps Lock.
  virtual bool IsSearchKeyMappedToCapsLock() = 0;

  // Returns accounts delegate for given user.
  virtual tray::UserAccountsDelegate* GetUserAccountsDelegate(
      const std::string& user_id) = 0;

  // Adding observers that are notified when supervised info is being changed.
  virtual void AddCustodianInfoTrayObserver(
      CustodianInfoTrayObserver* observer) = 0;

  virtual void RemoveCustodianInfoTrayObserver(
      CustodianInfoTrayObserver* observer) = 0;

  // Adds an observer whose |OnShutdownPolicyChanged| function is called when
  // the |DeviceRebootOnShutdown| policy changes. If this policy is set to
  // true, a device cannot be shut down anymore but only rebooted.
  virtual void AddShutdownPolicyObserver(ShutdownPolicyObserver* observer) = 0;

  virtual void RemoveShutdownPolicyObserver(
      ShutdownPolicyObserver* observer) = 0;

  // Determines whether the device is automatically rebooted when shut down as
  // specified by the device policy |DeviceRebootOnShutdown|. This function
  // asynchronously calls |callback| once a trusted policy becomes available.
  virtual void ShouldRebootOnShutdown(
      const RebootOnShutdownCallback& callback) = 0;
};

}  // namespace ash

#endif  // ASH_SYSTEM_TRAY_SYSTEM_TRAY_DELEGATE_H_
