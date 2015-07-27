// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PROXIMITY_AUTH_SECURE_CONTEXT_H
#define COMPONENTS_PROXIMITY_AUTH_SECURE_CONTEXT_H

#include "base/callback_forward.h"

namespace proximity_auth {

// An interface used to decode and encode messages.
class SecureContext {
 public:
  typedef base::Callback<void(const std::string& message)> MessageCallback;

  // The protocol version used during authentication.
  enum ProtocolVersion {
    PROTOCOL_VERSION_THREE_ZERO,  // 3.0
    PROTOCOL_VERSION_THREE_ONE,   // 3.1
  };

  virtual ~SecureContext() {}

  // Decodes the |encoded_message| and returns the result.
  // This function is asynchronous because the ChromeOS implementation requires
  // a DBus call.
  virtual void Decode(const std::string& encoded_message,
                      const MessageCallback& callback) = 0;

  // Encodes the |message| and returns the result.
  // This function is asynchronous because the ChromeOS implementation requires
  // a DBus call.
  virtual void Encode(const std::string& message,
                      const MessageCallback& callback) = 0;

  // Returns the protocol version that was used during authentication.
  virtual ProtocolVersion GetProtocolVersion() const = 0;
};

}  // namespace proximity_auth

#endif  // COMPONENTS_PROXIMITY_AUTH_SECURE_CONTEXT_H
