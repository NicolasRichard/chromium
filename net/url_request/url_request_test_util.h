// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_URL_REQUEST_URL_REQUEST_TEST_UTIL_H_
#define NET_URL_REQUEST_URL_REQUEST_TEST_UTIL_H_

#include <stdlib.h>

#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/path_service.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "net/base/io_buffer.h"
#include "net/base/load_timing_info.h"
#include "net/base/net_errors.h"
#include "net/base/network_delegate_impl.h"
#include "net/base/request_priority.h"
#include "net/base/sdch_manager.h"
#include "net/cert/cert_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/disk_cache/disk_cache.h"
#include "net/ftp/ftp_network_layer.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_layer.h"
#include "net/http/http_network_session.h"
#include "net/http/http_request_headers.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_job_factory.h"
#include "url/url_util.h"

using base::TimeDelta;

namespace net {

//-----------------------------------------------------------------------------

class TestURLRequestContext : public URLRequestContext {
 public:
  TestURLRequestContext();
  // Default constructor like TestURLRequestContext() but does not call
  // Init() in case |delay_initialization| is true. This allows modifying the
  // URLRequestContext before it is constructed completely. If
  // |delay_initialization| is true, Init() needs be be called manually.
  explicit TestURLRequestContext(bool delay_initialization);
  ~TestURLRequestContext() override;

  void Init();

  ClientSocketFactory* client_socket_factory() {
    return client_socket_factory_;
  }
  void set_client_socket_factory(ClientSocketFactory* factory) {
    client_socket_factory_ = factory;
  }

  void set_http_network_session_params(
      scoped_ptr<HttpNetworkSession::Params> params) {
    http_network_session_params_ = params.Pass();
  }

  void SetSdchManager(scoped_ptr<SdchManager> sdch_manager) {
    context_storage_.set_sdch_manager(sdch_manager.Pass());
  }

 private:
  bool initialized_;

  // Optional parameters to override default values.  Note that values that
  // point to other objects the TestURLRequestContext creates will be
  // overwritten.
  scoped_ptr<HttpNetworkSession::Params> http_network_session_params_;

  // Not owned:
  ClientSocketFactory* client_socket_factory_;

 protected:
  URLRequestContextStorage context_storage_;
};

//-----------------------------------------------------------------------------

// Used to return a dummy context, which lives on the message loop
// given in the constructor.
class TestURLRequestContextGetter : public URLRequestContextGetter {
 public:
  // |network_task_runner| must not be NULL.
  explicit TestURLRequestContextGetter(
      const scoped_refptr<base::SingleThreadTaskRunner>& network_task_runner);

  // Use to pass a pre-initialized |context|.
  TestURLRequestContextGetter(
      const scoped_refptr<base::SingleThreadTaskRunner>& network_task_runner,
      scoped_ptr<TestURLRequestContext> context);

  // URLRequestContextGetter implementation.
  TestURLRequestContext* GetURLRequestContext() override;
  scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner()
      const override;

 protected:
  ~TestURLRequestContextGetter() override;

 private:
  const scoped_refptr<base::SingleThreadTaskRunner> network_task_runner_;
  scoped_ptr<TestURLRequestContext> context_;
};

//-----------------------------------------------------------------------------

class TestDelegate : public URLRequest::Delegate {
 public:
  TestDelegate();
  ~TestDelegate() override;

  void set_cancel_in_received_redirect(bool val) { cancel_in_rr_ = val; }
  void set_cancel_in_response_started(bool val) { cancel_in_rs_ = val; }
  void set_cancel_in_received_data(bool val) { cancel_in_rd_ = val; }
  void set_cancel_in_received_data_pending(bool val) {
    cancel_in_rd_pending_ = val;
  }
  void set_quit_on_complete(bool val) { quit_on_complete_ = val; }
  void set_quit_on_redirect(bool val) { quit_on_redirect_ = val; }
  void set_quit_on_network_start(bool val) {
    quit_on_before_network_start_ = val;
  }
  void set_allow_certificate_errors(bool val) {
    allow_certificate_errors_ = val;
  }
  void set_credentials(const AuthCredentials& credentials) {
    credentials_ = credentials;
  }

  // query state
  const std::string& data_received() const { return data_received_; }
  int bytes_received() const { return static_cast<int>(data_received_.size()); }
  int response_started_count() const { return response_started_count_; }
  int received_redirect_count() const { return received_redirect_count_; }
  int received_before_network_start_count() const {
    return received_before_network_start_count_;
  }
  bool received_data_before_response() const {
    return received_data_before_response_;
  }
  bool request_failed() const { return request_failed_; }
  bool have_certificate_errors() const { return have_certificate_errors_; }
  bool certificate_errors_are_fatal() const {
    return certificate_errors_are_fatal_;
  }
  bool auth_required_called() const { return auth_required_; }
  bool have_full_request_headers() const { return have_full_request_headers_; }
  const HttpRequestHeaders& full_request_headers() const {
    return full_request_headers_;
  }
  void ClearFullRequestHeaders();

