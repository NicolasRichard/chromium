{
  'variables': {
    # Product name is used for Mac bundle.
    'app_shell_product_name': 'App Shell',
    # The version is high enough to be supported by Omaha (at least 31)
    # but fake enough to be obviously not a Chrome release.
    'app_shell_version': '38.1234.5678.9',
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'extensions_unittests',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:test_support_base',
        '../components/components.gyp:keyed_service_content',
        '../components/components.gyp:user_prefs',
        '../content/content_shell_and_tests.gyp:test_support_content',
        '../device/bluetooth/bluetooth.gyp:device_bluetooth_mocks',
        '../device/serial/serial.gyp:device_serial',
        '../device/serial/serial.gyp:device_serial_test_util',
        '../mojo/mojo_base.gyp:mojo_environment_chromium',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../third_party/leveldatabase/leveldatabase.gyp:leveldatabase',
        '../third_party/mojo/mojo_edk.gyp:mojo_js_lib',
        '../third_party/mojo/mojo_edk.gyp:mojo_system_impl',
        '../third_party/mojo/mojo_public.gyp:mojo_cpp_bindings',
        '../third_party/mojo/mojo_public.gyp:mojo_application_bindings',
        'common/api/api.gyp:cast_channel_proto',
        'extensions.gyp:extensions_common',
        'extensions.gyp:extensions_renderer',
        'extensions.gyp:extensions_shell_and_test_pak',
        'extensions.gyp:extensions_test_support',
        'extensions.gyp:extensions_utility',
        'extensions_resources.gyp:extensions_resources',
        'extensions_strings.gyp:extensions_strings',
      ],
      # Needed for third_party libraries like leveldb.
      'include_dirs': [
        '..',
      ],
      'sources': [
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
        'browser/api/cast_channel/test_util.cc',
        'browser/api/cast_channel/test_util.h',
        'browser/api/declarative/declarative_rule_unittest.cc',
        'browser/api/declarative/deduping_factory_unittest.cc',
        'browser/api/declarative/rules_registry_unittest.cc',
        'browser/api/declarative_webrequest/webrequest_condition_unittest.cc',
        'browser/api/declarative_webrequest/webrequest_condition_attribute_unittest.cc',
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
        'browser/event_listener_map_unittest.cc',
        'browser/event_router_unittest.cc',
        'browser/error_map_unittest.cc',
        'browser/extension_icon_image_unittest.cc',
        'browser/extension_pref_value_map_unittest.cc',
        'browser/extension_registry_unittest.cc',
        'browser/file_highlighter_unittest.cc',
        'browser/file_reader_unittest.cc',
        'browser/guest_view/guest_view_manager_unittest.cc',
        'browser/image_loader_unittest.cc',
        'browser/info_map_unittest.cc',
        'browser/lazy_background_task_queue_unittest.cc',
        'browser/management_policy_unittest.cc',
        'browser/mojo/keep_alive_impl_unittest.cc',
        'browser/mojo/stash_backend_unittest.cc',
        'browser/process_manager_unittest.cc',
        'browser/process_map_unittest.cc',
        'browser/quota_service_unittest.cc',
        'browser/runtime_data_unittest.cc',
        'browser/sandboxed_unpacker_unittest.cc',
        'browser/value_store/leveldb_value_store_unittest.cc',
        'browser/value_store/testing_value_store_unittest.cc',
        'browser/value_store/value_store_change_unittest.cc',
        'browser/value_store/value_store_frontend_unittest.cc',
        'browser/value_store/value_store_unittest.cc',
        'browser/value_store/value_store_unittest.h',
        'browser/verified_contents_unittest.cc',
        'browser/warning_service_unittest.cc',
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
        'renderer/activity_log_converter_strategy_unittest.cc',
        'renderer/api/mojo_private/mojo_private_unittest.cc',
        'renderer/api/serial/data_receiver_unittest.cc',
        'renderer/api/serial/data_sender_unittest.cc',
        'renderer/api/serial/serial_api_unittest.cc',
        'renderer/api_test_base.cc',
        'renderer/api_test_base.h',
        'renderer/api_test_base_unittest.cc',
        'renderer/event_unittest.cc',
        'renderer/json_schema_unittest.cc',
        'renderer/mojo/keep_alive_client_unittest.cc',
        'renderer/messaging_utils_unittest.cc',
        'renderer/module_system_test.cc',
        'renderer/module_system_test.h',
        'renderer/module_system_unittest.cc',
        'renderer/safe_builtins_unittest.cc',
        'renderer/script_context_set_unittest.cc',
        'renderer/script_context_unittest.cc',
        'renderer/utils_unittest.cc',
        'test/extensions_unittests_main.cc',
        'utility/unpacker_unittest.cc',
      ],
      # Disable c4267 warnings until we fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
      ],
    },
    {
      # GN version: //extensions:extensions_browsertests
      'target_name': 'extensions_browsertests',
      'type': '<(gtest_target_type)',
      'dependencies': [
        'extensions.gyp:extensions_test_support',
        'shell/app_shell.gyp:app_shell_lib',
        # TODO(yoz): find the right deps
        '<(DEPTH)/base/base.gyp:test_support_base',
        '<(DEPTH)/content/content.gyp:content_app_both',
        '<(DEPTH)/content/content_shell_and_tests.gyp:content_browser_test_support',
        '<(DEPTH)/content/content_shell_and_tests.gyp:test_support_content',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/testing/gtest.gyp:gtest',
      ],
      'defines': [
        'HAS_OUT_OF_PROC_TEST_RUNNER',
      ],
      'sources': [
        'browser/api/audio/audio_apitest.cc',
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
        'browser/guest_view/web_view/web_view_apitest.h',
        'browser/guest_view/web_view/web_view_apitest.cc',
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
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="mac"', {
          'dependencies': [
            'shell/app_shell.gyp:app_shell',  # Needed for App Shell.app's Helper.
          ],
        }],
        # This is only here to keep gyp happy. This target never builds on
        # mobile platforms.
        ['OS != "ios" and OS != "android"', {
          'dependencies': [
            '<(DEPTH)/components/components.gyp:storage_monitor_test_support',
          ],
        }],
      ]
    },
  ],
  'conditions': [
    ['test_isolation_mode != "noop"', {
      'targets': [
        {
          'target_name': 'extensions_browsertests_run',
          'type': 'none',
          'dependencies': [
            'extensions_browsertests',
          ],
          'includes': [
            '../build/isolate.gypi',
          ],
          'sources': [
            'extensions_browsertests.isolate',
          ],
        },
      ],
    }],
  ],
}
