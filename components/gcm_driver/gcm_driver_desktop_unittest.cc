// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/gcm_driver/gcm_driver_desktop.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/files/scoped_temp_dir.h"
#include "base/location.h"
#include "base/metrics/field_trial.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/testing_pref_service.h"
#include "base/run_loop.h"
#include "base/strings/string_util.h"
#include "base/test/test_simple_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "base/threading/thread.h"
#include "components/gcm_driver/fake_gcm_app_handler.h"
#include "components/gcm_driver/fake_gcm_client.h"
#include "components/gcm_driver/fake_gcm_client_factory.h"
#include "components/gcm_driver/gcm_app_handler.h"
#include "components/gcm_driver/gcm_channel_status_request.h"
#include "components/gcm_driver/gcm_channel_status_syncer.h"
#include "components/gcm_driver/gcm_client_factory.h"
#include "components/gcm_driver/gcm_connection_observer.h"
#include "net/url_request/test_url_fetcher_factory.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_test_util.h"
#include "sync/protocol/experiment_status.pb.h"
#include "sync/protocol/experiments_specifics.pb.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace gcm {

namespace {

const char kTestAppID1[] = "TestApp1";
const char kTestAppID2[] = "TestApp2";
const char kUserID1[] = "user1";
const char kScope[] = "GCM";
const char kInstanceID1[] = "IID1";
const char kInstanceID2[] = "IID2";

class FakeGCMConnectionObserver : public GCMConnectionObserver {
 public:
  FakeGCMConnectionObserver();
  ~FakeGCMConnectionObserver() override;

  // gcm::GCMConnectionObserver implementation:
  void OnConnected(const net::IPEndPoint& ip_endpoint) override;
  void OnDisconnected() override;

  bool connected() const { return connected_; }

 private:
  bool connected_;
};

FakeGCMConnectionObserver::FakeGCMConnectionObserver() : connected_(false) {
}

FakeGCMConnectionObserver::~FakeGCMConnectionObserver() {
}

void FakeGCMConnectionObserver::OnConnected(
    const net::IPEndPoint& ip_endpoint) {
  connected_ = true;
}

void FakeGCMConnectionObserver::OnDisconnected() {
  connected_ = false;
}

void PumpCurrentLoop() {
  base::MessageLoop::ScopedNestableTaskAllower
      nestable_task_allower(base::MessageLoop::current());
  base::RunLoop().RunUntilIdle();
}

void PumpUILoop() {
  PumpCurrentLoop();
}

std::vector<std::string> ToSenderList(const std::string& sender_ids) {
  return base::SplitString(
      sender_ids, ",", base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
}

}  // namespace

class GCMDriverTest : public testing::Test {
 public:
  enum WaitToFinish {
    DO_NOT_WAIT,
    WAIT
  };

  GCMDriverTest();
  ~GCMDriverTest() override;

  // testing::Test:
  void SetUp() override;
  void TearDown() override;

  GCMDriverDesktop* driver() { return driver_.get(); }
  FakeGCMAppHandler* gcm_app_handler() { return gcm_app_handler_.get(); }
  FakeGCMConnectionObserver* gcm_connection_observer() {
    return gcm_connection_observer_.get();
  }
  const std::string& registration_id() const { return registration_id_; }
  GCMClient::Result registration_result() const { return registration_result_; }
  const std::string& send_message_id() const { return send_message_id_; }
  GCMClient::Result send_result() const { return send_result_; }
  GCMClient::Result unregistration_result() const {
    return unregistration_result_;
  }

  void PumpIOLoop();

  void ClearResults();

  bool HasAppHandlers() const;
  FakeGCMClient* GetGCMClient();

  void CreateDriver();
  void ShutdownDriver();
  void AddAppHandlers();
  void RemoveAppHandlers();

  void Register(const std::string& app_id,
                const std::vector<std::string>& sender_ids,
                WaitToFinish wait_to_finish);
  void Send(const std::string& app_id,
            const std::string& receiver_id,
            const GCMClient::OutgoingMessage& message,
            WaitToFinish wait_to_finish);
  void Unregister(const std::string& app_id, WaitToFinish wait_to_finish);

  void WaitForAsyncOperation();

  void RegisterCompleted(const std::string& registration_id,
                         GCMClient::Result result);
  void SendCompleted(const std::string& message_id, GCMClient::Result result);
  void UnregisterCompleted(GCMClient::Result result);

  const base::Closure& async_operation_completed_callback() const {
    return async_operation_completed_callback_;
  }
  void set_async_operation_completed_callback(const base::Closure& callback) {
    async_operation_completed_callback_ = callback;
  }

 private:
  base::ScopedTempDir temp_dir_;
  TestingPrefServiceSimple prefs_;
  scoped_refptr<base::TestSimpleTaskRunner> task_runner_;
  base::MessageLoopForUI message_loop_;
  base::Thread io_thread_;
  base::FieldTrialList field_trial_list_;
  scoped_ptr<GCMDriverDesktop> driver_;
  scoped_ptr<FakeGCMAppHandler> gcm_app_handler_;
  scoped_ptr<FakeGCMConnectionObserver> gcm_connection_observer_;

  base::Closure async_operation_completed_callback_;

  std::string registration_id_;
  GCMClient::Result registration_result_;
  std::string send_message_id_;
  GCMClient::Result send_result_;
  GCMClient::Result unregistration_result_;