  // URLRequest::Delegate:
  void OnReceivedRedirect(URLRequest* request,
                          const RedirectInfo& redirect_info,
                          bool* defer_redirect) override;
  void OnBeforeNetworkStart(URLRequest* request, bool* defer) override;
  void OnAuthRequired(URLRequest* request,
                      AuthChallengeInfo* auth_info) override;
  // NOTE: |fatal| causes |certificate_errors_are_fatal_| to be set to true.
  // (Unit tests use this as a post-condition.) But for policy, this method
  // consults |allow_certificate_errors_|.
  void OnSSLCertificateError(URLRequest* request,
                             const SSLInfo& ssl_info,
                             bool fatal) override;
  void OnResponseStarted(URLRequest* request) override;
  void OnReadCompleted(URLRequest* request, int bytes_read) override;

 private:
  static const int kBufferSize = 4096;

  virtual void OnResponseCompleted(URLRequest* request);

  // options for controlling behavior
  bool cancel_in_rr_;
  bool cancel_in_rs_;
  bool cancel_in_rd_;
  bool cancel_in_rd_pending_;
  bool quit_on_complete_;
  bool quit_on_redirect_;
  bool quit_on_before_network_start_;
  bool allow_certificate_errors_;
  AuthCredentials credentials_;

  // tracks status of callbacks
  int response_started_count_;
  int received_bytes_count_;
  int received_redirect_count_;
  int received_before_network_start_count_;
  bool received_data_before_response_;
  bool request_failed_;
  bool have_certificate_errors_;
  bool certificate_errors_are_fatal_;
  bool auth_required_;
  std::string data_received_;
  bool have_full_request_headers_;
  HttpRequestHeaders full_request_headers_;

  // our read buffer
  scoped_refptr<IOBuffer> buf_;
};

//-----------------------------------------------------------------------------

class TestNetworkDelegate : public NetworkDelegateImpl {
 public:
  enum Options {
    NO_GET_COOKIES = 1 << 0,
    NO_SET_COOKIE  = 1 << 1,
  };

  TestNetworkDelegate();
  ~TestNetworkDelegate() override;

  // Writes the LoadTimingInfo during the most recent call to OnBeforeRedirect.
  bool GetLoadTimingInfoBeforeRedirect(
      LoadTimingInfo* load_timing_info_before_redirect) const;

  // Same as GetLoadTimingInfoBeforeRedirect, except for calls to
  // AuthRequiredResponse.
  bool GetLoadTimingInfoBeforeAuth(
      LoadTimingInfo* load_timing_info_before_auth) const;

  // Will redirect once to the given URL when the next set of headers are
  // received.
  void set_redirect_on_headers_received_url(
      GURL redirect_on_headers_received_url) {
    redirect_on_headers_received_url_ = redirect_on_headers_received_url;
  }

  void set_allowed_unsafe_redirect_url(GURL allowed_unsafe_redirect_url) {
    allowed_unsafe_redirect_url_ = allowed_unsafe_redirect_url;
  }

  void set_cookie_options(int o) {cookie_options_bit_mask_ = o; }

  int last_error() const { return last_error_; }
  int error_count() const { return error_count_; }
  int created_requests() const { return created_requests_; }
  int destroyed_requests() const { return destroyed_requests_; }
  int completed_requests() const { return completed_requests_; }
  int canceled_requests() const { return canceled_requests_; }
  int blocked_get_cookies_count() const { return blocked_get_cookies_count_; }
  int blocked_set_cookie_count() const { return blocked_set_cookie_count_; }
  int set_cookie_count() const { return set_cookie_count_; }

  void set_can_access_files(bool val) { can_access_files_ = val; }
  bool can_access_files() const { return can_access_files_; }

  void set_first_party_only_cookies_enabled(bool val) {
    first_party_only_cookies_enabled_ = val;
  }

  void set_can_throttle_requests(bool val) { can_throttle_requests_ = val; }
  bool can_throttle_requests() const { return can_throttle_requests_; }

  void set_cancel_request_with_policy_violating_referrer(bool val) {
    cancel_request_with_policy_violating_referrer_ = val;
  }

  int observed_before_proxy_headers_sent_callbacks() const {
    return observed_before_proxy_headers_sent_callbacks_;
  }
  int before_send_headers_count() const { return before_send_headers_count_; }
  int headers_received_count() const { return headers_received_count_; }

  // Last observed proxy in proxy header sent callback.
  HostPortPair last_observed_proxy() {
    return last_observed_proxy_;
  }

  void set_can_be_intercepted_on_error(bool can_be_intercepted_on_error) {
    will_be_intercepted_on_next_error_ = can_be_intercepted_on_error;
  }

