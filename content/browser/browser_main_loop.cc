// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/browser_main_loop.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/memory_pressure_monitor.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/pending_task.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/process/process_metrics.h"
#include "base/profiler/scoped_profile.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/system_monitor/system_monitor.h"
#include "base/thread_task_runner_handle.h"
#include "base/threading/thread_restrictions.h"
#include "base/timer/hi_res_timer_manager.h"
#include "base/trace_event/memory_dump_manager.h"
#include "base/trace_event/trace_event.h"
#include "content/browser/browser_thread_impl.h"
#include "content/browser/device_sensors/device_inertial_sensor_service.h"
#include "content/browser/dom_storage/dom_storage_area.h"
#include "content/browser/download/save_file_manager.h"
#include "content/browser/gamepad/gamepad_service.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/browser/gpu/browser_gpu_memory_buffer_manager.h"
#include "content/browser/gpu/compositor_util.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/browser/gpu/gpu_process_host.h"
#include "content/browser/gpu/gpu_process_host_ui_shim.h"
#include "content/browser/histogram_synchronizer.h"
#include "content/browser/loader/resource_dispatcher_host_impl.h"
#include "content/browser/media/media_internals.h"
#include "content/browser/mojo/mojo_shell_context.h"
#include "content/browser/net/browser_online_state_observer.h"
#include "content/browser/renderer_host/media/media_stream_manager.h"
#include "content/browser/speech/speech_recognition_manager_impl.h"
#include "content/browser/startup_task_runner.h"
#include "content/browser/time_zone_monitor.h"
#include "content/browser/webui/content_web_ui_controller_factory.h"
#include "content/browser/webui/url_data_manager.h"
#include "content/common/content_switches_internal.h"
#include "content/common/host_discardable_shared_memory_manager.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/tracing_controller.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/result_codes.h"
#include "crypto/nss_util.h"
#include "device/battery/battery_status_service.h"
#include "media/audio/audio_manager.h"
#include "media/base/media.h"
#include "media/base/user_input_monitor.h"
#include "media/midi/midi_manager.h"
#include "net/base/network_change_notifier.h"
#include "net/socket/client_socket_factory.h"
#include "net/ssl/ssl_config_service.h"
#include "ui/base/clipboard/clipboard.h"

#if defined(USE_AURA) || (defined(OS_MACOSX) && !defined(OS_IOS))
#include "content/browser/compositor/image_transport_factory.h"
#endif

#if defined(USE_AURA)
#include "content/public/browser/context_factory.h"
#include "ui/aura/env.h"
#endif

#if !defined(OS_IOS)
#include "content/browser/renderer_host/render_process_host_impl.h"
#endif

#if defined(OS_ANDROID)
#include "base/android/jni_android.h"
#include "content/browser/android/browser_startup_controller.h"
#include "content/browser/android/browser_surface_texture_manager.h"
#include "content/browser/android/in_process_surface_texture_manager.h"
#include "content/browser/android/tracing_controller_android.h"
#include "content/browser/screen_orientation/screen_orientation_delegate_android.h"
#include "content/public/browser/screen_orientation_provider.h"
#include "ui/gl/gl_surface.h"
#endif

#if defined(OS_MACOSX)
#include "media/base/mac/avfoundation_glue.h"
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include "base/memory/memory_pressure_monitor_mac.h"
#include "content/browser/bootstrap_sandbox_mac.h"
#include "content/browser/browser_io_surface_manager_mac.h"
#include "content/browser/cocoa/system_hotkey_helper_mac.h"
#include "content/browser/compositor/browser_compositor_view_mac.h"
#include "content/browser/in_process_io_surface_manager_mac.h"
#include "content/browser/theme_helper_mac.h"
#endif

#if defined(OS_WIN)
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#include "base/memory/memory_pressure_monitor_win.h"
#include "content/browser/system_message_window_win.h"
#include "content/common/sandbox_win.h"
#include "net/base/winsock_init.h"
#include "ui/base/l10n/l10n_util_win.h"
#endif

#if defined(OS_CHROMEOS)
#include "base/memory/memory_pressure_monitor_chromeos.h"
#include "chromeos/chromeos_switches.h"
#endif

#if defined(USE_GLIB)
#include <glib-object.h>
#endif

#if defined(OS_LINUX) && defined(USE_UDEV)
#include "content/browser/device_monitor_udev.h"
#elif defined(OS_MACOSX) && !defined(OS_IOS)
#include "content/browser/device_monitor_mac.h"
#endif

#if defined(OS_POSIX) && !defined(OS_MACOSX)
#include "content/browser/renderer_host/render_sandbox_host_linux.h"
#include "content/browser/zygote_host/zygote_host_impl_linux.h"
#include "sandbox/linux/suid/client/setuid_sandbox_host.h"
#endif

#if defined(ENABLE_PLUGINS)
#include "content/browser/plugin_service_impl.h"
#endif

#if defined(TCMALLOC_TRACE_MEMORY_SUPPORTED)
#include "third_party/tcmalloc/chromium/src/gperftools/heap-profiler.h"
#endif

#if defined(USE_X11)
#include "ui/gfx/x/x11_connection.h"
#include "ui/gfx/x/x11_types.h"
#endif

// One of the linux specific headers defines this as a macro.
#ifdef DestroyAll
#undef DestroyAll
#endif

