// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/bind.h"
#include "base/location.h"
#include "base/pickle.h"
#include "base/prefs/pref_service.h"
#include "base/single_thread_task_runner.h"
#include "base/stl_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/waitable_event.h"
#include "base/thread_task_runner_handle.h"
#include "chrome/browser/password_manager/native_backend_kwallet_x.h"
#include "chrome/test/base/testing_profile.h"
#include "components/autofill/core/common/password_form.h"
#include "components/password_manager/core/common/password_manager_pref_names.h"
#include "content/public/test/test_browser_thread.h"
#include "dbus/message.h"
#include "dbus/mock_bus.h"
#include "dbus/mock_object_proxy.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using autofill::PasswordForm;
using base::UTF8ToUTF16;
using content::BrowserThread;
using password_manager::PasswordStoreChange;
using password_manager::PasswordStoreChangeList;
using testing::_;
using testing::Invoke;
using testing::Return;

namespace {

// This class implements a very simple version of KWallet in memory.
// We only provide the parts we actually use; the real version has more.
class TestKWallet {
 public:
  typedef std::basic_string<uint8_t> Blob;  // std::string is binary-safe.

  TestKWallet() : reject_local_folders_(false) {}

  void set_reject_local_folders(bool value) { reject_local_folders_ = value; }

  // NOTE: The method names here are the same as the corresponding DBus
  // methods, and therefore have names that don't match our style guide.

  // Check for presence of a given password folder.
  bool hasFolder(const std::string& folder) const {
    return data_.find(folder) != data_.end();
  }

  // Check for presence of a given password in a given password folder.
  bool hasEntry(const std::string& folder, const std::string& key) const {
    Data::const_iterator it = data_.find(folder);
    return it != data_.end() && it->second.find(key) != it->second.end();
  }

  // Get a list of password keys in a given password folder.
  bool entryList(const std::string& folder,
                 std::vector<std::string>* entries) const {
    Data::const_iterator it = data_.find(folder);
    if (it == data_.end()) return false;
    for (Folder::const_iterator fit = it->second.begin();
         fit != it->second.end(); ++fit)
      entries->push_back(fit->first);
    return true;
  }

  // Read the password data for a given password in a given password folder.
  bool readEntry(const std::string& folder, const std::string& key,
                 Blob* value) const {
    Data::const_iterator it = data_.find(folder);
    if (it == data_.end()) return false;
    Folder::const_iterator fit = it->second.find(key);
    if (fit == it->second.end()) return false;
    *value = fit->second;
    return true;
  }

  // Create the given password folder.
  bool createFolder(const std::string& folder) {
    if (reject_local_folders_ && folder.find('(') != std::string::npos)
      return false;
    return data_.insert(make_pair(folder, Folder())).second;
  }

  // Remove the given password from the given password folder.
  bool removeEntry(const std::string& folder, const std::string& key) {
    Data::iterator it = data_.find(folder);
    if (it == data_.end()) return false;
    return it->second.erase(key) > 0;
  }

  // Write the given password data to the given password folder.
  bool writeEntry(const std::string& folder, const std::string& key,
                  const Blob& value) {
    Data::iterator it = data_.find(folder);
    if (it == data_.end()) return false;
    it->second[key] = value;
    return true;
  }

 private:
  typedef std::map<std::string, Blob> Folder;
  typedef std::map<std::string, Folder> Data;

  Data data_;
  // "Local" folders are folders containing local profile IDs in their names. We
  // can reject attempts to create them in order to make it easier to create
  // legacy shared passwords in these tests, for testing the migration code.
  bool reject_local_folders_;

  // No need to disallow copy and assign. This class is safe to copy and assign.
};

// Runs |backend->GetAutofillableLogins(forms)| and expects that the return
// value is false.
void CheckGetAutofillableLoginsFails(
    PasswordStoreX::NativeBackend* backend,
    ScopedVector<autofill::PasswordForm>* forms) {
  EXPECT_FALSE(backend->GetAutofillableLogins(forms));
}

}  // anonymous namespace

// Obscure magic: we need to declare storage for this constant because we use it
// in ways that require its address in this test, but not in the actual code.
const int NativeBackendKWallet::kInvalidKWalletHandle;

// Subclass NativeBackendKWallet to promote some members to public for testing.
class NativeBackendKWalletStub : public NativeBackendKWallet {
 public:
  explicit NativeBackendKWalletStub(LocalProfileId id)
      :  NativeBackendKWallet(id) {
  }
  using NativeBackendKWallet::InitWithBus;
  using NativeBackendKWallet::kInvalidKWalletHandle;
  using NativeBackendKWallet::DeserializeValue;
};

