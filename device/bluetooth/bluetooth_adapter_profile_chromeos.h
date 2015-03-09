// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_ADAPTER_PROFILE_CHROMEOS_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_ADAPTER_PROFILE_CHROMEOS_H_

#include "base/memory/weak_ptr.h"
#include "chromeos/dbus/bluetooth_profile_manager_client.h"
#include "chromeos/dbus/bluetooth_profile_service_provider.h"
#include "device/bluetooth/bluetooth_adapter_chromeos.h"
#include "device/bluetooth/bluetooth_export.h"

namespace device {
class BluetoothUUID;
}  // namespace device

namespace chromeos {

// The BluetoothAdapterProfileChromeOS class implements a multiplexing
// profile for custom Bluetooth services managed by a BluetoothAdapter.
// Maintains a list of delegates which may serve the profile.
// One delegate is allowed for each device.
//
// BluetoothAdapterProfileChromeOS objects are owned by the
// BluetoothAdapterChromeOS and allocated through Register()
class DEVICE_BLUETOOTH_EXPORT BluetoothAdapterProfileChromeOS
    : public chromeos::BluetoothProfileServiceProvider::Delegate {
 public:
  typedef base::Callback<void(scoped_ptr<BluetoothAdapterProfileChromeOS>
                                  profile)> ProfileRegisteredCallback;

  // Registers a profile with the BlueZ server for |uuid| with the
  // options |options|.  |success_callback| is provided with a newly
  // allocated profile if registration is successful, otherwise |error_callback|
  // will be called.
  static void Register(
      const device::BluetoothUUID& uuid,
      const BluetoothProfileManagerClient::Options& options,
      const ProfileRegisteredCallback& success_callback,
      const BluetoothProfileManagerClient::ErrorCallback& error_callback);

  ~BluetoothAdapterProfileChromeOS() override;

  // The object path of the profile.
  const dbus::ObjectPath& object_path() const { return object_path_; }

  // Returns the UUID of the profile
  const device::BluetoothUUID& uuid() const { return uuid_; }

  // Add a delegate for a device associated with this profile.
  // An empty |device_path| indicates a local listening service.
  // Returns true if the delegate was set, and false if the |device_path|
  // already had a delegate set.
  bool SetDelegate(const dbus::ObjectPath& device_path,
                   BluetoothProfileServiceProvider::Delegate* delegate);

  // Remove the delegate for a device. |unregistered_callback| will be called
  // if this unregisters the profile.
  void RemoveDelegate(const dbus::ObjectPath& device_path,
                      const base::Closure& unregistered_callback);

  // Returns the number of delegates for this profile.
  size_t DelegateCount() const { return delegates_.size(); }

 private:
  BluetoothAdapterProfileChromeOS(const device::BluetoothUUID& uuid);

  // BluetoothProfileServiceProvider::Delegate:
  void Released() override;
  void NewConnection(
      const dbus::ObjectPath& device_path,
      scoped_ptr<dbus::FileDescriptor> fd,
      const BluetoothProfileServiceProvider::Delegate::Options& options,
      const ConfirmationCallback& callback) override;
  void RequestDisconnection(const dbus::ObjectPath& device_path,
                            const ConfirmationCallback& callback) override;
  void Cancel() override;

  // Called by dbus:: on completion of the D-Bus method to unregister a profile.
  void OnUnregisterProfileError(const base::Closure& unregistered_callback,
                                const std::string& error_name,
                                const std::string& error_message);

  // List of delegates which this profile is multiplexing to.
  std::map<std::string, BluetoothProfileServiceProvider::Delegate*> delegates_;

  // The UUID that this profile represents.
  const device::BluetoothUUID& uuid_;

  // Registered dbus object path for this profile.
  dbus::ObjectPath object_path_;

  // Profile dbus object for receiving profile method calls from BlueZ
  scoped_ptr<BluetoothProfileServiceProvider> profile_;

  // Note: This should remain the last member so it'll be destroyed and
  // invalidate its weak pointers before any other members are destroyed.
  base::WeakPtrFactory<BluetoothAdapterProfileChromeOS> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothAdapterProfileChromeOS);
};

}  // namespace chromeos

#endif