namespace content {
namespace {

#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID)
void SetupSandbox(const base::CommandLine& parsed_command_line) {
  TRACE_EVENT0("startup", "SetupSandbox");
  base::FilePath sandbox_binary;

  scoped_ptr<sandbox::SetuidSandboxHost> setuid_sandbox_host(
      sandbox::SetuidSandboxHost::Create());

  const bool want_setuid_sandbox =
      !parsed_command_line.HasSwitch(switches::kNoSandbox) &&
      !parsed_command_line.HasSwitch(switches::kDisableSetuidSandbox) &&
      !setuid_sandbox_host->IsDisabledViaEnvironment();

  static const char no_suid_error[] =
      "Running without the SUID sandbox! See "
      "https://code.google.com/p/chromium/wiki/LinuxSUIDSandboxDevelopment "
      "for more information on developing with the sandbox on.";
  if (want_setuid_sandbox) {
    sandbox_binary = setuid_sandbox_host->GetSandboxBinaryPath();
    if (sandbox_binary.empty()) {
      // This needs to be fatal. Talk to security@chromium.org if you feel
      // otherwise.
      LOG(FATAL) << no_suid_error;
    }
  } else {
    LOG(ERROR) << no_suid_error;
  }

  // Tickle the sandbox host and zygote host so they fork now.
  RenderSandboxHostLinux::GetInstance()->Init();
  ZygoteHostImpl::GetInstance()->Init(sandbox_binary.value());
}
#endif

#if defined(USE_GLIB)
static void GLibLogHandler(const gchar* log_domain,
                           GLogLevelFlags log_level,
                           const gchar* message,
                           gpointer userdata) {
  if (!log_domain)
    log_domain = "<unknown>";
  if (!message)
    message = "<no message>";

  if (strstr(message, "Unable to retrieve the file info for")) {
    LOG(ERROR) << "GTK File code error: " << message;
  } else if (strstr(message, "Could not find the icon") &&
             strstr(log_domain, "Gtk")) {
    LOG(ERROR) << "GTK icon error: " << message;
  } else if (strstr(message, "Theme file for default has no") ||
             strstr(message, "Theme directory") ||
             strstr(message, "theme pixmap") ||
             strstr(message, "locate theme engine")) {
    LOG(ERROR) << "GTK theme error: " << message;
  } else if (strstr(message, "Unable to create Ubuntu Menu Proxy") &&
             strstr(log_domain, "<unknown>")) {
    LOG(ERROR) << "GTK menu proxy create failed";
  } else if (strstr(message, "Out of memory") &&
             strstr(log_domain, "<unknown>")) {
    LOG(ERROR) << "DBus call timeout or out of memory: "
               << "http://crosbug.com/15496";
  } else if (strstr(message, "Could not connect: Connection refused") &&
             strstr(log_domain, "<unknown>")) {
    LOG(ERROR) << "DConf settings backend could not connect to session bus: "
               << "http://crbug.com/179797";
  } else if (strstr(message, "Attempting to store changes into") ||
             strstr(message, "Attempting to set the permissions of")) {
    LOG(ERROR) << message << " (http://bugs.chromium.org/161366)";
  } else if (strstr(message, "drawable is not a native X11 window")) {
    LOG(ERROR) << message << " (http://bugs.chromium.org/329991)";
  } else if (strstr(message, "Cannot do system-bus activation with no user")) {
    LOG(ERROR) << message << " (http://crbug.com/431005)";
  } else {
    LOG(DFATAL) << log_domain << ": " << message;
  }
}

static void SetUpGLibLogHandler() {
  // Register GLib-handled assertions to go through our logging system.
  const char* kLogDomains[] = { NULL, "Gtk", "Gdk", "GLib", "GLib-GObject" };
  for (size_t i = 0; i < arraysize(kLogDomains); i++) {
    g_log_set_handler(kLogDomains[i],
                      static_cast<GLogLevelFlags>(G_LOG_FLAG_RECURSION |
                                                  G_LOG_FLAG_FATAL |
                                                  G_LOG_LEVEL_ERROR |
                                                  G_LOG_LEVEL_CRITICAL |
                                                  G_LOG_LEVEL_WARNING),
                      GLibLogHandler,
                      NULL);
  }
}
#endif

void OnStoppedStartupTracing(const base::FilePath& trace_file) {
  VLOG(0) << "Completed startup tracing to " << trace_file.value();
}

// Disable optimizations for this block of functions so the compiler doesn't
// merge them all together. This makes it possible to tell what thread was
// unresponsive by inspecting the callstack.
MSVC_DISABLE_OPTIMIZE()
MSVC_PUSH_DISABLE_WARNING(4748)

NOINLINE void ResetThread_DB(scoped_ptr<BrowserProcessSubThread> thread) {
  volatile int inhibit_comdat = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_comdat);
  thread.reset();
}

NOINLINE void ResetThread_FILE(scoped_ptr<BrowserProcessSubThread> thread) {
  volatile int inhibit_comdat = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_comdat);
  thread.reset();
}

NOINLINE void ResetThread_FILE_USER_BLOCKING(
    scoped_ptr<BrowserProcessSubThread> thread) {
  volatile int inhibit_comdat = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_comdat);
  thread.reset();
}

NOINLINE void ResetThread_PROCESS_LAUNCHER(
    scoped_ptr<BrowserProcessSubThread> thread) {
  volatile int inhibit_comdat = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_comdat);
  thread.reset();
}

NOINLINE void ResetThread_CACHE(scoped_ptr<BrowserProcessSubThread> thread) {
  volatile int inhibit_comdat = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_comdat);
  thread.reset();
}

NOINLINE void ResetThread_IO(scoped_ptr<BrowserProcessSubThread> thread) {
  volatile int inhibit_comdat = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_comdat);
  thread.reset();
}

#if !defined(OS_IOS)
NOINLINE void ResetThread_IndexedDb(scoped_ptr<base::Thread> thread) {
  volatile int inhibit_comdat = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_comdat);
  thread.reset();
}
#endif

MSVC_POP_WARNING()
MSVC_ENABLE_OPTIMIZE();

#if defined(OS_WIN)
// Creates a memory pressure monitor using automatic thresholds, or those
// specified on the command-line. Ownership is passed to the caller.
base::win::MemoryPressureMonitor* CreateWinMemoryPressureMonitor(
    const base::CommandLine& parsed_command_line) {
  std::vector<std::string> thresholds;
  base::SplitString(
      parsed_command_line.GetSwitchValueASCII(
          switches::kMemoryPressureThresholdsMb),
      ',',
      &thresholds);

  int moderate_threshold_mb = 0;
  int critical_threshold_mb = 0;
  if (thresholds.size() == 2 &&
      base::StringToInt(thresholds[0], &moderate_threshold_mb) &&
      base::StringToInt(thresholds[1], &critical_threshold_mb) &&
      moderate_threshold_mb >= critical_threshold_mb &&
      critical_threshold_mb >= 0) {
    return new base::win::MemoryPressureMonitor(moderate_threshold_mb,
                                                critical_threshold_mb);
  }

  // In absence of valid switches use the automatic defaults.
  return new base::win::MemoryPressureMonitor();
}
#endif  // defined(OS_WIN)

}  // namespace

// The currently-running BrowserMainLoop.  There can be one or zero.
BrowserMainLoop* g_current_browser_main_loop = NULL;

// For measuring memory usage after each task. Behind a command line flag.
class BrowserMainLoop::MemoryObserver : public base::MessageLoop::TaskObserver {
 public:
  MemoryObserver() {}
  ~MemoryObserver() override {}

  void WillProcessTask(const base::PendingTask& pending_task) override {}

