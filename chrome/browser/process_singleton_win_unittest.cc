// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/process_singleton.h"

#include <windows.h>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/process/launch.h"
#include "base/process/process.h"
#include "base/process/process_handle.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/test/multiprocess_test.h"
#include "base/win/scoped_handle.h"
#include "base/win/wrapped_window_proc.h"
#include "chrome/browser/chrome_process_finder_win.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/chrome_switches.h"
#include "content/public/common/result_codes.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/multiprocess_func_list.h"

namespace {

const char kReadyEventNameFlag[] = "ready_event_name";
const char kContinueEventNameFlag[] = "continue_event_name";
const char kCreateWindowFlag[] = "create_window";
const int kErrorResultCode = 0x345;

bool NotificationCallback(const base::CommandLine& command_line,
                          const base::FilePath& current_directory) {
  // This is never called in this test, but would signal that the singleton
  // notification was successfully handled.
  NOTREACHED();
  return true;
}

// The ProcessSingleton kills hung browsers with no visible windows without user
// interaction. If a hung browser has visible UI, however, it asks the user
// first.
// This class is the very minimal implementation to create a visible window
// in the hung test process to allow testing the latter path.
class ScopedVisibleWindow {
 public:
  ScopedVisibleWindow() : class_(0), window_(NULL) {}
  ~ScopedVisibleWindow() {
    if (window_)
      ::DestroyWindow(window_);
    if (class_)
      ::UnregisterClass(reinterpret_cast<LPCWSTR>(class_), NULL);
  }

  bool Create() {
    WNDCLASSEX wnd_cls = {0};
    base::win::InitializeWindowClass(
        L"ProcessSingletonTest", base::win::WrappedWindowProc<::DefWindowProc>,
        0,     // style
        0,     // class_extra
        0,     // window_extra
        NULL,  // cursor
        NULL,  // background
        L"",   // menu_name
        NULL,  // large_icon
        NULL,  // small_icon
        &wnd_cls);

    class_ = ::RegisterClassEx(&wnd_cls);
    if (!class_)
      return false;
    window_ = ::CreateWindow(reinterpret_cast<LPCWSTR>(class_), 0, WS_POPUP, 0,
                             0, 0, 0, 0, 0, NULL, 0);
    if (!window_)
      return false;
    ::ShowWindow(window_, SW_SHOW);

    DCHECK(window_);
    return true;
  }

 private:
  ATOM class_;
  HWND window_;

  DISALLOW_COPY_AND_ASSIGN(ScopedVisibleWindow);
};

MULTIPROCESS_TEST_MAIN(ProcessSingletonTestProcessMain) {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  base::FilePath user_data_dir =
      cmd_line->GetSwitchValuePath(switches::kUserDataDir);
  if (user_data_dir.empty())
    return kErrorResultCode;

  base::string16 ready_event_name =
      cmd_line->GetSwitchValueNative(kReadyEventNameFlag);

  base::win::ScopedHandle ready_event(
      ::OpenEvent(EVENT_MODIFY_STATE, FALSE, ready_event_name.c_str()));
  if (!ready_event.IsValid())
    return kErrorResultCode;

  base::string16 continue_event_name =
      cmd_line->GetSwitchValueNative(kContinueEventNameFlag);

  base::win::ScopedHandle continue_event(
      ::OpenEvent(SYNCHRONIZE, FALSE, continue_event_name.c_str()));
  if (!continue_event.IsValid())
    return kErrorResultCode;

  ScopedVisibleWindow visible_window;
  if (cmd_line->HasSwitch(kCreateWindowFlag)) {
    if (!visible_window.Create())
      return kErrorResultCode;
  }

  // Instantiate the process singleton.
  ProcessSingleton process_singleton(user_data_dir,
                                     base::Bind(&NotificationCallback));

  if (!process_singleton.Create())
    return kErrorResultCode;

  // Signal ready and block for the continue event.
  if (!::SetEvent(ready_event.Get()))
    return kErrorResultCode;

  if (::WaitForSingleObject(continue_event.Get(), INFINITE) != WAIT_OBJECT_0)
    return kErrorResultCode;

  return 0;
}

// This fixture is for testing the Windows platform-specific failure modes
// of rendezvous, specifically the ones where the singleton-owning process
// is hung.
class ProcessSingletonTest : public base::MultiProcessTest {
 protected:
  enum WindowOption { WITH_WINDOW, NO_WINDOW };

