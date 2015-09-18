// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/prefs/command_line_pref_store.h"

#include <gtest/gtest.h>

#include "base/command_line.h"
#include "base/memory/ref_counted.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "chrome/browser/prefs/command_line_pref_store.h"
#include "chrome/common/chrome_switches.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "components/syncable_prefs/pref_service_mock_factory.h"
#include "net/proxy/proxy_config_service_common_unittest.h"
#include "url/gurl.h"

namespace {

// Test parameter object for testing command line proxy configuration.
struct CommandLineTestParams {
  // Short description to identify the test.
  const char* description;

  // The command line to build a ProxyConfig from.
  struct SwitchValue {
    const char* name;
    const char* value;
  } switches[2];

  // Expected outputs (fields of the ProxyConfig).
  bool is_null;
  bool auto_detect;
  GURL pac_url;
  net::ProxyRulesExpectation proxy_rules;
};

void PrintTo(const CommandLineTestParams& params, std::ostream* os) {
  *os << params.description;
}

static const CommandLineTestParams kCommandLineTestParams[] = {
    {
        "Empty command line",
        // Input
        {},
        // Expected result
        true,    // is_null
        false,   // auto_detect
        GURL(),  // pac_url
        net::ProxyRulesExpectation::Empty(),
    },
    {
        "No proxy",
        // Input
        {
            {switches::kNoProxyServer, NULL},
        },
        // Expected result
        false,   // is_null
        false,   // auto_detect
        GURL(),  // pac_url
        net::ProxyRulesExpectation::Empty(),
    },
    {
        "No proxy with extra parameters.",
        // Input
        {
            {switches::kNoProxyServer, NULL},
            {switches::kProxyServer, "http://proxy:8888"},
        },
        // Expected result
        false,   // is_null
        false,   // auto_detect
        GURL(),  // pac_url
        net::ProxyRulesExpectation::Empty(),
    },
    {
        "Single proxy.",
        // Input
        {
            {switches::kProxyServer, "http://proxy:8888"},
        },
        // Expected result
        false,                                            // is_null
        false,                                            // auto_detect
        GURL(),                                           // pac_url
        net::ProxyRulesExpectation::Single("proxy:8888",  // single proxy
                                           ""),           // bypass rules
    },
    {
        "Per scheme proxy.",
        // Input
        {
            {switches::kProxyServer, "http=httpproxy:8888;ftp=ftpproxy:8889"},
        },
        // Expected result
        false,                                                   // is_null
        false,                                                   // auto_detect
        GURL(),                                                  // pac_url
        net::ProxyRulesExpectation::PerScheme("httpproxy:8888",  // http
                                              "",                // https
                                              "ftpproxy:8889",   // ftp
                                              ""),               // bypass rules
    },
    {
        "Per scheme proxy with bypass URLs.",
        // Input
        {
            {switches::kProxyServer, "http=httpproxy:8888;ftp=ftpproxy:8889"},
            {switches::kProxyBypassList,
             ".google.com, foo.com:99, 1.2.3.4:22, 127.0.0.1/8"},
        },
        // Expected result
        false,   // is_null
        false,   // auto_detect
        GURL(),  // pac_url
        net::ProxyRulesExpectation::PerScheme(
            "httpproxy:8888",  // http
            "",                // https
            "ftpproxy:8889",   // ftp
            "*.google.com,foo.com:99,1.2.3.4:22,127.0.0.1/8"),
    },
    {
        "Pac URL",
        // Input
        {
            {switches::kProxyPacUrl, "http://wpad/wpad.dat"},
        },
        // Expected result
        false,                         // is_null
        false,                         // auto_detect
        GURL("http://wpad/wpad.dat"),  // pac_url
        net::ProxyRulesExpectation::Empty(),
    },
    {
        "Autodetect",
        // Input
        {
            {switches::kProxyAutoDetect, NULL},
        },
        // Expected result
        false,   // is_null
        true,    // auto_detect
        GURL(),  // pac_url
        net::ProxyRulesExpectation::Empty(),
    },
};

}  // namespace

class CommandLinePrefStoreProxyTest
    : public testing::TestWithParam<CommandLineTestParams> {
 protected:
  CommandLinePrefStoreProxyTest()
      : command_line_(base::CommandLine::NO_PROGRAM) {}

  net::ProxyConfig* proxy_config() { return &proxy_config_; }

  void SetUp() override {
    for (size_t i = 0; i < arraysize(GetParam().switches); i++) {
      const char* name = GetParam().switches[i].name;
      const char* value = GetParam().switches[i].value;
      if (name && value)
        command_line_.AppendSwitchASCII(name, value);
      else if (name)
        command_line_.AppendSwitch(name);
    }
    scoped_refptr<PrefRegistrySimple> registry = new PrefRegistrySimple;
    PrefProxyConfigTrackerImpl::RegisterPrefs(registry.get());
    syncable_prefs::PrefServiceMockFactory factory;
    factory.set_command_line_prefs(new CommandLinePrefStore(&command_line_));
    pref_service_ = factory.Create(registry.get()).Pass();
    PrefProxyConfigTrackerImpl::ReadPrefConfig(pref_service_.get(),
                                               &proxy_config_);
  }

 private:
  base::CommandLine command_line_;
  scoped_ptr<PrefService> pref_service_;
  net::ProxyConfig proxy_config_;
};

TEST_P(CommandLinePrefStoreProxyTest, CommandLine) {
  EXPECT_EQ(GetParam().auto_detect, proxy_config()->auto_detect());
  EXPECT_EQ(GetParam().pac_url, proxy_config()->pac_url());
  EXPECT_TRUE(GetParam().proxy_rules.Matches(proxy_config()->proxy_rules()));
}

INSTANTIATE_TEST_CASE_P(CommandLinePrefStoreProxyTestInstance,
                        CommandLinePrefStoreProxyTest,
                        testing::ValuesIn(kCommandLineTestParams));