  void DidProcessTask(const base::PendingTask& pending_task) override {
#if !defined(OS_IOS)  // No ProcessMetrics on IOS.
    scoped_ptr<base::ProcessMetrics> process_metrics(
        base::ProcessMetrics::CreateProcessMetrics(
#if defined(OS_MACOSX)
            base::GetCurrentProcessHandle(), NULL));
#else
            base::GetCurrentProcessHandle()));
#endif
    size_t private_bytes;
    process_metrics->GetMemoryBytes(&private_bytes, NULL);
    LOCAL_HISTOGRAM_MEMORY_KB("Memory.BrowserUsed", private_bytes >> 10);
#endif
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(MemoryObserver);
};


// BrowserMainLoop construction / destruction =============================

BrowserMainLoop* BrowserMainLoop::GetInstance() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  return g_current_browser_main_loop;
}

BrowserMainLoop::BrowserMainLoop(const MainFunctionParams& parameters)
    : parameters_(parameters),
      parsed_command_line_(parameters.command_line),
      result_code_(RESULT_CODE_NORMAL_EXIT),
      created_threads_(false),
      // ContentMainRunner should have enabled tracing of the browser process
      // when kTraceStartup is in the command line.
      is_tracing_startup_(
          parameters.command_line.HasSwitch(switches::kTraceStartup)) {
  DCHECK(!g_current_browser_main_loop);
  g_current_browser_main_loop = this;
}

BrowserMainLoop::~BrowserMainLoop() {
  DCHECK_EQ(this, g_current_browser_main_loop);
#if !defined(OS_IOS)
  ui::Clipboard::DestroyClipboardForCurrentThread();
#endif  // !defined(OS_IOS)
  g_current_browser_main_loop = NULL;
}

void BrowserMainLoop::Init() {
  TRACE_EVENT0("startup", "BrowserMainLoop::Init");
  TRACK_SCOPED_REGION("Startup", "BrowserMainLoop::Init");

  parts_.reset(
      GetContentClient()->browser()->CreateBrowserMainParts(parameters_));
}

// BrowserMainLoop stages ==================================================

void BrowserMainLoop::EarlyInitialization() {
  TRACE_EVENT0("startup", "BrowserMainLoop::EarlyInitialization");
  TRACK_SCOPED_REGION("Startup", "BrowserMainLoop::EarlyInitialization");

#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID)
  // No thread should be created before this call, as SetupSandbox()
  // will end-up using fork().
  SetupSandbox(parsed_command_line_);
#endif

#if defined(USE_X11)
  if (UsingInProcessGpu()) {
    if (!gfx::InitializeThreadedX11()) {
      LOG(ERROR) << "Failed to put Xlib into threaded mode.";
    }
  }
#endif

  // GLib's spawning of new processes is buggy, so it's important that at this
  // point GLib does not need to start DBUS. Chrome should always start with
  // DBUS_SESSION_BUS_ADDRESS properly set. See crbug.com/309093.
#if defined(USE_GLIB)
  // g_type_init will be deprecated in 2.36. 2.35 is the development
  // version for 2.36, hence do not call g_type_init starting 2.35.
  // http://developer.gnome.org/gobject/unstable/gobject-Type-Information.html#g-type-init
#if !GLIB_CHECK_VERSION(2, 35, 0)
  // GLib type system initialization. Needed at least for gconf,
  // used in net/proxy/proxy_config_service_linux.cc. Most likely
  // this is superfluous as gtk_init() ought to do this. It's
  // definitely harmless, so retained as a reminder of this
  // requirement for gconf.
  g_type_init();
#endif

  SetUpGLibLogHandler();
#endif

  if (parts_)
    parts_->PreEarlyInitialization();

#if defined(OS_MACOSX)
  // We use quite a few file descriptors for our IPC, and the default limit on
  // the Mac is low (256), so bump it up.
  base::SetFdLimit(1024);
#endif

#if defined(OS_WIN)
  net::EnsureWinsockInit();
#endif

#if !defined(USE_OPENSSL)
  // We want to be sure to init NSPR on the main thread.
  crypto::EnsureNSPRInit();
#endif  // !defined(USE_OPENSSL)

#if !defined(OS_IOS)
  if (parsed_command_line_.HasSwitch(switches::kRendererProcessLimit)) {
    std::string limit_string = parsed_command_line_.GetSwitchValueASCII(
        switches::kRendererProcessLimit);
    size_t process_limit;
    if (base::StringToSizeT(limit_string, &process_limit)) {
      RenderProcessHost::SetMaxRendererProcessCount(process_limit);
    }
  }
#endif  // !defined(OS_IOS)

  if (parsed_command_line_.HasSwitch(switches::kEnableNativeGpuMemoryBuffers)) {
    BrowserGpuChannelHostFactory::EnableGpuMemoryBufferFactoryUsage(
        gfx::GpuMemoryBuffer::MAP);
    BrowserGpuChannelHostFactory::EnableGpuMemoryBufferFactoryUsage(
        gfx::GpuMemoryBuffer::PERSISTENT_MAP);
  }

#if defined(USE_OZONE)
  BrowserGpuChannelHostFactory::EnableGpuMemoryBufferFactoryUsage(
      gfx::GpuMemoryBuffer::SCANOUT);
#endif

  base::DiscardableMemoryAllocator::SetInstance(
      HostDiscardableSharedMemoryManager::current());

  if (parts_)
    parts_->PostEarlyInitialization();
}

void BrowserMainLoop::PreMainMessageLoopStart() {
  if (parts_) {
    TRACE_EVENT0("startup",
        "BrowserMainLoop::MainMessageLoopStart:PreMainMessageLoopStart");
    parts_->PreMainMessageLoopStart();
  }

#if defined(OS_WIN)
  // If we're running tests (ui_task is non-null), then the ResourceBundle
  // has already been initialized.
  if (!parameters_.ui_task) {
    // Override the configured locale with the user's preferred UI language.
    l10n_util::OverrideLocaleWithUILanguageList();
  }
#endif
}

void BrowserMainLoop::MainMessageLoopStart() {
  // DO NOT add more code here. Use PreMainMessageLoopStart() above or
  // PostMainMessageLoopStart() below.

  TRACE_EVENT0("startup", "BrowserMainLoop::MainMessageLoopStart");
  TRACK_SCOPED_REGION("Startup", "BrowserMainLoop::MainMessageLoopStart");

  // Create a MessageLoop if one does not already exist for the current thread.
  if (!base::MessageLoop::current())
    main_message_loop_.reset(new base::MessageLoopForUI);

  InitializeMainThread();
}

