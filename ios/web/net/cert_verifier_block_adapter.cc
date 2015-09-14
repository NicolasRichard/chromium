// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/net/cert_verifier_block_adapter.h"

#include "base/mac/bind_objc_block.h"
#include "net/base/net_errors.h"
#include "net/cert/crl_set.h"
#include "net/cert/x509_certificate.h"
#include "net/log/net_log.h"

namespace web {

namespace {

// Resource manager which keeps CertVerifyResult, X509Certificate and
// BoundNetLog alive until verification is completed. Also holds unowned pointer
// to |net::CertVerifier::Request|.
struct VerificationContext
    : public base::RefCountedThreadSafe<VerificationContext> {
  VerificationContext(scoped_refptr<net::X509Certificate> cert,
                      net::NetLog* net_log)
      : request(nullptr),
        cert(cert.Pass()),
        net_log(net::BoundNetLog::Make(
            net_log,
            net::NetLog::SOURCE_IOS_WEB_VIEW_CERT_VERIFIER)) {}
  // Unowned verification request.
  net::CertVerifier::Request* request;
  // The result of certificate verification.
  net::CertVerifyResult result;
  // Certificate being verified.
  scoped_refptr<net::X509Certificate> cert;
  // BoundNetLog required by CertVerifier.
  net::BoundNetLog net_log;

 private:
  friend class base::RefCountedThreadSafe<VerificationContext>;
  VerificationContext() = delete;
  ~VerificationContext() {}
};
}

CertVerifierBlockAdapter::CertVerifierBlockAdapter(
    net::CertVerifier* cert_verifier,
    net::NetLog* net_log)
    : cert_verifier_(cert_verifier), net_log_(net_log) {
  DCHECK(cert_verifier_);
}

CertVerifierBlockAdapter::~CertVerifierBlockAdapter() {
  DCHECK(thread_checker_.CalledOnValidThread());
}

CertVerifierBlockAdapter::Params::Params(
    const scoped_refptr<net::X509Certificate>& cert,
    const std::string& hostname)
    : cert(cert), hostname(hostname), flags(0) {}

CertVerifierBlockAdapter::Params::~Params() {
}

void CertVerifierBlockAdapter::Verify(
    const Params& params,
    void (^completion_handler)(net::CertVerifyResult, int)) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(completion_handler);
  if (!params.cert || params.hostname.empty()) {
    completion_handler(net::CertVerifyResult(), net::ERR_INVALID_ARGUMENT);
    return;
  }

  scoped_refptr<VerificationContext> context(
      new VerificationContext(params.cert, net_log_));
  net::CompletionCallback callback = base::BindBlock(^(int error) {
    // Remove pending request.
    auto request_iterator = std::find(
        pending_requests_.begin(), pending_requests_.end(), context->request);
    DCHECK(pending_requests_.end() != request_iterator);
    pending_requests_.erase(request_iterator);

    completion_handler(context->result, error);
  });
  scoped_ptr<net::CertVerifier::Request> request;
  int error = cert_verifier_->Verify(params.cert.get(), params.hostname,
                                     params.ocsp_response, params.flags,
                                     params.crl_set.get(), &(context->result),
                                     callback, &request, context->net_log);
  if (error == net::ERR_IO_PENDING) {
    // Make sure that |net::CertVerifier::Request| is alive until either
    // verification is completed or CertVerifierBlockAdapter is destroyed.
    pending_requests_.push_back(request.Pass());
    context->request = pending_requests_.back();
    // Completion handler will be called from |callback| when verification
    // request is completed.
    return;
  }

  // Verification has either failed or result was retrieved from the cache.
  completion_handler(context->result, error);
}

}  // namespace web
