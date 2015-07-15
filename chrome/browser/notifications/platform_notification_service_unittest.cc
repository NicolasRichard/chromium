// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "chrome/browser/notifications/notification_test_util.h"
#include "chrome/browser/notifications/platform_notification_service_impl.h"
#include "chrome/test/base/testing_profile.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "content/public/browser/desktop_notification_delegate.h"
#include "content/public/common/platform_notification_data.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkBitmap.h"

#if defined(ENABLE_EXTENSIONS)
#include "base/command_line.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/test_extension_system.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/process_map.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_builder.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/value_builder.h"
#endif

#if defined(ENABLE_EXTENSIONS) && defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/login/users/scoped_test_user_manager.h"
#include "chrome/browser/chromeos/settings/cros_settings.h"
#include "chrome/browser/chromeos/settings/device_settings_service.h"
#endif

namespace {

const int kNotificationVibrationPattern[] = { 100, 200, 300 };

#if !defined(OS_ANDROID)
const int64_t kPersistentNotificationId = 42;
#endif

class MockDesktopNotificationDelegate
    : public content::DesktopNotificationDelegate {
 public:
  MockDesktopNotificationDelegate()
      : displayed_(false),
        clicked_(false) {}

  ~MockDesktopNotificationDelegate() override {}

  // content::DesktopNotificationDelegate implementation.
  void NotificationDisplayed() override { displayed_ = true; }
  void NotificationClosed() override {}
  void NotificationClick() override { clicked_ = true; }

  bool displayed() const { return displayed_; }
  bool clicked() const { return clicked_; }

 private:
  bool displayed_;
  bool clicked_;
};

}  // namespace

class PlatformNotificationServiceTest : public testing::Test {
 public:
  void SetUp() override {
    ui_manager_.reset(new StubNotificationUIManager);
    profile_.reset(new TestingProfile());

    service()->SetNotificationUIManagerForTesting(ui_manager_.get());
  }

  void TearDown() override {
    service()->SetNotificationUIManagerForTesting(nullptr);

    profile_.reset();
    ui_manager_.reset();
  }

 protected:
  // Displays a simple, fake notifications and returns a weak pointer to the
  // delegate receiving events for it (ownership is transferred to the service).
  MockDesktopNotificationDelegate* CreateSimplePageNotification() const {
    return CreateSimplePageNotificationWithCloseClosure(nullptr);
  }

  // Displays a simple, fake notification and returns a weak pointer to the
  // delegate receiving events for it (ownership is transferred to the service).
  // The close closure may be specified if so desired.
  MockDesktopNotificationDelegate* CreateSimplePageNotificationWithCloseClosure(
      base::Closure* close_closure) const {
    content::PlatformNotificationData notification_data;
    notification_data.title = base::ASCIIToUTF16("My Notification");
    notification_data.body = base::ASCIIToUTF16("Hello, world!");

    MockDesktopNotificationDelegate* delegate =
        new MockDesktopNotificationDelegate();

    service()->DisplayNotification(profile(),
                                   GURL("https://chrome.com/"),
                                   SkBitmap(),
                                   notification_data,
                                   make_scoped_ptr(delegate),
                                   close_closure);

    return delegate;
  }

  // Returns the Platform Notification Service these unit tests are for.
  PlatformNotificationServiceImpl* service() const {
    return PlatformNotificationServiceImpl::GetInstance();
  }

  // Returns the Profile to be used for these tests.
  Profile* profile() const { return profile_.get(); }

  // Returns the UI Manager on which notifications will be displayed.
  StubNotificationUIManager* ui_manager() const { return ui_manager_.get(); }

 private:
  scoped_ptr<StubNotificationUIManager> ui_manager_;
  scoped_ptr<TestingProfile> profile_;

  content::TestBrowserThreadBundle thread_bundle_;
};

TEST_F(PlatformNotificationServiceTest, DisplayPageDisplayedEvent) {
  auto* delegate = CreateSimplePageNotification();

  EXPECT_EQ(1u, ui_manager()->GetNotificationCount());
  EXPECT_TRUE(delegate->displayed());
}

TEST_F(PlatformNotificationServiceTest, DisplayPageCloseClosure) {
  base::Closure close_closure;
  CreateSimplePageNotificationWithCloseClosure(&close_closure);

  EXPECT_EQ(1u, ui_manager()->GetNotificationCount());

  ASSERT_FALSE(close_closure.is_null());
  close_closure.Run();

  EXPECT_EQ(0u, ui_manager()->GetNotificationCount());

  // Note that we cannot verify whether the closed event was called on the
  // delegate given that it'd result in a use-after-free.
}