// Provide some test forms to avoid having to set them up in each test.
class NativeBackendKWalletTestBase : public testing::Test {
 protected:
  NativeBackendKWalletTestBase() {
    old_form_google_.origin = GURL("http://www.google.com/");
    old_form_google_.action = GURL("http://www.google.com/login");
    old_form_google_.username_element = UTF8ToUTF16("user");
    old_form_google_.username_value = UTF8ToUTF16("joeschmoe");
    old_form_google_.password_element = UTF8ToUTF16("pass");
    old_form_google_.password_value = UTF8ToUTF16("seekrit");
    old_form_google_.submit_element = UTF8ToUTF16("submit");
    old_form_google_.signon_realm = "Google";
    old_form_google_.date_created = base::Time::Now();

    form_google_ = old_form_google_;
    form_google_.times_used = 3;
    form_google_.type = PasswordForm::TYPE_GENERATED;
    form_google_.form_data.name = UTF8ToUTF16("form_name");
    form_google_.date_synced = base::Time::Now();
    form_google_.date_created = old_form_google_.date_created;
    form_google_.display_name = UTF8ToUTF16("Joe Schmoe");
    form_google_.avatar_url = GURL("http://www.google.com/avatar");
    form_google_.federation_url = GURL("http://www.google.com/federation_url");
    form_google_.skip_zero_click = true;
    form_google_.generation_upload_status = PasswordForm::NEGATIVE_SIGNAL_SENT;

    form_isc_.origin = GURL("http://www.isc.org/");
    form_isc_.action = GURL("http://www.isc.org/auth");
    form_isc_.username_element = UTF8ToUTF16("id");
    form_isc_.username_value = UTF8ToUTF16("janedoe");
    form_isc_.password_element = UTF8ToUTF16("passwd");
    form_isc_.password_value = UTF8ToUTF16("ihazabukkit");
    form_isc_.submit_element = UTF8ToUTF16("login");
    form_isc_.signon_realm = "ISC";
    form_isc_.date_synced = base::Time::Now();
    form_isc_.date_created = base::Time::Now();
  }

  static void CheckPasswordForm(const PasswordForm& expected,
                                const PasswordForm& actual,
                                bool check_date_created);
  static void CheckPasswordChanges(const PasswordStoreChangeList& expected,
                                   const PasswordStoreChangeList& actual);
  static void CheckPasswordChangesWithResult(
      const PasswordStoreChangeList* expected,
      const PasswordStoreChangeList* actual,
      bool result);

  PasswordForm old_form_google_;
  PasswordForm form_google_;
  PasswordForm form_isc_;
};

// static
void NativeBackendKWalletTestBase::CheckPasswordForm(
    const PasswordForm& expected,
    const PasswordForm& actual,
    bool check_date_created) {
  EXPECT_EQ(expected.origin, actual.origin);
  EXPECT_EQ(expected.password_value, actual.password_value);
  EXPECT_EQ(expected.action, actual.action);
  EXPECT_EQ(expected.username_element, actual.username_element);
  EXPECT_EQ(expected.username_value, actual.username_value);
  EXPECT_EQ(expected.password_element, actual.password_element);
  EXPECT_EQ(expected.submit_element, actual.submit_element);
  EXPECT_EQ(expected.signon_realm, actual.signon_realm);
  EXPECT_EQ(expected.ssl_valid, actual.ssl_valid);
  EXPECT_EQ(expected.preferred, actual.preferred);
  if (check_date_created) {
    EXPECT_EQ(expected.date_created, actual.date_created);
  }
  EXPECT_EQ(expected.blacklisted_by_user, actual.blacklisted_by_user);
  EXPECT_EQ(expected.type, actual.type);
  EXPECT_EQ(expected.times_used, actual.times_used);
  EXPECT_EQ(expected.scheme, actual.scheme);
  EXPECT_EQ(expected.date_synced, actual.date_synced);
  EXPECT_EQ(expected.display_name, actual.display_name);
  EXPECT_EQ(expected.avatar_url, actual.avatar_url);
  EXPECT_EQ(expected.federation_url, actual.federation_url);
  EXPECT_EQ(expected.skip_zero_click, actual.skip_zero_click);
  EXPECT_EQ(expected.generation_upload_status, actual.generation_upload_status);
}

// static
void NativeBackendKWalletTestBase::CheckPasswordChanges(
    const PasswordStoreChangeList& expected,
    const PasswordStoreChangeList& actual) {
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(expected[i].type(), actual[i].type());
    CheckPasswordForm(expected[i].form(), actual[i].form(), true);
  }
}

// static
void NativeBackendKWalletTestBase::CheckPasswordChangesWithResult(
    const PasswordStoreChangeList* expected,
    const PasswordStoreChangeList* actual,
    bool result) {
  EXPECT_TRUE(result);
  CheckPasswordChanges(*expected, *actual);
}

class NativeBackendKWalletTest : public NativeBackendKWalletTestBase {
 protected:
  NativeBackendKWalletTest()
      : ui_thread_(BrowserThread::UI, &message_loop_),
        db_thread_(BrowserThread::DB), klauncher_ret_(0),
        klauncher_contacted_(false), kwallet_runnable_(true),
        kwallet_running_(true), kwallet_enabled_(true) {
  }

  void SetUp() override;
  void TearDown() override;

  // Let the DB thread run to completion of all current tasks.
  void RunDBThread() {
    base::WaitableEvent event(false, false);
    BrowserThread::PostTask(BrowserThread::DB, FROM_HERE,
                            base::Bind(ThreadDone, &event));
    event.Wait();
    // Some of the tests may post messages to the UI thread, but we don't need
    // to run those until after the DB thread is finished. So run it here.
    message_loop_.RunUntilIdle();
  }
  static void ThreadDone(base::WaitableEvent* event) {
    event->Signal();
  }

  // Utilities to help verify sets of expectations.
  typedef std::vector<
              std::pair<std::string,
                        std::vector<const PasswordForm*> > > ExpectationArray;
  void CheckPasswordForms(const std::string& folder,
                          const ExpectationArray& sorted_expected);

  enum RemoveBetweenMethod {
    CREATED,
    SYNCED,
  };

  // Tests RemoveLoginsCreatedBetween or RemoveLoginsSyncedBetween.
  void TestRemoveLoginsBetween(RemoveBetweenMethod date_to_test);