 protected:
  // NetworkDelegate:
  int OnBeforeURLRequest(URLRequest* request,
                         const CompletionCallback& callback,
                         GURL* new_url) override;
  int OnBeforeSendHeaders(URLRequest* request,
                          const CompletionCallback& callback,
                          HttpRequestHeaders* headers) override;
  void OnBeforeSendProxyHeaders(net::URLRequest* request,
                                const net::ProxyInfo& proxy_info,
                                net::HttpRequestHeaders* headers) override;
  void OnSendHeaders(URLRequest* request,
                     const HttpRequestHeaders& headers) override;
  int OnHeadersReceived(
      URLRequest* request,
      const CompletionCallback& callback,
      const HttpResponseHeaders* original_response_headers,
      scoped_refptr<HttpResponseHeaders>* override_response_headers,
      GURL* allowed_unsafe_redirect_url) override;
  void OnBeforeRedirect(URLRequest* request, const GURL& new_location) override;
  void OnResponseStarted(URLRequest* request) override;
  void OnRawBytesRead(const URLRequest& request, int bytes_read) override;
  void OnCompleted(URLRequest* request, bool started) override;
  void OnURLRequestDestroyed(URLRequest* request) override;
  void OnPACScriptError(int line_number, const base::string16& error) override;
  NetworkDelegate::AuthRequiredResponse OnAuthRequired(
      URLRequest* request,
      const AuthChallengeInfo& auth_info,
      const AuthCallback& callback,
      AuthCredentials* credentials) override;
  bool OnCanGetCookies(const URLRequest& request,
                       const CookieList& cookie_list) override;
  bool OnCanSetCookie(const URLRequest& request,
                      const std::string& cookie_line,
                      CookieOptions* options) override;
  bool OnCanAccessFile(const URLRequest& request,
                       const base::FilePath& path) const override;
  bool OnCanThrottleRequest(const URLRequest& request) const override;
  bool OnFirstPartyOnlyCookieExperimentEnabled() const override;
  bool OnCancelURLRequestWithPolicyViolatingReferrerHeader(
      const URLRequest& request,
      const GURL& target_url,
      const GURL& referrer_url) const override;

  void InitRequestStatesIfNew(int request_id);

  GURL redirect_on_headers_received_url_;
  // URL marked as safe for redirection at the onHeadersReceived stage.
  GURL allowed_unsafe_redirect_url_;

  int last_error_;
  int error_count_;
  int created_requests_;
  int destroyed_requests_;
  int completed_requests_;
  int canceled_requests_;
  int cookie_options_bit_mask_;
  int blocked_get_cookies_count_;
  int blocked_set_cookie_count_;
  int set_cookie_count_;
  int observed_before_proxy_headers_sent_callbacks_;
  int before_send_headers_count_;
  int headers_received_count_;
  // Last observed proxy in before proxy header sent callback.
  HostPortPair last_observed_proxy_;

  // NetworkDelegate callbacks happen in a particular order (e.g.
  // OnBeforeURLRequest is always called before OnBeforeSendHeaders).
  // This bit-set indicates for each request id (key) what events may be sent
  // next.
  std::map<int, int> next_states_;

  // A log that records for each request id (key) the order in which On...
  // functions were called.
  std::map<int, std::string> event_order_;

  LoadTimingInfo load_timing_info_before_redirect_;
  bool has_load_timing_info_before_redirect_;

  LoadTimingInfo load_timing_info_before_auth_;
  bool has_load_timing_info_before_auth_;

  bool can_access_files_;  // true by default
  bool can_throttle_requests_;  // true by default
  bool first_party_only_cookies_enabled_;               // false by default
  bool cancel_request_with_policy_violating_referrer_;  // false by default
  bool will_be_intercepted_on_next_error_;
};

// Overrides the host used by the LocalHttpTestServer in
// url_request_unittest.cc . This is used by the chrome_frame_net_tests due to
// a mysterious bug when tests execute over the loopback adapter. See
// http://crbug.com/114369 .
class ScopedCustomUrlRequestTestHttpHost {
 public:
  // Sets the host name to be used. The previous hostname will be stored and
  // restored upon destruction. Note that if the lifetimes of two or more
  // instances of this class overlap, they must be strictly nested.
  explicit ScopedCustomUrlRequestTestHttpHost(const std::string& new_value);

  ~ScopedCustomUrlRequestTestHttpHost();

  // Returns the current value to be used by HTTP tests in
  // url_request_unittest.cc .
  static const std::string& value();

 private:
  static std::string value_;
  const std::string old_value_;
  const std::string new_value_;

  DISALLOW_COPY_AND_ASSIGN(ScopedCustomUrlRequestTestHttpHost);
};

//-----------------------------------------------------------------------------

// A simple ProtocolHandler that returns a pre-built URLRequestJob only once.
class TestJobInterceptor : public URLRequestJobFactory::ProtocolHandler {
 public:
  TestJobInterceptor();

  URLRequestJob* MaybeCreateJob(
      URLRequest* request,
      NetworkDelegate* network_delegate) const override;
  void set_main_intercept_job(URLRequestJob* job);

 private:
  mutable URLRequestJob* main_intercept_job_;
};

}  // namespace net

#endif  // NET_URL_REQUEST_URL_REQUEST_TEST_UTIL_H_
