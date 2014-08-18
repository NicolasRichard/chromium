// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "athena/wm/public/window_manager.h"

#include "athena/screen/public/screen_manager.h"
#include "athena/test/athena_test_base.h"
#include "athena/wm/public/window_list_provider.h"
#include "athena/wm/split_view_controller.h"
#include "athena/wm/window_manager_impl.h"
#include "ui/aura/client/window_tree_client.h"
#include "ui/aura/test/test_window_delegate.h"
#include "ui/aura/window.h"
#include "ui/base/hit_test.h"
#include "ui/events/test/event_generator.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"
#include "ui/wm/core/window_util.h"

namespace {

scoped_ptr<aura::Window> CreateWindow(aura::WindowDelegate* delegate) {
  scoped_ptr<aura::Window> window(new aura::Window(delegate));
  window->SetType(ui::wm::WINDOW_TYPE_NORMAL);
  window->Init(aura::WINDOW_LAYER_SOLID_COLOR);
  window->Show();
  return window.Pass();
}

}  // namespace

namespace athena {

class WindowManagerImplTestApi {
 public:
  WindowManagerImplTestApi()
      : wm_(static_cast<WindowManagerImpl*>(WindowManager::GetInstance())) {}
  ~WindowManagerImplTestApi() {}

  WindowManager* wm() { return wm_; }

  WindowListProvider* window_list_provider() {
    return wm_->window_list_provider_.get();
  }

  SplitViewController* split_view_controller() {
    return wm_->split_view_controller_.get();
  }

 private:
  WindowManagerImpl* wm_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerImplTestApi);
};

typedef test::AthenaTestBase WindowManagerTest;

TEST_F(WindowManagerTest, Empty) {
}

TEST_F(WindowManagerTest, OverviewModeBasics) {
  aura::test::TestWindowDelegate delegate;
  scoped_ptr<aura::Window> first(CreateWindow(&delegate));
  scoped_ptr<aura::Window> second(CreateWindow(&delegate));

  WindowManagerImplTestApi wm_api;
  aura::client::ParentWindowWithContext(
      first.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      second.get(), ScreenManager::Get()->GetContext(), gfx::Rect());

  ASSERT_FALSE(wm_api.wm()->IsOverviewModeActive());
  EXPECT_EQ(first->bounds().ToString(), second->bounds().ToString());
  EXPECT_EQ(gfx::Screen::GetNativeScreen()
                ->GetPrimaryDisplay()
                .work_area()
                .size()
                .ToString(),
            first->bounds().size().ToString());
  EXPECT_FALSE(wm_api.wm()->IsOverviewModeActive());

  // Tests that going into overview mode does not change the window bounds.
  wm_api.wm()->ToggleOverview();
  ASSERT_TRUE(wm_api.wm()->IsOverviewModeActive());
  EXPECT_EQ(first->bounds().ToString(), second->bounds().ToString());
  EXPECT_EQ(gfx::Screen::GetNativeScreen()
                ->GetPrimaryDisplay()
                .work_area()
                .size()
                .ToString(),
            first->bounds().size().ToString());
}

TEST_F(WindowManagerTest, BezelGestureToSplitViewMode) {
  aura::test::TestWindowDelegate delegate;
  scoped_ptr<aura::Window> first(CreateWindow(&delegate));
  scoped_ptr<aura::Window> second(CreateWindow(&delegate));
  scoped_ptr<aura::Window> third(CreateWindow(&delegate));

  WindowManagerImplTestApi wm_api;
  aura::client::ParentWindowWithContext(
      first.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      second.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      third.get(), ScreenManager::Get()->GetContext(), gfx::Rect());

  // Test that going into split-view mode with two-finger gesture selects the
  // correct windows on left and right splits.
  ui::test::EventGenerator generator(root_window());
  const gfx::Point start_points[2] = {
      gfx::Point(2, 10), gfx::Point(4, 20),
  };
  const int kEventTimeSepration = 16;
  int x_middle = root_window()->bounds().width() / 2;
  generator.GestureMultiFingerScroll(
      2, start_points, kEventTimeSepration, 1, x_middle, 0);
  ASSERT_TRUE(wm_api.split_view_controller()->IsSplitViewModeActive());
  EXPECT_EQ(second.get(), wm_api.split_view_controller()->left_window());
  EXPECT_EQ(third.get(), wm_api.split_view_controller()->right_window());
  EXPECT_EQ(second->bounds().size().ToString(),
            third->bounds().size().ToString());
}