  base::MessageLoopForUI message_loop_;
  content::TestBrowserThread ui_thread_;
  content::TestBrowserThread db_thread_;

  scoped_refptr<dbus::MockBus> mock_session_bus_;
  scoped_refptr<dbus::MockObjectProxy> mock_klauncher_proxy_;
  scoped_refptr<dbus::MockObjectProxy> mock_kwallet_proxy_;

  int klauncher_ret_;
  std::string klauncher_error_;
  bool klauncher_contacted_;

  bool kwallet_runnable_;
  bool kwallet_running_;
  bool kwallet_enabled_;

  TestKWallet wallet_;

  // For all method names contained in |failing_methods_|, the mocked KWallet
  // will return a null response.
  std::set<std::string> failing_methods_;

 private:
  dbus::Response* KLauncherMethodCall(
      dbus::MethodCall* method_call, testing::Unused);

  dbus::Response* KWalletMethodCall(
      dbus::MethodCall* method_call, testing::Unused);
};

void NativeBackendKWalletTest::SetUp() {
  ASSERT_TRUE(db_thread_.Start());

  dbus::Bus::Options options;
  options.bus_type = dbus::Bus::SESSION;
  mock_session_bus_ = new dbus::MockBus(options);

  mock_klauncher_proxy_ =
      new dbus::MockObjectProxy(mock_session_bus_.get(),
                                "org.kde.klauncher",
                                dbus::ObjectPath("/KLauncher"));
  EXPECT_CALL(*mock_klauncher_proxy_.get(), MockCallMethodAndBlock(_, _))
      .WillRepeatedly(
           Invoke(this, &NativeBackendKWalletTest::KLauncherMethodCall));

  mock_kwallet_proxy_ =
      new dbus::MockObjectProxy(mock_session_bus_.get(),
                                "org.kde.kwalletd",
                                dbus::ObjectPath("/modules/kwalletd"));
  EXPECT_CALL(*mock_kwallet_proxy_.get(), MockCallMethodAndBlock(_, _))
      .WillRepeatedly(
           Invoke(this, &NativeBackendKWalletTest::KWalletMethodCall));

  EXPECT_CALL(
      *mock_session_bus_.get(),
      GetObjectProxy("org.kde.klauncher", dbus::ObjectPath("/KLauncher")))
      .WillRepeatedly(Return(mock_klauncher_proxy_.get()));
  EXPECT_CALL(
      *mock_session_bus_.get(),
      GetObjectProxy("org.kde.kwalletd", dbus::ObjectPath("/modules/kwalletd")))
      .WillRepeatedly(Return(mock_kwallet_proxy_.get()));

  EXPECT_CALL(*mock_session_bus_.get(), ShutdownAndBlock()).WillOnce(Return())
      .WillRepeatedly(Return());
}

void NativeBackendKWalletTest::TearDown() {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::MessageLoop::QuitClosure());
  base::MessageLoop::current()->Run();
  db_thread_.Stop();
}

void NativeBackendKWalletTest::TestRemoveLoginsBetween(
    RemoveBetweenMethod date_to_test) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  form_google_.date_synced = base::Time();
  form_isc_.date_synced = base::Time();
  form_google_.date_created = base::Time();
  form_isc_.date_created = base::Time();
  base::Time now = base::Time::Now();
  base::Time next_day = now + base::TimeDelta::FromDays(1);
  if (date_to_test == CREATED) {
    form_google_.date_created = now;
    form_isc_.date_created = next_day;
  } else {
    form_google_.date_synced = now;
    form_isc_.date_synced = next_day;
  }

  BrowserThread::PostTask(
      BrowserThread::DB,
      FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend),
                 form_google_));
  BrowserThread::PostTask(
      BrowserThread::DB,
      FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend),
                 form_isc_));

  PasswordStoreChangeList expected_changes;
  expected_changes.push_back(
      PasswordStoreChange(PasswordStoreChange::REMOVE, form_google_));
  PasswordStoreChangeList changes;
  bool (NativeBackendKWallet::*method)(
      base::Time, base::Time, password_manager::PasswordStoreChangeList*) =
      date_to_test == CREATED
          ? &NativeBackendKWalletStub::RemoveLoginsCreatedBetween
          : &NativeBackendKWalletStub::RemoveLoginsSyncedBetween;
  BrowserThread::PostTaskAndReplyWithResult(
      BrowserThread::DB,
      FROM_HERE,
      base::Bind(
          method, base::Unretained(&backend), base::Time(), next_day, &changes),
      base::Bind(&NativeBackendKWalletTest::CheckPasswordChangesWithResult,
                 &expected_changes,
                 &changes));
  RunDBThread();

  std::vector<const PasswordForm*> forms;
  forms.push_back(&form_isc_);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_isc_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);

  // Remove form_isc_.
  expected_changes.clear();
  expected_changes.push_back(
      PasswordStoreChange(PasswordStoreChange::REMOVE, form_isc_));
  BrowserThread::PostTaskAndReplyWithResult(
      BrowserThread::DB,
      FROM_HERE,
      base::Bind(
          method, base::Unretained(&backend), next_day, base::Time(), &changes),
      base::Bind(&NativeBackendKWalletTest::CheckPasswordChangesWithResult,
                 &expected_changes,
                 &changes));
  RunDBThread();

  CheckPasswordForms("Chrome Form Data (42)", ExpectationArray());
}