void BrowserMainLoop::PostMainMessageLoopStart() {
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:SystemMonitor");
    system_monitor_.reset(new base::SystemMonitor);
  }
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:PowerMonitor");
    scoped_ptr<base::PowerMonitorSource> power_monitor_source(
      new base::PowerMonitorDeviceSource());
    power_monitor_.reset(new base::PowerMonitor(power_monitor_source.Pass()));
  }
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:HighResTimerManager");
    hi_res_timer_manager_.reset(new base::HighResolutionTimerManager);
  }
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:NetworkChangeNotifier");
    network_change_notifier_.reset(net::NetworkChangeNotifier::Create());
  }

#if !defined(OS_IOS)
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:MediaFeatures");
    media::InitializeMediaLibrary();
  }
  {
    TRACE_EVENT0("startup",
                 "BrowserMainLoop::Subsystem:ContentWebUIController");
    WebUIControllerFactory::RegisterFactory(
        ContentWebUIControllerFactory::GetInstance());
  }

  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:OnlineStateObserver");
    online_state_observer_.reset(new BrowserOnlineStateObserver);
  }

  {
    system_stats_monitor_.reset(
        new base::trace_event::TraceEventSystemStatsMonitor(
            base::ThreadTaskRunnerHandle::Get()));
  }
#endif  // !defined(OS_IOS)

#if defined(OS_WIN)
  system_message_window_.reset(new SystemMessageWindowWin);
#endif

  if (parts_)
    parts_->PostMainMessageLoopStart();

#if !defined(OS_IOS)
  // Start tracing to a file if needed. Only do this after starting the main
  // message loop to avoid calling MessagePumpForUI::ScheduleWork() before
  // MessagePumpForUI::Start() as it will crash the browser.
  if (is_tracing_startup_) {
    TRACE_EVENT0("startup", "BrowserMainLoop::InitStartupTracing");
    InitStartupTracing(parsed_command_line_);
  }
#endif  // !defined(OS_IOS)

#if defined(OS_ANDROID)
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:SurfaceTextureManager");
    if (parsed_command_line_.HasSwitch(switches::kSingleProcess)) {
      SurfaceTextureManager::SetInstance(
          InProcessSurfaceTextureManager::GetInstance());
    } else {
      SurfaceTextureManager::SetInstance(
          BrowserSurfaceTextureManager::GetInstance());
    }
  }

  if (!parsed_command_line_.HasSwitch(
      switches::kDisableScreenOrientationLock)) {
    TRACE_EVENT0("startup",
                 "BrowserMainLoop::Subsystem:ScreenOrientationProvider");
    screen_orientation_delegate_.reset(
        new ScreenOrientationDelegateAndroid());
    ScreenOrientationProvider::SetDelegate(screen_orientation_delegate_.get());
  }
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:IOSurfaceManager");
    if (parsed_command_line_.HasSwitch(switches::kSingleProcess)) {
      IOSurfaceManager::SetInstance(InProcessIOSurfaceManager::GetInstance());
    } else {
      IOSurfaceManager::SetInstance(BrowserIOSurfaceManager::GetInstance());
    }
  }
#endif

  if (parsed_command_line_.HasSwitch(switches::kMemoryMetrics)) {
    TRACE_EVENT0("startup", "BrowserMainLoop::Subsystem:MemoryObserver");
    memory_observer_.reset(new MemoryObserver());
    base::MessageLoop::current()->AddTaskObserver(memory_observer_.get());
  }

  if (parsed_command_line_.HasSwitch(
          switches::kEnableAggressiveDOMStorageFlushing)) {
    TRACE_EVENT0("startup",
                 "BrowserMainLoop::Subsystem:EnableAggressiveCommitDelay");
    DOMStorageArea::EnableAggressiveCommitDelay();
  }

  base::trace_event::MemoryDumpManager::GetInstance()->Initialize();

  // Enable the dump providers.
  base::trace_event::MemoryDumpManager::GetInstance()->RegisterDumpProvider(
      HostSharedBitmapManager::current());

#if defined(TCMALLOC_TRACE_MEMORY_SUPPORTED)
  trace_memory_controller_.reset(new base::trace_event::TraceMemoryController(
      base::MessageLoop::current()->task_runner(),
      ::HeapProfilerWithPseudoStackStart, ::HeapProfilerStop,
      ::GetHeapProfile));
#endif
}

int BrowserMainLoop::PreCreateThreads() {
  if (parts_) {
    TRACE_EVENT0("startup",
        "BrowserMainLoop::CreateThreads:PreCreateThreads");
    TRACK_SCOPED_REGION("Startup", "BrowserMainLoop::PreCreateThreads");

    result_code_ = parts_->PreCreateThreads();
  }

  // TODO(chrisha): Abstract away this construction mess to a helper function,
  // once MemoryPressureMonitor is made a concrete class.
#if defined(OS_CHROMEOS)
  if (chromeos::switches::MemoryPressureHandlingEnabled()) {
    memory_pressure_monitor_.reset(new base::chromeos::MemoryPressureMonitor(
        chromeos::switches::GetMemoryPressureThresholds()));
  }
#elif defined(OS_MACOSX) && !defined(OS_IOS)
  memory_pressure_monitor_.reset(new base::mac::MemoryPressureMonitor());
#elif defined(OS_WIN)
  memory_pressure_monitor_.reset(CreateWinMemoryPressureMonitor(
      parsed_command_line_));
#endif

#if defined(ENABLE_PLUGINS)
  // Prior to any processing happening on the io thread, we create the
  // plugin service as it is predominantly used from the io thread,
  // but must be created on the main thread. The service ctor is
  // inexpensive and does not invoke the io_thread() accessor.
  {
    TRACE_EVENT0("startup", "BrowserMainLoop::CreateThreads:PluginService");
    PluginService::GetInstance()->Init();
  }
#endif

#if defined(OS_MACOSX)
  {
    // Initialize AVFoundation if supported, for audio and video.
    TRACE_EVENT0("startup",
                 "BrowserMainLoop::CreateThreads:InitializeAVFoundation");
    AVFoundationGlue::InitializeAVFoundation();
  }
#endif

  // Need to initialize in-process GpuDataManager before creating threads.
  // It's unsafe to append the gpu command line switches to the global
  // CommandLine::ForCurrentProcess object after threads are created.
  if (UsingInProcessGpu()) {
    bool initialize_gpu_data_manager = true;
#if defined(OS_ANDROID)
    if (!gfx::GLSurface::InitializeOneOff()) {
      // Single-process Android WebView supports no gpu.
      LOG(ERROR) << "GLSurface::InitializeOneOff failed";
      initialize_gpu_data_manager = false;
    }
#endif

    // Initialize the GpuDataManager before we set up the MessageLoops because
    // otherwise we'll trigger the assertion about doing IO on the UI thread.
    if (initialize_gpu_data_manager)
      GpuDataManagerImpl::GetInstance()->Initialize();
  }

#if !defined(OS_IOS) && (!defined(GOOGLE_CHROME_BUILD) || defined(OS_ANDROID))
  // Single-process is an unsupported and not fully tested mode, so
  // don't enable it for official Chrome builds (except on Android).
  if (parsed_command_line_.HasSwitch(switches::kSingleProcess))
    RenderProcessHost::SetRunRendererInProcess(true);
#endif

  return result_code_;
}

