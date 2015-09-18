# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'extensions_browsertests_sources': [
      'browser/api/audio/audio_apitest.cc',
      'browser/api/bluetooth_socket/bluetooth_socket_apitest.cc',
      'browser/api/dns/dns_apitest.cc',
      'browser/api/hid/hid_apitest.cc',
      'browser/api/printer_provider/printer_provider_apitest.cc',
      'browser/api/socket/socket_apitest.cc',
      'browser/api/sockets_tcp/sockets_tcp_apitest.cc',
      'browser/api/sockets_tcp_server/sockets_tcp_server_apitest.cc',
      'browser/api/sockets_udp/sockets_udp_apitest.cc',
      'browser/api/system_cpu/system_cpu_apitest.cc',
      'browser/api/system_display/system_display_apitest.cc',
      'browser/api/system_memory/system_memory_apitest.cc',
      'browser/api/system_network/system_network_apitest.cc',
      'browser/api/system_storage/storage_api_test_util.cc',
      'browser/api/system_storage/storage_api_test_util.h',
      'browser/api/system_storage/system_storage_apitest.cc',
      'browser/api/system_storage/system_storage_eject_apitest.cc',
      'browser/api/usb/usb_apitest.cc',
      'browser/guest_view/app_view/app_view_apitest.cc',
      'browser/guest_view/web_view/web_view_apitest.cc',
      'browser/guest_view/web_view/web_view_apitest.h',
      'browser/guest_view/web_view/web_view_media_access_apitest.cc',
      'browser/updater/update_service_browsertest.cc',
      'shell/browser/geolocation/geolocation_apitest.cc',
      'shell/browser/shell_browsertest.cc',
      'shell/test/shell_apitest.cc',
      'shell/test/shell_apitest.h',
      'shell/test/shell_test.cc',
      'shell/test/shell_test.h',
      'shell/test/shell_test_launcher_delegate.cc',
      'shell/test/shell_test_launcher_delegate.h',
      'shell/test/shell_tests_main.cc',
    ],
    'extensions_unittests_sources': [
      'browser/api/alarms/alarms_api_unittest.cc',
      'browser/api/api_resource_manager_unittest.cc',
      'browser/api/bluetooth/bluetooth_event_router_unittest.cc',
      'browser/api/cast_channel/cast_auth_ica_unittest.cc',
      'browser/api/cast_channel/cast_auth_util_unittest.cc',
      'browser/api/cast_channel/cast_channel_api_unittest.cc',
      'browser/api/cast_channel/cast_framer_unittest.cc',
      'browser/api/cast_channel/cast_socket_unittest.cc',
      'browser/api/cast_channel/cast_transport_unittest.cc',
      'browser/api/cast_channel/keep_alive_delegate_unittest.cc',
      'browser/api/cast_channel/logger_unittest.cc',
      'browser/api/declarative/declarative_rule_unittest.cc',
      'browser/api/declarative/deduping_factory_unittest.cc',
      'browser/api/declarative/rules_registry_unittest.cc',
      'browser/api/declarative_webrequest/webrequest_condition_attribute_unittest.cc',
      'browser/api/declarative_webrequest/webrequest_condition_unittest.cc',
      'browser/api/device_permissions_prompt_unittest.cc',
      'browser/api/document_scan/document_scan_api_unittest.cc',
      'browser/api/document_scan/document_scan_interface_chromeos_unittest.cc',
      'browser/api/document_scan/mock_document_scan_interface.cc',
      'browser/api/idle/idle_api_unittest.cc',
      'browser/api/mime_handler_private/mime_handler_private_unittest.cc',
      'browser/api/networking_config/networking_config_service_chromeos_unittest.cc',
      'browser/api/power/power_api_unittest.cc',
      'browser/api/sockets_tcp/sockets_tcp_api_unittest.cc',
      'browser/api/sockets_udp/sockets_udp_api_unittest.cc',
      'browser/api/storage/settings_quota_unittest.cc',
      'browser/api/storage/storage_api_unittest.cc',
      'browser/api/storage/storage_frontend_unittest.cc',
      'browser/api/web_request/form_data_parser_unittest.cc',
      'browser/api/web_request/upload_data_presenter_unittest.cc',
      'browser/api/web_request/web_request_time_tracker_unittest.cc',
      'browser/app_window/app_window_geometry_cache_unittest.cc',
      'browser/computed_hashes_unittest.cc',
      'browser/content_hash_tree_unittest.cc',
      'browser/error_map_unittest.cc',
      'browser/event_listener_map_unittest.cc',
      'browser/event_router_unittest.cc',
      'browser/extension_icon_image_unittest.cc',
      'browser/extension_pref_value_map_unittest.cc',
      'browser/extension_registry_unittest.cc',
      'browser/file_highlighter_unittest.cc',
      'browser/file_reader_unittest.cc',
      'browser/image_loader_unittest.cc',
      'browser/info_map_unittest.cc',
      'browser/lazy_background_task_queue_unittest.cc',
      'browser/load_monitoring_extension_host_queue_unittest.cc',
      'browser/management_policy_unittest.cc',
      'browser/mojo/keep_alive_impl_unittest.cc',
      'browser/mojo/stash_backend_unittest.cc',
      'browser/process_manager_unittest.cc',
      'browser/process_map_unittest.cc',
      'browser/quota_service_unittest.cc',
      'browser/runtime_data_unittest.cc',
      'browser/sandboxed_unpacker_unittest.cc',
      'browser/extension_throttle_simulation_unittest.cc',
      'browser/extension_throttle_test_support.cc',
      'browser/extension_throttle_test_support.h',
      'browser/extension_throttle_unittest.cc',
      'browser/value_store/leveldb_value_store_unittest.cc',
      'browser/value_store/testing_value_store_unittest.cc',
      'browser/value_store/value_store_change_unittest.cc',
      'browser/value_store/value_store_frontend_unittest.cc',
      'browser/value_store/value_store_unittest.cc',
      'browser/value_store/value_store_unittest.h',
      'browser/verified_contents_unittest.cc',
      'browser/warning_service_unittest.cc',
      'common/api/declarative/declarative_manifest_unittest.cc',
      'common/api/printer_provider/usb_printer_manifest_unittest.cc',
      'common/api/sockets/sockets_manifest_permission_unittest.cc',
      'common/csp_validator_unittest.cc',
      'common/event_filter_unittest.cc',
      'common/extension_l10n_util_unittest.cc',
      'common/extension_resource_unittest.cc',
      'common/extension_set_unittest.cc',
      'common/features/base_feature_provider_unittest.cc',
      'common/features/complex_feature_unittest.cc',
      'common/features/simple_feature_unittest.cc',
      'common/file_util_unittest.cc',
      'common/image_util_unittest.cc',
      'common/manifest_handler_unittest.cc',
      'common/manifest_handlers/content_capabilities_manifest_unittest.cc',
      'common/manifest_handlers/default_locale_manifest_unittest.cc',
      'common/manifest_handlers/externally_connectable_unittest.cc',
      'common/manifest_handlers/file_handler_manifest_unittest.cc',
      'common/manifest_handlers/incognito_manifest_unittest.cc',
      'common/manifest_handlers/kiosk_mode_info_unittest.cc',
      'common/manifest_handlers/oauth2_manifest_unittest.cc',
      'common/manifest_handlers/shared_module_manifest_unittest.cc',
      'common/message_bundle_unittest.cc',
      'common/one_shot_event_unittest.cc',
      'common/permissions/api_permission_set_unittest.cc',
      'common/permissions/manifest_permission_set_unittest.cc',
      'common/permissions/socket_permission_unittest.cc',
      'common/permissions/usb_device_permission_unittest.cc',
      'common/stack_frame_unittest.cc',
      'common/url_pattern_set_unittest.cc',
      'common/url_pattern_unittest.cc',
      'common/user_script_unittest.cc',
      'common/update_manifest_unittest.cc',
      'renderer/activity_log_converter_strategy_unittest.cc',
      'renderer/api/mojo_private/mojo_private_unittest.cc',
      'renderer/api/serial/data_receiver_unittest.cc',
      'renderer/api/serial/data_sender_unittest.cc',
      'renderer/api/serial/serial_api_unittest.cc',
      'renderer/api_test_base.cc',
      'renderer/api_test_base.h',
      'renderer/api_test_base_unittest.cc',
      'renderer/event_unittest.cc',
      'renderer/gc_callback_unittest.cc',
      'renderer/json_schema_unittest.cc',
      'renderer/messaging_utils_unittest.cc',
      'renderer/module_system_test.cc',
      'renderer/module_system_test.h',
      'renderer/module_system_unittest.cc',
      'renderer/mojo/keep_alive_client_unittest.cc',
      'renderer/mojo/stash_client_unittest.cc',
      'renderer/safe_builtins_unittest.cc',
      'renderer/scoped_web_frame.cc',
      'renderer/scoped_web_frame.h',
      'renderer/script_context_set_unittest.cc',
      'renderer/script_context_unittest.cc',
      'renderer/utils_unittest.cc',
      'shell/browser/shell_web_contents_modal_dialog_manager.cc',
      'test/extensions_unittests_main.cc',
      'utility/unpacker_unittest.cc',
    ],
  },
}
