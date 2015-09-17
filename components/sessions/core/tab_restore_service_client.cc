// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sessions/core/tab_restore_service_client.h"

namespace sessions {

TabClientData::~TabClientData() {}

TabRestoreServiceClient::~TabRestoreServiceClient() {}

scoped_ptr<TabClientData>
TabRestoreServiceClient::GetTabClientDataForWebContents(
    content::WebContents* web_contents) {
  return nullptr;
}

void TabRestoreServiceClient::OnTabRestored(const GURL& url) {}

}  // namespace sessions