void BrowserMainLoop::CreateStartupTasks() {
  TRACE_EVENT0("startup", "BrowserMainLoop::CreateStartupTasks");
  TRACK_SCOPED_REGION("Startup", "BrowserMainLoop::CreateStartupTasks");

  // First time through, we really want to create all the tasks
  if (!startup_task_runner_.get()) {
#if defined(OS_ANDROID)
    startup_task_runner_ = make_scoped_ptr(
        new StartupTaskRunner(base::Bind(&BrowserStartupComplete),
                              base::ThreadTaskRunnerHandle::Get()));
#else
    startup_task_runner_ = make_scoped_ptr(
        new StartupTaskRunner(base::Callback<void(int)>(),
                              base::ThreadTaskRunnerHandle::Get()));
#endif
    StartupTask pre_create_threads =
        base::Bind(&BrowserMainLoop::PreCreateThreads, base::Unretained(this));
    startup_task_runner_->AddTask(pre_create_threads);

    StartupTask create_threads =
        base::Bind(&BrowserMainLoop::CreateThreads, base::Unretained(this));
    startup_task_runner_->AddTask(create_threads);

    StartupTask browser_thread_started = base::Bind(
        &BrowserMainLoop::BrowserThreadsStarted, base::Unretained(this));
    startup_task_runner_->AddTask(browser_thread_started);

    StartupTask pre_main_message_loop_run = base::Bind(
        &BrowserMainLoop::PreMainMessageLoopRun, base::Unretained(this));
    startup_task_runner_->AddTask(pre_main_message_loop_run);

#if defined(OS_ANDROID)
    if (BrowserMayStartAsynchronously()) {
      startup_task_runner_->StartRunningTasksAsync();
    }
#endif
  }
#if defined(OS_ANDROID)
  if (!BrowserMayStartAsynchronously()) {
    // A second request for asynchronous startup can be ignored, so
    // StartupRunningTasksAsync is only called first time through. If, however,
    // this is a request for synchronous startup then it must override any
    // previous call for async startup, so we call RunAllTasksNow()
    // unconditionally.
    startup_task_runner_->RunAllTasksNow();
  }
#else
  startup_task_runner_->RunAllTasksNow();
#endif
}

int BrowserMainLoop::CreateThreads() {
  TRACE_EVENT0("startup", "BrowserMainLoop::CreateThreads");
  TRACK_SCOPED_REGION("Startup", "BrowserMainLoop::CreateThreads");

  base::Thread::Options io_message_loop_options;
  io_message_loop_options.message_loop_type = base::MessageLoop::TYPE_IO;
  base::Thread::Options ui_message_loop_options;
  ui_message_loop_options.message_loop_type = base::MessageLoop::TYPE_UI;

  // Start threads in the order they occur in the BrowserThread::ID
  // enumeration, except for BrowserThread::UI which is the main
  // thread.
  //
  // Must be size_t so we can increment it.
  for (size_t thread_id = BrowserThread::UI + 1;
       thread_id < BrowserThread::ID_COUNT;
       ++thread_id) {
    scoped_ptr<BrowserProcessSubThread>* thread_to_start = NULL;
    base::Thread::Options options;

    switch (thread_id) {
      case BrowserThread::DB:
        TRACE_EVENT_BEGIN1("startup",
            "BrowserMainLoop::CreateThreads:start",
            "Thread", "BrowserThread::DB");
        thread_to_start = &db_thread_;
        options.timer_slack = base::TIMER_SLACK_MAXIMUM;
        break;
      case BrowserThread::FILE_USER_BLOCKING:
        TRACE_EVENT_BEGIN1("startup",
            "BrowserMainLoop::CreateThreads:start",
            "Thread", "BrowserThread::FILE_USER_BLOCKING");
        thread_to_start = &file_user_blocking_thread_;
        break;
      case BrowserThread::FILE:
        TRACE_EVENT_BEGIN1("startup",
            "BrowserMainLoop::CreateThreads:start",
            "Thread", "BrowserThread::FILE");
        thread_to_start = &file_thread_;
#if defined(OS_WIN)
        // On Windows, the FILE thread needs to be have a UI message loop
        // which pumps messages in such a way that Google Update can
        // communicate back to us.
        options = ui_message_loop_options;
#else
        options = io_message_loop_options;
#endif
        options.timer_slack = base::TIMER_SLACK_MAXIMUM;
        break;
      case BrowserThread::PROCESS_LAUNCHER:
        TRACE_EVENT_BEGIN1("startup",
            "BrowserMainLoop::CreateThreads:start",
            "Thread", "BrowserThread::PROCESS_LAUNCHER");
        thread_to_start = &process_launcher_thread_;
        options.timer_slack = base::TIMER_SLACK_MAXIMUM;
        break;
      case BrowserThread::CACHE:
        TRACE_EVENT_BEGIN1("startup",
            "BrowserMainLoop::CreateThreads:start",
            "Thread", "BrowserThread::CACHE");
        thread_to_start = &cache_thread_;
        options = io_message_loop_options;
        options.timer_slack = base::TIMER_SLACK_MAXIMUM;
        break;
      case BrowserThread::IO:
        TRACE_EVENT_BEGIN1("startup",
            "BrowserMainLoop::CreateThreads:start",
            "Thread", "BrowserThread::IO");
        thread_to_start = &io_thread_;
        options = io_message_loop_options;
        break;
      case BrowserThread::UI:
      case BrowserThread::ID_COUNT:
      default:
        NOTREACHED();
        break;
    }

    BrowserThread::ID id = static_cast<BrowserThread::ID>(thread_id);

    if (thread_to_start) {
      (*thread_to_start).reset(new BrowserProcessSubThread(id));
      if (!(*thread_to_start)->StartWithOptions(options)) {
        LOG(FATAL) << "Failed to start the browser thread: id == " << id;
      }
    } else {
      NOTREACHED();
    }

    TRACE_EVENT_END0("startup", "BrowserMainLoop::CreateThreads:start");
  }
  created_threads_ = true;
  return result_code_;
}

