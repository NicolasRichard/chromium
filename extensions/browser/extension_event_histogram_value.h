// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_EXTENSION_EVENT_HISTOGRAM_VALUE_H_
#define EXTENSIONS_BROWSER_EXTENSION_EVENT_HISTOGRAM_VALUE_H_

namespace extensions {
namespace events {

// TODO(kalman): I am still in the process of migrating Event construction away
// from using "UNKNOWN" to their real histogram values. See crbug.com/503402.
//
// Short version:
//  *Never* reorder or delete entries in the |HistogramValue| enumeration.
//  When creating a new extension event, add a new entry at the end of the
//  enum, just prior to ENUM_BOUNDARY.
//
// Long version: See extension_function_histogram_value.h
enum HistogramValue {
  UNKNOWN = 0,
  FOR_TEST,  // Tests should use this for a stub histogram value (not UNKNOWN).
  ACCESSIBILITY_PRIVATE_ON_INTRODUCE_CHROME_VOX,
  ACTIVITY_LOG_PRIVATE_ON_EXTENSION_ACTIVITY,
  ALARMS_ON_ALARM,
  APP_CURRENT_WINDOW_INTERNAL_ON_ALPHA_ENABLED_CHANGED,
  APP_CURRENT_WINDOW_INTERNAL_ON_BOUNDS_CHANGED,
  APP_CURRENT_WINDOW_INTERNAL_ON_CLOSED,
  APP_CURRENT_WINDOW_INTERNAL_ON_FULLSCREENED,
  APP_CURRENT_WINDOW_INTERNAL_ON_MAXIMIZED,
  APP_CURRENT_WINDOW_INTERNAL_ON_MINIMIZED,
  APP_CURRENT_WINDOW_INTERNAL_ON_RESTORED,
  APP_CURRENT_WINDOW_INTERNAL_ON_WINDOW_SHOWN_FOR_TESTS,
  APP_RUNTIME_ON_EMBED_REQUESTED,
  APP_RUNTIME_ON_LAUNCHED,
  APP_RUNTIME_ON_RESTARTED,
  APP_WINDOW_ON_BOUNDS_CHANGED,
  APP_WINDOW_ON_CLOSED,
  APP_WINDOW_ON_FULLSCREENED,
  APP_WINDOW_ON_MAXIMIZED,
  APP_WINDOW_ON_MINIMIZED,
  APP_WINDOW_ON_RESTORED,
  AUDIO_MODEM_ON_RECEIVED,
  AUDIO_MODEM_ON_TRANSMIT_FAIL,
  AUDIO_ON_DEVICE_CHANGED,
  AUDIO_ON_DEVICES_CHANGED,
  AUDIO_ON_LEVEL_CHANGED,
  AUDIO_ON_MUTE_CHANGED,
  AUTOFILL_PRIVATE_ON_ADDRESS_LIST_CHANGED,
  AUTOFILL_PRIVATE_ON_CREDIT_CARD_LIST_CHANGED,
  AUTOMATION_INTERNAL_ON_ACCESSIBILITY_EVENT,
  AUTOMATION_INTERNAL_ON_ACCESSIBILITY_TREE_DESTROYED,
  BLUETOOTH_LOW_ENERGY_ON_CHARACTERISTIC_VALUE_CHANGED,
  BLUETOOTH_LOW_ENERGY_ON_DESCRIPTOR_VALUE_CHANGED,
  BLUETOOTH_LOW_ENERGY_ON_SERVICE_ADDED,
  BLUETOOTH_LOW_ENERGY_ON_SERVICE_CHANGED,
  BLUETOOTH_LOW_ENERGY_ON_SERVICE_REMOVED,
  BLUETOOTH_ON_ADAPTER_STATE_CHANGED,
  BLUETOOTH_ON_DEVICE_ADDED,
  BLUETOOTH_ON_DEVICE_CHANGED,
  BLUETOOTH_ON_DEVICE_REMOVED,
  BLUETOOTH_PRIVATE_ON_PAIRING,
  BLUETOOTH_SOCKET_ON_ACCEPT,
  BLUETOOTH_SOCKET_ON_ACCEPT_ERROR,
  BLUETOOTH_SOCKET_ON_RECEIVE,
  BLUETOOTH_SOCKET_ON_RECEIVE_ERROR,
  BOOKMARK_MANAGER_PRIVATE_ON_DRAG_ENTER,
  BOOKMARK_MANAGER_PRIVATE_ON_DRAG_LEAVE,
  BOOKMARK_MANAGER_PRIVATE_ON_DROP,
  BOOKMARK_MANAGER_PRIVATE_ON_META_INFO_CHANGED,
  BOOKMARKS_ON_CHANGED,
  BOOKMARKS_ON_CHILDREN_REORDERED,
  BOOKMARKS_ON_CREATED,
  BOOKMARKS_ON_IMPORT_BEGAN,
  BOOKMARKS_ON_IMPORT_ENDED,
  BOOKMARKS_ON_MOVED,
  BOOKMARKS_ON_REMOVED,
  BRAILLE_DISPLAY_PRIVATE_ON_DISPLAY_STATE_CHANGED,
  BRAILLE_DISPLAY_PRIVATE_ON_KEY_EVENT,
  BROWSER_ACTION_ON_CLICKED,
  CAST_STREAMING_RTP_STREAM_ON_ERROR,
  CAST_STREAMING_RTP_STREAM_ON_STARTED,
  CAST_STREAMING_RTP_STREAM_ON_STOPPED,
  COMMANDS_ON_COMMAND,
  CONTEXT_MENUS_INTERNAL_ON_CLICKED,
  CONTEXT_MENUS_ON_CLICKED,
  COOKIES_ON_CHANGED,
  COPRESENCE_ON_MESSAGES_RECEIVED,
  COPRESENCE_ON_STATUS_UPDATED,
  COPRESENCE_PRIVATE_ON_CONFIG_AUDIO,
  COPRESENCE_PRIVATE_ON_DECODE_SAMPLES_REQUEST,
  COPRESENCE_PRIVATE_ON_ENCODE_TOKEN_REQUEST,
  DEBUGGER_ON_DETACH,
  DEBUGGER_ON_EVENT,
  DECLARATIVE_CONTENT_ON_PAGE_CHANGED,
  DECLARATIVE_WEB_REQUEST_ON_MESSAGE,
  DECLARATIVE_WEB_REQUEST_ON_REQUEST,
  DEVELOPER_PRIVATE_ON_ITEM_STATE_CHANGED,
  DEVELOPER_PRIVATE_ON_PROFILE_STATE_CHANGED,
  DEVTOOLS_INSPECTED_WINDOW_ON_RESOURCE_ADDED,
  DEVTOOLS_INSPECTED_WINDOW_ON_RESOURCE_CONTENT_COMMITTED,
  DEVTOOLS_NETWORK_ON_NAVIGATED,
  DEVTOOLS_NETWORK_ON_REQUEST_FINISHED,
  DOWNLOADS_ON_CHANGED,
  DOWNLOADS_ON_CREATED,
  DOWNLOADS_ON_DETERMINING_FILENAME,
  DOWNLOADS_ON_ERASED,
  EASY_UNLOCK_PRIVATE_ON_START_AUTO_PAIRING,
  EASY_UNLOCK_PRIVATE_ON_USER_INFO_UPDATED,
  EXPERIENCE_SAMPLING_PRIVATE_ON_DECISION,
  EXPERIENCE_SAMPLING_PRIVATE_ON_DISPLAYED,
  EXPERIMENTAL_DEVTOOLS_CONSOLE_ON_MESSAGE_ADDED,
  EXTENSION_ON_REQUEST,
  EXTENSION_ON_REQUEST_EXTERNAL,
  EXTENSION_OPTIONS_INTERNAL_ON_CLOSE,
  EXTENSION_OPTIONS_INTERNAL_ON_LOAD,
  EXTENSION_OPTIONS_INTERNAL_ON_PREFERRED_SIZE_CHANGED,
  FEEDBACK_PRIVATE_ON_FEEDBACK_REQUESTED,
  FILE_BROWSER_HANDLER_ON_EXECUTE,
  FILE_MANAGER_PRIVATE_ON_COPY_PROGRESS,
  FILE_MANAGER_PRIVATE_ON_DEVICE_CHANGED,
  FILE_MANAGER_PRIVATE_ON_DIRECTORY_CHANGED,
  FILE_MANAGER_PRIVATE_ON_DRIVE_CONNECTION_STATUS_CHANGED,
  FILE_MANAGER_PRIVATE_ON_DRIVE_SYNC_ERROR,
  FILE_MANAGER_PRIVATE_ON_FILE_TRANSFERS_UPDATED,
  FILE_MANAGER_PRIVATE_ON_MOUNT_COMPLETED,
  FILE_MANAGER_PRIVATE_ON_PREFERENCES_CHANGED,
  FILE_SYSTEM_ON_VOLUME_LIST_CHANGED,
  FILE_SYSTEM_PROVIDER_ON_ABORT_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_ADD_WATCHER_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_CLOSE_FILE_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_CONFIGURE_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_COPY_ENTRY_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_CREATE_DIRECTORY_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_CREATE_FILE_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_DELETE_ENTRY_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_GET_METADATA_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_MOUNT_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_MOVE_ENTRY_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_OPEN_FILE_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_READ_DIRECTORY_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_READ_FILE_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_REMOVE_WATCHER_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_TRUNCATE_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_UNMOUNT_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_WRITE_FILE_REQUESTED,
  FONT_SETTINGS_ON_DEFAULT_FIXED_FONT_SIZE_CHANGED,
  FONT_SETTINGS_ON_DEFAULT_FONT_SIZE_CHANGED,
  FONT_SETTINGS_ON_FONT_CHANGED,
  FONT_SETTINGS_ON_MINIMUM_FONT_SIZE_CHANGED,
  GCD_PRIVATE_ON_DEVICE_REMOVED,
  GCD_PRIVATE_ON_DEVICE_STATE_CHANGED,
  GCM_ON_MESSAGE,
  GCM_ON_MESSAGES_DELETED,
  GCM_ON_SEND_ERROR,
  HANGOUTS_PRIVATE_ON_HANGOUT_REQUESTED,
  HID_ON_DEVICE_ADDED,
  HID_ON_DEVICE_REMOVED,
  HISTORY_ON_VISITED,
  HISTORY_ON_VISIT_REMOVED,
  HOTWORD_PRIVATE_ON_DELETE_SPEAKER_MODEL,
  HOTWORD_PRIVATE_ON_ENABLED_CHANGED,
  HOTWORD_PRIVATE_ON_FINALIZE_SPEAKER_MODEL,
  HOTWORD_PRIVATE_ON_HOTWORD_SESSION_REQUESTED,
  HOTWORD_PRIVATE_ON_HOTWORD_SESSION_STOPPED,
  HOTWORD_PRIVATE_ON_HOTWORD_TRIGGERED,
  HOTWORD_PRIVATE_ON_MICROPHONE_STATE_CHANGED,
  HOTWORD_PRIVATE_ON_SPEAKER_MODEL_EXISTS,
  HOTWORD_PRIVATE_ON_SPEAKER_MODEL_SAVED,
  IDENTITY_ON_SIGN_IN_CHANGED,
  IDENTITY_PRIVATE_ON_WEB_FLOW_REQUEST,
  IDLE_ON_STATE_CHANGED,
  IMAGE_WRITER_PRIVATE_ON_DEVICE_INSERTED,
  IMAGE_WRITER_PRIVATE_ON_DEVICE_REMOVED,
  IMAGE_WRITER_PRIVATE_ON_WRITE_COMPLETE,
  IMAGE_WRITER_PRIVATE_ON_WRITE_ERROR,
  IMAGE_WRITER_PRIVATE_ON_WRITE_PROGRESS,
  INPUT_IME_ON_ACTIVATE,
  INPUT_IME_ON_BLUR,
  INPUT_IME_ON_CANDIDATE_CLICKED,
  INPUT_IME_ON_DEACTIVATED,
  INPUT_IME_ON_FOCUS,
  INPUT_IME_ON_INPUT_CONTEXT_UPDATE,
  INPUT_IME_ON_KEY_EVENT,
  INPUT_IME_ON_MENU_ITEM_ACTIVATED,
  INPUT_IME_ON_RESET,
  INPUT_IME_ON_SURROUNDING_TEXT_CHANGED,
  INPUT_METHOD_PRIVATE_ON_CHANGED,
  INPUT_METHOD_PRIVATE_ON_COMPOSITION_BOUNDS_CHANGED,
  INPUT_METHOD_PRIVATE_ON_DICTIONARY_CHANGED,
  INPUT_METHOD_PRIVATE_ON_DICTIONARY_LOADED,
  INSTANCE_ID_ON_TOKEN_REFRESH,
  LOCATION_ON_LOCATION_ERROR,
  LOCATION_ON_LOCATION_UPDATE,
  LOG_PRIVATE_ON_CAPTURED_EVENTS,
  MANAGEMENT_ON_DISABLED,
  MANAGEMENT_ON_ENABLED,
  MANAGEMENT_ON_INSTALLED,
  MANAGEMENT_ON_UNINSTALLED,
  MDNS_ON_SERVICE_LIST,
  MEDIA_GALLERIES_ON_GALLERY_CHANGED,
  MEDIA_GALLERIES_ON_SCAN_PROGRESS,
  MEDIA_PLAYER_PRIVATE_ON_NEXT_TRACK,
  MEDIA_PLAYER_PRIVATE_ON_PREV_TRACK,
  MEDIA_PLAYER_PRIVATE_ON_TOGGLE_PLAY_STATE,
  NETWORKING_CONFIG_ON_CAPTIVE_PORTAL_DETECTED,
  NETWORKING_PRIVATE_ON_DEVICE_STATE_LIST_CHANGED,
  NETWORKING_PRIVATE_ON_NETWORK_LIST_CHANGED,
  NETWORKING_PRIVATE_ON_NETWORKS_CHANGED,
  NETWORKING_PRIVATE_ON_PORTAL_DETECTION_COMPLETED,
  NOTIFICATION_PROVIDER_ON_CLEARED,
  NOTIFICATION_PROVIDER_ON_CREATED,
  NOTIFICATION_PROVIDER_ON_UPDATED,
  NOTIFICATIONS_ON_BUTTON_CLICKED,
  NOTIFICATIONS_ON_CLICKED,
  NOTIFICATIONS_ON_CLOSED,
  NOTIFICATIONS_ON_PERMISSION_LEVEL_CHANGED,
  NOTIFICATIONS_ON_SHOW_SETTINGS,
  OMNIBOX_ON_INPUT_CANCELLED,
  OMNIBOX_ON_INPUT_CHANGED,
  OMNIBOX_ON_INPUT_ENTERED,
  OMNIBOX_ON_INPUT_STARTED,
  PAGE_ACTION_ON_CLICKED,
  PASSWORDS_PRIVATE_ON_PASSWORD_EXCEPTIONS_LIST_CHANGED,
  PASSWORDS_PRIVATE_ON_PLAINTEXT_PASSWORD_RETRIEVED,
  PASSWORDS_PRIVATE_ON_SAVED_PASSWORDS_LIST_CHANGED,
  PERMISSIONS_ON_ADDED,
  PERMISSIONS_ON_REMOVED,
  PRINTER_PROVIDER_ON_GET_CAPABILITY_REQUESTED,
  PRINTER_PROVIDER_ON_GET_PRINTERS_REQUESTED,
  PRINTER_PROVIDER_ON_GET_USB_PRINTER_INFO_REQUESTED,
  PRINTER_PROVIDER_ON_PRINT_REQUESTED,
  PROCESSES_ON_CREATED,
  PROCESSES_ON_EXITED,
  PROCESSES_ON_UNRESPONSIVE,
  PROCESSES_ON_UPDATED,
  PROCESSES_ON_UPDATED_WITH_MEMORY,
  PROXY_ON_PROXY_ERROR,
  RUNTIME_ON_BROWSER_UPDATE_AVAILABLE,
  RUNTIME_ON_CONNECT,
  RUNTIME_ON_CONNECT_EXTERNAL,
  RUNTIME_ON_INSTALLED,
  RUNTIME_ON_MESSAGE,
  RUNTIME_ON_MESSAGE_EXTERNAL,
  RUNTIME_ON_RESTART_REQUIRED,
  RUNTIME_ON_STARTUP,
  RUNTIME_ON_SUSPEND,
  RUNTIME_ON_SUSPEND_CANCELED,
  RUNTIME_ON_UPDATE_AVAILABLE,
  SEARCH_ENGINES_PRIVATE_ON_SEARCH_ENGINES_CHANGED,
  SERIAL_ON_RECEIVE,
  SERIAL_ON_RECEIVE_ERROR,
  SESSIONS_ON_CHANGED,
  SETTINGS_PRIVATE_ON_PREFS_CHANGED,
  SIGNED_IN_DEVICES_ON_DEVICE_INFO_CHANGE,
  SOCKETS_TCP_ON_RECEIVE,
  SOCKETS_TCP_ON_RECEIVE_ERROR,
  SOCKETS_TCP_SERVER_ON_ACCEPT,
  SOCKETS_TCP_SERVER_ON_ACCEPT_ERROR,
  SOCKETS_UDP_ON_RECEIVE,
  SOCKETS_UDP_ON_RECEIVE_ERROR,
  STORAGE_ON_CHANGED,
  STREAMS_PRIVATE_ON_EXECUTE_MIME_TYPE_HANDLER,
  SYNC_FILE_SYSTEM_ON_FILE_STATUS_CHANGED,
  SYNC_FILE_SYSTEM_ON_SERVICE_STATUS_CHANGED,
  SYSTEM_DISPLAY_ON_DISPLAY_CHANGED,
  SYSTEM_INDICATOR_ON_CLICKED,
  SYSTEM_PRIVATE_ON_BRIGHTNESS_CHANGED,
  SYSTEM_PRIVATE_ON_SCREEN_UNLOCKED,
  SYSTEM_PRIVATE_ON_VOLUME_CHANGED,
  SYSTEM_PRIVATE_ON_WOKE_UP,
  SYSTEM_STORAGE_ON_ATTACHED,
  SYSTEM_STORAGE_ON_DETACHED,
  TAB_CAPTURE_ON_STATUS_CHANGED,
  TABS_ON_ACTIVATED,
  TABS_ON_ACTIVE_CHANGED,
  TABS_ON_ATTACHED,
  TABS_ON_CREATED,
  TABS_ON_DETACHED,
  TABS_ON_HIGHLIGHT_CHANGED,
  TABS_ON_HIGHLIGHTED,
  TABS_ON_MOVED,
  TABS_ON_REMOVED,
  TABS_ON_REPLACED,
  TABS_ON_SELECTION_CHANGED,
  TABS_ON_UPDATED,
  TABS_ON_ZOOM_CHANGE,
  TERMINAL_PRIVATE_ON_PROCESS_OUTPUT,
  TEST_ON_MESSAGE,
  TTS_ENGINE_ON_PAUSE,
  TTS_ENGINE_ON_RESUME,
  TTS_ENGINE_ON_SPEAK,
  TTS_ENGINE_ON_STOP,
  USB_ON_DEVICE_ADDED,
  USB_ON_DEVICE_REMOVED,
  VIRTUAL_KEYBOARD_PRIVATE_ON_BOUNDS_CHANGED,
  VIRTUAL_KEYBOARD_PRIVATE_ON_TEXT_INPUT_BOX_FOCUSED,
  VPN_PROVIDER_ON_CONFIG_CREATED,
  VPN_PROVIDER_ON_CONFIG_REMOVED,
  VPN_PROVIDER_ON_PACKET_RECEIVED,
  VPN_PROVIDER_ON_PLATFORM_MESSAGE,
  VPN_PROVIDER_ON_UIEVENT,
  WALLPAPER_PRIVATE_ON_WALLPAPER_CHANGED_BY_3RD_PARTY,
  WEB_NAVIGATION_ON_BEFORE_NAVIGATE,
  WEB_NAVIGATION_ON_COMMITTED,
  WEB_NAVIGATION_ON_COMPLETED,
  WEB_NAVIGATION_ON_CREATED_NAVIGATION_TARGET,
  WEB_NAVIGATION_ON_DOM_CONTENT_LOADED,
  WEB_NAVIGATION_ON_ERROR_OCCURRED,
  WEB_NAVIGATION_ON_HISTORY_STATE_UPDATED,
  WEB_NAVIGATION_ON_REFERENCE_FRAGMENT_UPDATED,
  WEB_NAVIGATION_ON_TAB_REPLACED,
  WEB_REQUEST_ON_AUTH_REQUIRED,
  WEB_REQUEST_ON_BEFORE_REDIRECT,
  WEB_REQUEST_ON_BEFORE_REQUEST,
  WEB_REQUEST_ON_BEFORE_SEND_HEADERS,
  WEB_REQUEST_ON_COMPLETED,
  WEB_REQUEST_ON_ERROR_OCCURRED,
  WEB_REQUEST_ON_HEADERS_RECEIVED,
  WEB_REQUEST_ON_RESPONSE_STARTED,
  WEB_REQUEST_ON_SEND_HEADERS,
  WEBRTC_AUDIO_PRIVATE_ON_SINKS_CHANGED,
  WEBSTORE_ON_DOWNLOAD_PROGRESS,
  WEBSTORE_ON_INSTALL_STAGE_CHANGED,
  WEBSTORE_WIDGET_PRIVATE_ON_SHOW_WIDGET,
  WEBVIEW_TAG_CLOSE,
  WEBVIEW_TAG_CONSOLEMESSAGE,
  WEBVIEW_TAG_CONTENTLOAD,
  WEBVIEW_TAG_DIALOG,
  WEBVIEW_TAG_EXIT,
  WEBVIEW_TAG_FINDUPDATE,
  WEBVIEW_TAG_LOADABORT,
  WEBVIEW_TAG_LOADCOMMIT,
  WEBVIEW_TAG_LOADREDIRECT,
  WEBVIEW_TAG_LOADSTART,
  WEBVIEW_TAG_LOADSTOP,
  WEBVIEW_TAG_NEWWINDOW,
  WEBVIEW_TAG_PERMISSIONREQUEST,
  WEBVIEW_TAG_RESPONSIVE,
  WEBVIEW_TAG_SIZECHANGED,
  WEBVIEW_TAG_UNRESPONSIVE,
  WEBVIEW_TAG_ZOOMCHANGE,
  WINDOWS_ON_CREATED,
  WINDOWS_ON_FOCUS_CHANGED,
  WINDOWS_ON_REMOVED,
  FILE_SYSTEM_PROVIDER_ON_EXECUTE_ACTION_REQUESTED,
  FILE_SYSTEM_PROVIDER_ON_GET_ACTIONS_REQUESTED,
  LAUNCHER_SEARCH_PROVIDER_ON_QUERY_STARTED,
  LAUNCHER_SEARCH_PROVIDER_ON_QUERY_ENDED,
  LAUNCHER_SEARCH_PROVIDER_ON_OPEN_RESULT,
  CHROME_WEB_VIEW_INTERNAL_ON_CLICKED,
  WEB_VIEW_INTERNAL_CONTEXT_MENUS,
  CONTEXT_MENUS,
  TTS_ON_EVENT,
  LAUNCHER_PAGE_ON_TRANSITION_CHANGED,
  LAUNCHER_PAGE_ON_POP_SUBPAGE,
  DIAL_ON_DEVICE_LIST,
  DIAL_ON_ERROR,
  CAST_CHANNEL_ON_MESSAGE,
  CAST_CHANNEL_ON_ERROR,
  SCREENLOCK_PRIVATE_ON_CHANGED,
  SCREENLOCK_PRIVATE_ON_AUTH_ATTEMPTED,
  // Last entry: Add new entries above, then run:
  // python tools/metrics/histograms/update_extension_histograms.py
  ENUM_BOUNDARY
};

}  // namespace events
}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_EXTENSION_EVENT_HISTOGRAM_VALUE_H_