TEST_F(WindowManagerTest, BezelGestureToSwitchBetweenWindows) {
  aura::test::TestWindowDelegate delegate;
  scoped_ptr<aura::Window> first(CreateWindow(&delegate));
  scoped_ptr<aura::Window> second(CreateWindow(&delegate));
  scoped_ptr<aura::Window> third(CreateWindow(&delegate));

  WindowManagerImplTestApi wm_api;
  aura::client::ParentWindowWithContext(
      first.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      second.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      third.get(), ScreenManager::Get()->GetContext(), gfx::Rect());

  EXPECT_EQ(third.get(), wm_api.window_list_provider()->GetWindowList().back());

  // Do a two-finger swipe from the left bezel.
  ui::test::EventGenerator generator(root_window());
  const gfx::Point left_bezel_points[2] = {
      gfx::Point(2, 10), gfx::Point(4, 20),
  };
  const int kEventTimeSepration = 16;
  int width = root_window()->bounds().width();
  generator.GestureMultiFingerScroll(
      2, left_bezel_points, kEventTimeSepration, 1, width, 0);
  EXPECT_TRUE(wm::IsActiveWindow(second.get()));
  EXPECT_EQ(second.get(),
            wm_api.window_list_provider()->GetWindowList().back());
}

TEST_F(WindowManagerTest, TitleDragSwitchBetweenWindows) {
  aura::test::TestWindowDelegate delegate;
  delegate.set_window_component(HTCAPTION);
  scoped_ptr<aura::Window> first(CreateWindow(&delegate));
  scoped_ptr<aura::Window> second(CreateWindow(&delegate));
  scoped_ptr<aura::Window> third(CreateWindow(&delegate));

  WindowManagerImplTestApi wm_api;
  aura::client::ParentWindowWithContext(
      first.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      second.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      third.get(), ScreenManager::Get()->GetContext(), gfx::Rect());

  EXPECT_EQ(third.get(), wm_api.window_list_provider()->GetWindowList().back());

  // Do a title-swipe from the top to switch to the previous window.
  ui::test::EventGenerator generator(root_window());
  generator.GestureScrollSequence(gfx::Point(20, 10),
                                  gfx::Point(20, 400),
                                  base::TimeDelta::FromMilliseconds(20),
                                  5);
  EXPECT_TRUE(wm::IsActiveWindow(second.get()));
  EXPECT_EQ(second.get(),
            wm_api.window_list_provider()->GetWindowList().back());

  // Performing the same gesture again will switch back to |third|.
  generator.GestureScrollSequence(gfx::Point(20, 10),
                                  gfx::Point(20, 400),
                                  base::TimeDelta::FromMilliseconds(20),
                                  5);
  EXPECT_TRUE(wm::IsActiveWindow(third.get()));
  EXPECT_EQ(third.get(), wm_api.window_list_provider()->GetWindowList().back());
}

