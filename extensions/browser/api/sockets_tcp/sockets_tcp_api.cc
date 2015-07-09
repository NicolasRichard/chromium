// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/sockets_tcp/sockets_tcp_api.h"

#include "content/public/browser/browser_context.h"
#include "content/public/common/socket_permission_request.h"
#include "extensions/browser/api/socket/tcp_socket.h"
#include "extensions/browser/api/socket/tls_socket.h"
#include "extensions/browser/api/sockets_tcp/tcp_socket_event_dispatcher.h"
#include "extensions/common/api/sockets/sockets_manifest_data.h"
#include "net/base/net_errors.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

using extensions::ResumableTCPSocket;
using extensions::core_api::sockets_tcp::SocketInfo;
using extensions::core_api::sockets_tcp::SocketProperties;

namespace {

const char kSocketNotFoundError[] = "Socket not found";
const char kPermissionError[] = "Does not have permission";
const char kInvalidSocketStateError[] =
    "Socket must be a connected client TCP socket.";
const char kSocketNotConnectedError[] = "Socket not connected";

linked_ptr<SocketInfo> CreateSocketInfo(int socket_id,
                                        ResumableTCPSocket* socket) {
  linked_ptr<SocketInfo> socket_info(new SocketInfo());
  // This represents what we know about the socket, and does not call through
  // to the system.
  socket_info->socket_id = socket_id;
  if (!socket->name().empty()) {
    socket_info->name.reset(new std::string(socket->name()));
  }
  socket_info->persistent = socket->persistent();
  if (socket->buffer_size() > 0) {
    socket_info->buffer_size.reset(new int(socket->buffer_size()));
  }
  socket_info->paused = socket->paused();
  socket_info->connected = socket->IsConnected();

  // Grab the local address as known by the OS.
  net::IPEndPoint localAddress;
  if (socket->GetLocalAddress(&localAddress)) {
    socket_info->local_address.reset(
        new std::string(localAddress.ToStringWithoutPort()));
    socket_info->local_port.reset(new int(localAddress.port()));
  }

  // Grab the peer address as known by the OS. This and the call below will
  // always succeed while the socket is connected, even if the socket has
  // been remotely closed by the peer; only reading the socket will reveal
  // that it should be closed locally.
  net::IPEndPoint peerAddress;
  if (socket->GetPeerAddress(&peerAddress)) {
    socket_info->peer_address.reset(
        new std::string(peerAddress.ToStringWithoutPort()));
    socket_info->peer_port.reset(new int(peerAddress.port()));
  }

  return socket_info;
}

void SetSocketProperties(ResumableTCPSocket* socket,
                         SocketProperties* properties) {
  if (properties->name.get()) {
    socket->set_name(*properties->name.get());
  }
  if (properties->persistent.get()) {
    socket->set_persistent(*properties->persistent.get());
  }
  if (properties->buffer_size.get()) {
    // buffer size is validated when issuing the actual Recv operation
    // on the socket.
    socket->set_buffer_size(*properties->buffer_size.get());
  }
}

}  // namespace