  DISALLOW_COPY_AND_ASSIGN(GCMDriverTest);
};

GCMDriverTest::GCMDriverTest()
    : task_runner_(new base::TestSimpleTaskRunner()),
      io_thread_("IOThread"),
      field_trial_list_(NULL),
      registration_result_(GCMClient::UNKNOWN_ERROR),
      send_result_(GCMClient::UNKNOWN_ERROR),
      unregistration_result_(GCMClient::UNKNOWN_ERROR) {
}

GCMDriverTest::~GCMDriverTest() {
}

void GCMDriverTest::SetUp() {
  GCMChannelStatusSyncer::RegisterPrefs(prefs_.registry());
  io_thread_.Start();
  ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
}

void GCMDriverTest::TearDown() {
  if (!driver_)
    return;

  ShutdownDriver();
  driver_.reset();
  PumpIOLoop();

  io_thread_.Stop();
}

void GCMDriverTest::PumpIOLoop() {
  base::RunLoop run_loop;
  io_thread_.task_runner()->PostTaskAndReply(
      FROM_HERE, base::Bind(&PumpCurrentLoop), run_loop.QuitClosure());
  run_loop.Run();
}

void GCMDriverTest::ClearResults() {
  registration_id_.clear();
  registration_result_ = GCMClient::UNKNOWN_ERROR;

  send_message_id_.clear();
  send_result_ = GCMClient::UNKNOWN_ERROR;

  unregistration_result_ = GCMClient::UNKNOWN_ERROR;
}

bool GCMDriverTest::HasAppHandlers() const {
  return !driver_->app_handlers().empty();
}

FakeGCMClient* GCMDriverTest::GetGCMClient() {
  return static_cast<FakeGCMClient*>(driver_->GetGCMClientForTesting());
}

void GCMDriverTest::CreateDriver() {
  scoped_refptr<net::URLRequestContextGetter> request_context =
      new net::TestURLRequestContextGetter(io_thread_.task_runner());
  // TODO(johnme): Need equivalent test coverage of GCMDriverAndroid.
  driver_.reset(new GCMDriverDesktop(
      scoped_ptr<GCMClientFactory>(
          new FakeGCMClientFactory(base::ThreadTaskRunnerHandle::Get(),
                                   io_thread_.task_runner())).Pass(),
      GCMClient::ChromeBuildInfo(), "http://channel.status.request.url",
      "user-agent-string", &prefs_, temp_dir_.path(), request_context,
      base::ThreadTaskRunnerHandle::Get(), io_thread_.task_runner(),
      task_runner_));

  gcm_app_handler_.reset(new FakeGCMAppHandler);
  gcm_connection_observer_.reset(new FakeGCMConnectionObserver);

  driver_->AddConnectionObserver(gcm_connection_observer_.get());
}

void GCMDriverTest::ShutdownDriver() {
  if (gcm_connection_observer())
    driver()->RemoveConnectionObserver(gcm_connection_observer());
  driver()->Shutdown();
}

void GCMDriverTest::AddAppHandlers() {
  driver_->AddAppHandler(kTestAppID1, gcm_app_handler_.get());
  driver_->AddAppHandler(kTestAppID2, gcm_app_handler_.get());
}

void GCMDriverTest::RemoveAppHandlers() {
  driver_->RemoveAppHandler(kTestAppID1);
  driver_->RemoveAppHandler(kTestAppID2);
}

void GCMDriverTest::Register(const std::string& app_id,
                             const std::vector<std::string>& sender_ids,
                             WaitToFinish wait_to_finish) {
  base::RunLoop run_loop;
  async_operation_completed_callback_ = run_loop.QuitClosure();
  driver_->Register(app_id,
                    sender_ids,
                    base::Bind(&GCMDriverTest::RegisterCompleted,
                               base::Unretained(this)));
  if (wait_to_finish == WAIT)
    run_loop.Run();
}

void GCMDriverTest::Send(const std::string& app_id,
                         const std::string& receiver_id,
                         const GCMClient::OutgoingMessage& message,
                         WaitToFinish wait_to_finish) {
  base::RunLoop run_loop;
  async_operation_completed_callback_ = run_loop.QuitClosure();
  driver_->Send(app_id,
                receiver_id,
                message,
                base::Bind(&GCMDriverTest::SendCompleted,
                           base::Unretained(this)));
  if (wait_to_finish == WAIT)
    run_loop.Run();
}

void GCMDriverTest::Unregister(const std::string& app_id,
                               WaitToFinish wait_to_finish) {
  base::RunLoop run_loop;
  async_operation_completed_callback_ = run_loop.QuitClosure();
  driver_->Unregister(app_id,
                      base::Bind(&GCMDriverTest::UnregisterCompleted,
                                 base::Unretained(this)));
  if (wait_to_finish == WAIT)
    run_loop.Run();
}

void GCMDriverTest::WaitForAsyncOperation() {
  base::RunLoop run_loop;
  async_operation_completed_callback_ = run_loop.QuitClosure();
  run_loop.Run();
}

void GCMDriverTest::RegisterCompleted(const std::string& registration_id,
                                      GCMClient::Result result) {
  registration_id_ = registration_id;
  registration_result_ = result;
  if (!async_operation_completed_callback_.is_null())
    async_operation_completed_callback_.Run();
}

void GCMDriverTest::SendCompleted(const std::string& message_id,
                                  GCMClient::Result result) {
  send_message_id_ = message_id;
  send_result_ = result;
  if (!async_operation_completed_callback_.is_null())
    async_operation_completed_callback_.Run();
}

void GCMDriverTest::UnregisterCompleted(GCMClient::Result result) {
  unregistration_result_ = result;
  if (!async_operation_completed_callback_.is_null())
    async_operation_completed_callback_.Run();
}

TEST_F(GCMDriverTest, Create) {
  // Create GCMDriver first. By default GCM is set to delay start.
  CreateDriver();
  EXPECT_FALSE(driver()->IsStarted());

  // Adding an app handler will not start GCM.
  AddAppHandlers();
  PumpIOLoop();
  PumpUILoop();
  EXPECT_FALSE(driver()->IsStarted());
  EXPECT_FALSE(driver()->IsConnected());
  EXPECT_FALSE(gcm_connection_observer()->connected());

  // The GCM registration will kick off the GCM.
  Register(kTestAppID1, ToSenderList("sender"), GCMDriverTest::WAIT);
  EXPECT_TRUE(driver()->IsStarted());
  EXPECT_TRUE(driver()->IsConnected());
  EXPECT_TRUE(gcm_connection_observer()->connected());
}

TEST_F(GCMDriverTest, Shutdown) {
  CreateDriver();
  EXPECT_FALSE(HasAppHandlers());

  AddAppHandlers();
  EXPECT_TRUE(HasAppHandlers());

  ShutdownDriver();
  EXPECT_FALSE(HasAppHandlers());
  EXPECT_FALSE(driver()->IsConnected());
  EXPECT_FALSE(gcm_connection_observer()->connected());
}

TEST_F(GCMDriverTest, DisableAndReenableGCM) {
  CreateDriver();
  AddAppHandlers();
  PumpIOLoop();
  PumpUILoop();
  EXPECT_FALSE(driver()->IsStarted());

  // The GCM registration will kick off the GCM.
  Register(kTestAppID1, ToSenderList("sender"), GCMDriverTest::WAIT);
  EXPECT_TRUE(driver()->IsStarted());

  // Disables the GCM. GCM will be stopped.
  driver()->Disable();
  PumpIOLoop();
  PumpUILoop();
  EXPECT_FALSE(driver()->IsStarted());

  // Enables the GCM. GCM will be started.
  driver()->Enable();
  PumpIOLoop();
  PumpUILoop();
  EXPECT_TRUE(driver()->IsStarted());
}

TEST_F(GCMDriverTest, StartOrStopGCMOnDemand) {
  CreateDriver();
  PumpIOLoop();
  PumpUILoop();
  EXPECT_FALSE(driver()->IsStarted());

  // Adding an app handler will not start GCM.
  driver()->AddAppHandler(kTestAppID1, gcm_app_handler());
  PumpIOLoop();
  PumpUILoop();
  EXPECT_FALSE(driver()->IsStarted());

  // The GCM registration will kick off the GCM.
  Register(kTestAppID1, ToSenderList("sender"), GCMDriverTest::WAIT);
  EXPECT_TRUE(driver()->IsStarted());

  // Add another app handler.
  driver()->AddAppHandler(kTestAppID2, gcm_app_handler());
  PumpIOLoop();
  PumpUILoop();
  EXPECT_TRUE(driver()->IsStarted());

  // GCMClient remains active after one app handler is gone.
  driver()->RemoveAppHandler(kTestAppID1);
  PumpIOLoop();
  PumpUILoop();
  EXPECT_TRUE(driver()->IsStarted());

  // GCMClient should be stopped after the last app handler is gone.
  driver()->RemoveAppHandler(kTestAppID2);
  PumpIOLoop();
  PumpUILoop();
  EXPECT_FALSE(driver()->IsStarted());

  // GCMClient is restarted after an app handler has been added.
  driver()->AddAppHandler(kTestAppID2, gcm_app_handler());
  PumpIOLoop();
  PumpUILoop();
  EXPECT_TRUE(driver()->IsStarted());
}

TEST_F(GCMDriverTest, RegisterFailed) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");