dbus::Response* NativeBackendKWalletTest::KLauncherMethodCall(
    dbus::MethodCall* method_call, testing::Unused) {
  EXPECT_EQ("org.kde.KLauncher", method_call->GetInterface());
  EXPECT_EQ("start_service_by_desktop_name", method_call->GetMember());

  klauncher_contacted_ = true;

  dbus::MessageReader reader(method_call);
  std::string service_name;
  std::vector<std::string> urls;
  std::vector<std::string> envs;
  std::string startup_id;
  bool blind = false;

  EXPECT_TRUE(reader.PopString(&service_name));
  EXPECT_TRUE(reader.PopArrayOfStrings(&urls));
  EXPECT_TRUE(reader.PopArrayOfStrings(&envs));
  EXPECT_TRUE(reader.PopString(&startup_id));
  EXPECT_TRUE(reader.PopBool(&blind));

  EXPECT_EQ("kwalletd", service_name);
  EXPECT_TRUE(urls.empty());
  EXPECT_TRUE(envs.empty());
  EXPECT_TRUE(startup_id.empty());
  EXPECT_FALSE(blind);

  if (kwallet_runnable_)
    kwallet_running_ = true;

  scoped_ptr<dbus::Response> response(dbus::Response::CreateEmpty());
  dbus::MessageWriter writer(response.get());
  writer.AppendInt32(klauncher_ret_);
  writer.AppendString(std::string());  // dbus_name
  writer.AppendString(klauncher_error_);
  writer.AppendInt32(1234);  // pid
  return response.release();
}

dbus::Response* NativeBackendKWalletTest::KWalletMethodCall(
    dbus::MethodCall* method_call, testing::Unused) {
  if (!kwallet_running_)
    return nullptr;
  EXPECT_EQ("org.kde.KWallet", method_call->GetInterface());

  if (ContainsKey(failing_methods_, method_call->GetMember()))
    return nullptr;
  scoped_ptr<dbus::Response> response;
  if (method_call->GetMember() == "isEnabled") {
    response = dbus::Response::CreateEmpty();
    dbus::MessageWriter writer(response.get());
    writer.AppendBool(kwallet_enabled_);
  } else if (method_call->GetMember() == "networkWallet") {
    response = dbus::Response::CreateEmpty();
    dbus::MessageWriter writer(response.get());
    writer.AppendString("test_wallet");  // Should match |open| below.
  } else if (method_call->GetMember() == "open") {
    dbus::MessageReader reader(method_call);
    std::string wallet_name;
    int64_t wallet_id;
    std::string app_name;
    EXPECT_TRUE(reader.PopString(&wallet_name));
    EXPECT_TRUE(reader.PopInt64(&wallet_id));
    EXPECT_TRUE(reader.PopString(&app_name));
    EXPECT_EQ("test_wallet", wallet_name);  // Should match |networkWallet|.
    response = dbus::Response::CreateEmpty();
    dbus::MessageWriter writer(response.get());
    writer.AppendInt32(1);  // Can be anything but kInvalidKWalletHandle.
  } else if (method_call->GetMember() == "hasFolder" ||
             method_call->GetMember() == "createFolder") {
    dbus::MessageReader reader(method_call);
    int handle = NativeBackendKWalletStub::kInvalidKWalletHandle;
    std::string folder_name;
    std::string app_name;
    EXPECT_TRUE(reader.PopInt32(&handle));
    EXPECT_TRUE(reader.PopString(&folder_name));
    EXPECT_TRUE(reader.PopString(&app_name));
    EXPECT_NE(NativeBackendKWalletStub::kInvalidKWalletHandle, handle);
    response = dbus::Response::CreateEmpty();
    dbus::MessageWriter writer(response.get());
    if (method_call->GetMember() == "hasFolder")
      writer.AppendBool(wallet_.hasFolder(folder_name));
    else
      writer.AppendBool(wallet_.createFolder(folder_name));
  } else if (method_call->GetMember() == "hasEntry" ||
             method_call->GetMember() == "removeEntry") {
    dbus::MessageReader reader(method_call);
    int handle = NativeBackendKWalletStub::kInvalidKWalletHandle;
    std::string folder_name;
    std::string key;
    std::string app_name;
    EXPECT_TRUE(reader.PopInt32(&handle));
    EXPECT_TRUE(reader.PopString(&folder_name));
    EXPECT_TRUE(reader.PopString(&key));
    EXPECT_TRUE(reader.PopString(&app_name));
    EXPECT_NE(NativeBackendKWalletStub::kInvalidKWalletHandle, handle);
    response = dbus::Response::CreateEmpty();
    dbus::MessageWriter writer(response.get());
    if (method_call->GetMember() == "hasEntry")
      writer.AppendBool(wallet_.hasEntry(folder_name, key));
    else
      writer.AppendInt32(wallet_.removeEntry(folder_name, key) ? 0 : 1);
  } else if (method_call->GetMember() == "entryList") {
    dbus::MessageReader reader(method_call);
    int handle = NativeBackendKWalletStub::kInvalidKWalletHandle;
    std::string folder_name;
    std::string app_name;
    EXPECT_TRUE(reader.PopInt32(&handle));
    EXPECT_TRUE(reader.PopString(&folder_name));
    EXPECT_TRUE(reader.PopString(&app_name));
    EXPECT_NE(NativeBackendKWalletStub::kInvalidKWalletHandle, handle);
    std::vector<std::string> entries;
    if (wallet_.entryList(folder_name, &entries)) {
      response = dbus::Response::CreateEmpty();
      dbus::MessageWriter writer(response.get());
      writer.AppendArrayOfStrings(entries);
    }
  } else if (method_call->GetMember() == "readEntry") {
    dbus::MessageReader reader(method_call);
    int handle = NativeBackendKWalletStub::kInvalidKWalletHandle;
    std::string folder_name;
    std::string key;
    std::string app_name;
    EXPECT_TRUE(reader.PopInt32(&handle));
    EXPECT_TRUE(reader.PopString(&folder_name));
    EXPECT_TRUE(reader.PopString(&key));
    EXPECT_TRUE(reader.PopString(&app_name));
    EXPECT_NE(NativeBackendKWalletStub::kInvalidKWalletHandle, handle);
    TestKWallet::Blob value;
    if (wallet_.readEntry(folder_name, key, &value)) {
      response = dbus::Response::CreateEmpty();
      dbus::MessageWriter writer(response.get());
      writer.AppendArrayOfBytes(value.data(), value.size());
    }
  } else if (method_call->GetMember() == "writeEntry") {
    dbus::MessageReader reader(method_call);
    int handle = NativeBackendKWalletStub::kInvalidKWalletHandle;
    std::string folder_name;
    std::string key;
    const uint8_t* bytes = nullptr;
    size_t length = 0;
    std::string app_name;
    EXPECT_TRUE(reader.PopInt32(&handle));
    EXPECT_TRUE(reader.PopString(&folder_name));
    EXPECT_TRUE(reader.PopString(&key));
    EXPECT_TRUE(reader.PopArrayOfBytes(&bytes, &length));
    EXPECT_TRUE(reader.PopString(&app_name));
    EXPECT_NE(NativeBackendKWalletStub::kInvalidKWalletHandle, handle);
    response = dbus::Response::CreateEmpty();
    dbus::MessageWriter writer(response.get());
    writer.AppendInt32(
        wallet_.writeEntry(folder_name, key,
                           TestKWallet::Blob(bytes, length)) ? 0 : 1);
  }

  EXPECT_TRUE(response);
  return response.release();
}

