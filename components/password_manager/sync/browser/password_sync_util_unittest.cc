// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/password_manager/sync/browser/password_sync_util.h"

#include "base/macros.h"
#include "components/autofill/core/common/password_form.h"
#include "components/password_manager/sync/browser/sync_username_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"

using autofill::PasswordForm;

namespace password_manager {
namespace sync_util {

typedef SyncUsernameTestBase PasswordSyncUtilTest;

TEST_F(PasswordSyncUtilTest, GetSyncUsernameIfSyncingPasswords) {
  const struct TestCase {
    enum { SYNCING_PASSWORDS, NOT_SYNCING_PASSWORDS } password_sync;
    std::string fake_sync_username;
    std::string expected_result;
    const sync_driver::SyncService* sync_service;
    const SigninManagerBase* signin_manager;
  } kTestCases[] = {
      {TestCase::NOT_SYNCING_PASSWORDS, "a@example.org", std::string(),
       sync_service(), signin_manager()},

      {TestCase::SYNCING_PASSWORDS, "a@example.org", "a@example.org",
       sync_service(), signin_manager()},

      // If sync_service is not available, we assume passwords are synced, even
      // if they are not.
      {TestCase::NOT_SYNCING_PASSWORDS, "a@example.org", "a@example.org",
       nullptr, signin_manager()},

      {TestCase::SYNCING_PASSWORDS, "a@example.org", std::string(),
       sync_service(), nullptr},

      {TestCase::SYNCING_PASSWORDS, "a@example.org", std::string(), nullptr,
       nullptr},
  };

  for (size_t i = 0; i < arraysize(kTestCases); ++i) {
    SCOPED_TRACE(testing::Message() << "i=" << i);
    SetSyncingPasswords(kTestCases[i].password_sync ==
                        TestCase::SYNCING_PASSWORDS);
    FakeSigninAs(kTestCases[i].fake_sync_username);
    EXPECT_EQ(kTestCases[i].expected_result,
              GetSyncUsernameIfSyncingPasswords(kTestCases[i].sync_service,
                                                kTestCases[i].signin_manager));
  }
}

TEST_F(PasswordSyncUtilTest, IsSyncAccountCredential) {
  const struct {
    PasswordForm form;
    std::string fake_sync_username;
    bool expected_result;
  } kTestCases[] = {
      {SimpleGaiaForm("sync_user@example.org"), "sync_user@example.org", true},
      {SimpleGaiaForm("non_sync_user@example.org"), "sync_user@example.org",
       false},
      {SimpleNonGaiaForm("sync_user@example.org"), "sync_user@example.org",
       false},
  };

  for (size_t i = 0; i < arraysize(kTestCases); ++i) {
    SCOPED_TRACE(testing::Message() << "i=" << i);
    SetSyncingPasswords(true);
    FakeSigninAs(kTestCases[i].fake_sync_username);
    EXPECT_EQ(kTestCases[i].expected_result,
              IsSyncAccountCredential(kTestCases[i].form, sync_service(),
                                      signin_manager()));
  }
}

}  // namespace sync_util
}  // namespace password_manager