  CreateDriver();

  // Registration fails when the no app handler is added.
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  EXPECT_TRUE(registration_id().empty());
  EXPECT_EQ(GCMClient::UNKNOWN_ERROR, registration_result());

  ClearResults();

  // Registration fails when GCM is disabled.
  AddAppHandlers();
  driver()->Disable();
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  EXPECT_TRUE(registration_id().empty());
  EXPECT_EQ(GCMClient::GCM_DISABLED, registration_result());
}

TEST_F(GCMDriverTest, UnregisterFailed) {
  CreateDriver();

  // Unregistration fails when the no app handler is added.
  Unregister(kTestAppID1, GCMDriverTest::WAIT);
  EXPECT_EQ(GCMClient::UNKNOWN_ERROR, unregistration_result());

  ClearResults();

  // Unregistration fails when GCM is disabled.
  AddAppHandlers();
  driver()->Disable();
  Unregister(kTestAppID1, GCMDriverTest::WAIT);
  EXPECT_EQ(GCMClient::GCM_DISABLED, unregistration_result());
}

TEST_F(GCMDriverTest, SendFailed) {
  GCMClient::OutgoingMessage message;
  message.id = "1";
  message.data["key1"] = "value1";

  CreateDriver();

  // Sending fails when the no app handler is added.
  Send(kTestAppID1, kUserID1, message, GCMDriverTest::WAIT);
  EXPECT_TRUE(send_message_id().empty());
  EXPECT_EQ(GCMClient::UNKNOWN_ERROR, send_result());

  ClearResults();

  // Sending fails when GCM is disabled.
  AddAppHandlers();
  driver()->Disable();
  Send(kTestAppID1, kUserID1, message, GCMDriverTest::WAIT);
  EXPECT_TRUE(send_message_id().empty());
  EXPECT_EQ(GCMClient::GCM_DISABLED, send_result());
}

