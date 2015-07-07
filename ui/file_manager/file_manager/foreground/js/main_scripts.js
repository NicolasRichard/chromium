// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The include directives are put into Javascript-style comments to prevent
// parsing errors in non-flattened mode. The flattener still sees them.
// Note that this makes the flattener to comment out the first line of the
// included file but that's all right since any javascript file should start
// with a copyright comment anyway.

// If you add a new dependency, you should update build files by rerunning
// gyp. Otherwise, you'll be bitten by a dependency issue like:
//
// 1) You add a new dependency to "whatever.js"
// 2) You make changes in "whatever.js"
// 3) Rebuild "resources.pak" and open Files.app
// 4) You don't see the changes in "whatever.js". Why is that?
//
// Because the dependencies are computed at gyp time, the existing build
// files don't know that "resources.pak" now has a dependency to
// "whatever.js". You should rerun gyp to let the build files know.
//
// metrics_base.js and metrics.js initiates load performance tracking
// so we want to parse it as early as possible.
//<include src="../../common/js/metrics_base.js">
//<include src="../../common/js/metrics.js">
//<include src="metrics_start.js">
//
//<include src="../../common/js/lru_cache.js">
//<include src="../../../image_loader/image_loader_client.js">
//
//<include src="../../../../webui/resources/js/load_time_data.js">
//<include src="../../../../webui/resources/js/cr.js">
//<include src="../../../../webui/resources/js/util.js">
//<include src="../../../../webui/resources/js/i18n_template_no_process.js">
//
//<include src="../../../../webui/resources/js/event_tracker.js">
//<include src="../../../../webui/resources/js/cr/ui.js">
//<include src="../../../../webui/resources/js/cr/event_target.js">
//<include src="../../../../webui/resources/js/cr/ui/touch_handler.js">
//<include src="../../../../webui/resources/js/cr/ui/array_data_model.js">
//<include src="../../../../webui/resources/js/cr/ui/dialogs.js">
//<include src="../../../../webui/resources/js/cr/ui/list_item.js">
//<include src="../../../../webui/resources/js/cr/ui/list_selection_model.js">
//<include src="../../../../webui/resources/js/cr/ui/list_single_selection_model.js">
//<include src="../../../../webui/resources/js/cr/ui/list_selection_controller.js">
//<include src="../../../../webui/resources/js/cr/ui/list.js">
//<include src="../../../../webui/resources/js/cr/ui/tree.js">
//<include src="../../../../webui/resources/js/cr/ui/autocomplete_list.js">

//<include src="../../../../webui/resources/js/cr/ui/splitter.js">
//<include src="../../../../webui/resources/js/cr/ui/table/table_splitter.js">
//
//<include src="../../../../webui/resources/js/cr/ui/table/table_column.js">
//<include src="../../../../webui/resources/js/cr/ui/table/table_column_model.js">
//<include src="../../../../webui/resources/js/cr/ui/table/table_header.js">
//<include src="../../../../webui/resources/js/cr/ui/table/table_list.js">
//<include src="../../../../webui/resources/js/cr/ui/table.js">
//
//<include src="../../../../webui/resources/js/cr/ui/grid.js">
//
//<include src="../../../../webui/resources/js/cr/ui/command.js">
//<include src="../../../../webui/resources/js/cr/ui/position_util.js">
//<include src="../../../../webui/resources/js/cr/ui/menu_item.js">
//<include src="../../../../webui/resources/js/cr/ui/menu.js">
//<include src="../../../../webui/resources/js/cr/ui/menu_button.js">
//<include src="../../../../webui/resources/js/cr/ui/context_menu_handler.js">
//
//<include src="../../../../webui/resources/js/analytics.js">
// metrics_events.js must be loaded after the analytics package.
//<include src="../../common/js/metrics_events.js">