TEST_F(WindowManagerTest, TitleDragSwitchBetweenWindowsInSplitViewMode) {
  aura::test::TestWindowDelegate delegate;
  delegate.set_window_component(HTCAPTION);
  scoped_ptr<aura::Window> first(CreateWindow(&delegate));
  scoped_ptr<aura::Window> second(CreateWindow(&delegate));
  scoped_ptr<aura::Window> third(CreateWindow(&delegate));
  scoped_ptr<aura::Window> fourth(CreateWindow(&delegate));

  WindowManagerImplTestApi wm_api;
  aura::client::ParentWindowWithContext(
      first.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      second.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      third.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  aura::client::ParentWindowWithContext(
      fourth.get(), ScreenManager::Get()->GetContext(), gfx::Rect());

  // Test that going into split-view mode with two-finger gesture selects the
  // correct windows on left and right splits.
  ui::test::EventGenerator generator(root_window());
  const gfx::Point start_points[2] = {
      gfx::Point(2, 10), gfx::Point(4, 20),
  };
  const int kEventTimeSepration = 16;
  int x_middle = root_window()->bounds().width() / 2;
  generator.GestureMultiFingerScroll(
      2, start_points, kEventTimeSepration, 1, x_middle, 0);
  ASSERT_TRUE(wm_api.split_view_controller()->IsSplitViewModeActive());
  EXPECT_EQ(third.get(), wm_api.split_view_controller()->left_window());
  EXPECT_EQ(fourth.get(), wm_api.split_view_controller()->right_window());

  // Swipe the title of the left window. It should switch to |second|.
  generator.GestureScrollSequence(gfx::Point(20, 10),
                                  gfx::Point(20, 400),
                                  base::TimeDelta::FromMilliseconds(20),
                                  5);
  EXPECT_EQ(second.get(), wm_api.split_view_controller()->left_window());
  EXPECT_EQ(fourth.get(), wm_api.split_view_controller()->right_window());
  aura::Window::Windows windows =
      wm_api.window_list_provider()->GetWindowList();
  ASSERT_EQ(4u, windows.size());
  EXPECT_EQ(second.get(), windows[3]);
  EXPECT_EQ(third.get(), windows[2]);
  EXPECT_EQ(fourth.get(), windows[1]);

  // Swipe the title of the right window now. It should switch to |third|.
  generator.GestureScrollSequence(gfx::Point(x_middle + 20, 10),
                                  gfx::Point(x_middle + 20, 400),
                                  base::TimeDelta::FromMilliseconds(20),
                                  5);
  EXPECT_EQ(second.get(), wm_api.split_view_controller()->left_window());
  EXPECT_EQ(third.get(), wm_api.split_view_controller()->right_window());
}

TEST_F(WindowManagerTest, NewWindowBounds) {
  aura::test::TestWindowDelegate delegate;
  scoped_ptr<aura::Window> first(CreateWindow(&delegate));

  WindowManagerImplTestApi wm_api;
  aura::client::ParentWindowWithContext(
      first.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  // The window should have the same size as the container.
  const gfx::Size work_area =
      gfx::Screen::GetNativeScreen()->GetPrimaryDisplay().work_area().size();
  EXPECT_EQ(work_area.ToString(),
            first->bounds().size().ToString());
  EXPECT_TRUE(first->bounds().origin().IsOrigin());

  // A second window should have the same bounds as the first one.
  scoped_ptr<aura::Window> second(CreateWindow(&delegate));
  aura::client::ParentWindowWithContext(
      second.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  EXPECT_EQ(first->bounds().ToString(), second->bounds().ToString());

  // Get into split view.
  wm_api.split_view_controller()->ActivateSplitMode(NULL, NULL);
  const gfx::Rect left_bounds =
      wm_api.split_view_controller()->left_window()->bounds();
  EXPECT_NE(work_area.ToString(),
            left_bounds.size().ToString());

  scoped_ptr<aura::Window> third(CreateWindow(&delegate));
  aura::client::ParentWindowWithContext(
      third.get(), ScreenManager::Get()->GetContext(), gfx::Rect());
  EXPECT_NE(wm_api.split_view_controller()->left_window(), third.get());
  EXPECT_EQ(left_bounds.ToString(), third->bounds().ToString());

  third->Hide();
  EXPECT_EQ(left_bounds.ToString(),
            wm_api.split_view_controller()->left_window()->bounds().ToString());
}

}  // namespace athena