TEST_F(GCMDriverTest, GCMClientNotReadyBeforeRegistration) {
  CreateDriver();
  PumpIOLoop();
  PumpUILoop();

  // Make GCMClient not ready until PerformDelayedStart is called.
  GetGCMClient()->set_start_mode_overridding(
      FakeGCMClient::FORCE_TO_ALWAYS_DELAY_START_GCM);

  AddAppHandlers();

  // The registration is on hold until GCMClient is ready.
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  Register(kTestAppID1,
                     sender_ids,
                     GCMDriverTest::DO_NOT_WAIT);
  PumpIOLoop();
  PumpUILoop();
  EXPECT_TRUE(registration_id().empty());
  EXPECT_EQ(GCMClient::UNKNOWN_ERROR, registration_result());

  // Register operation will be invoked after GCMClient becomes ready.
  GetGCMClient()->PerformDelayedStart();
  WaitForAsyncOperation();
  EXPECT_FALSE(registration_id().empty());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverTest, GCMClientNotReadyBeforeSending) {
  CreateDriver();
  PumpIOLoop();
  PumpUILoop();

  // Make GCMClient not ready until PerformDelayedStart is called.
  GetGCMClient()->set_start_mode_overridding(
      FakeGCMClient::FORCE_TO_ALWAYS_DELAY_START_GCM);

  AddAppHandlers();

  // The sending is on hold until GCMClient is ready.
  GCMClient::OutgoingMessage message;
  message.id = "1";
  message.data["key1"] = "value1";
  message.data["key2"] = "value2";
  Send(kTestAppID1, kUserID1, message, GCMDriverTest::DO_NOT_WAIT);
  PumpIOLoop();
  PumpUILoop();

  EXPECT_TRUE(send_message_id().empty());
  EXPECT_EQ(GCMClient::UNKNOWN_ERROR, send_result());

  // Send operation will be invoked after GCMClient becomes ready.
  GetGCMClient()->PerformDelayedStart();
  WaitForAsyncOperation();
  EXPECT_EQ(message.id, send_message_id());
  EXPECT_EQ(GCMClient::SUCCESS, send_result());
}

// Tests a single instance of GCMDriver.
class GCMDriverFunctionalTest : public GCMDriverTest {
 public:
  GCMDriverFunctionalTest();
  ~GCMDriverFunctionalTest() override;

  // GCMDriverTest:
  void SetUp() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(GCMDriverFunctionalTest);
};

GCMDriverFunctionalTest::GCMDriverFunctionalTest() {
}

GCMDriverFunctionalTest::~GCMDriverFunctionalTest() {
}

void GCMDriverFunctionalTest::SetUp() {
  GCMDriverTest::SetUp();

  CreateDriver();
  AddAppHandlers();
  PumpIOLoop();
  PumpUILoop();
}

TEST_F(GCMDriverFunctionalTest, Register) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  const std::string expected_registration_id =
      FakeGCMClient::GenerateGCMRegistrationID(sender_ids);

  EXPECT_EQ(expected_registration_id, registration_id());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverFunctionalTest, RegisterError) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1@error");
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);

  EXPECT_TRUE(registration_id().empty());
  EXPECT_NE(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverFunctionalTest, RegisterAgainWithSameSenderIDs) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  sender_ids.push_back("sender2");
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  const std::string expected_registration_id =
      FakeGCMClient::GenerateGCMRegistrationID(sender_ids);

  EXPECT_EQ(expected_registration_id, registration_id());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());

  // Clears the results the would be set by the Register callback in preparation
  // to call register 2nd time.
  ClearResults();

  // Calling register 2nd time with the same set of sender IDs but different
  // ordering will get back the same registration ID.
  std::vector<std::string> another_sender_ids;
  another_sender_ids.push_back("sender2");
  another_sender_ids.push_back("sender1");
  Register(kTestAppID1, another_sender_ids, GCMDriverTest::WAIT);

  EXPECT_EQ(expected_registration_id, registration_id());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverFunctionalTest, RegisterAgainWithDifferentSenderIDs) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  const std::string expected_registration_id =
      FakeGCMClient::GenerateGCMRegistrationID(sender_ids);

  EXPECT_EQ(expected_registration_id, registration_id());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());

  // Make sender IDs different.
  sender_ids.push_back("sender2");
  const std::string expected_registration_id2 =
      FakeGCMClient::GenerateGCMRegistrationID(sender_ids);

  // Calling register 2nd time with the different sender IDs will get back a new
  // registration ID.
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  EXPECT_EQ(expected_registration_id2, registration_id());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverFunctionalTest, UnregisterExplicitly) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);

  EXPECT_FALSE(registration_id().empty());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());

  Unregister(kTestAppID1, GCMDriverTest::WAIT);

  EXPECT_EQ(GCMClient::SUCCESS, unregistration_result());
}

TEST_F(GCMDriverFunctionalTest, UnregisterWhenAsyncOperationPending) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  // First start registration without waiting for it to complete.
  Register(kTestAppID1, sender_ids, GCMDriverTest::DO_NOT_WAIT);

  // Test that unregistration fails with async operation pending when there is a
  // registration already in progress.
  Unregister(kTestAppID1, GCMDriverTest::WAIT);
  EXPECT_EQ(GCMClient::ASYNC_OPERATION_PENDING,
            unregistration_result());

  // Complete the unregistration.
  WaitForAsyncOperation();
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());

  // Start unregistration without waiting for it to complete. This time no async
  // operation is pending.
  Unregister(kTestAppID1, GCMDriverTest::DO_NOT_WAIT);

  // Test that unregistration fails with async operation pending when there is
  // an unregistration already in progress.
  Unregister(kTestAppID1, GCMDriverTest::WAIT);
  EXPECT_EQ(GCMClient::ASYNC_OPERATION_PENDING,
            unregistration_result());
  ClearResults();

  // Complete unregistration.
  WaitForAsyncOperation();
  EXPECT_EQ(GCMClient::SUCCESS, unregistration_result());
}