void NativeBackendKWalletTest::CheckPasswordForms(
    const std::string& folder, const ExpectationArray& sorted_expected) {
  EXPECT_TRUE(wallet_.hasFolder(folder));
  std::vector<std::string> entries;
  EXPECT_TRUE(wallet_.entryList(folder, &entries));
  EXPECT_EQ(sorted_expected.size(), entries.size());
  std::sort(entries.begin(), entries.end());
  for (size_t i = 0; i < entries.size() && i < sorted_expected.size(); ++i) {
    EXPECT_EQ(sorted_expected[i].first, entries[i]);
    TestKWallet::Blob value;
    EXPECT_TRUE(wallet_.readEntry(folder, entries[i], &value));
    base::Pickle pickle(reinterpret_cast<const char*>(value.data()),
                        value.size());
    ScopedVector<autofill::PasswordForm> forms =
        NativeBackendKWalletStub::DeserializeValue(entries[i], pickle);
    const std::vector<const PasswordForm*>& expect = sorted_expected[i].second;
    EXPECT_EQ(expect.size(), forms.size());
    for (size_t j = 0; j < forms.size() && j < expect.size(); ++j)
      CheckPasswordForm(*expect[j], *forms[j], true);
  }
}

TEST_F(NativeBackendKWalletTest, NotEnabled) {
  NativeBackendKWalletStub kwallet(42);
  kwallet_enabled_ = false;
  EXPECT_FALSE(kwallet.InitWithBus(mock_session_bus_));
  EXPECT_FALSE(klauncher_contacted_);
}

TEST_F(NativeBackendKWalletTest, NotRunnable) {
  NativeBackendKWalletStub kwallet(42);
  kwallet_runnable_ = false;
  kwallet_running_ = false;
  EXPECT_FALSE(kwallet.InitWithBus(mock_session_bus_));
  EXPECT_TRUE(klauncher_contacted_);
}

TEST_F(NativeBackendKWalletTest, NotRunningOrEnabled) {
  NativeBackendKWalletStub kwallet(42);
  kwallet_running_ = false;
  kwallet_enabled_ = false;
  EXPECT_FALSE(kwallet.InitWithBus(mock_session_bus_));
  EXPECT_TRUE(klauncher_contacted_);
}

TEST_F(NativeBackendKWalletTest, NotRunning) {
  NativeBackendKWalletStub kwallet(42);
  kwallet_running_ = false;
  EXPECT_TRUE(kwallet.InitWithBus(mock_session_bus_));
  EXPECT_TRUE(klauncher_contacted_);
}

TEST_F(NativeBackendKWalletTest, BasicStartup) {
  NativeBackendKWalletStub kwallet(42);
  EXPECT_TRUE(kwallet.InitWithBus(mock_session_bus_));
  EXPECT_FALSE(klauncher_contacted_);
}

