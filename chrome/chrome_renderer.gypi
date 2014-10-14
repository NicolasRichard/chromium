# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chrome_renderer_sources': [
      'renderer/benchmarking_extension.cc',
      'renderer/benchmarking_extension.h',
      'renderer/isolated_world_ids.h',
      'renderer/loadtimes_extension_bindings.cc',
      'renderer/loadtimes_extension_bindings.h',
      'renderer/media/cast_rtp_stream.cc',
      'renderer/media/cast_rtp_stream.h',
      'renderer/media/cast_ipc_dispatcher.cc',
      'renderer/media/cast_ipc_dispatcher.h',
      'renderer/media/cast_session.cc',
      'renderer/media/cast_session.h',
      'renderer/media/cast_session_delegate.cc',
      'renderer/media/cast_session_delegate.h',
      'renderer/media/cast_threads.cc',
      'renderer/media/cast_threads.h',
      'renderer/media/cast_transport_sender_ipc.cc',
      'renderer/media/cast_transport_sender_ipc.h',
      'renderer/media/cast_udp_transport.cc',
      'renderer/media/cast_udp_transport.h',
      'renderer/media/chrome_key_systems.cc',
      'renderer/media/chrome_key_systems.h',
      'renderer/net/net_error_helper.cc',
      'renderer/net/net_error_helper.h',
      'renderer/net/net_error_helper_core.cc',
      'renderer/net/net_error_helper_core.h',
      'renderer/net/net_error_page_controller.cc',
      'renderer/net/net_error_page_controller.h',
      'renderer/net/predictor_queue.cc',
      'renderer/net/predictor_queue.h',
      'renderer/net/prescient_networking_dispatcher.cc',
      'renderer/net/prescient_networking_dispatcher.h',
      'renderer/net/renderer_net_predictor.cc',
      'renderer/net/renderer_net_predictor.h',
      'renderer/net_benchmarking_extension.cc',
      'renderer/net_benchmarking_extension.h',
      'renderer/playback_extension.cc',
      'renderer/playback_extension.h',
      'renderer/principals_extension_bindings.cc',
      'renderer/principals_extension_bindings.h',
      'renderer/chrome_content_renderer_client.cc',
      'renderer/chrome_content_renderer_client.h',
      'renderer/chrome_render_frame_observer.cc',
      'renderer/chrome_render_frame_observer.h',
      'renderer/chrome_render_process_observer.cc',
      'renderer/chrome_render_process_observer.h',
      'renderer/chrome_render_view_observer.cc',
      'renderer/chrome_render_view_observer.h',
      'renderer/content_settings_observer.cc',
      'renderer/content_settings_observer.h',
      'renderer/custom_menu_commands.h',
      'renderer/external_extension.cc',
      'renderer/external_extension.h',
      'renderer/page_load_histograms.cc',
      'renderer/page_load_histograms.h',
      'renderer/plugins/chrome_plugin_placeholder.cc',
      'renderer/plugins/chrome_plugin_placeholder.h',
      'renderer/plugins/plugin_uma.cc',
      'renderer/plugins/plugin_uma.h',
      'renderer/prefetch_helper.cc',
      'renderer/prefetch_helper.h',
      'renderer/prerender/prerender_dispatcher.cc',
      'renderer/prerender/prerender_dispatcher.h',
      'renderer/prerender/prerender_extra_data.cc',
      'renderer/prerender/prerender_extra_data.h',
      'renderer/prerender/prerender_helper.cc',
      'renderer/prerender/prerender_helper.h',
      'renderer/prerender/prerenderer_client.cc',
      'renderer/prerender/prerenderer_client.h',
      'renderer/searchbox/search_bouncer.cc',
      'renderer/searchbox/search_bouncer.h',
      'renderer/searchbox/searchbox.cc',
      'renderer/searchbox/searchbox.h',
      'renderer/searchbox/searchbox_extension.cc',
      'renderer/searchbox/searchbox_extension.h',
      'renderer/security_filter_peer.cc',
      'renderer/security_filter_peer.h',
      'renderer/tts_dispatcher.cc',
      'renderer/tts_dispatcher.h',
      'renderer/web_apps.cc',
      'renderer/web_apps.h',
      'renderer/webview_color_overlay.cc',
      'renderer/webview_color_overlay.h',
      'renderer/worker_permission_client_proxy.cc',
      'renderer/worker_permission_client_proxy.h',
    ],
    'chrome_renderer_webrtc_sources': [
      'renderer/media/chrome_webrtc_log_message_delegate.cc',
      'renderer/media/chrome_webrtc_log_message_delegate.h',
      'renderer/media/webrtc_logging_message_filter.cc',
      'renderer/media/webrtc_logging_message_filter.h',
    ],
    'chrome_renderer_webrtc_extensions_sources': [
      'renderer/extensions/cast_streaming_native_handler.cc',
      'renderer/extensions/cast_streaming_native_handler.h',
    ],
    'chrome_renderer_extensions_sources': [
      'renderer/extensions/app_bindings.cc',
      'renderer/extensions/app_bindings.h',
      'renderer/extensions/automation_internal_custom_bindings.cc',
      'renderer/extensions/automation_internal_custom_bindings.h',
      'renderer/extensions/chrome_extensions_dispatcher_delegate.cc',
      'renderer/extensions/chrome_extensions_dispatcher_delegate.h',
      'renderer/extensions/chrome_extensions_renderer_client.cc',
      'renderer/extensions/chrome_extensions_renderer_client.h',
      'renderer/extensions/chrome_v8_context.cc',
      'renderer/extensions/chrome_v8_context.h',
      'renderer/extensions/chrome_v8_extension_handler.cc',
      'renderer/extensions/chrome_v8_extension_handler.h',
      'renderer/extensions/enterprise_platform_keys_natives.cc',
      'renderer/extensions/enterprise_platform_keys_natives.h',
      'renderer/extensions/extension_frame_helper.cc',
      'renderer/extensions/extension_frame_helper.h',
      'renderer/extensions/extension_localization_peer.cc',
      'renderer/extensions/extension_localization_peer.h',
      'renderer/extensions/file_browser_handler_custom_bindings.cc',
      'renderer/extensions/file_browser_handler_custom_bindings.h',
      'renderer/extensions/file_manager_private_custom_bindings.cc',
      'renderer/extensions/file_manager_private_custom_bindings.h',
      'renderer/extensions/media_galleries_custom_bindings.cc',
      'renderer/extensions/media_galleries_custom_bindings.h',
      'renderer/extensions/notifications_native_handler.cc',
      'renderer/extensions/notifications_native_handler.h',
      'renderer/extensions/page_capture_custom_bindings.cc',
      'renderer/extensions/page_capture_custom_bindings.h',
      'renderer/extensions/renderer_permissions_policy_delegate.cc',
      'renderer/extensions/renderer_permissions_policy_delegate.h',
      'renderer/extensions/resource_request_policy.cc',
      'renderer/extensions/resource_request_policy.h',
      'renderer/extensions/sync_file_system_custom_bindings.cc',
      'renderer/extensions/sync_file_system_custom_bindings.h',
      'renderer/extensions/tab_finder.cc',
      'renderer/extensions/tab_finder.h',
      'renderer/extensions/tabs_custom_bindings.cc',
      'renderer/extensions/tabs_custom_bindings.h',
      'renderer/extensions/webstore_bindings.cc',
      'renderer/extensions/webstore_bindings.h',
      'renderer/resources/extensions/app_custom_bindings.js',
      'renderer/resources/extensions/app_view.js',
      'renderer/resources/extensions/automation_custom_bindings.js',
      'renderer/resources/extensions/browser_action_custom_bindings.js',
      'renderer/resources/extensions/chrome_direct_setting.js',
      'renderer/resources/extensions/chrome_setting.js',
      'renderer/resources/extensions/content_setting.js',
      'renderer/resources/extensions/declarative_content_custom_bindings.js',
      'renderer/resources/extensions/enterprise_platform_keys_custom_bindings.js',
      'renderer/resources/extensions/extension_options.js',
      'renderer/resources/extensions/feedback_private_custom_bindings.js',
      'renderer/resources/extensions/file_browser_handler_custom_bindings.js',
      'renderer/resources/extensions/file_entry_binding_util.js',
      'renderer/resources/extensions/file_manager_private_custom_bindings.js',
      'renderer/resources/extensions/file_system_custom_bindings.js',
      'renderer/resources/extensions/file_system_provider_custom_bindings.js',
      'renderer/resources/extensions/gcm_custom_bindings.js',
      'renderer/resources/extensions/identity_custom_bindings.js',
      'renderer/resources/extensions/image_writer_private_custom_bindings.js',
      'renderer/resources/extensions/input.ime_custom_bindings.js',
      'renderer/resources/extensions/log_private_custom_bindings.js',
      'renderer/resources/extensions/notifications_custom_bindings.js',
      'renderer/resources/extensions/omnibox_custom_bindings.js',
      'renderer/resources/extensions/page_action_custom_bindings.js',
      'renderer/resources/extensions/page_capture_custom_bindings.js',
      'renderer/resources/extensions/system_indicator_custom_bindings.js',
      'renderer/resources/extensions/tts_custom_bindings.js',
      'renderer/resources/extensions/tts_engine_custom_bindings.js',
    ],
    'chrome_renderer_non_android_sources': [
      'renderer/prerender/prerender_media_load_deferrer.cc',
      'renderer/prerender/prerender_media_load_deferrer.h',
    ],
    'chrome_renderer_plugin_sources': [
      'renderer/pepper/chrome_renderer_pepper_host_factory.cc',
      'renderer/pepper/chrome_renderer_pepper_host_factory.h',
      'renderer/pepper/pepper_flash_drm_renderer_host.cc',
      'renderer/pepper/pepper_flash_drm_renderer_host.h',
      'renderer/pepper/pepper_flash_font_file_host.cc',
      'renderer/pepper/pepper_flash_font_file_host.h',
      'renderer/pepper/pepper_flash_fullscreen_host.cc',
      'renderer/pepper/pepper_flash_fullscreen_host.h',
      'renderer/pepper/pepper_flash_menu_host.cc',
      'renderer/pepper/pepper_flash_menu_host.h',
      'renderer/pepper/pepper_flash_renderer_host.cc',
      'renderer/pepper/pepper_flash_renderer_host.h',
      'renderer/pepper/pepper_helper.cc',
      'renderer/pepper/pepper_helper.h',
      'renderer/pepper/pepper_shared_memory_message_filter.cc',
      'renderer/pepper/pepper_shared_memory_message_filter.h',
      'renderer/pepper/pepper_uma_host.cc',
      'renderer/pepper/pepper_uma_host.h',
    ],
    # For safe_browsing==1 or safe_browsing==2.
    'chrome_renderer_basic_safe_browsing_sources': [
      'renderer/safe_browsing/malware_dom_details.cc',
      'renderer/safe_browsing/malware_dom_details.h',
    ],
    # For safe_browsing==1 only.
    'chrome_renderer_safe_browsing_sources': [
      'renderer/safe_browsing/feature_extractor_clock.cc',
      'renderer/safe_browsing/feature_extractor_clock.h',
      'renderer/safe_browsing/features.cc',
      'renderer/safe_browsing/features.h',
      'renderer/safe_browsing/murmurhash3_util.cc',
      'renderer/safe_browsing/murmurhash3_util.h',
      'renderer/safe_browsing/phishing_classifier.cc',
      'renderer/safe_browsing/phishing_classifier.h',
      'renderer/safe_browsing/phishing_classifier_delegate.cc',
      'renderer/safe_browsing/phishing_classifier_delegate.h',
      'renderer/safe_browsing/phishing_dom_feature_extractor.cc',
      'renderer/safe_browsing/phishing_dom_feature_extractor.h',
      'renderer/safe_browsing/phishing_term_feature_extractor.cc',
      'renderer/safe_browsing/phishing_term_feature_extractor.h',
      'renderer/safe_browsing/phishing_url_feature_extractor.cc',
      'renderer/safe_browsing/phishing_url_feature_extractor.h',
      'renderer/safe_browsing/scorer.cc',
      'renderer/safe_browsing/scorer.h',
    ],
    'chrome_renderer_spellchecker_sources': [
      'renderer/spellchecker/cocoa_spelling_engine_mac.cc',
      'renderer/spellchecker/cocoa_spelling_engine_mac.h',
      'renderer/spellchecker/custom_dictionary_engine.cc',
      'renderer/spellchecker/custom_dictionary_engine.h',
      'renderer/spellchecker/hunspell_engine.cc',
      'renderer/spellchecker/hunspell_engine.h',
      'renderer/spellchecker/spellcheck_provider.cc',
      'renderer/spellchecker/spellcheck_provider.h',
      'renderer/spellchecker/spellcheck.cc',
      'renderer/spellchecker/spellcheck.h',
      'renderer/spellchecker/spellcheck_language.cc',
      'renderer/spellchecker/spellcheck_language.h',
      'renderer/spellchecker/spellcheck_worditerator.cc',
      'renderer/spellchecker/spellcheck_worditerator.h',
      'renderer/spellchecker/spelling_engine.h',
    ],
    'chrome_renderer_printing_sources': [
      'renderer/printing/print_web_view_helper.cc',
      'renderer/printing/print_web_view_helper.h',
      'renderer/printing/print_web_view_helper_android.cc',
      'renderer/printing/print_web_view_helper_linux.cc',
      'renderer/printing/print_web_view_helper_mac.mm',
      'renderer/printing/print_web_view_helper_pdf_win.cc',
    ],
    'chrome_renderer_full_printing_sources': [
      'renderer/pepper/chrome_pdf_print_client.cc',
      'renderer/pepper/chrome_pdf_print_client.h',
    ],
  },
  'targets': [
    {
      'target_name': 'renderer',
      'type': 'static_library',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'dependencies': [
        'common',
        'common_net',
        'chrome_resources.gyp:chrome_resources',
        'chrome_resources.gyp:chrome_strings',
        '../third_party/re2/re2.gyp:re2',
        '../components/components.gyp:autofill_content_renderer',
        '../components/components.gyp:cdm_renderer',
        '../components/components.gyp:data_reduction_proxy_core_common',
        '../components/components.gyp:startup_metric_utils',
        '../components/components.gyp:password_manager_content_renderer',
        '../components/components.gyp:plugins_renderer',
        '../components/components.gyp:translate_content_renderer',
        '../components/components.gyp:visitedlink_renderer',
        '../components/components.gyp:web_cache_renderer',
        '../content/app/resources/content_resources.gyp:content_resources',
        '../content/app/strings/content_strings.gyp:content_strings',
        '../content/content.gyp:content_renderer',
        '../extensions/extensions_resources.gyp:extensions_resources',
        '../media/cast/cast.gyp:cast_logging_proto',
        '../media/cast/cast.gyp:cast_net',
        '../media/cast/cast.gyp:cast_sender',
        '../net/net.gyp:net',
        '../skia/skia.gyp:skia',
        '../third_party/WebKit/public/blink.gyp:blink',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../third_party/npapi/npapi.gyp:npapi',
        '../third_party/widevine/cdm/widevine_cdm.gyp:widevine_cdm_version_h',
        '../ui/surface/surface.gyp:surface',
      ],
      'include_dirs': [
        '..',
        # Needed by chrome_content_renderer_client.cc.
        '<(SHARED_INTERMEDIATE_DIR)',
      ],
      'sources': [
        '<@(chrome_renderer_sources)',
      ],
      'conditions': [
        ['disable_nacl!=1', {
          'dependencies': [
            '../components/nacl.gyp:nacl',
            '../components/nacl.gyp:nacl_renderer',
          ],
        }],
        ['enable_plugins==1', {
          'sources': [
            '<@(chrome_renderer_plugin_sources)',
          ],
          'dependencies': [
            '../components/components.gyp:pdf_renderer',
            '../ppapi/ppapi_internal.gyp:ppapi_host',
            '../ppapi/ppapi_internal.gyp:ppapi_proxy',
            '../ppapi/ppapi_internal.gyp:ppapi_ipc',
            '../ppapi/ppapi_internal.gyp:ppapi_shared',
          ],
        }],
        ['safe_browsing==1 or safe_browsing==2', {
          'sources': [
            '<@(chrome_renderer_basic_safe_browsing_sources)',
          ],
        }],
        ['safe_browsing==1', {
          'sources': [
            '<@(chrome_renderer_safe_browsing_sources)',
          ],
          'defines': [
            'FULL_SAFE_BROWSING',
          ],
          'dependencies': [
            'safe_browsing_proto',
            '../third_party/smhasher/smhasher.gyp:murmurhash3',
          ],
        }],
        ['safe_browsing==2', {
          'defines': [
            'MOBILE_SAFE_BROWSING',
          ],
        }],
        ['enable_extensions==1', {
          'dependencies': [
            '../extensions/extensions.gyp:extensions_renderer',
            # TODO(hclam): See crbug.com/298380 for details.
            # We should isolate the APIs needed by the renderer.
            '<(DEPTH)/chrome/common/extensions/api/api.gyp:chrome_api',
          ],
          'sources': [
            '<@(chrome_renderer_extensions_sources)',
          ],
        }],
        ['enable_webrtc==1', {
          'sources': [
            '<@(chrome_renderer_webrtc_sources)',
          ],
        }],
        ['enable_extensions==1 and enable_webrtc==1', {
          'sources': [
            '<@(chrome_renderer_webrtc_extensions_sources)',
          ],
        }],
        ['enable_spellcheck==1', {
          'sources': [
            '<@(chrome_renderer_spellchecker_sources)',
          ],
          'dependencies': [
            '../third_party/hunspell/hunspell.gyp:hunspell',
          ],
        }],
        ['OS=="mac"', {
          'dependencies': [
            '../third_party/mach_override/mach_override.gyp:mach_override',
          ],
        }],
        ['enable_printing!=0', {
          'dependencies': [
            '../printing/printing.gyp:printing',
          ],
          'sources': [
            '<@(chrome_renderer_printing_sources)',
          ],
        }],
        ['enable_printing==1', {
          'sources': [
            '<@(chrome_renderer_full_printing_sources)',
          ],
        }],
        ['OS!="android"', {
          'sources': [
            '<@(chrome_renderer_non_android_sources)',
          ],
        }],
        ['OS=="win"', {
          'dependencies': [
            '../chrome_elf/chrome_elf.gyp:chrome_elf',
            '../components/components.gyp:dom_distiller_core',  # Needed by chrome_content_renderer_client.cc.
          ],
          'include_dirs': [
            '<(DEPTH)/third_party/wtl/include',
          ],
          'conditions': [
            ['win_use_allocator_shim==1', {
              'dependencies': [
                '<(allocator_target)',
              ],
              'export_dependent_settings': [
                '<(allocator_target)',
              ],
            }],
          ],
        }],
      ],
    },
  ],
}