TEST_F(GCMDriverFunctionalTest, RegisterWhenAsyncOperationPending) {
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  // First start registration without waiting for it to complete.
  Register(kTestAppID1, sender_ids, GCMDriverTest::DO_NOT_WAIT);

  // Test that registration fails with async operation pending when there is a
  // registration already in progress.
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  EXPECT_EQ(GCMClient::ASYNC_OPERATION_PENDING,
            registration_result());
  ClearResults();

  // Complete the registration.
  WaitForAsyncOperation();
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverFunctionalTest, RegisterAfterUnfinishedUnregister) {
  // Register and wait for it to complete.
  std::vector<std::string> sender_ids;
  sender_ids.push_back("sender1");
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
  EXPECT_EQ(FakeGCMClient::GenerateGCMRegistrationID(sender_ids),
            registration_id());

  // Clears the results the would be set by the Register callback in preparation
  // to call register 2nd time.
  ClearResults();

  // Start unregistration without waiting for it to complete.
  Unregister(kTestAppID1, GCMDriverTest::DO_NOT_WAIT);

  // Register immeidately after unregistration is not completed.
  sender_ids.push_back("sender2");
  Register(kTestAppID1, sender_ids, GCMDriverTest::WAIT);

  // We need one more waiting since the waiting in Register is indeed for
  // uncompleted Unregister.
  WaitForAsyncOperation();
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
  EXPECT_EQ(FakeGCMClient::GenerateGCMRegistrationID(sender_ids),
            registration_id());
}

TEST_F(GCMDriverFunctionalTest, Send) {
  GCMClient::OutgoingMessage message;
  message.id = "1@ack";
  message.data["key1"] = "value1";
  message.data["key2"] = "value2";
  Send(kTestAppID1, kUserID1, message, GCMDriverTest::WAIT);

  EXPECT_EQ(message.id, send_message_id());
  EXPECT_EQ(GCMClient::SUCCESS, send_result());

  gcm_app_handler()->WaitForNotification();
  EXPECT_EQ(message.id, gcm_app_handler()->acked_message_id());
  EXPECT_EQ(kTestAppID1, gcm_app_handler()->app_id());
}

TEST_F(GCMDriverFunctionalTest, SendError) {
  GCMClient::OutgoingMessage message;
  // Embedding error in id will tell the mock to simulate the send error.
  message.id = "1@error";
  message.data["key1"] = "value1";
  message.data["key2"] = "value2";
  Send(kTestAppID1, kUserID1, message, GCMDriverTest::WAIT);

  EXPECT_EQ(message.id, send_message_id());
  EXPECT_EQ(GCMClient::SUCCESS, send_result());

  // Wait for the send error.
  gcm_app_handler()->WaitForNotification();
  EXPECT_EQ(FakeGCMAppHandler::SEND_ERROR_EVENT,
            gcm_app_handler()->received_event());
  EXPECT_EQ(kTestAppID1, gcm_app_handler()->app_id());
  EXPECT_EQ(message.id,
            gcm_app_handler()->send_error_details().message_id);
  EXPECT_NE(GCMClient::SUCCESS,
            gcm_app_handler()->send_error_details().result);
  EXPECT_EQ(message.data,
            gcm_app_handler()->send_error_details().additional_data);
}

TEST_F(GCMDriverFunctionalTest, MessageReceived) {
  // GCM registration has to be performed otherwise GCM will not be started.
  Register(kTestAppID1, ToSenderList("sender"), GCMDriverTest::WAIT);

  GCMClient::IncomingMessage message;
  message.data["key1"] = "value1";
  message.data["key2"] = "value2";
  message.sender_id = "sender";
  GetGCMClient()->ReceiveMessage(kTestAppID1, message);
  gcm_app_handler()->WaitForNotification();
  EXPECT_EQ(FakeGCMAppHandler::MESSAGE_EVENT,
            gcm_app_handler()->received_event());
  EXPECT_EQ(kTestAppID1, gcm_app_handler()->app_id());
  EXPECT_EQ(message.data, gcm_app_handler()->message().data);
  EXPECT_TRUE(gcm_app_handler()->message().collapse_key.empty());
  EXPECT_EQ(message.sender_id, gcm_app_handler()->message().sender_id);
}

TEST_F(GCMDriverFunctionalTest, MessageWithCollapseKeyReceived) {
  // GCM registration has to be performed otherwise GCM will not be started.
  Register(kTestAppID1, ToSenderList("sender"), GCMDriverTest::WAIT);

  GCMClient::IncomingMessage message;
  message.data["key1"] = "value1";
  message.collapse_key = "collapse_key_value";
  message.sender_id = "sender";
  GetGCMClient()->ReceiveMessage(kTestAppID1, message);
  gcm_app_handler()->WaitForNotification();
  EXPECT_EQ(FakeGCMAppHandler::MESSAGE_EVENT,
            gcm_app_handler()->received_event());
  EXPECT_EQ(kTestAppID1, gcm_app_handler()->app_id());
  EXPECT_EQ(message.data, gcm_app_handler()->message().data);
  EXPECT_EQ(message.collapse_key,
            gcm_app_handler()->message().collapse_key);
}

TEST_F(GCMDriverFunctionalTest, MessagesDeleted) {
  // GCM registration has to be performed otherwise GCM will not be started.
  Register(kTestAppID1, ToSenderList("sender"), GCMDriverTest::WAIT);

  GetGCMClient()->DeleteMessages(kTestAppID1);
  gcm_app_handler()->WaitForNotification();
  EXPECT_EQ(FakeGCMAppHandler::MESSAGES_DELETED_EVENT,
            gcm_app_handler()->received_event());
  EXPECT_EQ(kTestAppID1, gcm_app_handler()->app_id());
}

TEST_F(GCMDriverFunctionalTest, LastTokenFetchTime) {
  // GCM registration has to be performed otherwise GCM will not be started.
  Register(kTestAppID1, ToSenderList("sender"), GCMDriverTest::WAIT);

  EXPECT_EQ(base::Time(), driver()->GetLastTokenFetchTime());
  base::Time fetch_time = base::Time::Now();
  driver()->SetLastTokenFetchTime(fetch_time);
  EXPECT_EQ(fetch_time, driver()->GetLastTokenFetchTime());
}

// Tests a single instance of GCMDriver.
class GCMChannelStatusSyncerTest : public GCMDriverTest {
 public:
  GCMChannelStatusSyncerTest();
  ~GCMChannelStatusSyncerTest() override;

