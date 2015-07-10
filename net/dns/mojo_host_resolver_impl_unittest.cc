// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/dns/mojo_host_resolver_impl.h"

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/run_loop.h"
#include "base/time/time.h"
#include "net/base/address_list.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/dns/mock_host_resolver.h"
#include "net/dns/mojo_host_type_converters.h"
#include "net/log/net_log.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/mojo/src/mojo/public/cpp/bindings/binding.h"
#include "third_party/mojo/src/mojo/public/cpp/bindings/interface_request.h"

namespace net {

namespace {

class TestRequestClient : public interfaces::HostResolverRequestClient {
 public:
  explicit TestRequestClient(
      mojo::InterfaceRequest<interfaces::HostResolverRequestClient> req)
      : done_(false), binding_(this, req.Pass()) {
    binding_.set_connection_error_handler(base::Bind(
        &TestRequestClient::OnConnectionError, base::Unretained(this)));
  }

  void WaitForResult();
  void WaitForConnectionError();

  int32_t error_;
  interfaces::AddressListPtr results_;

 private:
  // Overridden from interfaces::HostResolverRequestClient.
  void ReportResult(int32_t error, interfaces::AddressListPtr results) override;

  // Mojo error handler.
  void OnConnectionError();

  bool done_;
  base::Closure run_loop_quit_closure_;
  base::Closure connection_error_quit_closure_;

  mojo::Binding<interfaces::HostResolverRequestClient> binding_;
};

void TestRequestClient::WaitForResult() {
  if (done_)
    return;

  base::RunLoop run_loop;
  run_loop_quit_closure_ = run_loop.QuitClosure();
  run_loop.Run();
  ASSERT_TRUE(done_);
}

void TestRequestClient::WaitForConnectionError() {
  base::RunLoop run_loop;
  connection_error_quit_closure_ = run_loop.QuitClosure();
  run_loop.Run();
}

void TestRequestClient::ReportResult(int32_t error,
                                     interfaces::AddressListPtr results) {
  if (!run_loop_quit_closure_.is_null()) {
    run_loop_quit_closure_.Run();
  }
  ASSERT_FALSE(done_);
  error_ = error;
  results_ = results.Pass();
  done_ = true;
}

void TestRequestClient::OnConnectionError() {
  if (!connection_error_quit_closure_.is_null())
    connection_error_quit_closure_.Run();
}

class CallbackMockHostResolver : public MockHostResolver {
 public:
  CallbackMockHostResolver() {}
  ~CallbackMockHostResolver() override {}

  // Set a callback to run whenever Resolve is called. Callback is cleared after
  // every run.
  void SetResolveCallback(base::Closure callback) {
    resolve_callback_ = callback;
  }

  // Overridden from MockHostResolver.
  int Resolve(const RequestInfo& info,
              RequestPriority priority,
              AddressList* addresses,
              const CompletionCallback& callback,
              RequestHandle* out_req,
              const BoundNetLog& net_log) override;

 private:
  base::Closure resolve_callback_;
};

int CallbackMockHostResolver::Resolve(const RequestInfo& info,
                                      RequestPriority priority,
                                      AddressList* addresses,
                                      const CompletionCallback& callback,
                                      RequestHandle* out_req,
                                      const BoundNetLog& net_log) {
  int result = MockHostResolver::Resolve(info, priority, addresses, callback,
                                         out_req, net_log);
  if (!resolve_callback_.is_null()) {
    resolve_callback_.Run();
    resolve_callback_.Reset();
  }
  return result;
}

}  // namespace

class MojoHostResolverImplTest : public testing::Test {
 protected:
  void SetUp() override {
    mock_host_resolver_.rules()->AddRule("example.com", "1.2.3.4");
    mock_host_resolver_.rules()->AddRule("chromium.org", "8.8.8.8");
    mock_host_resolver_.rules()->AddSimulatedFailure("failure.fail");

    resolver_service_.reset(
        new MojoHostResolverImpl(&mock_host_resolver_, BoundNetLog()));
  }

  interfaces::HostResolverRequestInfoPtr CreateRequest(const std::string& host,
                                                       uint16_t port,
                                                       bool is_my_ip_address) {
    interfaces::HostResolverRequestInfoPtr request =
        interfaces::HostResolverRequestInfo::New();
    request->host = host;
    request->port = port;
    request->address_family = interfaces::ADDRESS_FAMILY_IPV4;
    request->is_my_ip_address = is_my_ip_address;
    return request.Pass();
  }

  // Wait until the mock resolver has received |num| resolve requests.
  void WaitForRequests(size_t num) {
    while (mock_host_resolver_.num_resolve() < num) {
      base::RunLoop run_loop;
      mock_host_resolver_.SetResolveCallback(run_loop.QuitClosure());
      run_loop.Run();
    }
  }

