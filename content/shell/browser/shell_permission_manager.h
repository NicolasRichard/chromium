// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_BROWSER_SHELL_PERMISSION_MANAGER_H_
#define CONTENT_SHELL_BROWSER_SHELL_PERMISSION_MANAGER_H_

#include "base/callback_forward.h"
#include "base/macros.h"
#include "content/public/browser/permission_manager.h"

namespace content {

class ShellPermissionManager : public PermissionManager {
 public:
  ShellPermissionManager();
  ~ShellPermissionManager() override;

  // PermissionManager implementation.
  void RequestPermission(
      PermissionType permission,
      RenderFrameHost* render_frame_host,
      int request_id,
      const GURL& requesting_origin,
      bool user_gesture,
      const base::Callback<void(PermissionStatus)>& callback) override;
  void CancelPermissionRequest(PermissionType permission,
                               RenderFrameHost* render_frame_host,
                               int request_id,
                               const GURL& requesting_origin) override;
  void ResetPermission(PermissionType permission,
                       const GURL& requesting_origin,
                       const GURL& embedding_origin) override;
  PermissionStatus GetPermissionStatus(PermissionType permission,
                                       const GURL& requesting_origin,
                                       const GURL& embedding_origin) override;
  void RegisterPermissionUsage(PermissionType permission,
                               const GURL& requesting_origin,
                               const GURL& embedding_origin) override;
  int SubscribePermissionStatusChange(
      PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin,
      const base::Callback<void(PermissionStatus)>& callback) override;
  void UnsubscribePermissionStatusChange(int subscription_id) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ShellPermissionManager);
};

}  // namespace content

#endif // CONTENT_SHELL_BROWSER_SHELL_PERMISSION_MANAGER_H
