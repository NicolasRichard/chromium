// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/app_list/views/start_page_view.h"

#include "base/i18n/rtl.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/app_list/app_list_constants.h"
#include "ui/app_list/app_list_item.h"
#include "ui/app_list/app_list_model.h"
#include "ui/app_list/app_list_view_delegate.h"
#include "ui/app_list/search_result.h"
#include "ui/app_list/views/all_apps_tile_item_view.h"
#include "ui/app_list/views/app_list_main_view.h"
#include "ui/app_list/views/contents_view.h"
#include "ui/app_list/views/search_box_view.h"
#include "ui/app_list/views/search_result_container_view.h"
#include "ui/app_list/views/search_result_tile_item_view.h"
#include "ui/app_list/views/tile_item_view.h"
#include "ui/gfx/canvas.h"
#include "ui/views/background.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/widget/widget.h"

namespace app_list {

namespace {

// Layout constants.
const int kInstantContainerSpacing = 24;
const int kSearchBoxAndTilesSpacing = 35;
const int kStartPageSearchBoxWidth = 480;

// WebView constants.
const int kWebViewWidth = 700;
const int kWebViewHeight = 244;

// Tile container constants.
const size_t kNumStartPageTiles = 4;
const int kTileSpacing = 7;

// An invisible placeholder view which fills the space for the search box view
// in a box layout. The search box view itself is a child of the AppListView
// (because it is visible on many different pages).
class SearchBoxSpacerView : public views::View {
 public:
  explicit SearchBoxSpacerView(const gfx::Size& search_box_size)
      : size_(kStartPageSearchBoxWidth, search_box_size.height()) {}

  ~SearchBoxSpacerView() override {}

  // Overridden from views::View:
  gfx::Size GetPreferredSize() const override { return size_; }

 private:
  gfx::Size size_;