TEST_F(NativeBackendKWalletTest, BasicAddLogin) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), form_google_));

  RunDBThread();

  EXPECT_FALSE(wallet_.hasFolder("Chrome Form Data"));

  std::vector<const PasswordForm*> forms;
  forms.push_back(&form_google_);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_google_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, BasicUpdateLogin) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), form_google_));

  RunDBThread();

  PasswordForm new_form_google(form_google_);
  new_form_google.times_used = 10;
  new_form_google.action = GURL("http://www.google.com/different/login");

  // Update login
  PasswordStoreChangeList changes;
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::UpdateLogin),
                 base::Unretained(&backend),
                 new_form_google,
                 base::Unretained(&changes)));
  RunDBThread();

  ASSERT_EQ(1u, changes.size());
  EXPECT_EQ(PasswordStoreChange::UPDATE, changes.front().type());
  EXPECT_EQ(new_form_google, changes.front().form());

  std::vector<const PasswordForm*> forms;
  forms.push_back(&new_form_google);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_google_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, BasicListLogins) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), form_google_));

  ScopedVector<autofill::PasswordForm> form_list;
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(
          base::IgnoreResult(&NativeBackendKWalletStub::GetAutofillableLogins),
          base::Unretained(&backend), &form_list));

  RunDBThread();

  // Quick check that we got something back.
  EXPECT_EQ(1u, form_list.size());

  EXPECT_FALSE(wallet_.hasFolder("Chrome Form Data"));

  std::vector<const PasswordForm*> forms;
  forms.push_back(&form_google_);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_google_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, BasicRemoveLogin) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), form_google_));

  RunDBThread();

  EXPECT_FALSE(wallet_.hasFolder("Chrome Form Data"));

  std::vector<const PasswordForm*> forms;
  forms.push_back(&form_google_);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_google_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);

  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::RemoveLogin),
                 base::Unretained(&backend), form_google_));

  RunDBThread();

  expected.clear();
  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, UpdateNonexistentLogin) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  // First add an unrelated login.
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), form_google_));

  RunDBThread();

  std::vector<const PasswordForm*> forms;
  forms.push_back(&form_google_);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_google_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);

  // Attempt to update a login that doesn't exist.
  PasswordStoreChangeList changes;
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::UpdateLogin),
                 base::Unretained(&backend),
                 form_isc_,
                 base::Unretained(&changes)));

  RunDBThread();

  EXPECT_EQ(PasswordStoreChangeList(), changes);
  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, RemoveNonexistentLogin) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  // First add an unrelated login.
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), form_google_));

  RunDBThread();

  EXPECT_FALSE(wallet_.hasFolder("Chrome Form Data"));

  std::vector<const PasswordForm*> forms;
  forms.push_back(&form_google_);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_google_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);

  // Attempt to remove a login that doesn't exist.
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::RemoveLogin),
                 base::Unretained(&backend), form_isc_));

  // Make sure we can still get the first form back.
  ScopedVector<autofill::PasswordForm> form_list;
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(
          base::IgnoreResult(&NativeBackendKWalletStub::GetAutofillableLogins),
          base::Unretained(&backend), &form_list));

  RunDBThread();

  // Quick check that we got something back.
  EXPECT_EQ(1u, form_list.size());

  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, AddDuplicateLogin) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  PasswordStoreChangeList changes;
  changes.push_back(PasswordStoreChange(PasswordStoreChange::ADD,
                                        form_google_));
  BrowserThread::PostTaskAndReplyWithResult(
      BrowserThread::DB, FROM_HERE,
      base::Bind(&NativeBackendKWalletStub::AddLogin,
                 base::Unretained(&backend), form_google_),
      base::Bind(&NativeBackendKWalletTest::CheckPasswordChanges,
                 changes));

  changes.clear();
  changes.push_back(PasswordStoreChange(PasswordStoreChange::REMOVE,
                                        form_google_));
  form_google_.times_used++;
  changes.push_back(PasswordStoreChange(PasswordStoreChange::ADD,
                                        form_google_));

  BrowserThread::PostTaskAndReplyWithResult(
      BrowserThread::DB, FROM_HERE,
      base::Bind(&NativeBackendKWalletStub::AddLogin,
                 base::Unretained(&backend), form_google_),
      base::Bind(&NativeBackendKWalletTest::CheckPasswordChanges,
                 changes));

  RunDBThread();

  EXPECT_FALSE(wallet_.hasFolder("Chrome Form Data"));

  std::vector<const PasswordForm*> forms;
  forms.push_back(&form_google_);
  ExpectationArray expected;
  expected.push_back(make_pair(std::string(form_google_.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, AndroidCredentials) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));

  PasswordForm observed_android_form;
  observed_android_form.scheme = PasswordForm::SCHEME_HTML;
  observed_android_form.signon_realm =
      "android://7x7IDboo8u9YKraUsbmVkuf1-@net.rateflix.app/";
  PasswordForm saved_android_form = observed_android_form;
  saved_android_form.username_value = base::UTF8ToUTF16("randomusername");
  saved_android_form.password_value = base::UTF8ToUTF16("password");

  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), saved_android_form));

  ScopedVector<autofill::PasswordForm> form_list;
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(
          base::IgnoreResult(&NativeBackendKWalletStub::GetLogins),
          base::Unretained(&backend), observed_android_form, &form_list));

  RunDBThread();

  EXPECT_EQ(1u, form_list.size());

  std::vector<const PasswordForm*> forms;
  forms.push_back(&saved_android_form);
  ExpectationArray expected;
  expected.push_back(
      make_pair(std::string(saved_android_form.signon_realm), forms));
  CheckPasswordForms("Chrome Form Data (42)", expected);
}

TEST_F(NativeBackendKWalletTest, RemoveLoginsCreatedBetween) {
  TestRemoveLoginsBetween(CREATED);
}

TEST_F(NativeBackendKWalletTest, RemoveLoginsSyncedBetween) {
  TestRemoveLoginsBetween(SYNCED);
}