  // testing::Test:
  void SetUp() override;

  void CompleteGCMChannelStatusRequest(bool enabled, int poll_interval_seconds);
  bool CompareDelaySeconds(int64 expected_delay_seconds,
                           int64 actual_delay_seconds);

  GCMChannelStatusSyncer* syncer() {
    return driver()->gcm_channel_status_syncer_for_testing();
  }

 private:
  net::TestURLFetcherFactory url_fetcher_factory_;

  DISALLOW_COPY_AND_ASSIGN(GCMChannelStatusSyncerTest);
};

GCMChannelStatusSyncerTest::GCMChannelStatusSyncerTest() {
}

GCMChannelStatusSyncerTest::~GCMChannelStatusSyncerTest() {
}

void GCMChannelStatusSyncerTest::SetUp() {
  GCMDriverTest::SetUp();

  url_fetcher_factory_.set_remove_fetcher_on_delete(true);

  // Turn on all-user support.
  ASSERT_TRUE(base::FieldTrialList::CreateFieldTrial("GCM", "Enabled"));
}

void GCMChannelStatusSyncerTest::CompleteGCMChannelStatusRequest(
    bool enabled, int poll_interval_seconds) {
  sync_pb::ExperimentStatusResponse response_proto;
  sync_pb::ExperimentsSpecifics* experiment_specifics =
      response_proto.add_experiment();
  experiment_specifics->mutable_gcm_channel()->set_enabled(enabled);

  if (poll_interval_seconds)
    response_proto.set_poll_interval_seconds(poll_interval_seconds);

  std::string response_string;
  response_proto.SerializeToString(&response_string);

  net::TestURLFetcher* fetcher = url_fetcher_factory_.GetFetcherByID(0);
  ASSERT_TRUE(fetcher);
  fetcher->set_response_code(net::HTTP_OK);
  fetcher->SetResponseString(response_string);
  fetcher->delegate()->OnURLFetchComplete(fetcher);
}

bool GCMChannelStatusSyncerTest::CompareDelaySeconds(
    int64 expected_delay_seconds, int64 actual_delay_seconds) {
  // Most of time, the actual delay should not be smaller than the expected
  // delay.
  if (actual_delay_seconds >= expected_delay_seconds)
    return true;
  // It is also OK that the actual delay is a bit smaller than the expected
  // delay in case that the test runs slowly.
  return expected_delay_seconds - actual_delay_seconds < 30;
}

TEST_F(GCMChannelStatusSyncerTest, DisableAndEnable) {
  // Create GCMDriver first. By default, GCM is enabled.
  CreateDriver();
  EXPECT_TRUE(driver()->gcm_enabled());
  EXPECT_TRUE(syncer()->gcm_enabled());

  // Remove delay such that the request could be executed immediately.
  syncer()->set_delay_removed_for_testing(true);

  // GCM is still enabled at this point.
  AddAppHandlers();
  EXPECT_TRUE(driver()->gcm_enabled());
  EXPECT_TRUE(syncer()->gcm_enabled());

  // Wait until the GCM channel status request gets triggered.
  PumpUILoop();

  // Complete the request that disables the GCM.
  CompleteGCMChannelStatusRequest(false, 0);
  EXPECT_FALSE(driver()->gcm_enabled());
  EXPECT_FALSE(syncer()->gcm_enabled());
  EXPECT_FALSE(driver()->IsStarted());

  // Wait until next GCM channel status request gets triggered.
  PumpUILoop();

  // Complete the request that enables the GCM.
  CompleteGCMChannelStatusRequest(true, 0);
  EXPECT_TRUE(driver()->gcm_enabled());
  EXPECT_TRUE(syncer()->gcm_enabled());
}

TEST_F(GCMChannelStatusSyncerTest, DisableRestartAndEnable) {
  // Create GCMDriver first. By default, GCM is enabled.
  CreateDriver();
  EXPECT_TRUE(driver()->gcm_enabled());
  EXPECT_TRUE(syncer()->gcm_enabled());

  // Remove delay such that the request could be executed immediately.
  syncer()->set_delay_removed_for_testing(true);

  // GCM is still enabled at this point.
  AddAppHandlers();
  EXPECT_TRUE(driver()->gcm_enabled());
  EXPECT_TRUE(syncer()->gcm_enabled());

  // Wait until the GCM channel status request gets triggered.
  PumpUILoop();

  // Complete the request that disables the GCM.
  CompleteGCMChannelStatusRequest(false, 0);
  EXPECT_FALSE(driver()->gcm_enabled());
  EXPECT_FALSE(syncer()->gcm_enabled());

  // Simulate browser start by recreating GCMDriver.
  ShutdownDriver();
  CreateDriver();

  // Remove delay such that the request could be executed immediately.
  syncer()->set_delay_removed_for_testing(true);

  // GCM is still disabled.
  EXPECT_FALSE(driver()->gcm_enabled());
  EXPECT_FALSE(syncer()->gcm_enabled());

  AddAppHandlers();
  EXPECT_FALSE(driver()->gcm_enabled());
  EXPECT_FALSE(syncer()->gcm_enabled());

  // Wait until the GCM channel status request gets triggered.
  PumpUILoop();

  // Complete the request that re-enables the GCM.
  CompleteGCMChannelStatusRequest(true, 0);
  EXPECT_TRUE(driver()->gcm_enabled());
  EXPECT_TRUE(syncer()->gcm_enabled());
}

TEST_F(GCMChannelStatusSyncerTest, FirstTimePolling) {
  // Start GCM.
  CreateDriver();
  AddAppHandlers();

  // The 1st request should be triggered shortly without jittering.
  EXPECT_EQ(GCMChannelStatusSyncer::first_time_delay_seconds(),
            syncer()->current_request_delay_interval().InSeconds());
}

