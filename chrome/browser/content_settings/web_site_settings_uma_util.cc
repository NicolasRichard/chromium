// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/content_settings/web_site_settings_uma_util.h"

#include "base/metrics/histogram_macros.h"

void WebSiteSettingsUmaUtil::LogPermissionChange(ContentSettingsType type,
                                                 ContentSetting setting) {
  ContentSettingsTypeHistogram histogram_value =
      ContentSettingTypeToHistogramValue(type);
  DCHECK_NE(histogram_value, CONTENT_SETTINGS_TYPE_HISTOGRAM_INVALID);
  UMA_HISTOGRAM_ENUMERATION("WebsiteSettings.Menu.PermissionChanged",
                            histogram_value,
                            CONTENT_SETTINGS_HISTOGRAM_NUM_TYPES);

  if (setting == ContentSetting::CONTENT_SETTING_ALLOW) {
    UMA_HISTOGRAM_ENUMERATION("WebsiteSettings.Menu.PermissionChanged.Allowed",
                              histogram_value,
                              CONTENT_SETTINGS_HISTOGRAM_NUM_TYPES);
  } else if (setting == ContentSetting::CONTENT_SETTING_BLOCK) {
    UMA_HISTOGRAM_ENUMERATION("WebsiteSettings.Menu.PermissionChanged.Blocked",
                              histogram_value,
                              CONTENT_SETTINGS_HISTOGRAM_NUM_TYPES);
  } else if (setting == ContentSetting::CONTENT_SETTING_DEFAULT) {
    UMA_HISTOGRAM_ENUMERATION("WebsiteSettings.Menu.PermissionChanged.Reset",
                              histogram_value,
                              CONTENT_SETTINGS_HISTOGRAM_NUM_TYPES);
  } else {
    NOTREACHED() << "Requested to log permission change " << type << " to "
                 << setting;
  }
}