(function() {
// 'strict mode' is invoked for this scope.
'use strict';

// error_util.js must be loaded before all other Files.app's scripts.
//<include src="../../common/js/error_util.js">
//
//<include src="../../common/js/async_util.js">
//<include src="../../common/js/file_type.js">
//<include src="../../common/js/volume_manager_common.js">
//<include src="../../common/js/util.js">
//<include src="../../common/js/progress_center_common.js">
//<include src="../../common/js/importer_common.js">
//
//<include src="ui/combobutton.js">
//<include src="ui/commandbutton.js">
//<include src="ui/file_manager_dialog_base.js">
//<include src="metadata/metadata_cache_set.js">
//<include src="metadata/new_metadata_provider.js">
//
//<include src="../../../../../components/chrome_apps/webstore_widget/cws_widget/app_installer.js">
//<include src="../../../../../components/chrome_apps/webstore_widget/cws_widget/cws_webview_client.js">
//<include src="../../../../../components/chrome_apps/webstore_widget/cws_widget/cws_widget_container.js">
//<include src="../../../../../components/chrome_apps/webstore_widget/cws_widget/cws_widget_container_error_dialog.js">
//
//<include src="app_state_controller.js">
//<include src="column_visibility_controller.js">
//<include src="dialog_action_controller.js">
//<include src="dialog_type.js">
//<include src="directory_contents.js">
//<include src="directory_model.js">
//<include src="empty_folder_controller.js">
//<include src="file_manager.js">
//<include src="file_manager_commands.js">
//<include src="file_selection.js">
//<include src="file_tasks.js">
//<include src="file_transfer_controller.js">
//<include src="file_watcher.js">
//<include src="folder_shortcuts_data_model.js">
//<include src="sort_menu_controller.js">
//<include src="gear_menu_controller.js">
//<include src="import_controller.js">
//<include src="launch_param.js">
//<include src="metadata/content_metadata_provider.js">
//<include src="metadata/external_metadata_provider.js">
//<include src="metadata/file_system_metadata_provider.js">
//<include src="metadata/metadata_cache_item.js">
//<include src="metadata/metadata_item.js">
//<include src="metadata/metadata_model.js">
//<include src="metadata/multi_metadata_provider.js">
//<include src="metadata/thumbnail_model.js">
//<include src="metadata_update_controller.js">
//<include src="naming_controller.js">
//<include src="navigation_list_model.js">
//<include src="progress_center_item_group.js">
//<include src="scan_controller.js">
//<include src="search_controller.js">
//<include src="share_client.js">
//<include src="spinner_controller.js">
//<include src="task_controller.js">
//<include src="toolbar_controller.js">
//<include src="tooltip_controller.js">
//<include src="thumbnail_loader.js">
//<include src="list_thumbnail_loader.js">
//<include src="providers_model.js">
//<include src="ui/banners.js" >
//<include src="ui/conflict_dialog.js">
//<include src="ui/default_action_dialog.js">
//<include src="ui/dialog_footer.js">
//<include src="ui/directory_tree.js">
//<include src="ui/drag_selector.js">
//<include src="ui/empty_folder.js">
//<include src="ui/error_dialog.js">
//<include src="ui/file_grid.js">
//<include src="ui/file_manager_ui.js">
//<include src="ui/file_list_selection_model.js">
//<include src="ui/file_table.js">
//<include src="ui/file_table_list.js">
//<include src="ui/gear_menu.js">
//<include src="ui/list_container.js">
//<include src="ui/location_line.js">
//<include src="ui/multi_profile_share_dialog.js">
//<include src="ui/progress_center_panel.js">
//<include src="ui/providers_menu.js">
//<include src="ui/scrollbar.js">
//<include src="ui/search_box.js">
//<include src="ui/share_dialog.js">
//<include src="ui/suggest_apps_dialog.js">
//<include src="main_window_component.js">
//<include src="volume_manager_wrapper.js">

// // For accurate load performance tracking place main.js should be
// // the last include to include.
//<include src="main.js">

// Global fileManager reference useful for poking at from the console.
window.fileManager = fileManager;

// Exports
window.util = util;
})();
