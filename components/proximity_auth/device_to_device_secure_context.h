// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PROXIMITY_AUTH_DEVICE_TO_DEVICE_SECURE_CONTEXT_H
#define COMPONENTS_PROXIMITY_AUTH_DEVICE_TO_DEVICE_SECURE_CONTEXT_H

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/proximity_auth/secure_context.h"

namespace securemessage {
class Header;
}

namespace proximity_auth {

class SecureMessageDelegate;

// SecureContext implementation for the DeviceToDevice protocol.
class DeviceToDeviceSecureContext : public SecureContext {
 public:
  DeviceToDeviceSecureContext(
      scoped_ptr<SecureMessageDelegate> secure_message_delegate,
      const std::string& symmetric_key,
      const std::string& responder_auth_message_,
      ProtocolVersion protocol_version);

  ~DeviceToDeviceSecureContext() override;

  // SecureContext:
  void Decode(const std::string& encoded_message,
              const MessageCallback& callback) override;
  void Encode(const std::string& message,
              const MessageCallback& callback) override;
  ProtocolVersion GetProtocolVersion() const override;

  // Returns the message received from the remote device that authenticates it.
  // This message should have been received during the handshake that
  // establishes the secure channel.
  std::string GetReceivedAuthMessage() const;

 private:
  // Callback for unwrapping a secure message. |callback| will be invoked with
  // the decrypted payload if the message is unwrapped successfully; otherwise
  // it will be invoked with an empty string.
  void HandleUnwrapResult(
      const DeviceToDeviceSecureContext::MessageCallback& callback,
      bool verified,
      const std::string& payload,
      const securemessage::Header& header);

  // Delegate for handling the creation and unwrapping of SecureMessages.
  scoped_ptr<SecureMessageDelegate> secure_message_delegate_;

  // The symmetric key used to create and unwrap messages.
  const std::string symmetric_key_;

  // The [Responder Auth] message received from the remote device during
  // authentication.
  const std::string responder_auth_message_;

  // The protocol version supported by the remote device.
  const ProtocolVersion protocol_version_;

  // The last sequence number of the message sent or received.
  int last_sequence_number_;

  base::WeakPtrFactory<DeviceToDeviceSecureContext> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(DeviceToDeviceSecureContext);
};

}  // namespace proximity_auth

#endif  // COMPONENTS_PROXIMITY_AUTH_DEVICE_TO_DEVICE_SECURE_CONTEXT_H
