// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libudev.h>

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "device/hid/input_service_linux.h"

namespace device {

namespace {

const char kSubsystemHid[] = "hid";
const char kSubsystemInput[] = "input";
const char kTypeBluetooth[] = "bluetooth";
const char kTypeUsb[] = "usb";
const char kTypeSerio[] = "serio";
const char kIdInputAccelerometer[] = "ID_INPUT_ACCELEROMETER";
const char kIdInputJoystick[] = "ID_INPUT_JOYSTICK";
const char kIdInputKey[] = "ID_INPUT_KEY";
const char kIdInputKeyboard[] = "ID_INPUT_KEYBOARD";
const char kIdInputMouse[] = "ID_INPUT_MOUSE";
const char kIdInputTablet[] = "ID_INPUT_TABLET";
const char kIdInputTouchpad[] = "ID_INPUT_TOUCHPAD";
const char kIdInputTouchscreen[] = "ID_INPUT_TOUCHSCREEN";

// The instance will be reset when message loop destroys.
base::LazyInstance<scoped_ptr<InputServiceLinux> >::Leaky
    g_input_service_linux_ptr = LAZY_INSTANCE_INITIALIZER;

bool GetBoolProperty(udev_device* device, const char* key) {
  CHECK(device);
  CHECK(key);
  const char* property = udev_device_get_property_value(device, key);
  if (!property)
    return false;
  int value;
  if (!base::StringToInt(property, &value)) {
    LOG(ERROR) << "Not an integer value for " << key << " property";
    return false;
  }
  return (value != 0);
}

InputServiceLinux::InputDeviceInfo::Type GetDeviceType(udev_device* device) {
  if (udev_device_get_parent_with_subsystem_devtype(
          device, kTypeBluetooth, NULL)) {
    return InputServiceLinux::InputDeviceInfo::TYPE_BLUETOOTH;
  }
  if (udev_device_get_parent_with_subsystem_devtype(device, kTypeUsb, NULL))
    return InputServiceLinux::InputDeviceInfo::TYPE_USB;
  if (udev_device_get_parent_with_subsystem_devtype(device, kTypeSerio, NULL))
    return InputServiceLinux::InputDeviceInfo::TYPE_SERIO;
  return InputServiceLinux::InputDeviceInfo::TYPE_UNKNOWN;
}

class InputServiceLinuxImpl : public InputServiceLinux,
                              public DeviceMonitorLinux::Observer {
 public:
  // Implements DeviceMonitorLinux::Observer:
  virtual void OnDeviceAdded(udev_device* device) OVERRIDE;
  virtual void OnDeviceRemoved(udev_device* device) OVERRIDE;

 private:
  friend class InputServiceLinux;

  InputServiceLinuxImpl();
  virtual ~InputServiceLinuxImpl();

  DISALLOW_COPY_AND_ASSIGN(InputServiceLinuxImpl);
};

InputServiceLinuxImpl::InputServiceLinuxImpl() {
  DeviceMonitorLinux::GetInstance()->AddObserver(this);
  DeviceMonitorLinux::GetInstance()->Enumerate(base::Bind(
      &InputServiceLinuxImpl::OnDeviceAdded, base::Unretained(this)));
}

InputServiceLinuxImpl::~InputServiceLinuxImpl() {
  if (DeviceMonitorLinux::HasInstance())
    DeviceMonitorLinux::GetInstance()->RemoveObserver(this);
}

void InputServiceLinuxImpl::OnDeviceAdded(udev_device* device) {
  DCHECK(CalledOnValidThread());
  if (!device)
    return;
  const char* path = udev_device_get_syspath(device);
  if (!path)
    return;

  InputDeviceInfo info;
  info.id = path;

  const char* name = udev_device_get_property_value(device, "NAME");
  if (name)
    info.name = name;

  const char* subsystem = udev_device_get_subsystem(device);
  if (!subsystem)
    return;
  else if (strcmp(subsystem, kSubsystemHid) == 0)
    info.subsystem = InputServiceLinux::InputDeviceInfo::SUBSYSTEM_HID;
  else if (strcmp(subsystem, kSubsystemInput) == 0)
    info.subsystem = InputServiceLinux::InputDeviceInfo::SUBSYSTEM_INPUT;
  else
    return;

  info.type = GetDeviceType(device);

  info.is_accelerometer = GetBoolProperty(device, kIdInputAccelerometer);
  info.is_joystick = GetBoolProperty(device, kIdInputJoystick);
  info.is_key = GetBoolProperty(device, kIdInputKey);
  info.is_keyboard = GetBoolProperty(device, kIdInputKeyboard);
  info.is_mouse = GetBoolProperty(device, kIdInputMouse);
  info.is_tablet = GetBoolProperty(device, kIdInputTablet);
  info.is_touchpad = GetBoolProperty(device, kIdInputTouchpad);
  info.is_touchscreen = GetBoolProperty(device, kIdInputTouchscreen);

  AddDevice(info);
}

void InputServiceLinuxImpl::OnDeviceRemoved(udev_device* device) {
  DCHECK(CalledOnValidThread());
  if (!device)
    return;
  const char* path = udev_device_get_syspath(device);
  if (!path)
    return;
  RemoveDevice(path);
}

}  // namespace

InputServiceLinux::InputDeviceInfo::InputDeviceInfo()
    : subsystem(SUBSYSTEM_UNKNOWN),
      type(TYPE_UNKNOWN),
      is_accelerometer(false),
      is_joystick(false),
      is_key(false),
      is_keyboard(false),
      is_mouse(false),
      is_tablet(false),
      is_touchpad(false),
      is_touchscreen(false) {}

InputServiceLinux::InputServiceLinux() {
  base::ThreadRestrictions::AssertIOAllowed();
  base::MessageLoop::current()->AddDestructionObserver(this);
}

InputServiceLinux::~InputServiceLinux() {
  DCHECK(CalledOnValidThread());
  base::MessageLoop::current()->RemoveDestructionObserver(this);
}

// static
InputServiceLinux* InputServiceLinux::GetInstance() {
  if (!HasInstance())
    g_input_service_linux_ptr.Get().reset(new InputServiceLinuxImpl());
  return g_input_service_linux_ptr.Get().get();
}

// static
bool InputServiceLinux::HasInstance() {
  return g_input_service_linux_ptr.Get().get();
}

// static
void InputServiceLinux::SetForTesting(InputServiceLinux* service) {
  g_input_service_linux_ptr.Get().reset(service);
}

void InputServiceLinux::AddObserver(Observer* observer) {
  DCHECK(CalledOnValidThread());
  if (observer)
    observers_.AddObserver(observer);
}

void InputServiceLinux::RemoveObserver(Observer* observer) {
  DCHECK(CalledOnValidThread());
  if (observer)
    observers_.RemoveObserver(observer);
}

void InputServiceLinux::GetDevices(std::vector<InputDeviceInfo>* devices) {
  DCHECK(CalledOnValidThread());
  for (DeviceMap::iterator it = devices_.begin(), ie = devices_.end(); it != ie;
       ++it) {
    devices->push_back(it->second);
  }
}

bool InputServiceLinux::GetDeviceInfo(const std::string& id,
                                      InputDeviceInfo* info) const {
  DCHECK(CalledOnValidThread());
  DeviceMap::const_iterator it = devices_.find(id);
  if (it == devices_.end())
    return false;
  *info = it->second;
  return true;
}

void InputServiceLinux::WillDestroyCurrentMessageLoop() {
  DCHECK(CalledOnValidThread());
  g_input_service_linux_ptr.Get().reset(NULL);
}

void InputServiceLinux::AddDevice(const InputDeviceInfo& info) {
  devices_[info.id] = info;
  FOR_EACH_OBSERVER(Observer, observers_, OnInputDeviceAdded(info));
}

void InputServiceLinux::RemoveDevice(const std::string& id) {
  devices_.erase(id);
  FOR_EACH_OBSERVER(Observer, observers_, OnInputDeviceRemoved(id));
}

bool InputServiceLinux::CalledOnValidThread() const {
  return thread_checker_.CalledOnValidThread();
}

}  // namespace device
