// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/android/java/gin_java_bridge_message_filter.h"

#include "base/auto_reset.h"
#include "content/browser/android/java/gin_java_bridge_dispatcher_host.h"
#include "content/browser/android/java/java_bridge_thread.h"
#include "content/common/gin_java_bridge_messages.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"

#if !defined(OS_ANDROID)
#error "JavaBridge only supports OS_ANDROID"
#endif

namespace {

const char kGinJavaBridgeMessageFilterKey[] = "GinJavaBridgeMessageFilter";

}  // namespace

namespace content {

GinJavaBridgeMessageFilter::GinJavaBridgeMessageFilter()
    : BrowserMessageFilter(GinJavaBridgeMsgStart),
      current_routing_id_(MSG_ROUTING_NONE) {
}

GinJavaBridgeMessageFilter::~GinJavaBridgeMessageFilter() {
}

void GinJavaBridgeMessageFilter::OnDestruct() const {
  if (BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    delete this;
  } else {
    BrowserThread::DeleteSoon(BrowserThread::UI, FROM_HERE, this);
  }
}

bool GinJavaBridgeMessageFilter::OnMessageReceived(
    const IPC::Message& message) {
  DCHECK(JavaBridgeThread::CurrentlyOn());
  base::AutoReset<int32> routing_id(&current_routing_id_, message.routing_id());
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(GinJavaBridgeMessageFilter, message)
    IPC_MESSAGE_HANDLER(GinJavaBridgeHostMsg_GetMethods, OnGetMethods)
    IPC_MESSAGE_HANDLER(GinJavaBridgeHostMsg_HasMethod, OnHasMethod)
    IPC_MESSAGE_HANDLER(GinJavaBridgeHostMsg_InvokeMethod, OnInvokeMethod)
    IPC_MESSAGE_HANDLER(GinJavaBridgeHostMsg_ObjectWrapperDeleted,
                        OnObjectWrapperDeleted)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

base::TaskRunner* GinJavaBridgeMessageFilter::OverrideTaskRunnerForMessage(
    const IPC::Message& message) {
  // As the filter is only invoked for the messages of the particular class,
  // we can return the task runner unconditionally.
  return JavaBridgeThread::GetTaskRunner();
}

void GinJavaBridgeMessageFilter::AddRoutingIdForHost(
      GinJavaBridgeDispatcherHost* host,
      RenderFrameHost* render_frame_host) {
  base::AutoLock locker(hosts_lock_);
  hosts_[render_frame_host->GetRoutingID()] = host;
}

void GinJavaBridgeMessageFilter::RemoveHost(GinJavaBridgeDispatcherHost* host) {
  base::AutoLock locker(hosts_lock_);
  auto iter = hosts_.begin();
  while (iter != hosts_.end()) {
    if (iter->second == host)
      hosts_.erase(iter++);
    else
      ++iter;
  }
}

// static
scoped_refptr<GinJavaBridgeMessageFilter> GinJavaBridgeMessageFilter::FromHost(
    GinJavaBridgeDispatcherHost* host, bool create_if_not_exists) {
  RenderProcessHost* rph = host->web_contents()->GetRenderProcessHost();
  scoped_refptr<GinJavaBridgeMessageFilter> filter =
      base::UserDataAdapter<GinJavaBridgeMessageFilter>::Get(
          rph, kGinJavaBridgeMessageFilterKey);
  if (!filter && create_if_not_exists) {
    filter = new GinJavaBridgeMessageFilter();
    rph->AddFilter(filter.get());
    rph->SetUserData(kGinJavaBridgeMessageFilterKey,
                     new base::UserDataAdapter<GinJavaBridgeMessageFilter>(
                         filter.get()));
  }
  return filter;
}

GinJavaBridgeDispatcherHost* GinJavaBridgeMessageFilter::FindHost() {
  base::AutoLock locker(hosts_lock_);
  auto iter = hosts_.find(current_routing_id_);
  if (iter != hosts_.end())
    return iter->second;
  LOG(ERROR) << "WebView: Unknown frame routing id: " << current_routing_id_;
  return nullptr;
}

void GinJavaBridgeMessageFilter::OnGetMethods(
    GinJavaBoundObject::ObjectID object_id,
    std::set<std::string>* returned_method_names) {
  DCHECK(JavaBridgeThread::CurrentlyOn());
  GinJavaBridgeDispatcherHost* host = FindHost();
  if (host) {
    host->OnGetMethods(object_id, returned_method_names);
  } else {
    *returned_method_names = std::set<std::string>();
  }
}

void GinJavaBridgeMessageFilter::OnHasMethod(
    GinJavaBoundObject::ObjectID object_id,
    const std::string& method_name,
    bool* result) {
  DCHECK(JavaBridgeThread::CurrentlyOn());
  GinJavaBridgeDispatcherHost* host = FindHost();
  if (host) {
    host->OnHasMethod(object_id, method_name, result);
  } else {
    *result = false;
  }
}

void GinJavaBridgeMessageFilter::OnInvokeMethod(
    GinJavaBoundObject::ObjectID object_id,
    const std::string& method_name,
    const base::ListValue& arguments,
    base::ListValue* wrapped_result,
    content::GinJavaBridgeError* error_code) {
  DCHECK(JavaBridgeThread::CurrentlyOn());
  GinJavaBridgeDispatcherHost* host = FindHost();
  if (host) {
    host->OnInvokeMethod(current_routing_id_, object_id, method_name, arguments,
                         wrapped_result, error_code);
  } else {
    wrapped_result->Append(base::Value::CreateNullValue());
    *error_code = kGinJavaBridgeRenderFrameDeleted;
  }
}

void GinJavaBridgeMessageFilter::OnObjectWrapperDeleted(
    GinJavaBoundObject::ObjectID object_id) {
  DCHECK(JavaBridgeThread::CurrentlyOn());
  GinJavaBridgeDispatcherHost* host = FindHost();
  if (host)
    host->OnObjectWrapperDeleted(current_routing_id_, object_id);
}

}  // namespace content