// TODO(mdm): add more basic tests here at some point.
// (For example tests for storing >1 password per realm pickle.)

class NativeBackendKWalletPickleTest : public NativeBackendKWalletTestBase {
 protected:
  // Based on |form|, fills |pickle| with data conforming to
  // |effective_version|, but marking the pickle version as |stored_version|. In
  // most cases the two versions should be the same.
  void CreateVersion1PlusPickle(const PasswordForm& form,
                                base::Pickle* pickle,
                                int stored_version,
                                int effective_version);
  void CreateVersion0Pickle(bool size_32,
                            const PasswordForm& form,
                            base::Pickle* pickle);
  // As explained in http://crbug.com/494229#c11, version 6 added a new optional
  // field to version 5. This field became required in version 7. Depending on
  // |with_optional_field|, this method checks deserialization with or without
  // the optional field.
  void CheckVersion6Pickle(bool with_optional_field);
  void CheckVersion5Pickle();
  void CheckVersion3Pickle();
  void CheckVersion2Pickle();
  void CheckVersion1Pickle();
  void CheckVersion0Pickle(bool size_32, PasswordForm::Scheme scheme);

 private:
  // Creates a Pickle from |form|. If |size_32| is true, stores the number of
  // forms in the pickle as a 32bit uint, otherwise as 64 bit size_t. The latter
  // should be the case for versions > 0. If |date_created_internal| is true,
  // stores |date_created| as base::Time's internal value, otherwise as time_t.
  void CreatePickle(bool size_32,
                    bool date_created_internal,
                    const PasswordForm& form,
                    base::Pickle* pickle);
};

void NativeBackendKWalletPickleTest::CreateVersion1PlusPickle(
    const PasswordForm& form,
    base::Pickle* pickle,
    int stored_version,
    int effective_version) {
  pickle->WriteInt(stored_version);
  CreatePickle(false, true, form, pickle);
  if (effective_version < 2)
    return;
  pickle->WriteInt(form.type);
  pickle->WriteInt(form.times_used);
  autofill::SerializeFormData(form.form_data, pickle);
  if (effective_version < 3)
    return;
  pickle->WriteInt64(form.date_synced.ToInternalValue());
  if (effective_version < 4)
    return;
  pickle->WriteString16(form.display_name);
  pickle->WriteString(form.avatar_url.spec());
  pickle->WriteString(form.federation_url.spec());
  pickle->WriteBool(form.skip_zero_click);
  if (effective_version < 7)
    return;
  pickle->WriteInt(form.generation_upload_status);
}

void NativeBackendKWalletPickleTest::CreateVersion0Pickle(
    bool size_32,
    const PasswordForm& form,
    base::Pickle* pickle) {
  pickle->WriteInt(0);
  CreatePickle(size_32, false, form, pickle);
}

void NativeBackendKWalletPickleTest::CreatePickle(bool size_32,
                                                  bool date_created_internal,
                                                  const PasswordForm& form,
                                                  base::Pickle* pickle) {
  if (size_32)
    pickle->WriteUInt32(1);  // Size of form list. 32 bits.
  else
    pickle->WriteSizeT(1);  // Size of form list. 64 bits.
  pickle->WriteInt(form.scheme);
  pickle->WriteString(form.origin.spec());
  pickle->WriteString(form.action.spec());
  pickle->WriteString16(form.username_element);
  pickle->WriteString16(form.username_value);
  pickle->WriteString16(form.password_element);
  pickle->WriteString16(form.password_value);
  pickle->WriteString16(form.submit_element);
  pickle->WriteBool(form.ssl_valid);
  pickle->WriteBool(form.preferred);
  pickle->WriteBool(form.blacklisted_by_user);
  if (date_created_internal)
    pickle->WriteInt64(form.date_created.ToInternalValue());
  else
    pickle->WriteInt64(form.date_created.ToTimeT());
}

void NativeBackendKWalletPickleTest::CheckVersion6Pickle(
    bool with_optional_field) {
  base::Pickle pickle;
  PasswordForm form = form_google_;
  if (!with_optional_field) {
    PasswordForm default_values;
    form.generation_upload_status = default_values.generation_upload_status;
  }
  CreateVersion1PlusPickle(form, &pickle, 6, with_optional_field ? 7 : 5);

  ScopedVector<PasswordForm> form_list =
      NativeBackendKWalletStub::DeserializeValue(form.signon_realm, pickle);

  EXPECT_EQ(1u, form_list.size());
  if (form_list.size() > 0)
    CheckPasswordForm(form, *form_list[0], true);

}

void NativeBackendKWalletPickleTest::CheckVersion5Pickle() {
  base::Pickle pickle;
  PasswordForm default_values;
  PasswordForm form = form_google_;
  // Remove the field which was not present in version #5.
  form.generation_upload_status = default_values.generation_upload_status;
  CreateVersion1PlusPickle(form, &pickle, 5, 5);

  ScopedVector<PasswordForm> form_list =
      NativeBackendKWalletStub::DeserializeValue(form.signon_realm, pickle);

  EXPECT_EQ(1u, form_list.size());
  if (form_list.size() > 0)
    CheckPasswordForm(form, *form_list[0], true);
}

