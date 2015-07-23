// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/search_engines/ui_thread_search_terms_data.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/metrics/field_trial.h"
#include "base/prefs/pref_service.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/google/google_brand.h"
#include "chrome/browser/google/google_url_tracker_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search/instant_service.h"
#include "chrome/browser/search/instant_service_factory.h"
#include "chrome/browser/search/search.h"
#include "chrome/browser/themes/theme_service.h"
#include "chrome/browser/themes/theme_service_factory.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/chrome_version_info.h"
#include "chrome/common/pref_names.h"
#include "components/google/core/browser/google_url_tracker.h"
#include "components/google/core/browser/google_util.h"
#include "components/omnibox/browser/omnibox_field_trial.h"
#include "components/search/search.h"
#include "content/public/browser/browser_thread.h"
#include "ui/base/device_form_factor.h"
#include "url/gurl.h"

#if defined(ENABLE_RLZ)
#include "components/rlz/rlz_tracker.h"
#endif

using content::BrowserThread;

// static
std::string* UIThreadSearchTermsData::google_base_url_ = NULL;

UIThreadSearchTermsData::UIThreadSearchTermsData(Profile* profile)
    : profile_(profile) {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
      BrowserThread::CurrentlyOn(BrowserThread::UI));
}

std::string UIThreadSearchTermsData::GoogleBaseURLValue() const {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
      BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (google_base_url_)
    return *google_base_url_;
  GURL base_url(google_util::CommandLineGoogleBaseURL());
  if (base_url.is_valid())
    return base_url.spec();

  if (!profile_)
    return SearchTermsData::GoogleBaseURLValue();

  const GoogleURLTracker* tracker =
      GoogleURLTrackerFactory::GetForProfile(profile_);
  return tracker ?
      tracker->google_url().spec() : GoogleURLTracker::kDefaultGoogleHomepage;
}

std::string UIThreadSearchTermsData::GetApplicationLocale() const {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
      BrowserThread::CurrentlyOn(BrowserThread::UI));
  return g_browser_process->GetApplicationLocale();
}

// Android implementations are in ui_thread_search_terms_data_android.cc.
#if !defined(OS_ANDROID)
base::string16 UIThreadSearchTermsData::GetRlzParameterValue(
    bool from_app_list) const {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
      BrowserThread::CurrentlyOn(BrowserThread::UI));
  base::string16 rlz_string;
#if defined(ENABLE_RLZ)
  // For organic brandcodes do not use rlz at all. Empty brandcode usually
  // means a chromium install. This is ok.
  std::string brand;
  if (google_brand::GetBrand(&brand) && !brand.empty() &&
      !google_brand::IsOrganic(brand)) {
    // This call will return false the first time(s) it is called until the
    // value has been cached. This normally would mean that at most one omnibox
    // search might not send the RLZ data but this is not really a problem.
    rlz_lib::AccessPoint access_point = rlz::RLZTracker::ChromeOmnibox();
#if !defined(OS_IOS)
    if (from_app_list)
      access_point = rlz::RLZTracker::ChromeAppList();
#endif
    rlz::RLZTracker::GetAccessPointRlz(access_point, &rlz_string);
  }
#endif
  return rlz_string;
}

// We can enable this on non-Android if other platforms ever want a non-empty
// search client string.  There is already a unit test in place for Android
// called TemplateURLTest::SearchClient.
std::string UIThreadSearchTermsData::GetSearchClient() const {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
      BrowserThread::CurrentlyOn(BrowserThread::UI));
  return std::string();
}
#endif

std::string UIThreadSearchTermsData::GetSuggestClient() const {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
      BrowserThread::CurrentlyOn(BrowserThread::UI));
#if defined(OS_ANDROID)
  return ui::GetDeviceFormFactor() == ui::DEVICE_FORM_FACTOR_PHONE ?
      "chrome" : "chrome-omni";
#else
  return chrome::IsInstantExtendedAPIEnabled() ? "chrome-omni" : "chrome";
#endif
}

std::string UIThreadSearchTermsData::GetSuggestRequestIdentifier() const {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
      BrowserThread::CurrentlyOn(BrowserThread::UI));
#if defined(OS_ANDROID)
  if (ui::GetDeviceFormFactor() == ui::DEVICE_FORM_FACTOR_PHONE)
    return "chrome-mobile-ext-ansg";
#endif
  return "chrome-ext-ansg";
}

bool UIThreadSearchTermsData::IsShowingSearchTermsOnSearchResultsPages() const {
  return chrome::IsInstantExtendedAPIEnabled() &&
      chrome::IsQueryExtractionEnabled();
}

std::string UIThreadSearchTermsData::InstantExtendedEnabledParam(
    bool for_search) const {
  return chrome::InstantExtendedEnabledParam(for_search);
}

std::string UIThreadSearchTermsData::ForceInstantResultsParam(
    bool for_prerender) const {
  return chrome::ForceInstantResultsParam(for_prerender);
}

int UIThreadSearchTermsData::OmniboxStartMargin() const {
  InstantService* instant_service =
      InstantServiceFactory::GetForProfile(profile_);
  // Android and iOS have no InstantService.
  return instant_service ?
      instant_service->omnibox_start_margin() : chrome::kDisableStartMargin;
}

std::string UIThreadSearchTermsData::NTPIsThemedParam() const {
  DCHECK(!BrowserThread::IsThreadInitialized(BrowserThread::UI) ||
         BrowserThread::CurrentlyOn(BrowserThread::UI));
#if defined(ENABLE_THEMES)
  if (!chrome::IsInstantExtendedAPIEnabled())
    return std::string();

  // TODO(dhollowa): Determine fraction of custom themes that don't affect the
  // NTP background and/or color.
  ThemeService* theme_service = ThemeServiceFactory::GetForProfile(profile_);
  // NTP is considered themed if the theme is not default and not native (GTK+).
  if (theme_service && !theme_service->UsingDefaultTheme() &&
      !theme_service->UsingSystemTheme())
    return "es_th=1&";
#endif  // defined(ENABLE_THEMES)

  return std::string();
}

// It's acutally OK to call this method on any thread, but it's currently placed
// in UIThreadSearchTermsData since SearchTermsData cannot depend on
// VersionInfo.
std::string UIThreadSearchTermsData::GoogleImageSearchSource() const {
  chrome::VersionInfo version_info;
  std::string version(version_info.Name() + " " + version_info.Version());
  if (version_info.IsOfficialBuild())
    version += " (Official)";
  version += " " + version_info.OSType();
  std::string modifier(version_info.GetVersionStringModifier());
  if (!modifier.empty())
    version += " " + modifier;
  return version;
}

std::string UIThreadSearchTermsData::GetAcceptLanguages() const {
  return profile_ ? profile_->GetPrefs()->GetString(prefs::kAcceptLanguages)
                  : std::string();
}

// static
void UIThreadSearchTermsData::SetGoogleBaseURL(const std::string& base_url) {
  delete google_base_url_;
  google_base_url_ = base_url.empty() ? NULL : new std::string(base_url);
}