int BrowserMainLoop::PreMainMessageLoopRun() {
  if (parts_) {
    TRACE_EVENT0("startup",
        "BrowserMainLoop::CreateThreads:PreMainMessageLoopRun");
    TRACK_SCOPED_REGION(
        "Startup", "BrowserMainLoop::PreMainMessageLoopRun");

    parts_->PreMainMessageLoopRun();
  }

  // If the UI thread blocks, the whole UI is unresponsive.
  // Do not allow disk IO from the UI thread.
  base::ThreadRestrictions::SetIOAllowed(false);
  base::ThreadRestrictions::DisallowWaiting();
  return result_code_;
}

void BrowserMainLoop::RunMainMessageLoopParts() {
  TRACE_EVENT_BEGIN_ETW("BrowserMain:MESSAGE_LOOP", 0, "");

  bool ran_main_loop = false;
  if (parts_)
    ran_main_loop = parts_->MainMessageLoopRun(&result_code_);

  if (!ran_main_loop)
    MainMessageLoopRun();

  TRACE_EVENT_END_ETW("BrowserMain:MESSAGE_LOOP", 0, "");
}

void BrowserMainLoop::ShutdownThreadsAndCleanUp() {
  if (!created_threads_) {
    // Called early, nothing to do
    return;
  }
  TRACE_EVENT0("shutdown", "BrowserMainLoop::ShutdownThreadsAndCleanUp");

  // Teardown may start in PostMainMessageLoopRun, and during teardown we
  // need to be able to perform IO.
  base::ThreadRestrictions::SetIOAllowed(true);
  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(base::IgnoreResult(&base::ThreadRestrictions::SetIOAllowed),
                 true));

  mojo_shell_context_.reset();

#if !defined(OS_IOS)
  if (RenderProcessHost::run_renderer_in_process())
    RenderProcessHostImpl::ShutDownInProcessRenderer();
#endif

  if (parts_) {
    TRACE_EVENT0("shutdown",
                 "BrowserMainLoop::Subsystem:PostMainMessageLoopRun");
    parts_->PostMainMessageLoopRun();
  }

  trace_memory_controller_.reset();
  system_stats_monitor_.reset();

#if !defined(OS_IOS)
  // Destroying the GpuProcessHostUIShims on the UI thread posts a task to
  // delete related objects on the GPU thread. This must be done before
  // stopping the GPU thread. The GPU thread will close IPC channels to renderer
  // processes so this has to happen before stopping the IO thread.
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:GPUProcessHostShim");
    GpuProcessHostUIShim::DestroyAll();
  }
  // Cancel pending requests and prevent new requests.
  if (resource_dispatcher_host_) {
    TRACE_EVENT0("shutdown",
                 "BrowserMainLoop::Subsystem:ResourceDispatcherHost");
    resource_dispatcher_host_.get()->Shutdown();
  }

  memory_pressure_monitor_.reset();

#if defined(OS_MACOSX)
  BrowserCompositorMac::DisableRecyclingForShutdown();
#endif

#if defined(USE_AURA) || defined(OS_MACOSX)
  {
    TRACE_EVENT0("shutdown",
                 "BrowserMainLoop::Subsystem:ImageTransportFactory");
    ImageTransportFactory::Terminate();
  }
#endif

#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID)
  ZygoteHostImpl::GetInstance()->TearDownAfterLastChild();
#endif  // defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID)

  // The device monitors are using |system_monitor_| as dependency, so delete
  // them before |system_monitor_| goes away.
  // On Mac and windows, the monitor needs to be destroyed on the same thread
  // as they were created. On Linux, the monitor will be deleted when IO thread
  // goes away.
#if defined(OS_WIN)
  system_message_window_.reset();
#elif defined(OS_MACOSX)
  device_monitor_mac_.reset();
#endif
#endif  // !defined(OS_IOS)

  // Must be size_t so we can subtract from it.
  for (size_t thread_id = BrowserThread::ID_COUNT - 1;
       thread_id >= (BrowserThread::UI + 1);
       --thread_id) {
    // Find the thread object we want to stop. Looping over all valid
    // BrowserThread IDs and DCHECKing on a missing case in the switch
    // statement helps avoid a mismatch between this code and the
    // BrowserThread::ID enumeration.
    //
    // The destruction order is the reverse order of occurrence in the
    // BrowserThread::ID list. The rationale for the order is as
    // follows (need to be filled in a bit):
    //
    //
    // - The IO thread is the only user of the CACHE thread.
    //
    // - The PROCESS_LAUNCHER thread must be stopped after IO in case
    //   the IO thread posted a task to terminate a process on the
    //   process launcher thread.
    //
    // - (Not sure why DB stops last.)
    switch (thread_id) {
      case BrowserThread::DB: {
        TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:DBThread");
        ResetThread_DB(db_thread_.Pass());
        break;
      }
      case BrowserThread::FILE: {
        TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:FileThread");
#if !defined(OS_IOS)
        // Clean up state that lives on or uses the file_thread_ before
        // it goes away.
        if (resource_dispatcher_host_)
          resource_dispatcher_host_.get()->save_file_manager()->Shutdown();
#endif  // !defined(OS_IOS)
        ResetThread_FILE(file_thread_.Pass());
        break;
      }
      case BrowserThread::FILE_USER_BLOCKING: {
        TRACE_EVENT0("shutdown",
                      "BrowserMainLoop::Subsystem:FileUserBlockingThread");
        ResetThread_FILE_USER_BLOCKING(file_user_blocking_thread_.Pass());
        break;
      }
      case BrowserThread::PROCESS_LAUNCHER: {
        TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:LauncherThread");
        ResetThread_PROCESS_LAUNCHER(process_launcher_thread_.Pass());
        break;
      }
      case BrowserThread::CACHE: {
        TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:CacheThread");
        ResetThread_CACHE(cache_thread_.Pass());
        break;
      }
      case BrowserThread::IO: {
        TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:IOThread");
        ResetThread_IO(io_thread_.Pass());
        break;
      }
      case BrowserThread::UI:
      case BrowserThread::ID_COUNT:
      default:
        NOTREACHED();
        break;
    }
  }

#if !defined(OS_IOS)
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:IndexedDBThread");
    ResetThread_IndexedDb(indexed_db_thread_.Pass());
  }