  ProcessSingletonTest()
      : window_option_(NO_WINDOW), should_kill_called_(false) {}

  void SetUp() override {
    ASSERT_NO_FATAL_FAILURE(base::MultiProcessTest::SetUp());

    // Drop the process finder notification timeout to one second for testing.
    old_notification_timeout_ = chrome::SetNotificationTimeoutForTesting(
        base::TimeDelta::FromSeconds(1));
  }

  void TearDown() override {
    chrome::SetNotificationTimeoutForTesting(old_notification_timeout_);

    if (browser_victim_.IsValid()) {
      EXPECT_TRUE(::SetEvent(continue_event_.Get()));
      int exit_code = 0;
      EXPECT_TRUE(browser_victim_.WaitForExit(&exit_code));
    }

    base::MultiProcessTest::TearDown();
  }

  void LaunchHungBrowserProcess(WindowOption window_option) {
    // Create a unique user data dir to rendezvous on.
    ASSERT_TRUE(user_data_dir_.CreateUniqueTempDir());

    // Create the named "ready" event, this is unique to our process.
    ready_event_name_ =
        base::StringPrintf(L"ready-event-%d", base::GetCurrentProcId());
    base::win::ScopedHandle ready_event(
        ::CreateEvent(NULL, TRUE, FALSE, ready_event_name_.c_str()));
    ASSERT_TRUE(ready_event.IsValid());

    // Create the named "continue" event, this is unique to our process.
    continue_event_name_ =
        base::StringPrintf(L"continue-event-%d", base::GetCurrentProcId());
    continue_event_.Set(
        ::CreateEvent(NULL, TRUE, FALSE, continue_event_name_.c_str()));
    ASSERT_TRUE(continue_event_.IsValid());

    window_option_ = window_option;

    base::LaunchOptions options;
    options.start_hidden = true;
    browser_victim_ =
        SpawnChildWithOptions("ProcessSingletonTestProcessMain", options);

    // Wait for the ready event (or process exit).
    HANDLE handles[] = {ready_event.Get(), browser_victim_.Handle()};
    // The wait should always return because either |ready_event| is signaled or
    // |browser_victim_| died unexpectedly or exited on error.
    DWORD result =
        ::WaitForMultipleObjects(arraysize(handles), handles, FALSE, INFINITE);
    ASSERT_EQ(WAIT_OBJECT_0, result);
  }

  base::CommandLine MakeCmdLine(const std::string& procname) override {
    base::CommandLine cmd_line = base::MultiProcessTest::MakeCmdLine(procname);

    cmd_line.AppendSwitchPath(switches::kUserDataDir, user_data_dir_.path());
    cmd_line.AppendSwitchNative(kReadyEventNameFlag, ready_event_name_);
    cmd_line.AppendSwitchNative(kContinueEventNameFlag, continue_event_name_);
    if (window_option_ == WITH_WINDOW)
      cmd_line.AppendSwitch(kCreateWindowFlag);

    return cmd_line;
  }

  void PrepareTest(WindowOption window_option, bool allow_kill) {
    ASSERT_NO_FATAL_FAILURE(LaunchHungBrowserProcess(window_option));

    // The ready event has been signalled - the process singleton is held by
    // the hung sub process.
    test_singleton_.reset(new ProcessSingleton(
        user_data_dir(), base::Bind(&NotificationCallback)));

    test_singleton_->OverrideShouldKillRemoteProcessCallbackForTesting(
        base::Bind(&ProcessSingletonTest::MockShouldKillRemoteProcess,
                   base::Unretained(this), allow_kill));
  }

