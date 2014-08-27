// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains the zero-suggest autocomplete provider. This experimental
// provider is invoked when the user focuses in the omnibox prior to editing,
// and generates search query suggestions based on the current URL.

#ifndef CHROME_BROWSER_AUTOCOMPLETE_ZERO_SUGGEST_PROVIDER_H_
#define CHROME_BROWSER_AUTOCOMPLETE_ZERO_SUGGEST_PROVIDER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/history/history_types.h"
#include "components/metrics/proto/omnibox_event.pb.h"
#include "components/omnibox/base_search_provider.h"
#include "components/omnibox/search_provider.h"
#include "net/url_request/url_fetcher_delegate.h"

class AutocompleteProviderListener;
class Profile;
class TemplateURLService;

namespace base {
class ListValue;
class Value;
}

namespace net {
class URLFetcher;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

// Autocomplete provider for searches based on the current URL.
//
// The controller will call StartZeroSuggest when the user focuses in the
// omnibox. After construction, the autocomplete controller repeatedly calls
// Start() with some user input, each time expecting to receive an updated
// set of matches.
//
// TODO(jered): Consider deleting this class and building this functionality
// into SearchProvider after dogfood and after we break the association between
// omnibox text and suggestions.
class ZeroSuggestProvider : public BaseSearchProvider,
                            public net::URLFetcherDelegate {
 public:
  // Creates and returns an instance of this provider.
  static ZeroSuggestProvider* Create(AutocompleteProviderListener* listener,
                                     TemplateURLService* template_url_service,
                                     Profile* profile);

  // Registers a preference used to cache zero suggest results.
  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

  // AutocompleteProvider:
  virtual void Start(const AutocompleteInput& input,
                     bool minimal_changes) OVERRIDE;
  virtual void Stop(bool clear_cached_results) OVERRIDE;
  virtual void DeleteMatch(const AutocompleteMatch& match) OVERRIDE;
  virtual void AddProviderInfo(ProvidersInfo* provider_info) const OVERRIDE;

  // Sets |field_trial_triggered_| to false.
  virtual void ResetSession() OVERRIDE;

 private:
  ZeroSuggestProvider(AutocompleteProviderListener* listener,
                      TemplateURLService* template_url_service,
                      Profile* profile);

  virtual ~ZeroSuggestProvider();

  // BaseSearchProvider:
  virtual const TemplateURL* GetTemplateURL(bool is_keyword) const OVERRIDE;
  virtual const AutocompleteInput GetInput(bool is_keyword) const OVERRIDE;
  virtual bool ShouldAppendExtraParams(
      const SearchSuggestionParser::SuggestResult& result) const OVERRIDE;
  virtual void RecordDeletionResult(bool success) OVERRIDE;

  // net::URLFetcherDelegate:
  virtual void OnURLFetchComplete(const net::URLFetcher* source) OVERRIDE;

  // Optionally, cache the received |json_data| and return true if we want
  // to stop processing results at this point. The |parsed_data| is the parsed
  // version of |json_data| used to determine if we received an empty result.
  bool StoreSuggestionResponse(const std::string& json_data,
                               const base::Value& parsed_data);

  // Adds AutocompleteMatches for each of the suggestions in |results| to
  // |map|.
  void AddSuggestResultsToMap(
      const SearchSuggestionParser::SuggestResults& results,
      MatchMap* map);

  // Returns an AutocompleteMatch for a navigational suggestion |navigation|.
  AutocompleteMatch NavigationToMatch(
      const SearchSuggestionParser::NavigationResult& navigation);

  // Fetches zero-suggest suggestions by sending a request using |suggest_url|.
  void Run(const GURL& suggest_url);

  // Converts the parsed results to a set of AutocompleteMatches and adds them
  // to |matches_|.  Also update the histograms for how many results were
  // received.
  void ConvertResultsToAutocompleteMatches();

  // Returns an AutocompleteMatch for the current URL. The match should be in
  // the top position so that pressing enter has the effect of reloading the
  // page.
  AutocompleteMatch MatchForCurrentURL();

  // When the user is in the Most Visited field trial, we ask the TopSites
  // service for the most visited URLs during Run().  It calls back to this
  // function to return those |urls|.
  void OnMostVisitedUrlsAvailable(const history::MostVisitedURLList& urls);

  // Returns the relevance score for the verbatim result.
  int GetVerbatimRelevance() const;

  // Whether we can show zero suggest on |current_page_url| without
  // sending |current_page_url| as a parameter to the server at |suggest_url|.
  bool CanShowZeroSuggestWithoutSendingURL(const GURL& suggest_url,
                                           const GURL& current_page_url) const;

  // Checks whether we have a set of zero suggest results cached, and if so
  // populates |matches_| with cached results.
  void MaybeUseCachedSuggestions();

  AutocompleteProviderListener* listener_;
  Profile* profile_;

  // The URL for which a suggestion fetch is pending.
  std::string current_query_;

  // The type of page the user is viewing (a search results page doing search
  // term replacement, an arbitrary URL, etc.).
  metrics::OmniboxEventProto::PageClassification current_page_classification_;

  // Copy of OmniboxEditModel::permanent_text_.
  base::string16 permanent_text_;

  // Fetcher used to retrieve results.
  scoped_ptr<net::URLFetcher> fetcher_;

  // Suggestion for the current URL.
  AutocompleteMatch current_url_match_;

  // Contains suggest and navigation results as well as relevance parsed from
  // the response for the most recent zero suggest input URL.
  SearchSuggestionParser::Results results_;

  // Whether we are currently showing cached zero suggest results.
  bool results_from_cache_;

  history::MostVisitedURLList most_visited_urls_;

  // For callbacks that may be run after destruction.
  base::WeakPtrFactory<ZeroSuggestProvider> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ZeroSuggestProvider);
};

#endif  // CHROME_BROWSER_AUTOCOMPLETE_ZERO_SUGGEST_PROVIDER_H_