#endif

  // Close the blocking I/O pool after the other threads. Other threads such
  // as the I/O thread may need to schedule work like closing files or flushing
  // data during shutdown, so the blocking pool needs to be available. There
  // may also be slow operations pending that will blcok shutdown, so closing
  // it here (which will block until required operations are complete) gives
  // more head start for those operations to finish.
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:ThreadPool");
    BrowserThreadImpl::ShutdownThreadPool();
  }

#if !defined(OS_IOS)
  // Must happen after the IO thread is shutdown since this may be accessed from
  // it.
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:GPUChannelFactory");
    if (BrowserGpuChannelHostFactory::instance())
      BrowserGpuChannelHostFactory::Terminate();
  }

  // Must happen after the I/O thread is shutdown since this class lives on the
  // I/O thread and isn't threadsafe.
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:GamepadService");
    GamepadService::GetInstance()->Terminate();
  }
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:SensorService");
    DeviceInertialSensorService::GetInstance()->Shutdown();
  }
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:BatteryStatusService");
    device::BatteryStatusService::GetInstance()->Shutdown();
  }
  {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:DeleteDataSources");
    URLDataManager::DeleteDataSources();
  }
#endif  // !defined(OS_IOS)

  if (parts_) {
    TRACE_EVENT0("shutdown", "BrowserMainLoop::Subsystem:PostDestroyThreads");
    parts_->PostDestroyThreads();
  }
}

void BrowserMainLoop::StopStartupTracingTimer() {
  startup_trace_timer_.Stop();
}

void BrowserMainLoop::InitializeMainThread() {
  TRACE_EVENT0("startup", "BrowserMainLoop::InitializeMainThread");
  static const char kThreadName[] = "CrBrowserMain";
  base::PlatformThread::SetName(kThreadName);
  if (main_message_loop_)
    main_message_loop_->set_thread_name(kThreadName);

  // Register the main thread by instantiating it, but don't call any methods.
  main_thread_.reset(
      new BrowserThreadImpl(BrowserThread::UI, base::MessageLoop::current()));
}

int BrowserMainLoop::BrowserThreadsStarted() {
  TRACE_EVENT0("startup", "BrowserMainLoop::BrowserThreadsStarted");

#if !defined(OS_IOS)
  indexed_db_thread_.reset(new base::Thread("IndexedDB"));
  indexed_db_thread_->Start();
#endif

#if !defined(OS_IOS)
  HistogramSynchronizer::GetInstance();


  // GpuDataManager for in-process initialized in PreCreateThreads.
  bool initialize_gpu_data_manager = !UsingInProcessGpu();
#if defined(OS_ANDROID)
  // Up the priority of anything that touches with display tasks
  // (this thread is UI thread, and io_thread_ is for IPCs).
  io_thread_->SetPriority(base::ThreadPriority::DISPLAY);
  base::PlatformThread::SetThreadPriority(base::PlatformThread::CurrentHandle(),
                                          base::ThreadPriority::DISPLAY);

  // On Android, GLSurface::InitializeOneOff() must be called before
  // initalizing the GpuDataManagerImpl as it uses the GL bindings.
  // TODO(sievers): Shouldn't need to init full bindings to determine GL
  // version/vendor strings. crbug.com/326295
  if (initialize_gpu_data_manager) {
    // Note InitializeOneOff is not safe either for in-process gpu after
    // creating threads, since it may race with the gpu thread.
    if (!gfx::GLSurface::InitializeOneOff()) {
      LOG(FATAL) << "GLSurface::InitializeOneOff failed";
    }
  }
#endif

  if (initialize_gpu_data_manager)
    GpuDataManagerImpl::GetInstance()->Initialize();

  bool always_uses_gpu = true;
  bool established_gpu_channel = false;
#if defined(OS_ANDROID)
  // TODO(crbug.com/439322): This should be set to |true|.
  established_gpu_channel = false;
  BrowserGpuChannelHostFactory::Initialize(established_gpu_channel);
#elif defined(USE_AURA) || defined(OS_MACOSX)
  established_gpu_channel = true;
  if (!GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor() ||
      parsed_command_line_.HasSwitch(switches::kDisableGpuEarlyInit)) {
    established_gpu_channel = always_uses_gpu = false;
  }
  BrowserGpuChannelHostFactory::Initialize(established_gpu_channel);
  ImageTransportFactory::Initialize();
#if defined(USE_AURA)
  if (aura::Env::GetInstance()) {
    aura::Env::GetInstance()->set_context_factory(GetContextFactory());
  }
#endif  // defined(USE_AURA)
#endif  // defined(OS_ANDROID)

  // Enable the GpuMemoryBuffer dump provider with IO thread affinity. Note that
  // unregistration happens on the IO thread (See
  // BrowserProcessSubThread::IOThreadPreCleanUp).
  base::trace_event::MemoryDumpManager::GetInstance()->RegisterDumpProvider(
      BrowserGpuMemoryBufferManager::current(), io_thread_->task_runner());

  {
    TRACE_EVENT0("startup", "BrowserThreadsStarted::Subsystem:AudioMan");
    audio_manager_.reset(media::AudioManager::CreateWithHangTimer(
        MediaInternals::GetInstance(), io_thread_->task_runner()));
  }

  {
    TRACE_EVENT0("startup", "BrowserThreadsStarted::Subsystem:MidiManager");
    midi_manager_.reset(media::midi::MidiManager::Create());
  }

#if defined(OS_LINUX) && defined(USE_UDEV)
  device_monitor_linux_.reset(new DeviceMonitorLinux());
#elif defined(OS_MACOSX)
  device_monitor_mac_.reset(new DeviceMonitorMac());
#endif

#if defined(OS_WIN)
  UMA_HISTOGRAM_BOOLEAN("Windows.Win32kRendererLockdown",
                        IsWin32kRendererLockdownEnabled());
#endif
  // RDH needs the IO thread to be created
  {
    TRACE_EVENT0("startup",
      "BrowserMainLoop::BrowserThreadsStarted:InitResourceDispatcherHost");
    resource_dispatcher_host_.reset(new ResourceDispatcherHostImpl());
  }

  // MediaStreamManager needs the IO thread to be created.
  {
    TRACE_EVENT0("startup",
      "BrowserMainLoop::BrowserThreadsStarted:InitMediaStreamManager");
    media_stream_manager_.reset(new MediaStreamManager(audio_manager_.get()));
  }

  {
    TRACE_EVENT0("startup",
      "BrowserMainLoop::BrowserThreadsStarted:InitSpeechRecognition");
    speech_recognition_manager_.reset(new SpeechRecognitionManagerImpl(
        audio_manager_.get(), media_stream_manager_.get()));
  }

  {
    TRACE_EVENT0(
        "startup",
        "BrowserMainLoop::BrowserThreadsStarted::InitUserInputMonitor");
    user_input_monitor_ = media::UserInputMonitor::Create(
        io_thread_->task_runner(), main_thread_->task_runner());
  }

  {
    TRACE_EVENT0("startup",
                 "BrowserMainLoop::BrowserThreadsStarted::TimeZoneMonitor");
    time_zone_monitor_ = TimeZoneMonitor::Create();
  }

  // Alert the clipboard class to which threads are allowed to access the
  // clipboard:
  std::vector<base::PlatformThreadId> allowed_clipboard_threads;
  // The current thread is the UI thread.
  allowed_clipboard_threads.push_back(base::PlatformThread::CurrentId());
#if defined(OS_WIN)
  // On Windows, clipboards are also used on the FILE or IO threads.
  allowed_clipboard_threads.push_back(file_thread_->thread_id());
  allowed_clipboard_threads.push_back(io_thread_->thread_id());
#endif
  ui::Clipboard::SetAllowedThreads(allowed_clipboard_threads);

  // When running the GPU thread in-process, avoid optimistically starting it
  // since creating the GPU thread races against creation of the one-and-only
  // ChildProcess instance which is created by the renderer thread.
  if (GpuDataManagerImpl::GetInstance()->GpuAccessAllowed(NULL) &&
      !established_gpu_channel &&
      always_uses_gpu &&
      !UsingInProcessGpu()) {
    TRACE_EVENT_INSTANT0("gpu", "Post task to launch GPU process",
                         TRACE_EVENT_SCOPE_THREAD);
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(base::IgnoreResult(&GpuProcessHost::Get),
                   GpuProcessHost::GPU_PROCESS_KIND_SANDBOXED,
                   CAUSE_FOR_GPU_LAUNCH_BROWSER_STARTUP));
  }

