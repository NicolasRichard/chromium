// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_AUTOCOMPLETE_CHROME_AUTOCOMPLETE_PROVIDER_CLIENT_H_
#define CHROME_BROWSER_AUTOCOMPLETE_CHROME_AUTOCOMPLETE_PROVIDER_CLIENT_H_

#include "chrome/browser/autocomplete/chrome_autocomplete_scheme_classifier.h"
#include "chrome/browser/search_engines/ui_thread_search_terms_data.h"
#include "components/omnibox/autocomplete_provider_client.h"

class Profile;

class ChromeAutocompleteProviderClient : public AutocompleteProviderClient {
 public:
  explicit ChromeAutocompleteProviderClient(Profile* profile);
  ~ChromeAutocompleteProviderClient() override;

  // AutocompleteProviderClient:
  net::URLRequestContextGetter* GetRequestContext() override;
  const AutocompleteSchemeClassifier& GetSchemeClassifier() override;
  history::HistoryService* GetHistoryService() override;
  bookmarks::BookmarkModel* GetBookmarkModel() override;
  history::URLDatabase* GetInMemoryDatabase() override;
  TemplateURLService* GetTemplateURLService() override;
  const SearchTermsData& GetSearchTermsData() override;
  std::string GetAcceptLanguages() override;
  bool IsOffTheRecord() override;
  bool SearchSuggestEnabled() override;
  bool ShowBookmarkBar() override;
  bool TabSyncEnabledAndUnencrypted() override;
  void Classify(
      const base::string16& text,
      bool prefer_keyword,
      bool allow_exact_keyword_match,
      metrics::OmniboxEventProto::PageClassification page_classification,
      AutocompleteMatch* match,
      GURL* alternate_nav_url) override;
  void DeleteMatchingURLsForKeywordFromHistory(
      history::KeywordID keyword_id,
      const base::string16& term) override;
  void PrefetchImage(const GURL& url) override;

 private:
  Profile* profile_;
  ChromeAutocompleteSchemeClassifier scheme_classifier_;
  UIThreadSearchTermsData search_terms_data_;

  DISALLOW_COPY_AND_ASSIGN(ChromeAutocompleteProviderClient);
};

#endif  // CHROME_BROWSER_AUTOCOMPLETE_CHROME_AUTOCOMPLETE_PROVIDER_CLIENT_H_
