// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/declarative_content/declarative_content_page_url_condition_tracker.h"

#include "base/bind.h"
#include "base/stl_util.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/browser/extensions/api/declarative_content/content_constants.h"
#include "components/url_matcher/url_matcher_factory.h"
#include "content/public/browser/web_contents.h"

namespace extensions {

namespace {

const char kInvalidTypeOfParameter[] = "Attribute '%s' has an invalid type";

static url_matcher::URLMatcherConditionSet::ID g_next_id = 0;

}  // namespace

//
// DeclarativeContentPageUrlPredicate
//

DeclarativeContentPageUrlPredicate::~DeclarativeContentPageUrlPredicate() {
}

// static
scoped_ptr<DeclarativeContentPageUrlPredicate>
DeclarativeContentPageUrlPredicate::Create(
    ContentPredicateEvaluator* evaluator,
    url_matcher::URLMatcherConditionFactory* url_matcher_condition_factory,
    const base::Value& value,
    std::string* error) {
  scoped_refptr<url_matcher::URLMatcherConditionSet> url_matcher_condition_set;
  const base::DictionaryValue* dict = nullptr;
  if (!value.GetAsDictionary(&dict)) {
    *error = base::StringPrintf(kInvalidTypeOfParameter,
                                declarative_content_constants::kPageUrl);
    return scoped_ptr<DeclarativeContentPageUrlPredicate>();
  } else {
    url_matcher_condition_set =
        url_matcher::URLMatcherFactory::CreateFromURLFilterDictionary(
            url_matcher_condition_factory, dict, ++g_next_id, error);
    if (!url_matcher_condition_set)
      return scoped_ptr<DeclarativeContentPageUrlPredicate>();
    return make_scoped_ptr(
        new DeclarativeContentPageUrlPredicate(evaluator,
                                               url_matcher_condition_set));
  }
}

ContentPredicateEvaluator*
DeclarativeContentPageUrlPredicate::GetEvaluator() const {
  return evaluator_;
}

DeclarativeContentPageUrlPredicate::DeclarativeContentPageUrlPredicate(
    ContentPredicateEvaluator* evaluator,
    scoped_refptr<url_matcher::URLMatcherConditionSet>
        url_matcher_condition_set)
    : evaluator_(evaluator),
      url_matcher_condition_set_(url_matcher_condition_set) {
  DCHECK(url_matcher_condition_set);
}

//
// PerWebContentsTracker
//

DeclarativeContentPageUrlConditionTracker::PerWebContentsTracker::
PerWebContentsTracker(
    content::WebContents* contents,
    url_matcher::URLMatcher* url_matcher,
    const RequestEvaluationCallback& request_evaluation,
    const WebContentsDestroyedCallback& web_contents_destroyed)
    : WebContentsObserver(contents),
      url_matcher_(url_matcher),
      request_evaluation_(request_evaluation),
      web_contents_destroyed_(web_contents_destroyed) {
}

DeclarativeContentPageUrlConditionTracker::PerWebContentsTracker::
~PerWebContentsTracker() {
}

void DeclarativeContentPageUrlConditionTracker::PerWebContentsTracker::
UpdateMatchesForCurrentUrl(bool request_evaluation_if_unchanged) {
  std::set<url_matcher::URLMatcherConditionSet::ID> new_matches =
      url_matcher_->MatchURL(web_contents()->GetVisibleURL());
  matches_.swap(new_matches);
  if (matches_ != new_matches || request_evaluation_if_unchanged)
    request_evaluation_.Run(web_contents());
}

void DeclarativeContentPageUrlConditionTracker::PerWebContentsTracker::
WebContentsDestroyed() {
  web_contents_destroyed_.Run(web_contents());
}

//
// DeclarativeContentPageUrlConditionTracker
//

DeclarativeContentPageUrlConditionTracker::
DeclarativeContentPageUrlConditionTracker(Delegate* delegate)
    : delegate_(delegate) {
}

DeclarativeContentPageUrlConditionTracker::
~DeclarativeContentPageUrlConditionTracker() {
}

std::string DeclarativeContentPageUrlConditionTracker::
GetPredicateApiAttributeName() const {
  return declarative_content_constants::kPageUrl;
}

scoped_ptr<const ContentPredicate> DeclarativeContentPageUrlConditionTracker::
CreatePredicate(const Extension* extension,
                const base::Value& value,
                std::string* error) {
  return DeclarativeContentPageUrlPredicate::Create(this,
      url_matcher_.condition_factory(), value, error);
}

void DeclarativeContentPageUrlConditionTracker::TrackPredicates(
    const std::map<const void*, std::vector<const ContentPredicate*>>&
        predicates) {
  if (predicates.empty()) {
    // Clean up temporary condition sets created during rule creation.
    url_matcher_.ClearUnusedConditionSets();
    return;
  }

  // Record the URL patterns in |url_matcher_|.
  url_matcher::URLMatcherConditionSet::Vector new_condition_sets;
  for (const auto& group_predicates_pair : predicates) {
    for (const ContentPredicate* predicate : group_predicates_pair.second) {
      DCHECK_EQ(this, predicate->GetEvaluator());
      const DeclarativeContentPageUrlPredicate* typed_predicate =
          static_cast<const DeclarativeContentPageUrlPredicate*>(predicate);
      new_condition_sets.push_back(
          typed_predicate->url_matcher_condition_set());
      tracked_predicates_[group_predicates_pair.first].push_back(
          typed_predicate);
    }
  }
  url_matcher_.AddConditionSets(new_condition_sets);
  UpdateMatchesForAllTrackers();
}

void DeclarativeContentPageUrlConditionTracker::StopTrackingPredicates(
    const std::vector<const void*>& predicate_groups) {
  // Condition set ids to be removed from |url_matcher_|.
  std::vector<url_matcher::URLMatcherConditionSet::ID>
      condition_set_ids_to_remove;
  for (const void* group : predicate_groups) {
    auto loc = tracked_predicates_.find(group);
    if (loc == tracked_predicates_.end())
      continue;
    for (const DeclarativeContentPageUrlPredicate* predicate : loc->second) {
      condition_set_ids_to_remove.push_back(
          predicate->url_matcher_condition_set()->id());
    }
    tracked_predicates_.erase(group);
  }

  if (!condition_set_ids_to_remove.empty()) {
    url_matcher_.RemoveConditionSets(condition_set_ids_to_remove);
    UpdateMatchesForAllTrackers();
  }
}

void DeclarativeContentPageUrlConditionTracker::TrackForWebContents(
    content::WebContents* contents) {
  per_web_contents_tracker_[contents] =
      make_linked_ptr(new PerWebContentsTracker(
          contents,
          &url_matcher_,
          base::Bind(&Delegate::RequestEvaluation, base::Unretained(delegate_)),
          base::Bind(&DeclarativeContentPageUrlConditionTracker::
                     DeletePerWebContentsTracker,
                     base::Unretained(this))));
  per_web_contents_tracker_[contents]->UpdateMatchesForCurrentUrl(true);
}

void DeclarativeContentPageUrlConditionTracker::OnWebContentsNavigation(
    content::WebContents* contents,
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  DCHECK(ContainsKey(per_web_contents_tracker_, contents));
  per_web_contents_tracker_[contents]->UpdateMatchesForCurrentUrl(true);
}

bool DeclarativeContentPageUrlConditionTracker::EvaluatePredicate(
    const ContentPredicate* predicate,
    content::WebContents* tab) const {
  DCHECK_EQ(this, predicate->GetEvaluator());
  const DeclarativeContentPageUrlPredicate* typed_predicate =
      static_cast<const DeclarativeContentPageUrlPredicate*>(predicate);
  auto loc = per_web_contents_tracker_.find(tab);
  DCHECK(loc != per_web_contents_tracker_.end());
  const std::set<url_matcher::URLMatcherConditionSet::ID>&
      web_contents_id_matches = loc->second->matches();
  return ContainsKey(web_contents_id_matches,
                     typed_predicate->url_matcher_condition_set()->id());
}

bool DeclarativeContentPageUrlConditionTracker::IsEmpty() const {
  return url_matcher_.IsEmpty();
}

void DeclarativeContentPageUrlConditionTracker::DeletePerWebContentsTracker(
    content::WebContents* contents) {
  DCHECK(ContainsKey(per_web_contents_tracker_, contents));
  per_web_contents_tracker_.erase(contents);
}

void DeclarativeContentPageUrlConditionTracker::UpdateMatchesForAllTrackers() {
  for (const auto& web_contents_tracker_pair : per_web_contents_tracker_)
    web_contents_tracker_pair.second->UpdateMatchesForCurrentUrl(false);
}

}  // namespace extensions