  DISALLOW_COPY_AND_ASSIGN(SearchBoxSpacerView);
};

}  // namespace

// A container that holds the start page recommendation tiles and the all apps
// tile.
class StartPageView::StartPageTilesContainer
    : public SearchResultContainerView {
 public:
  StartPageTilesContainer(ContentsView* contents_view,
                          AllAppsTileItemView* all_apps_button);
  ~StartPageTilesContainer() override;

  TileItemView* GetTileItemView(int index);

  const std::vector<SearchResultTileItemView*>& tile_views() const {
    return search_result_tile_views_;
  }

  AllAppsTileItemView* all_apps_button() { return all_apps_button_; }

  // Overridden from SearchResultContainerView:
  int Update() override;
  void UpdateSelectedIndex(int old_selected, int new_selected) override;
  void OnContainerSelected(bool from_bottom) override;

 private:
  ContentsView* contents_view_;

  std::vector<SearchResultTileItemView*> search_result_tile_views_;
  AllAppsTileItemView* all_apps_button_;

  DISALLOW_COPY_AND_ASSIGN(StartPageTilesContainer);
};

StartPageView::StartPageTilesContainer::StartPageTilesContainer(
    ContentsView* contents_view,
    AllAppsTileItemView* all_apps_button)
    : contents_view_(contents_view), all_apps_button_(all_apps_button) {
  views::BoxLayout* tiles_layout_manager =
      new views::BoxLayout(views::BoxLayout::kHorizontal, 0, 0, kTileSpacing);
  tiles_layout_manager->set_main_axis_alignment(
      views::BoxLayout::MAIN_AXIS_ALIGNMENT_CENTER);
  SetLayoutManager(tiles_layout_manager);
  set_background(
      views::Background::CreateSolidBackground(kLabelBackgroundColor));

  // Add SearchResultTileItemViews to the container.
  for (size_t i = 0; i < kNumStartPageTiles; ++i) {
    SearchResultTileItemView* tile_item = new SearchResultTileItemView(this);
    AddChildView(tile_item);
    tile_item->SetParentBackgroundColor(kLabelBackgroundColor);
    search_result_tile_views_.push_back(tile_item);
  }

  // Also add a special "all apps" button to the end of the container.
  all_apps_button_->UpdateIcon();
  all_apps_button_->SetParentBackgroundColor(kLabelBackgroundColor);
  AddChildView(all_apps_button_);
}

StartPageView::StartPageTilesContainer::~StartPageTilesContainer() {
}

TileItemView* StartPageView::StartPageTilesContainer::GetTileItemView(
    int index) {
  DCHECK_GT(num_results(), index);
  if (index == num_results() - 1)
    return all_apps_button_;

  return search_result_tile_views_[index];
}

int StartPageView::StartPageTilesContainer::Update() {
  // Ignore updates and disable buttons when transitioning to a different
  // state.
  if (contents_view_->GetActiveState() != AppListModel::STATE_START) {
    for (auto* view : search_result_tile_views_)
      view->SetEnabled(false);

    return num_results();
  }

  std::vector<SearchResult*> display_results =
      AppListModel::FilterSearchResultsByDisplayType(
          results(), SearchResult::DISPLAY_RECOMMENDATION, kNumStartPageTiles);

  // Update the tile item results.
  for (size_t i = 0; i < search_result_tile_views_.size(); ++i) {
    SearchResult* item = NULL;
    if (i < display_results.size())
      item = display_results[i];
    search_result_tile_views_[i]->SetSearchResult(item);
    search_result_tile_views_[i]->SetEnabled(true);
  }

  Layout();
  parent()->Layout();
  // Add 1 to the results size to account for the all apps button.
  return display_results.size() + 1;
}

void StartPageView::StartPageTilesContainer::UpdateSelectedIndex(
    int old_selected,
    int new_selected) {
  if (old_selected >= 0)
    GetTileItemView(old_selected)->SetSelected(false);

  if (new_selected >= 0)
    GetTileItemView(new_selected)->SetSelected(true);
}

void StartPageView::StartPageTilesContainer::OnContainerSelected(
    bool from_bottom) {
}

////////////////////////////////////////////////////////////////////////////////
// StartPageView implementation:
StartPageView::StartPageView(AppListMainView* app_list_main_view,
                             AppListViewDelegate* view_delegate)
    : app_list_main_view_(app_list_main_view),
      view_delegate_(view_delegate),
      search_box_spacer_view_(new SearchBoxSpacerView(
          app_list_main_view->search_box_view()->GetPreferredSize())),
      instant_container_(new views::View),
      tiles_container_(new StartPageTilesContainer(
          app_list_main_view->contents_view(),
          new AllAppsTileItemView(
              app_list_main_view_->contents_view(),
              view_delegate_->GetModel()->top_level_item_list()))) {
  // The view containing the start page WebContents and SearchBoxSpacerView.
  InitInstantContainer();
  AddChildView(instant_container_);

  // The view containing the start page tiles.
  AddChildView(tiles_container_);

  tiles_container_->SetResults(view_delegate_->GetModel()->results());
  Reset();
}

StartPageView::~StartPageView() {
}

void StartPageView::InitInstantContainer() {
  views::BoxLayout* instant_layout_manager = new views::BoxLayout(
      views::BoxLayout::kVertical, 0, 0, kInstantContainerSpacing);
  instant_layout_manager->set_inside_border_insets(
      gfx::Insets(0, 0, kSearchBoxAndTilesSpacing, 0));
  instant_layout_manager->set_main_axis_alignment(
      views::BoxLayout::MAIN_AXIS_ALIGNMENT_END);
  instant_layout_manager->set_cross_axis_alignment(
      views::BoxLayout::CROSS_AXIS_ALIGNMENT_CENTER);
  instant_container_->SetLayoutManager(instant_layout_manager);

  views::View* web_view = view_delegate_->CreateStartPageWebView(
      gfx::Size(kWebViewWidth, kWebViewHeight));
  if (web_view) {
    web_view->SetFocusable(false);
    instant_container_->AddChildView(web_view);
  }

  instant_container_->AddChildView(search_box_spacer_view_);
}

void StartPageView::MaybeOpenCustomLauncherPage() {
  // Switch to the custom page.
  ContentsView* contents_view = app_list_main_view_->contents_view();
  if (!app_list_main_view_->ShouldShowCustomLauncherPage())
    return;

  int custom_page_index = contents_view->GetPageIndexForState(
      AppListModel::STATE_CUSTOM_LAUNCHER_PAGE);
  contents_view->SetActivePage(custom_page_index);
}

void StartPageView::Reset() {
  tiles_container_->Update();
}

void StartPageView::UpdateForTesting() {
  tiles_container_->Update();
}

const std::vector<SearchResultTileItemView*>& StartPageView::tile_views()
    const {
  return tiles_container_->tile_views();
}

TileItemView* StartPageView::all_apps_button() const {
  return tiles_container_->all_apps_button();
}

void StartPageView::OnShow() {
  tiles_container_->Update();
  tiles_container_->ClearSelectedIndex();
}

void StartPageView::Layout() {
  gfx::Rect bounds(GetContentsBounds());
  bounds.set_height(instant_container_->GetHeightForWidth(bounds.width()));
  instant_container_->SetBoundsRect(bounds);

  // Tiles begin where the instant container ends.
  bounds.set_y(bounds.bottom());
  bounds.set_height(tiles_container_->GetHeightForWidth(bounds.width()));
  tiles_container_->SetBoundsRect(bounds);
}

bool StartPageView::OnKeyPressed(const ui::KeyEvent& event) {
  int selected_index = tiles_container_->selected_index();
  if (selected_index >= 0 &&
      tiles_container_->GetTileItemView(selected_index)->OnKeyPressed(event))
    return true;

  const int forward_dir = base::i18n::IsRTL() ? -1 : 1;
  int dir = 0;
  switch (event.key_code()) {
    case ui::VKEY_LEFT:
      dir = -forward_dir;
      break;
    case ui::VKEY_RIGHT:
      dir = forward_dir;
      break;
    case ui::VKEY_DOWN:
      // Down selects the first tile if nothing is selected.
      if (!tiles_container_->IsValidSelectionIndex(selected_index))
        dir = 1;
      break;
    case ui::VKEY_TAB:
      dir = event.IsShiftDown() ? -1 : 1;
      break;
    default:
      break;
  }

  if (dir == 0)
    return false;

  if (!tiles_container_->IsValidSelectionIndex(selected_index)) {
    tiles_container_->SetSelectedIndex(
        dir == -1 ? tiles_container_->num_results() - 1 : 0);
    return true;
  }

  int selection_index = selected_index + dir;
  if (tiles_container_->IsValidSelectionIndex(selection_index)) {
    tiles_container_->SetSelectedIndex(selection_index);
    return true;
  }

  if (event.key_code() == ui::VKEY_TAB && selection_index == -1)
    tiles_container_->ClearSelectedIndex();  // ContentsView will handle focus.

  return false;
}

bool StartPageView::OnMousePressed(const ui::MouseEvent& event) {
  ContentsView* contents_view = app_list_main_view_->contents_view();
  if (!contents_view->GetCustomPageCollapsedBounds().Contains(event.location()))
    return false;

  MaybeOpenCustomLauncherPage();
  return true;
}

bool StartPageView::OnMouseWheel(const ui::MouseWheelEvent& event) {
  // Negative y_offset is a downward scroll.
  if (event.y_offset() < 0) {
    MaybeOpenCustomLauncherPage();
    return true;
  }

  return false;
}

void StartPageView::OnGestureEvent(ui::GestureEvent* event) {
  if (event->type() == ui::ET_GESTURE_SCROLL_BEGIN &&
      event->details().scroll_y_hint() < 0)
    MaybeOpenCustomLauncherPage();

  ContentsView* contents_view = app_list_main_view_->contents_view();
  if (event->type() == ui::ET_GESTURE_TAP &&
      contents_view->GetCustomPageCollapsedBounds().Contains(event->location()))
    MaybeOpenCustomLauncherPage();
}

void StartPageView::OnScrollEvent(ui::ScrollEvent* event) {
  // Negative y_offset is a downward scroll (or upward, if Australian Scrolling
  // is enabled).
  if (event->type() == ui::ET_SCROLL && event->y_offset() < 0)
    MaybeOpenCustomLauncherPage();
}

gfx::Rect StartPageView::GetSearchBoxBounds() const {
  return search_box_spacer_view_->bounds();
}

TileItemView* StartPageView::GetTileItemView(size_t index) {
  return tiles_container_->GetTileItemView(index);
}

}  // namespace app_list
