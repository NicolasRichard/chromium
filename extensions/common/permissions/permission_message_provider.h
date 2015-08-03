// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_COMMON_PERMISSIONS_PERMISSION_MESSAGE_PROVIDER_H_
#define EXTENSIONS_COMMON_PERMISSIONS_PERMISSION_MESSAGE_PROVIDER_H_

#include <vector>

#include "extensions/common/manifest.h"
#include "extensions/common/permissions/coalesced_permission_message.h"
#include "extensions/common/permissions/permission_message.h"

namespace extensions {

class PermissionIDSet;
class PermissionSet;

// Temporary type to help the transition between old and new system.
// Essentially a CoalescedPermissionMessage minus the IDs.
// TODO(treib): Remove this once we've switched to the new system.
struct PermissionMessageString {
  PermissionMessageString(const CoalescedPermissionMessage& message);
  PermissionMessageString(const base::string16& message);
  PermissionMessageString(const base::string16& message,
                          const std::vector<base::string16>& submessages);
  PermissionMessageString(const base::string16& message,
                          const base::string16& details);
  ~PermissionMessageString();

  base::string16 message;
  std::vector<base::string16> submessages;
};
typedef std::vector<PermissionMessageString> PermissionMessageStrings;

// The PermissionMessageProvider interprets permissions, translating them
// into warning messages to show to the user. It also determines whether
// a new set of permissions entails showing new warning messages.
class PermissionMessageProvider {
 public:
  PermissionMessageProvider() {}
  virtual ~PermissionMessageProvider() {}

  // Return the global permission message provider.
  static const PermissionMessageProvider* Get();

  // Calculates and returns the full list of permission messages for the given
  // |permissions|. This forwards to GetCoalescedPermissionMessages.
  // TODO(treib): Remove this and update all callers to use
  // GetCoalescedPermissionMessages directly.
  PermissionMessageStrings GetPermissionMessageStrings(
      const PermissionSet* permissions,
      Manifest::Type extension_type) const;

  // Calculates and returns the full list of permission messages for the given
  // |permissions|. This involves converting the given PermissionIDs into
  // localized messages, as well as coalescing and parameterizing any messages
  // that require the permission ID's argument in their message.
  // TODO(sashab): Rename this to GetPermissionMessages().
  virtual CoalescedPermissionMessages GetCoalescedPermissionMessages(
      const PermissionIDSet& permissions) const = 0;

  // Returns true if |new_permissions| has a greater privilege level than
  // |old_permissions|.
  // Whether certain permissions are considered varies by extension type.
  // TODO(sashab): Add an implementation of this method that uses
  // PermissionIDSet instead, then deprecate this one.
  virtual bool IsPrivilegeIncrease(
      const PermissionSet* old_permissions,
      const PermissionSet* new_permissions,
      Manifest::Type extension_type) const = 0;

  // Given the permissions for an extension, finds the IDs of all the
  // permissions for that extension (including API, manifest and host
  // permissions).
  // TODO(sashab): This uses the legacy PermissionSet type. Deprecate or rename
  // this type, and make this take as little as is needed to work out the
  // PermissionIDSet.
  virtual PermissionIDSet GetAllPermissionIDs(
      const PermissionSet* permissions,
      Manifest::Type extension_type) const = 0;
};

}  // namespace extensions

#endif  // EXTENSIONS_COMMON_PERMISSIONS_PERMISSION_MESSAGE_PROVIDER_H_