// TODO(peter): Re-enable this test when //content is responsible for creating
// the notification delegate ids.
#if !defined(OS_ANDROID)
TEST_F(PlatformNotificationServiceTest, PersistentNotificationDisplay) {
  content::PlatformNotificationData notification_data;
  notification_data.title = base::ASCIIToUTF16("My notification's title");
  notification_data.body = base::ASCIIToUTF16("Hello, world!");

  service()->DisplayPersistentNotification(
      profile(), kPersistentNotificationId, GURL("https://chrome.com/"),
      SkBitmap(), notification_data);

  ASSERT_EQ(1u, ui_manager()->GetNotificationCount());

  const Notification& notification = ui_manager()->GetNotificationAt(0);
  EXPECT_EQ("https://chrome.com/", notification.origin_url().spec());
  EXPECT_EQ("My notification's title",
      base::UTF16ToUTF8(notification.title()));
  EXPECT_EQ("Hello, world!",
      base::UTF16ToUTF8(notification.message()));

  service()->ClosePersistentNotification(profile(), kPersistentNotificationId);
  EXPECT_EQ(0u, ui_manager()->GetNotificationCount());
}
#endif  // !defined(OS_ANDROID)

TEST_F(PlatformNotificationServiceTest, DisplayPageNotificationMatches) {
  std::vector<int> vibration_pattern(
      kNotificationVibrationPattern,
      kNotificationVibrationPattern + arraysize(kNotificationVibrationPattern));

  content::PlatformNotificationData notification_data;
  notification_data.title = base::ASCIIToUTF16("My notification's title");
  notification_data.body = base::ASCIIToUTF16("Hello, world!");
  notification_data.vibration_pattern = vibration_pattern;
  notification_data.silent = true;

  MockDesktopNotificationDelegate* delegate
      = new MockDesktopNotificationDelegate();
  service()->DisplayNotification(profile(),
                                 GURL("https://chrome.com/"),
                                 SkBitmap(),
                                 notification_data,
                                 make_scoped_ptr(delegate),
                                 nullptr);

  ASSERT_EQ(1u, ui_manager()->GetNotificationCount());

  const Notification& notification = ui_manager()->GetNotificationAt(0);
  EXPECT_EQ("https://chrome.com/", notification.origin_url().spec());
  EXPECT_EQ("My notification's title",
      base::UTF16ToUTF8(notification.title()));
  EXPECT_EQ("Hello, world!",
      base::UTF16ToUTF8(notification.message()));

  EXPECT_THAT(notification.vibration_pattern(),
      testing::ElementsAreArray(kNotificationVibrationPattern));

  EXPECT_TRUE(notification.silent());
}

TEST_F(PlatformNotificationServiceTest, DisplayNameForOrigin) {
  base::string16 display_name =
      service()->DisplayNameForOrigin(profile(), GURL("https://chrome.com/"));

  EXPECT_EQ(base::ASCIIToUTF16("chrome.com"), display_name);

  // TODO(peter): Include unit tests for the extension-name translation
  // functionality of DisplayNameForOriginInProcessId.
}

TEST_F(PlatformNotificationServiceTest, TestWebOriginDisplayName) {
  std::string language("en-us");

  GURL https_origin("https://mail.google.com/");
  base::string16 expected_display_name = base::ASCIIToUTF16("mail.google.com");
  EXPECT_EQ(expected_display_name,
            PlatformNotificationServiceImpl::WebOriginDisplayName(https_origin,
                                                                  language));

  GURL https_origin_standard_port("https://mail.google.com:443/");
  expected_display_name = base::ASCIIToUTF16("mail.google.com");
  EXPECT_EQ(expected_display_name,
            PlatformNotificationServiceImpl::WebOriginDisplayName(
                https_origin_standard_port, language));

  GURL https_origin_nonstandard_port("https://mail.google.com:444/");
  expected_display_name = base::ASCIIToUTF16("mail.google.com:444");
  EXPECT_EQ(expected_display_name,
            PlatformNotificationServiceImpl::WebOriginDisplayName(
                https_origin_nonstandard_port, language));

  GURL http_origin("http://mail.google.com/");
  expected_display_name = base::ASCIIToUTF16("http://mail.google.com");
  EXPECT_EQ(expected_display_name,
            PlatformNotificationServiceImpl::WebOriginDisplayName(http_origin,
                                                                  language));

  GURL http_origin_standard_port("http://mail.google.com:80/");
  expected_display_name = base::ASCIIToUTF16("http://mail.google.com");
  EXPECT_EQ(expected_display_name,
            PlatformNotificationServiceImpl::WebOriginDisplayName(
                http_origin_standard_port, language));

  GURL http_origin_nonstandard_port("http://mail.google.com:81/");
  expected_display_name = base::ASCIIToUTF16("http://mail.google.com:81");
  EXPECT_EQ(expected_display_name,
            PlatformNotificationServiceImpl::WebOriginDisplayName(
                http_origin_nonstandard_port, language));
  // TODO(dewittj): Add file origin once it's supported.
}

