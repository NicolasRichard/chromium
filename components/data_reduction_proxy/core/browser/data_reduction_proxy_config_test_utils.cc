// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/data_reduction_proxy/core/browser/data_reduction_proxy_config_test_utils.h"

#include "components/data_reduction_proxy/core/common/data_reduction_proxy_params_test_utils.h"
#include "net/base/net_util.h"
#include "net/url_request/test_url_fetcher_factory.h"
#include "net/url_request/url_request_test_util.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace data_reduction_proxy {

TestDataReductionProxyConfig::TestDataReductionProxyConfig()
    : TestDataReductionProxyConfig(DataReductionProxyParams::kAllowed |
                                   DataReductionProxyParams::kFallbackAllowed |
                                   DataReductionProxyParams::kPromoAllowed) {
}

TestDataReductionProxyConfig::TestDataReductionProxyConfig(int flags)
    : DataReductionProxyConfig(
          scoped_ptr<TestDataReductionProxyParams>(
              new TestDataReductionProxyParams(
                  flags,
                  TestDataReductionProxyParams::HAS_EVERYTHING &
                      ~TestDataReductionProxyParams::HAS_DEV_ORIGIN &
                      ~TestDataReductionProxyParams::HAS_DEV_FALLBACK_ORIGIN))
              .Pass()) {
  network_interfaces_.reset(new net::NetworkInterfaceList());
}

TestDataReductionProxyConfig::~TestDataReductionProxyConfig() {
}

void TestDataReductionProxyConfig::GetNetworkList(
    net::NetworkInterfaceList* interfaces,
    int policy) {
  for (size_t i = 0; i < network_interfaces_->size(); ++i)
    interfaces->push_back(network_interfaces_->at(i));
}

void TestDataReductionProxyConfig::ResetParamFlagsForTest(int flags) {
  params_ = make_scoped_ptr(
                new TestDataReductionProxyParams(
                    flags,
                    TestDataReductionProxyParams::HAS_EVERYTHING &
                        ~TestDataReductionProxyParams::HAS_DEV_ORIGIN &
                        ~TestDataReductionProxyParams::HAS_DEV_FALLBACK_ORIGIN))
                .Pass();
}

TestDataReductionProxyParams* TestDataReductionProxyConfig::test_params() {
  return static_cast<TestDataReductionProxyParams*>(params_.get());
}

void TestDataReductionProxyConfig::SetStateForTest(
    bool enabled_by_user,
    bool alternative_enabled_by_user,
    bool restricted_by_carrier,
    bool at_startup) {
  enabled_by_user_ = enabled_by_user;
  alternative_enabled_by_user_ = alternative_enabled_by_user;
  restricted_by_carrier_ = restricted_by_carrier;
  SetProxyConfigs(enabled_by_user_, alternative_enabled_by_user_,
                  restricted_by_carrier_, at_startup);
}

MockDataReductionProxyConfig::MockDataReductionProxyConfig(int flags)
    : TestDataReductionProxyConfig(flags) {
}

MockDataReductionProxyConfig::~MockDataReductionProxyConfig() {
}

void MockDataReductionProxyConfig::SetProxyConfigs(bool enabled,
                                                   bool alternative_enabled,
                                                   bool restricted,
                                                   bool at_startup) {
  EXPECT_CALL(*this, LogProxyState(enabled, restricted, at_startup)).Times(1);
  DataReductionProxyConfig::SetProxyConfigs(enabled, alternative_enabled,
                                            restricted, at_startup);
}

}  // namespace data_reduction_proxy