namespace extensions {
namespace core_api {

using content::SocketPermissionRequest;

TCPSocketAsyncApiFunction::~TCPSocketAsyncApiFunction() {}

scoped_ptr<SocketResourceManagerInterface>
TCPSocketAsyncApiFunction::CreateSocketResourceManager() {
  return scoped_ptr<SocketResourceManagerInterface>(
             new SocketResourceManager<ResumableTCPSocket>()).Pass();
}

ResumableTCPSocket* TCPSocketAsyncApiFunction::GetTcpSocket(int socket_id) {
  return static_cast<ResumableTCPSocket*>(GetSocket(socket_id));
}

TCPSocketExtensionWithDnsLookupFunction::
    ~TCPSocketExtensionWithDnsLookupFunction() {}

scoped_ptr<SocketResourceManagerInterface>
TCPSocketExtensionWithDnsLookupFunction::CreateSocketResourceManager() {
  return scoped_ptr<SocketResourceManagerInterface>(
             new SocketResourceManager<ResumableTCPSocket>()).Pass();
}

ResumableTCPSocket* TCPSocketExtensionWithDnsLookupFunction::GetTcpSocket(
    int socket_id) {
  return static_cast<ResumableTCPSocket*>(GetSocket(socket_id));
}

SocketsTcpCreateFunction::SocketsTcpCreateFunction() {}

SocketsTcpCreateFunction::~SocketsTcpCreateFunction() {}

bool SocketsTcpCreateFunction::Prepare() {
  params_ = sockets_tcp::Create::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsTcpCreateFunction::Work() {
  ResumableTCPSocket* socket = new ResumableTCPSocket(extension_->id());

  sockets_tcp::SocketProperties* properties = params_.get()->properties.get();
  if (properties) {
    SetSocketProperties(socket, properties);
  }

  sockets_tcp::CreateInfo create_info;
  create_info.socket_id = AddSocket(socket);
  results_ = sockets_tcp::Create::Results::Create(create_info);
}

SocketsTcpUpdateFunction::SocketsTcpUpdateFunction() {}

SocketsTcpUpdateFunction::~SocketsTcpUpdateFunction() {}

bool SocketsTcpUpdateFunction::Prepare() {
  params_ = sockets_tcp::Update::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsTcpUpdateFunction::Work() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  SetSocketProperties(socket, &params_.get()->properties);
  results_ = sockets_tcp::Update::Results::Create();
}

SocketsTcpSetPausedFunction::SocketsTcpSetPausedFunction()
    : socket_event_dispatcher_(NULL) {}

SocketsTcpSetPausedFunction::~SocketsTcpSetPausedFunction() {}

bool SocketsTcpSetPausedFunction::Prepare() {
  params_ = core_api::sockets_tcp::SetPaused::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());

  socket_event_dispatcher_ = TCPSocketEventDispatcher::Get(browser_context());
  DCHECK(socket_event_dispatcher_)
      << "There is no socket event dispatcher. "
         "If this assertion is failing during a test, then it is likely that "
         "TestExtensionSystem is failing to provide an instance of "
         "TCPSocketEventDispatcher.";
  return socket_event_dispatcher_ != NULL;
}

void SocketsTcpSetPausedFunction::Work() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  if (socket->paused() != params_->paused) {
    socket->set_paused(params_->paused);
    if (socket->IsConnected() && !params_->paused) {
      socket_event_dispatcher_->OnSocketResume(extension_->id(),
                                               params_->socket_id);
    }
  }

  results_ = sockets_tcp::SetPaused::Results::Create();
}

SocketsTcpSetKeepAliveFunction::SocketsTcpSetKeepAliveFunction() {}

SocketsTcpSetKeepAliveFunction::~SocketsTcpSetKeepAliveFunction() {}

bool SocketsTcpSetKeepAliveFunction::Prepare() {
  params_ = core_api::sockets_tcp::SetKeepAlive::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsTcpSetKeepAliveFunction::Work() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  int delay = params_->delay ? *params_->delay.get() : 0;

  bool success = socket->SetKeepAlive(params_->enable, delay);
  int net_result = (success ? net::OK : net::ERR_FAILED);
  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_tcp::SetKeepAlive::Results::Create(net_result);
}

SocketsTcpSetNoDelayFunction::SocketsTcpSetNoDelayFunction() {}

SocketsTcpSetNoDelayFunction::~SocketsTcpSetNoDelayFunction() {}

bool SocketsTcpSetNoDelayFunction::Prepare() {
  params_ = core_api::sockets_tcp::SetNoDelay::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsTcpSetNoDelayFunction::Work() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  bool success = socket->SetNoDelay(params_->no_delay);
  int net_result = (success ? net::OK : net::ERR_FAILED);
  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_tcp::SetNoDelay::Results::Create(net_result);
}

SocketsTcpConnectFunction::SocketsTcpConnectFunction()
    : socket_event_dispatcher_(NULL) {}

SocketsTcpConnectFunction::~SocketsTcpConnectFunction() {}

bool SocketsTcpConnectFunction::Prepare() {
  params_ = sockets_tcp::Connect::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());

  socket_event_dispatcher_ = TCPSocketEventDispatcher::Get(browser_context());
  DCHECK(socket_event_dispatcher_)
      << "There is no socket event dispatcher. "
         "If this assertion is failing during a test, then it is likely that "
         "TestExtensionSystem is failing to provide an instance of "
         "TCPSocketEventDispatcher.";
  return socket_event_dispatcher_ != NULL;
}