TEST_F(GCMChannelStatusSyncerTest, SubsequentPollingWithDefaultInterval) {
  // Create GCMDriver first. GCM is not started.
  CreateDriver();

  // Remove delay such that the request could be executed immediately.
  syncer()->set_delay_removed_for_testing(true);

  // Now GCM is started.
  AddAppHandlers();

  // Wait until the GCM channel status request gets triggered.
  PumpUILoop();

  // Keep delay such that we can find out the computed delay time.
  syncer()->set_delay_removed_for_testing(false);

  // Complete the request. The default interval is intact.
  CompleteGCMChannelStatusRequest(true, 0);

  // The next request should be scheduled at the expected default interval.
  int64 actual_delay_seconds =
      syncer()->current_request_delay_interval().InSeconds();
  int64 expected_delay_seconds =
      GCMChannelStatusRequest::default_poll_interval_seconds();
  EXPECT_TRUE(CompareDelaySeconds(expected_delay_seconds, actual_delay_seconds))
      << "expected delay: " << expected_delay_seconds
      << " actual delay: " << actual_delay_seconds;

  // Simulate browser start by recreating GCMDriver.
  ShutdownDriver();
  CreateDriver();
  AddAppHandlers();

  // After start-up, the request should still be scheduled at the expected
  // default interval.
  actual_delay_seconds =
      syncer()->current_request_delay_interval().InSeconds();
  EXPECT_TRUE(CompareDelaySeconds(expected_delay_seconds, actual_delay_seconds))
      << "expected delay: " << expected_delay_seconds
      << " actual delay: " << actual_delay_seconds;
}

TEST_F(GCMChannelStatusSyncerTest, SubsequentPollingWithUpdatedInterval) {
  // Create GCMDriver first. GCM is not started.
  CreateDriver();

  // Remove delay such that the request could be executed immediately.
  syncer()->set_delay_removed_for_testing(true);

  // Now GCM is started.
  AddAppHandlers();

  // Wait until the GCM channel status request gets triggered.
  PumpUILoop();

  // Keep delay such that we can find out the computed delay time.
  syncer()->set_delay_removed_for_testing(false);

  // Complete the request. The interval is being changed.
  int new_poll_interval_seconds =
      GCMChannelStatusRequest::default_poll_interval_seconds() * 2;
  CompleteGCMChannelStatusRequest(true, new_poll_interval_seconds);

  // The next request should be scheduled at the expected updated interval.
  int64 actual_delay_seconds =
      syncer()->current_request_delay_interval().InSeconds();
  int64 expected_delay_seconds = new_poll_interval_seconds;
  EXPECT_TRUE(CompareDelaySeconds(expected_delay_seconds, actual_delay_seconds))
      << "expected delay: " << expected_delay_seconds
      << " actual delay: " << actual_delay_seconds;

  // Simulate browser start by recreating GCMDriver.
  ShutdownDriver();
  CreateDriver();
  AddAppHandlers();

  // After start-up, the request should still be scheduled at the expected
  // updated interval.
  actual_delay_seconds =
      syncer()->current_request_delay_interval().InSeconds();
  EXPECT_TRUE(CompareDelaySeconds(expected_delay_seconds, actual_delay_seconds))
      << "expected delay: " << expected_delay_seconds
      << " actual delay: " << actual_delay_seconds;
}

class GCMDriverInstanceIDTest : public GCMDriverTest {
 public:
  GCMDriverInstanceIDTest();
  ~GCMDriverInstanceIDTest() override;

  void GetReady();
  void GetInstanceID(const std::string& app_id, WaitToFinish wait_to_finish);
  void GetInstanceIDDataCompleted(const std::string& instance_id,
                                  const std::string& extra_data);
  void GetToken(const std::string& app_id,
                const std::string& authorized_entity,
                const std::string& scope,
                WaitToFinish wait_to_finish);
  void DeleteToken(const std::string& app_id,
                   const std::string& authorized_entity,
                   const std::string& scope,
                   WaitToFinish wait_to_finish);

  std::string instance_id() const { return instance_id_; }
  std::string extra_data() const { return extra_data_; }

 private:
  std::string instance_id_;
  std::string extra_data_;

  DISALLOW_COPY_AND_ASSIGN(GCMDriverInstanceIDTest);
};

GCMDriverInstanceIDTest::GCMDriverInstanceIDTest() {
}

GCMDriverInstanceIDTest::~GCMDriverInstanceIDTest() {
}

void GCMDriverInstanceIDTest::GetReady() {
  CreateDriver();
  AddAppHandlers();
  PumpIOLoop();
  PumpUILoop();
}

void GCMDriverInstanceIDTest::GetInstanceID(const std::string& app_id,
                                            WaitToFinish wait_to_finish) {
  base::RunLoop run_loop;
  set_async_operation_completed_callback(run_loop.QuitClosure());
  driver()->GetInstanceIDData(app_id,
      base::Bind(&GCMDriverInstanceIDTest::GetInstanceIDDataCompleted,
                 base::Unretained(this)));
  if (wait_to_finish == WAIT)
    run_loop.Run();
}

void GCMDriverInstanceIDTest::GetInstanceIDDataCompleted(
    const std::string& instance_id, const std::string& extra_data) {
  instance_id_ = instance_id;
  extra_data_ = extra_data;
  if (!async_operation_completed_callback().is_null())
    async_operation_completed_callback().Run();
}

void GCMDriverInstanceIDTest::GetToken(const std::string& app_id,
                                       const std::string& authorized_entity,
                                       const std::string& scope,
                                       WaitToFinish wait_to_finish) {
  base::RunLoop run_loop;
  set_async_operation_completed_callback(run_loop.QuitClosure());
  std::map<std::string, std::string> options;
  driver()->GetToken(app_id,
                     authorized_entity,
                     scope,
                     options,
                     base::Bind(&GCMDriverTest::RegisterCompleted,
                                base::Unretained(this)));
  if (wait_to_finish == WAIT)
    run_loop.Run();
}

