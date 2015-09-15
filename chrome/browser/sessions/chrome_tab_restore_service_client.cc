// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sessions/chrome_tab_restore_service_client.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sessions/session_service.h"
#include "chrome/browser/sessions/session_service_factory.h"
#include "chrome/common/url_constants.h"
#include "content/public/browser/browser_thread.h"

#if defined(ENABLE_EXTENSIONS)
#include "chrome/browser/extensions/tab_helper.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/common/extensions/extension_metrics.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_set.h"
#endif

#if !defined(OS_ANDROID)
#include "chrome/browser/ui/browser_tab_restore_service_delegate.h"
#endif

namespace {

void RecordAppLaunch(Profile* profile, const GURL& url) {
#if defined(ENABLE_EXTENSIONS)
  const extensions::Extension* extension =
      extensions::ExtensionRegistry::Get(profile)
          ->enabled_extensions()
          .GetAppByURL(url);
  if (!extension)
    return;

  extensions::RecordAppLaunchType(
      extension_misc::APP_LAUNCH_NTP_RECENTLY_CLOSED, extension->GetType());
#endif  // defined(ENABLE_EXTENSIONS)
}

}  // namespace

ChromeTabRestoreServiceClient::ChromeTabRestoreServiceClient(Profile* profile)
    : profile_(profile) {}

ChromeTabRestoreServiceClient::~ChromeTabRestoreServiceClient() {}

TabRestoreServiceDelegate*
ChromeTabRestoreServiceClient::CreateTabRestoreServiceDelegate(
    int host_desktop_type,
    const std::string& app_name) {
#if defined(OS_ANDROID)
  // Android does not support TabRestoreServiceDelegate, as tab persistence
  // is implemented on the Java side.
  return nullptr;
#else
  return BrowserTabRestoreServiceDelegate::Create(
      profile_, static_cast<chrome::HostDesktopType>(host_desktop_type),
      app_name);
#endif
}

TabRestoreServiceDelegate*
ChromeTabRestoreServiceClient::FindTabRestoreServiceDelegateForWebContents(
    const content::WebContents* contents) {
#if defined(OS_ANDROID)
  // Android does not support TabRestoreServiceDelegate, as tab persistence
  // is implemented on the Java side.
  return nullptr;
#else
  return BrowserTabRestoreServiceDelegate::FindDelegateForWebContents(contents);
#endif
}

TabRestoreServiceDelegate*
ChromeTabRestoreServiceClient::FindTabRestoreServiceDelegateWithID(
    SessionID::id_type desired_id,
    int host_desktop_type) {
#if defined(OS_ANDROID)
  // Android does not support TabRestoreServiceDelegate, as tab persistence
  // is implemented on the Java side.
  return nullptr;
#else
  return BrowserTabRestoreServiceDelegate::FindDelegateWithID(
      desired_id, static_cast<chrome::HostDesktopType>(host_desktop_type));
#endif
}

bool ChromeTabRestoreServiceClient::ShouldTrackURLForRestore(const GURL& url) {
  return ::ShouldTrackURLForRestore(url);
}

std::string ChromeTabRestoreServiceClient::GetExtensionAppIDForWebContents(
    content::WebContents* web_contents) {
  std::string extension_app_id;

#if defined(ENABLE_EXTENSIONS)
  extensions::TabHelper* extensions_tab_helper =
      extensions::TabHelper::FromWebContents(web_contents);
  // extensions_tab_helper is NULL in some browser tests.
  if (extensions_tab_helper) {
    const extensions::Extension* extension =
        extensions_tab_helper->extension_app();
    if (extension)
      extension_app_id = extension->id();
  }
#endif

  return extension_app_id;
}

base::SequencedWorkerPool* ChromeTabRestoreServiceClient::GetBlockingPool() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return content::BrowserThread::GetBlockingPool();
}

base::FilePath ChromeTabRestoreServiceClient::GetPathToSaveTo() {
  return profile_->GetPath();
}

GURL ChromeTabRestoreServiceClient::GetNewTabURL() {
  return GURL(chrome::kChromeUINewTabURL);
}

bool ChromeTabRestoreServiceClient::HasLastSession() {
#if defined(ENABLE_SESSION_SERVICE)
  SessionService* session_service =
      SessionServiceFactory::GetForProfile(profile_);
  Profile::ExitType exit_type = profile_->GetLastSessionExitType();
  // The previous session crashed and wasn't restored, or was a forced
  // shutdown. Both of which won't have notified us of the browser close so
  // that we need to load the windows from session service (which will have
  // saved them).
  return (!profile_->restored_last_session() && session_service &&
          (exit_type == Profile::EXIT_CRASHED ||
           exit_type == Profile::EXIT_SESSION_ENDED));
#else
  return false;
#endif
}

void ChromeTabRestoreServiceClient::GetLastSession(
    const sessions::GetLastSessionCallback& callback,
    base::CancelableTaskTracker* tracker) {
  DCHECK(HasLastSession());
#if defined(ENABLE_SESSION_SERVICE)
  SessionServiceFactory::GetForProfile(profile_)
      ->GetLastSession(callback, tracker);
#endif
}

void ChromeTabRestoreServiceClient::OnTabRestored(const GURL& url) {
  RecordAppLaunch(profile_, url);
}