void SocketsTcpConnectFunction::AsyncWorkStart() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    AsyncWorkCompleted();
    return;
  }

  socket->set_hostname(params_->peer_address);

  content::SocketPermissionRequest param(SocketPermissionRequest::TCP_CONNECT,
                                         params_->peer_address,
                                         params_->peer_port);
  if (!SocketsManifestData::CheckRequest(extension(), param)) {
    error_ = kPermissionError;
    AsyncWorkCompleted();
    return;
  }

  StartDnsLookup(net::HostPortPair(params_->peer_address, params_->peer_port));
}

void SocketsTcpConnectFunction::AfterDnsLookup(int lookup_result) {
  if (lookup_result == net::OK) {
    StartConnect();
  } else {
    OnCompleted(lookup_result);
  }
}

void SocketsTcpConnectFunction::StartConnect() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    AsyncWorkCompleted();
    return;
  }

  socket->Connect(addresses_,
                  base::Bind(&SocketsTcpConnectFunction::OnCompleted, this));
}

void SocketsTcpConnectFunction::OnCompleted(int net_result) {
  if (net_result == net::OK) {
    socket_event_dispatcher_->OnSocketConnect(extension_->id(),
                                              params_->socket_id);
  }

  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_tcp::Connect::Results::Create(net_result);
  AsyncWorkCompleted();
}

SocketsTcpDisconnectFunction::SocketsTcpDisconnectFunction() {}

SocketsTcpDisconnectFunction::~SocketsTcpDisconnectFunction() {}

bool SocketsTcpDisconnectFunction::Prepare() {
  params_ = sockets_tcp::Disconnect::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsTcpDisconnectFunction::Work() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  socket->Disconnect();
  results_ = sockets_tcp::Disconnect::Results::Create();
}

SocketsTcpSendFunction::SocketsTcpSendFunction() : io_buffer_size_(0) {}

SocketsTcpSendFunction::~SocketsTcpSendFunction() {}

bool SocketsTcpSendFunction::Prepare() {
  params_ = sockets_tcp::Send::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  io_buffer_size_ = params_->data.size();
  io_buffer_ = new net::WrappedIOBuffer(params_->data.data());
  return true;
}

void SocketsTcpSendFunction::AsyncWorkStart() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    AsyncWorkCompleted();
    return;
  }

  socket->Write(io_buffer_,
                io_buffer_size_,
                base::Bind(&SocketsTcpSendFunction::OnCompleted, this));
}

void SocketsTcpSendFunction::OnCompleted(int net_result) {
  if (net_result >= net::OK) {
    SetSendResult(net::OK, net_result);
  } else {
    SetSendResult(net_result, -1);
  }
}

void SocketsTcpSendFunction::SetSendResult(int net_result, int bytes_sent) {
  CHECK(net_result <= net::OK) << "Network status code must be <= net::OK";

  sockets_tcp::SendInfo send_info;
  send_info.result_code = net_result;
  if (net_result == net::OK) {
    send_info.bytes_sent.reset(new int(bytes_sent));
  }

  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_tcp::Send::Results::Create(send_info);
  AsyncWorkCompleted();
}

SocketsTcpCloseFunction::SocketsTcpCloseFunction() {}

SocketsTcpCloseFunction::~SocketsTcpCloseFunction() {}

bool SocketsTcpCloseFunction::Prepare() {
  params_ = sockets_tcp::Close::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsTcpCloseFunction::Work() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  RemoveSocket(params_->socket_id);
  results_ = sockets_tcp::Close::Results::Create();
}

SocketsTcpGetInfoFunction::SocketsTcpGetInfoFunction() {}

SocketsTcpGetInfoFunction::~SocketsTcpGetInfoFunction() {}