void GCMDriverInstanceIDTest::DeleteToken(const std::string& app_id,
                                          const std::string& authorized_entity,
                                          const std::string& scope,
                                          WaitToFinish wait_to_finish) {
  base::RunLoop run_loop;
  set_async_operation_completed_callback(run_loop.QuitClosure());
  driver()->DeleteToken(app_id,
                        authorized_entity,
                        scope,
                        base::Bind(&GCMDriverTest::UnregisterCompleted,
                                   base::Unretained(this)));
  if (wait_to_finish == WAIT)
    run_loop.Run();
}

TEST_F(GCMDriverInstanceIDTest, InstanceIDData) {
  GetReady();

  driver()->AddInstanceIDData(kTestAppID1, kInstanceID1, "Foo");
  GetInstanceID(kTestAppID1, GCMDriverTest::WAIT);

  EXPECT_EQ(kInstanceID1, instance_id());
  EXPECT_EQ("Foo", extra_data());

  driver()->RemoveInstanceIDData(kTestAppID1);
  GetInstanceID(kTestAppID1, GCMDriverTest::WAIT);

  EXPECT_TRUE(instance_id().empty());
  EXPECT_TRUE(extra_data().empty());
}

TEST_F(GCMDriverInstanceIDTest, GCMClientNotReadyBeforeInstanceIDData) {
  CreateDriver();
  PumpIOLoop();
  PumpUILoop();

  // Make GCMClient not ready until PerformDelayedStart is called.
  GetGCMClient()->set_start_mode_overridding(
      FakeGCMClient::FORCE_TO_ALWAYS_DELAY_START_GCM);

  AddAppHandlers();

  // All operations are on hold until GCMClient is ready.
  driver()->AddInstanceIDData(kTestAppID1, kInstanceID1, "Foo");
  driver()->AddInstanceIDData(kTestAppID2, kInstanceID2, "Bar");
  driver()->RemoveInstanceIDData(kTestAppID1);
  GetInstanceID(kTestAppID2, GCMDriverTest::DO_NOT_WAIT);
  PumpIOLoop();
  PumpUILoop();
  EXPECT_TRUE(instance_id().empty());
  EXPECT_TRUE(extra_data().empty());

  // All operations will be performed after GCMClient becomes ready.
  GetGCMClient()->PerformDelayedStart();
  WaitForAsyncOperation();
  EXPECT_EQ(kInstanceID2, instance_id());
  EXPECT_EQ("Bar", extra_data());
}

TEST_F(GCMDriverInstanceIDTest, GetToken) {
  GetReady();

  const std::string expected_token =
      FakeGCMClient::GenerateInstanceIDToken(kUserID1, kScope);
  GetToken(kTestAppID1, kUserID1, kScope, GCMDriverTest::WAIT);

  EXPECT_EQ(expected_token, registration_id());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverInstanceIDTest, GetTokenError) {
  GetReady();

  std::string error_entity = "sender@error";
  GetToken(kTestAppID1, error_entity, kScope, GCMDriverTest::WAIT);

  EXPECT_TRUE(registration_id().empty());
  EXPECT_NE(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverInstanceIDTest, GCMClientNotReadyBeforeGetToken) {
  CreateDriver();
  PumpIOLoop();
  PumpUILoop();

  // Make GCMClient not ready until PerformDelayedStart is called.
  GetGCMClient()->set_start_mode_overridding(
      FakeGCMClient::FORCE_TO_ALWAYS_DELAY_START_GCM);

  AddAppHandlers();

  // GetToken operation is on hold until GCMClient is ready.
  GetToken(kTestAppID1, kUserID1, kScope, GCMDriverTest::DO_NOT_WAIT);
  PumpIOLoop();
  PumpUILoop();
  EXPECT_TRUE(registration_id().empty());
  EXPECT_EQ(GCMClient::UNKNOWN_ERROR, registration_result());

  // GetToken operation will be invoked after GCMClient becomes ready.
  GetGCMClient()->PerformDelayedStart();
  WaitForAsyncOperation();
  EXPECT_FALSE(registration_id().empty());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());
}

TEST_F(GCMDriverInstanceIDTest, DeleteToken) {
  GetReady();

  const std::string expected_token =
      FakeGCMClient::GenerateInstanceIDToken(kUserID1, kScope);
  GetToken(kTestAppID1, kUserID1, kScope, GCMDriverTest::WAIT);
  EXPECT_EQ(expected_token, registration_id());
  EXPECT_EQ(GCMClient::SUCCESS, registration_result());

  DeleteToken(kTestAppID1, kUserID1, kScope, GCMDriverTest::WAIT);
  EXPECT_EQ(GCMClient::SUCCESS, unregistration_result());
}

TEST_F(GCMDriverInstanceIDTest, GCMClientNotReadyBeforeDeleteToken) {
  CreateDriver();
  PumpIOLoop();
  PumpUILoop();

  // Make GCMClient not ready until PerformDelayedStart is called.
  GetGCMClient()->set_start_mode_overridding(
      FakeGCMClient::FORCE_TO_ALWAYS_DELAY_START_GCM);

  AddAppHandlers();

  // DeleteToken operation is on hold until GCMClient is ready.
  DeleteToken(kTestAppID1, kUserID1, kScope, GCMDriverTest::DO_NOT_WAIT);
  PumpIOLoop();
  PumpUILoop();
  EXPECT_EQ(GCMClient::UNKNOWN_ERROR, unregistration_result());

  // DeleteToken operation will be invoked after GCMClient becomes ready.
  GetGCMClient()->PerformDelayedStart();
  WaitForAsyncOperation();
  EXPECT_EQ(GCMClient::SUCCESS, unregistration_result());
}

}  // namespace gcm