  CallbackMockHostResolver mock_host_resolver_;
  scoped_ptr<MojoHostResolverImpl> resolver_service_;
};

TEST_F(MojoHostResolverImplTest, Resolve) {
  interfaces::HostResolverRequestClientPtr client_ptr;
  TestRequestClient client(mojo::GetProxy(&client_ptr));

  interfaces::HostResolverRequestInfoPtr request =
      CreateRequest("example.com", 80, false);
  resolver_service_->Resolve(request.Pass(), client_ptr.Pass());
  client.WaitForResult();

  EXPECT_EQ(net::OK, client.error_);
  AddressList address_list = (*client.results_).To<AddressList>();
  EXPECT_EQ(1U, address_list.size());
  EXPECT_EQ("1.2.3.4:80", address_list[0].ToString());
}

TEST_F(MojoHostResolverImplTest, ResolveSynchronous) {
  interfaces::HostResolverRequestClientPtr client_ptr;
  TestRequestClient client(mojo::GetProxy(&client_ptr));

  mock_host_resolver_.set_synchronous_mode(true);

  interfaces::HostResolverRequestInfoPtr request =
      CreateRequest("example.com", 80, false);
  resolver_service_->Resolve(request.Pass(), client_ptr.Pass());
  client.WaitForResult();

  EXPECT_EQ(net::OK, client.error_);
  AddressList address_list = (*client.results_).To<AddressList>();
  EXPECT_EQ(1U, address_list.size());
  EXPECT_EQ("1.2.3.4:80", address_list[0].ToString());
}

TEST_F(MojoHostResolverImplTest, ResolveMultiple) {
  interfaces::HostResolverRequestClientPtr client1_ptr;
  TestRequestClient client1(mojo::GetProxy(&client1_ptr));
  interfaces::HostResolverRequestClientPtr client2_ptr;
  TestRequestClient client2(mojo::GetProxy(&client2_ptr));

  mock_host_resolver_.set_ondemand_mode(true);

  interfaces::HostResolverRequestInfoPtr request1 =
      CreateRequest("example.com", 80, false);
  resolver_service_->Resolve(request1.Pass(), client1_ptr.Pass());
  interfaces::HostResolverRequestInfoPtr request2 =
      CreateRequest("chromium.org", 80, false);
  resolver_service_->Resolve(request2.Pass(), client2_ptr.Pass());
  WaitForRequests(2);
  mock_host_resolver_.ResolveAllPending();

  client1.WaitForResult();
  client2.WaitForResult();

  EXPECT_EQ(net::OK, client1.error_);
  AddressList address_list = (*client1.results_).To<AddressList>();
  EXPECT_EQ(1U, address_list.size());
  EXPECT_EQ("1.2.3.4:80", address_list[0].ToString());
  EXPECT_EQ(net::OK, client2.error_);
  address_list = (*client2.results_).To<AddressList>();
  EXPECT_EQ(1U, address_list.size());
  EXPECT_EQ("8.8.8.8:80", address_list[0].ToString());
}

TEST_F(MojoHostResolverImplTest, ResolveDuplicate) {
  interfaces::HostResolverRequestClientPtr client1_ptr;
  TestRequestClient client1(mojo::GetProxy(&client1_ptr));
  interfaces::HostResolverRequestClientPtr client2_ptr;
  TestRequestClient client2(mojo::GetProxy(&client2_ptr));

  mock_host_resolver_.set_ondemand_mode(true);

  interfaces::HostResolverRequestInfoPtr request1 =
      CreateRequest("example.com", 80, false);
  resolver_service_->Resolve(request1.Pass(), client1_ptr.Pass());
  interfaces::HostResolverRequestInfoPtr request2 =
      CreateRequest("example.com", 80, false);
  resolver_service_->Resolve(request2.Pass(), client2_ptr.Pass());
  WaitForRequests(2);
  mock_host_resolver_.ResolveAllPending();

  client1.WaitForResult();
  client2.WaitForResult();

  EXPECT_EQ(net::OK, client1.error_);
  AddressList address_list = (*client1.results_).To<AddressList>();
  EXPECT_EQ(1U, address_list.size());
  EXPECT_EQ("1.2.3.4:80", address_list[0].ToString());
  EXPECT_EQ(net::OK, client2.error_);
  address_list = (*client2.results_).To<AddressList>();
  EXPECT_EQ(1U, address_list.size());
  EXPECT_EQ("1.2.3.4:80", address_list[0].ToString());
}

TEST_F(MojoHostResolverImplTest, ResolveFailure) {
  interfaces::HostResolverRequestClientPtr client_ptr;
  TestRequestClient client(mojo::GetProxy(&client_ptr));

  interfaces::HostResolverRequestInfoPtr request =
      CreateRequest("failure.fail", 80, false);
  resolver_service_->Resolve(request.Pass(), client_ptr.Pass());
  client.WaitForResult();

  EXPECT_EQ(net::ERR_NAME_NOT_RESOLVED, client.error_);
  EXPECT_TRUE(client.results_.is_null());
}

TEST_F(MojoHostResolverImplTest, DestroyClient) {
  interfaces::HostResolverRequestClientPtr client_ptr;
  scoped_ptr<TestRequestClient> client(
      new TestRequestClient(mojo::GetProxy(&client_ptr)));

  mock_host_resolver_.set_ondemand_mode(true);

  interfaces::HostResolverRequestInfoPtr request =
      CreateRequest("example.com", 80, false);
  resolver_service_->Resolve(request.Pass(), client_ptr.Pass());
  WaitForRequests(1);

  client.reset();
  base::RunLoop().RunUntilIdle();

  mock_host_resolver_.ResolveAllPending();
  base::RunLoop().RunUntilIdle();
}

}  // namespace net