bool SocketsTcpGetInfoFunction::Prepare() {
  params_ = sockets_tcp::GetInfo::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsTcpGetInfoFunction::Work() {
  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  linked_ptr<sockets_tcp::SocketInfo> socket_info =
      CreateSocketInfo(params_->socket_id, socket);
  results_ = sockets_tcp::GetInfo::Results::Create(*socket_info);
}

SocketsTcpGetSocketsFunction::SocketsTcpGetSocketsFunction() {}

SocketsTcpGetSocketsFunction::~SocketsTcpGetSocketsFunction() {}

bool SocketsTcpGetSocketsFunction::Prepare() { return true; }

void SocketsTcpGetSocketsFunction::Work() {
  std::vector<linked_ptr<sockets_tcp::SocketInfo> > socket_infos;
  base::hash_set<int>* resource_ids = GetSocketIds();
  if (resource_ids != NULL) {
    for (base::hash_set<int>::iterator it = resource_ids->begin();
         it != resource_ids->end();
         ++it) {
      int socket_id = *it;
      ResumableTCPSocket* socket = GetTcpSocket(socket_id);
      if (socket) {
        socket_infos.push_back(CreateSocketInfo(socket_id, socket));
      }
    }
  }
  results_ = sockets_tcp::GetSockets::Results::Create(socket_infos);
}

SocketsTcpSecureFunction::SocketsTcpSecureFunction() {
}

SocketsTcpSecureFunction::~SocketsTcpSecureFunction() {
}

bool SocketsTcpSecureFunction::Prepare() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  params_ = core_api::sockets_tcp::Secure::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  url_request_getter_ = browser_context()->GetRequestContext();
  return true;
}

// Override the regular implementation, which would call AsyncWorkCompleted
// immediately after Work().
void SocketsTcpSecureFunction::AsyncWorkStart() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  ResumableTCPSocket* socket = GetTcpSocket(params_->socket_id);
  if (!socket) {
    SetResult(new base::FundamentalValue(net::ERR_INVALID_ARGUMENT));
    error_ = kSocketNotFoundError;
    AsyncWorkCompleted();
    return;
  }

  paused_ = socket->paused();
  persistent_ = socket->persistent();

  // Make sure it's a connected TCP client socket. Error out if it's already
  // secure()'d.
  if (socket->GetSocketType() != Socket::TYPE_TCP ||
      socket->ClientStream() == NULL) {
    SetResult(new base::FundamentalValue(net::ERR_INVALID_ARGUMENT));
    error_ = kInvalidSocketStateError;
    AsyncWorkCompleted();
    return;
  }

  if (!socket->IsConnected()) {
    SetResult(new base::FundamentalValue(net::ERR_INVALID_ARGUMENT));
    error_ = kSocketNotConnectedError;
    AsyncWorkCompleted();
    return;
  }

  net::URLRequestContext* url_request_context =
      url_request_getter_->GetURLRequestContext();

  // UpgradeSocketToTLS() uses the older API's SecureOptions. Copy over the
  // only values inside -- TLSVersionConstraints's |min| and |max|,
  core_api::socket::SecureOptions legacy_params;
  if (params_->options.get() && params_->options->tls_version.get()) {
    legacy_params.tls_version.reset(
        new core_api::socket::TLSVersionConstraints);
    if (params_->options->tls_version->min.get()) {
      legacy_params.tls_version->min.reset(
          new std::string(*params_->options->tls_version->min.get()));
    }
    if (params_->options->tls_version->max.get()) {
      legacy_params.tls_version->max.reset(
          new std::string(*params_->options->tls_version->max.get()));
    }
  }

  TLSSocket::UpgradeSocketToTLS(
      socket,
      url_request_context->ssl_config_service(),
      url_request_context->cert_verifier(),
      url_request_context->transport_security_state(),
      extension_id(),
      &legacy_params,
      base::Bind(&SocketsTcpSecureFunction::TlsConnectDone, this));
}

void SocketsTcpSecureFunction::TlsConnectDone(scoped_ptr<TLSSocket> socket,
                                              int result) {
  // If an error occurred, socket MUST be NULL
  DCHECK(result == net::OK || socket == NULL);

  if (socket && result == net::OK) {
    socket->set_persistent(persistent_);
    socket->set_paused(paused_);
    ReplaceSocket(params_->socket_id, socket.release());
  } else {
    RemoveSocket(params_->socket_id);
    error_ = net::ErrorToString(result);
  }

  results_ = core_api::sockets_tcp::Secure::Results::Create(result);
  AsyncWorkCompleted();
}

}  // namespace api
}  // namespace extensions