#if defined(OS_MACOSX)
  ThemeHelperMac::GetInstance();
  SystemHotkeyHelperMac::GetInstance()->DeferredLoadSystemHotkeys();
  if (ShouldEnableBootstrapSandbox()) {
    TRACE_EVENT0("startup",
                 "BrowserMainLoop::BrowserThreadsStarted:BootstrapSandbox");
    CHECK(GetBootstrapSandbox());
  }
#endif  // defined(OS_MACOSX)

#endif  // !defined(OS_IOS)

  mojo_shell_context_.reset(new MojoShellContext);

  return result_code_;
}

bool BrowserMainLoop::UsingInProcessGpu() const {
  return parsed_command_line_.HasSwitch(switches::kSingleProcess) ||
         parsed_command_line_.HasSwitch(switches::kInProcessGPU);
}

bool BrowserMainLoop::InitializeToolkit() {
  TRACE_EVENT0("startup", "BrowserMainLoop::InitializeToolkit");
  TRACK_SCOPED_REGION("Startup", "BrowserMainLoop::InitializeToolkit");

  // TODO(evan): this function is rather subtle, due to the variety
  // of intersecting ifdefs we have.  To keep it easy to follow, there
  // are no #else branches on any #ifs.
  // TODO(stevenjb): Move platform specific code into platform specific Parts
  // (Need to add InitializeToolkit stage to BrowserParts).
  // See also GTK setup in EarlyInitialization, above, and associated comments.

#if defined(OS_WIN)
  // Init common control sex.
  INITCOMMONCONTROLSEX config;
  config.dwSize = sizeof(config);
  config.dwICC = ICC_WIN95_CLASSES;
  if (!InitCommonControlsEx(&config))
    PLOG(FATAL);
#endif

#if defined(USE_AURA)

#if defined(USE_X11)
  if (!gfx::GetXDisplay())
    return false;
#endif

  // Env creates the compositor. Aura widgets need the compositor to be created
  // before they can be initialized by the browser.
  aura::Env::CreateInstance(true);
#endif  // defined(USE_AURA)

  if (parts_)
    parts_->ToolkitInitialized();

  return true;
}

void BrowserMainLoop::MainMessageLoopRun() {
#if defined(OS_ANDROID)
  // Android's main message loop is the Java message loop.
  NOTREACHED();
#else
  DCHECK(base::MessageLoopForUI::IsCurrent());
  if (parameters_.ui_task) {
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                  *parameters_.ui_task);
  }

  base::RunLoop run_loop;
  run_loop.Run();
#endif
}

base::FilePath BrowserMainLoop::GetStartupTraceFileName(
    const base::CommandLine& command_line) const {
  base::FilePath trace_file = command_line.GetSwitchValuePath(
      switches::kTraceStartupFile);
  // trace_file = "none" means that startup events will show up for the next
  // begin/end tracing (via about:tracing or AutomationProxy::BeginTracing/
  // EndTracing, for example).
  if (trace_file == base::FilePath().AppendASCII("none"))
    return trace_file;

  if (trace_file.empty()) {
#if defined(OS_ANDROID)
    TracingControllerAndroid::GenerateTracingFilePath(&trace_file);
#else
    // Default to saving the startup trace into the current dir.
    trace_file = base::FilePath().AppendASCII("chrometrace.log");
#endif
  }

  return trace_file;
}

void BrowserMainLoop::InitStartupTracing(
    const base::CommandLine& command_line) {
  DCHECK(is_tracing_startup_);

  startup_trace_file_ = GetStartupTraceFileName(parsed_command_line_);

  std::string delay_str = command_line.GetSwitchValueASCII(
      switches::kTraceStartupDuration);
  int delay_secs = 5;
  if (!delay_str.empty() && !base::StringToInt(delay_str, &delay_secs)) {
    DLOG(WARNING) << "Could not parse --" << switches::kTraceStartupDuration
        << "=" << delay_str << " defaulting to 5 (secs)";
    delay_secs = 5;
  }

  startup_trace_timer_.Start(FROM_HERE,
                             base::TimeDelta::FromSeconds(delay_secs),
                             this,
                             &BrowserMainLoop::EndStartupTracing);
}

void BrowserMainLoop::EndStartupTracing() {
  DCHECK(is_tracing_startup_);

  is_tracing_startup_ = false;
  TracingController::GetInstance()->DisableRecording(
      TracingController::CreateFileSink(
          startup_trace_file_,
          base::Bind(OnStoppedStartupTracing, startup_trace_file_)));
}

}  // namespace content
