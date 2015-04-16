// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/chromeos/login/existing_user_controller.h"
#include "chrome/browser/chromeos/login/test/oobe_base_test.h"
#include "chrome/browser/chromeos/login/test/oobe_screen_waiter.h"
#include "chrome/browser/chromeos/login/ui/login_display_host_impl.h"
#include "chrome/browser/chromeos/login/ui/webui_login_display.h"
#include "chrome/browser/chromeos/login/wizard_controller.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/ui/webui/chromeos/login/signin_screen_handler.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chromeos/chromeos_switches.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "google_apis/gaia/gaia_switches.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "net/test/embedded_test_server/http_response.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/test/ui_controls.h"
#include "ui/views/widget/widget.h"

using namespace net::test_server;

namespace chromeos {

// Boolean parameter is used to run this test for webview (true) and for
// iframe (false) GAIA sign in.
class OobeTest : public OobeBaseTest, public testing::WithParamInterface<bool> {
 public:
  OobeTest() { set_use_webview(GetParam()); }
  ~OobeTest() override {}

  void SetUpCommandLine(base::CommandLine* command_line) override {
    command_line->AppendSwitch(switches::kOobeSkipPostLogin);

    OobeBaseTest::SetUpCommandLine(command_line);
  }

  void TearDownOnMainThread() override {
    // If the login display is still showing, exit gracefully.
    if (LoginDisplayHostImpl::default_host()) {
      base::MessageLoop::current()->PostTask(FROM_HERE,
                                             base::Bind(&chrome::AttemptExit));
      content::RunMessageLoop();
    }

    OobeBaseTest::TearDownOnMainThread();
  }

  chromeos::WebUILoginDisplay* GetLoginDisplay() {
    chromeos::ExistingUserController* controller =
        chromeos::ExistingUserController::current_controller();
    CHECK(controller);
    return static_cast<chromeos::WebUILoginDisplay*>(
        controller->login_display());
  }

  views::Widget* GetLoginWindowWidget() {
    return static_cast<chromeos::LoginDisplayHostImpl*>(
        chromeos::LoginDisplayHostImpl::default_host())
        ->login_window_for_test();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(OobeTest);
};

IN_PROC_BROWSER_TEST_P(OobeTest, NewUser) {
  WaitForGaiaPageLoad();

  content::WindowedNotificationObserver session_start_waiter(
      chrome::NOTIFICATION_SESSION_STARTED,
      content::NotificationService::AllSources());

  GetLoginDisplay()->ShowSigninScreenForCreds(OobeBaseTest::kFakeUserEmail,
                                              OobeBaseTest::kFakeUserPassword);

  session_start_waiter.Wait();
}

IN_PROC_BROWSER_TEST_P(OobeTest, Accelerator) {
  WaitForGaiaPageLoad();

  gfx::NativeWindow login_window = GetLoginWindowWidget()->GetNativeWindow();

  ui_controls::SendKeyPress(login_window,
                            ui::VKEY_E,
                            true,    // control
                            false,   // shift
                            true,    // alt
                            false);  // command
  OobeScreenWaiter(OobeDisplay::SCREEN_OOBE_ENROLLMENT).Wait();
}

INSTANTIATE_TEST_CASE_P(OobeSuite, OobeTest, testing::Bool());

}  // namespace chromeos
