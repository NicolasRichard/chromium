// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/media_stream_device_permission_context.h"

#include "base/bind.h"
#include "chrome/browser/infobars/infobar_service.h"
#include "chrome/browser/permissions/permission_queue_controller.h"
#include "chrome/browser/permissions/permission_request_id.h"
#include "chrome/browser/ui/website_settings/permission_bubble_manager.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "chrome/test/base/testing_profile.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/mock_render_process_host.h"
#include "content/public/test/web_contents_tester.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
class TestPermissionContext : public MediaStreamDevicePermissionContext {
 public:
  TestPermissionContext(Profile* profile,
                        const ContentSettingsType permission_type)
      : MediaStreamDevicePermissionContext(profile, permission_type) {}

  ~TestPermissionContext() override {}
};

}  // anonymous namespace

// TODO(raymes): many tests in MediaStreamDevicesControllerTest should be
// converted to tests in this file.
class MediaStreamDevicePermissionContextTests
    : public ChromeRenderViewHostTestHarness {
 protected:
  MediaStreamDevicePermissionContextTests() = default;

  void TestInsecureQueryingUrl(ContentSettingsType permission_type) {
    TestPermissionContext permission_context(profile(), permission_type);
    GURL insecure_url("http://www.example.com");
    GURL secure_url("https://www.example.com");

    // Check that there is no saved content settings.
    EXPECT_EQ(CONTENT_SETTING_ASK,
              profile()->GetHostContentSettingsMap()->GetContentSetting(
                  insecure_url.GetOrigin(), insecure_url.GetOrigin(),
                  permission_type, std::string()));
    EXPECT_EQ(CONTENT_SETTING_ASK,
              profile()->GetHostContentSettingsMap()->GetContentSetting(
                  secure_url.GetOrigin(), insecure_url.GetOrigin(),
                  permission_type, std::string()));
    EXPECT_EQ(CONTENT_SETTING_ASK,
              profile()->GetHostContentSettingsMap()->GetContentSetting(
                  insecure_url.GetOrigin(), secure_url.GetOrigin(),
                  permission_type, std::string()));

    EXPECT_EQ(CONTENT_SETTING_ASK, permission_context.GetPermissionStatus(
                                       insecure_url, insecure_url));
    EXPECT_EQ(CONTENT_SETTING_ASK,
              permission_context.GetPermissionStatus(insecure_url, secure_url));
  }

  void TestSecureQueryingUrl(ContentSettingsType permission_type) {
    TestPermissionContext permission_context(profile(), permission_type);
    GURL secure_url("https://www.example.com");

    // Check that there is no saved content settings.
    EXPECT_EQ(CONTENT_SETTING_ASK,
              profile()->GetHostContentSettingsMap()->GetContentSetting(
                  secure_url.GetOrigin(), secure_url.GetOrigin(),
                  permission_type, std::string()));

    EXPECT_EQ(CONTENT_SETTING_ASK,
              permission_context.GetPermissionStatus(secure_url, secure_url));
  }

 private:
  // ChromeRenderViewHostTestHarness:
  void SetUp() override {
    ChromeRenderViewHostTestHarness::SetUp();
    InfoBarService::CreateForWebContents(web_contents());
    PermissionBubbleManager::CreateForWebContents(web_contents());
  }

  DISALLOW_COPY_AND_ASSIGN(MediaStreamDevicePermissionContextTests);
};

// MEDIASTREAM_MIC permission status should be ask for insecure origin to
// accommodate the usage case of Flash.
TEST_F(MediaStreamDevicePermissionContextTests, TestMicInsecureQueryingUrl) {
  TestInsecureQueryingUrl(CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC);
}

// MEDIASTREAM_CAMERA permission status should be ask for insecure origin to
// accommodate the usage case of Flash.
TEST_F(MediaStreamDevicePermissionContextTests, TestCameraInsecureQueryingUrl) {
  TestInsecureQueryingUrl(CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA);
}

// MEDIASTREAM_MIC permission status should be ask for Secure origin.
TEST_F(MediaStreamDevicePermissionContextTests, TestMicSecureQueryingUrl) {
  TestSecureQueryingUrl(CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC);
}

// MEDIASTREAM_CAMERA permission status should be ask for Secure origin.
TEST_F(MediaStreamDevicePermissionContextTests, TestCameraSecureQueryingUrl) {
  TestSecureQueryingUrl(CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA);
}
