// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/declarative_content/chrome_content_rules_registry.h"

#include <string>

#include "base/bind.h"
#include "base/test/values_test_util.h"
#include "chrome/browser/extensions/api/declarative_content/content_predicate.h"
#include "chrome/browser/extensions/api/declarative_content/content_predicate_evaluator.h"
#include "chrome/browser/extensions/test_extension_environment.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/frame_navigate_params.h"
#include "extensions/common/extension.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace extensions {

namespace {

class TestPredicateEvaluator;

class TestPredicate : public ContentPredicate {
 public:
  explicit TestPredicate(ContentPredicateEvaluator* evaluator)
      : evaluator_(evaluator) {
  }

  ContentPredicateEvaluator* GetEvaluator() const override {
    return evaluator_;
  }

 private:
  ContentPredicateEvaluator* evaluator_;

  DISALLOW_COPY_AND_ASSIGN(TestPredicate);
};

class TestPredicateEvaluator : public ContentPredicateEvaluator {
 public:
  explicit TestPredicateEvaluator(ContentPredicateEvaluator::Delegate* delegate)
      : delegate_(delegate),
        contents_for_next_operation_evaluation_(nullptr),
        next_evaluation_result_(false) {
  }

  std::string GetPredicateApiAttributeName() const override {
    return "test_predicate";
  }

  scoped_ptr<const ContentPredicate> CreatePredicate(
      const Extension* extension,
      const base::Value& value,
      std::string* error) override {
    RequestEvaluationIfSpecified();
    return make_scoped_ptr(new TestPredicate(this));
  }

  void TrackPredicates(
      const std::map<const void*,
          std::vector<const ContentPredicate*>>& predicates) override {
    RequestEvaluationIfSpecified();
  }

  void StopTrackingPredicates(
      const std::vector<const void*>& predicate_groups) override {
    RequestEvaluationIfSpecified();
  }

  void TrackForWebContents(content::WebContents* contents) override {
    RequestEvaluationIfSpecified();
  }

  void OnWebContentsNavigation(
      content::WebContents* contents,
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override {
    RequestEvaluationIfSpecified();
  }

  bool EvaluatePredicate(const ContentPredicate* predicate,
                         content::WebContents* tab) const override {
    bool result = next_evaluation_result_;
    next_evaluation_result_ = false;
    return result;
  }

  void RequestImmediateEvaluation(content::WebContents* contents,
                                  bool evaluation_result) {
    next_evaluation_result_ = evaluation_result;
    delegate_->RequestEvaluation(contents);
  }

  void RequestEvaluationOnNextOperation(content::WebContents* contents,
                                        bool evaluation_result) {
    contents_for_next_operation_evaluation_ = contents;
    next_evaluation_result_ = evaluation_result;
  }

 private:
  void RequestEvaluationIfSpecified() {
    if (contents_for_next_operation_evaluation_)
      delegate_->RequestEvaluation(contents_for_next_operation_evaluation_);
    contents_for_next_operation_evaluation_ = nullptr;
  }

  ContentPredicateEvaluator::Delegate* delegate_;
  content::WebContents* contents_for_next_operation_evaluation_;
  mutable bool next_evaluation_result_;

  DISALLOW_COPY_AND_ASSIGN(TestPredicateEvaluator);
};

// Create the test evaluator and set |evaluator| to its pointer.
ScopedVector<ContentPredicateEvaluator> CreateTestEvaluator(
    TestPredicateEvaluator** evaluator,
    ContentPredicateEvaluator::Delegate* delegate) {
  ScopedVector<ContentPredicateEvaluator> evaluators;
  *evaluator = new TestPredicateEvaluator(delegate);
  evaluators.push_back(*evaluator);
  return evaluators.Pass();
}

}  // namespace

class DeclarativeChromeContentRulesRegistryTest : public testing::Test {
 public:
  DeclarativeChromeContentRulesRegistryTest() {}

 protected:
  TestExtensionEnvironment* env() { return &env_; }

 private:
  TestExtensionEnvironment env_;

  DISALLOW_COPY_AND_ASSIGN(DeclarativeChromeContentRulesRegistryTest);
};

TEST_F(DeclarativeChromeContentRulesRegistryTest, ActiveRulesDoesntGrow) {
  TestPredicateEvaluator* evaluator = nullptr;
  scoped_refptr<ChromeContentRulesRegistry> registry(
      new ChromeContentRulesRegistry(env()->profile(), nullptr,
                                     base::Bind(&CreateTestEvaluator,
                                                &evaluator)));

  EXPECT_EQ(0u, registry->GetActiveRulesCountForTesting());

  scoped_ptr<content::WebContents> tab = env()->MakeTab();
  registry->MonitorWebContentsForRuleEvaluation(tab.get());
  registry->DidNavigateMainFrame(tab.get(), content::LoadCommittedDetails(),
                                 content::FrameNavigateParams());
  EXPECT_EQ(0u, registry->GetActiveRulesCountForTesting());

  // Add a rule.
  linked_ptr<api::events::Rule> rule(new api::events::Rule);
  api::events::Rule::Populate(
      *base::test::ParseJson(
          "{\n"
          "  \"id\": \"rule1\",\n"
          "  \"priority\": 100,\n"
          "  \"conditions\": [\n"
          "    {\n"
          "      \"instanceType\": \"declarativeContent.PageStateMatcher\",\n"
          "      \"test_predicate\": []\n"
          "    }],\n"
          "  \"actions\": [\n"
          "    { \"instanceType\": \"declarativeContent.ShowPageAction\" }\n"
          "  ]\n"
          "}"),
      rule.get());
  std::vector<linked_ptr<api::events::Rule>> rules;
  rules.push_back(rule);

  const Extension* extension = env()->MakeExtension(*base::test::ParseJson(
      "{\"page_action\": {}}"));
  registry->AddRulesImpl(extension->id(), rules);

  registry->DidNavigateMainFrame(tab.get(), content::LoadCommittedDetails(),
                                 content::FrameNavigateParams());
  EXPECT_EQ(0u, registry->GetActiveRulesCountForTesting());

  evaluator->RequestImmediateEvaluation(tab.get(), true);
  EXPECT_EQ(1u, registry->GetActiveRulesCountForTesting());

  // Closing the tab should erase its entry from active_rules_.
  tab.reset();
  EXPECT_EQ(0u, registry->GetActiveRulesCountForTesting());

  tab = env()->MakeTab();
  registry->MonitorWebContentsForRuleEvaluation(tab.get());
  evaluator->RequestImmediateEvaluation(tab.get(), true);
  EXPECT_EQ(1u, registry->GetActiveRulesCountForTesting());

  evaluator->RequestEvaluationOnNextOperation(tab.get(), false);
  registry->DidNavigateMainFrame(tab.get(), content::LoadCommittedDetails(),
                                 content::FrameNavigateParams());
  EXPECT_EQ(0u, registry->GetActiveRulesCountForTesting());
}

}  // namespace extensions