TEST_F(PlatformNotificationServiceTest, NotificationPermissionLastUsage) {
  // Both page and persistent notifications should update the last usage
  // time of the notification permission for the origin.
  GURL origin("https://chrome.com/");
  base::Time begin_time;

  CreateSimplePageNotification();

  base::Time after_page_notification =
      profile()->GetHostContentSettingsMap()->GetLastUsage(
          origin, origin, CONTENT_SETTINGS_TYPE_NOTIFICATIONS);
  EXPECT_GT(after_page_notification, begin_time);

  // Ensure that there is at least some time between the two calls.
  base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(1));

  service()->DisplayPersistentNotification(
      profile(), 42 /* sw_registration_id */, origin, SkBitmap(),
      content::PlatformNotificationData());

  base::Time after_persistent_notification =
      profile()->GetHostContentSettingsMap()->GetLastUsage(
          origin, origin, CONTENT_SETTINGS_TYPE_NOTIFICATIONS);
  EXPECT_GT(after_persistent_notification, after_page_notification);
}

#if defined(ENABLE_EXTENSIONS)

TEST_F(PlatformNotificationServiceTest, ExtensionPermissionChecks) {
#if defined(OS_CHROMEOS)
  // The ExtensionService on Chrome OS requires these objects to be initialized.
  chromeos::ScopedTestDeviceSettingsService test_device_settings_service;
  chromeos::ScopedTestCrosSettings test_cros_settings;
  chromeos::ScopedTestUserManager test_user_manager;
#endif

  base::CommandLine command_line(base::CommandLine::NO_PROGRAM);
  extensions::TestExtensionSystem* test_extension_system =
      static_cast<extensions::TestExtensionSystem*>(
          extensions::ExtensionSystem::Get(profile()));

  ExtensionService* extension_service =
      test_extension_system->CreateExtensionService(
          &command_line, base::FilePath() /* install_directory */,
          false /* autoupdate_enabled*/);

  // Create a mocked extension that has the notifications API permission.
  scoped_refptr<extensions::Extension> extension =
      extensions::ExtensionBuilder().SetManifest(
          extensions::DictionaryBuilder()
              .Set("name", "NotificationTest")
              .Set("version", "1.0")
              .Set("manifest_version", 2)
              .Set("description", "Test Extension")
              .Set("permissions",
                   extensions::ListBuilder().Append("notifications"))).Build();

  // Install the extension on the faked extension service, and verify that it
  // has been added to the extension registry successfully.
  extension_service->AddExtension(extension.get());
  extensions::ExtensionRegistry* registry =
      extensions::ExtensionRegistry::Get(profile());

  ASSERT_TRUE(registry->GetExtensionById(
      extension->id(), extensions::ExtensionRegistry::ENABLED));

  const int kFakeRenderProcessId = 42;

  // Mock that the extension is running in a fake render process id.
  extensions::ProcessMap::Get(profile())->Insert(extension->id(),
                                                 kFakeRenderProcessId,
                                                 -1 /* site_instance_id */);

  // Verify that the service indicates that permission has been granted. We only
  // check the UI thread-method for now, as that's the one guarding the behavior
  // in the browser process.
  EXPECT_EQ(blink::WebNotificationPermissionAllowed,
            service()->CheckPermissionOnUIThread(profile(),
                                                 extension->url(),
                                                 kFakeRenderProcessId));
}

#endif  // defined(ENABLE_EXTENSIONS)