  base::Process* browser_victim() { return &browser_victim_; }
  const base::FilePath& user_data_dir() const { return user_data_dir_.path(); }
  ProcessSingleton* test_singleton() const { return test_singleton_.get(); }
  bool should_kill_called() const { return should_kill_called_; }

 private:
  bool MockShouldKillRemoteProcess(bool allow_kill) {
    should_kill_called_ = true;
    return allow_kill;
  }

  base::string16 ready_event_name_;
  base::string16 continue_event_name_;

  WindowOption window_option_;
  base::ScopedTempDir user_data_dir_;
  base::Process browser_victim_;
  base::win::ScopedHandle continue_event_;

  scoped_ptr<ProcessSingleton> test_singleton_;

  base::TimeDelta old_notification_timeout_;
  bool should_kill_called_;

  DISALLOW_COPY_AND_ASSIGN(ProcessSingletonTest);
};

}  // namespace

TEST_F(ProcessSingletonTest, KillsHungBrowserWithNoWindows) {
  ASSERT_NO_FATAL_FAILURE(PrepareTest(NO_WINDOW, false));

  // As the hung browser has no visible window, it'll be killed without
  // user interaction.
  ProcessSingleton::NotifyResult notify_result =
      test_singleton()->NotifyOtherProcessOrCreate();
  ASSERT_EQ(ProcessSingleton::PROFILE_IN_USE, notify_result);

  // The should-kill callback should not have been called, as the "browser" does
  // not have visible window.
  EXPECT_FALSE(should_kill_called());

  // Verify that the hung browser has beem terminated with the
  // RESULT_CODE_HUNG exit code.
  int exit_code = 0;
  EXPECT_TRUE(
      browser_victim()->WaitForExitWithTimeout(base::TimeDelta(), &exit_code));
  EXPECT_EQ(content::RESULT_CODE_HUNG, exit_code);
}

TEST_F(ProcessSingletonTest, DoesntKillWithoutUserPermission) {
  ASSERT_NO_FATAL_FAILURE(PrepareTest(WITH_WINDOW, false));

  // As the hung browser has a visible window, this should query the user
  // before killing the hung process.
  ProcessSingleton::NotifyResult notify_result =
      test_singleton()->NotifyOtherProcessOrCreate();
  ASSERT_EQ(ProcessSingleton::PROCESS_NOTIFIED, notify_result);

  // The should-kill callback should have been called, as the "browser" has a
  // visible window.
  EXPECT_TRUE(should_kill_called());

  // Make sure the process hasn't been killed.
  int exit_code = 0;
  EXPECT_FALSE(
      browser_victim()->WaitForExitWithTimeout(base::TimeDelta(), &exit_code));
}

TEST_F(ProcessSingletonTest, KillWithUserPermission) {
  ASSERT_NO_FATAL_FAILURE(PrepareTest(WITH_WINDOW, true));

  // As the hung browser has a visible window, this should query the user
  // before killing the hung process.
  ProcessSingleton::NotifyResult notify_result =
      test_singleton()->NotifyOtherProcessOrCreate();
  ASSERT_EQ(ProcessSingleton::PROFILE_IN_USE, notify_result);

  // The should-kill callback should have been called, as the "browser" has a
  // visible window.
  EXPECT_TRUE(should_kill_called());

  // Verify that the hung browser has beem terminated with the
  // RESULT_CODE_HUNG exit code.
  int exit_code = 0;
  EXPECT_TRUE(
      browser_victim()->WaitForExitWithTimeout(base::TimeDelta(), &exit_code));
  EXPECT_EQ(content::RESULT_CODE_HUNG, exit_code);
}