void NativeBackendKWalletPickleTest::CheckVersion3Pickle() {
  base::Pickle pickle;
  PasswordForm default_values;
  PasswordForm form = form_google_;
  // Remove the fields which were not present in version #3.
  form.display_name = default_values.display_name;
  form.avatar_url = default_values.avatar_url;
  form.federation_url = default_values.federation_url;
  form.skip_zero_click = default_values.skip_zero_click;
  form.generation_upload_status = default_values.generation_upload_status;
  CreateVersion1PlusPickle(form, &pickle, 3, 3);

  ScopedVector<PasswordForm> form_list =
      NativeBackendKWalletStub::DeserializeValue(form.signon_realm, pickle);

  EXPECT_EQ(1u, form_list.size());
  if (form_list.size() > 0)
    CheckPasswordForm(form, *form_list[0], false);
}

void NativeBackendKWalletPickleTest::CheckVersion2Pickle() {
  base::Pickle pickle;
  PasswordForm form = old_form_google_;
  form.times_used = form_google_.times_used;
  form.type = form_google_.type;
  form.form_data = form_google_.form_data;
  CreateVersion1PlusPickle(form, &pickle, 2, 2);

  ScopedVector<PasswordForm> form_list =
      NativeBackendKWalletStub::DeserializeValue(form.signon_realm, pickle);

  EXPECT_EQ(1u, form_list.size());
  if (form_list.size() > 0)
    CheckPasswordForm(form, *form_list[0], false);
}

// Make sure that we can still read version 1 pickles.
void NativeBackendKWalletPickleTest::CheckVersion1Pickle() {
  base::Pickle pickle;
  PasswordForm form = form_google_;
  CreateVersion1PlusPickle(form, &pickle, 1, 1);

  ScopedVector<autofill::PasswordForm> form_list =
      NativeBackendKWalletStub::DeserializeValue(form.signon_realm, pickle);

  // This will match |old_form_google_| because not all the fields present in
  // |form_google_| will be deserialized.
  EXPECT_EQ(1u, form_list.size());
  if (form_list.size() > 0)
    CheckPasswordForm(old_form_google_, *form_list[0], false);
}

void NativeBackendKWalletPickleTest::CheckVersion0Pickle(
    bool size_32, PasswordForm::Scheme scheme) {
  base::Pickle pickle;
  PasswordForm form = old_form_google_;
  form.scheme = scheme;
  CreateVersion0Pickle(size_32, form, &pickle);
  ScopedVector<autofill::PasswordForm> form_list =
      NativeBackendKWalletStub::DeserializeValue(form.signon_realm, pickle);
  EXPECT_EQ(1u, form_list.size());
  if (form_list.size() > 0)
    CheckPasswordForm(form, *form_list[0], false);
}

// Check that if KWallet fails to respond, the backend propagates the error.
TEST_F(NativeBackendKWalletTest, GetAllLoginsErrorHandling) {
  NativeBackendKWalletStub backend(42);
  EXPECT_TRUE(backend.InitWithBus(mock_session_bus_));
  // Make KWallet fail on calling readEntry.
  failing_methods_.insert("readEntry");

  // Store some non-blacklisted logins to be potentially returned.
  BrowserThread::PostTask(
      BrowserThread::DB, FROM_HERE,
      base::Bind(base::IgnoreResult(&NativeBackendKWalletStub::AddLogin),
                 base::Unretained(&backend), form_google_));

  // Verify that nothing is in fact returned, because KWallet fails to respond.
  ScopedVector<autofill::PasswordForm> form_list;
  BrowserThread::PostTask(BrowserThread::DB, FROM_HERE,
                          base::Bind(&CheckGetAutofillableLoginsFails,
                                     base::Unretained(&backend), &form_list));
  RunDBThread();
  EXPECT_EQ(0u, form_list.size());
}

// We try both SCHEME_HTML and SCHEME_BASIC since the scheme is stored right
// after the size in the pickle, so it's what gets read as part of the count
// when reading 32-bit pickles on 64-bit systems. SCHEME_HTML is 0 (so we'll
// detect errors later) while SCHEME_BASIC is 1 (so we'll detect it then). We
// try both 32-bit and 64-bit pickles since only one will be the "other" size
// for whatever architecture we're running on, but we want to make sure we can
// read all combinations in any event.

TEST_F(NativeBackendKWalletPickleTest, ReadsOld32BitHTMLPickles) {
  CheckVersion0Pickle(true, PasswordForm::SCHEME_HTML);
}

TEST_F(NativeBackendKWalletPickleTest, ReadsOld32BitHTTPPickles) {
  CheckVersion0Pickle(true, PasswordForm::SCHEME_BASIC);
}

TEST_F(NativeBackendKWalletPickleTest, ReadsOld64BitHTMLPickles) {
  CheckVersion0Pickle(false, PasswordForm::SCHEME_HTML);
}

TEST_F(NativeBackendKWalletPickleTest, ReadsOld64BitHTTPPickles) {
  CheckVersion0Pickle(false, PasswordForm::SCHEME_BASIC);
}

TEST_F(NativeBackendKWalletPickleTest, CheckVersion1Pickle) {
  CheckVersion1Pickle();
}

TEST_F(NativeBackendKWalletPickleTest, CheckVersion2Pickle) {
  CheckVersion2Pickle();
}

TEST_F(NativeBackendKWalletPickleTest, CheckVersion3Pickle) {
  CheckVersion3Pickle();
}

TEST_F(NativeBackendKWalletPickleTest, CheckVersion5Pickle) {
  CheckVersion5Pickle();
}

TEST_F(NativeBackendKWalletPickleTest, CheckVersion6Pickle) {
  CheckVersion6Pickle(false);
  CheckVersion6Pickle(true);
}
