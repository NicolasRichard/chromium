// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_GUEST_VIEW_EXTENSIONS_GUEST_VIEW_CONTAINER_H_
#define EXTENSIONS_RENDERER_GUEST_VIEW_EXTENSIONS_GUEST_VIEW_CONTAINER_H_

#include <queue>

#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "extensions/renderer/guest_view/guest_view_container.h"
#include "extensions/renderer/scoped_persistent.h"

namespace extensions {

class ExtensionsGuestViewContainer : public GuestViewContainer {
 public:

  class Request {
   public:
    Request(GuestViewContainer* container,
            v8::Handle<v8::Function> callback,
            v8::Isolate* isolate);
    virtual ~Request();

    virtual void PerformRequest() = 0;
    virtual void HandleResponse(const IPC::Message& message) = 0;

    GuestViewContainer* container() const { return container_; }

    bool HasCallback() const;

    v8::Handle<v8::Function> GetCallback() const;

    v8::Isolate* isolate() const { return isolate_; }

   private:
    GuestViewContainer* container_;
    ScopedPersistent<v8::Function> callback_;
    v8::Isolate* const isolate_;
  };

  // This class represents an AttachGuest request from Javascript. It includes
  // the input parameters and the callback function. The Attach operation may
  // not execute immediately, if the container is not ready or if there are
  // other attach operations in flight.
  class AttachRequest : public Request {
   public:
    AttachRequest(GuestViewContainer* container,
                  int guest_instance_id,
                  scoped_ptr<base::DictionaryValue> params,
                  v8::Handle<v8::Function> callback,
                  v8::Isolate* isolate);
    ~AttachRequest() override;

    void PerformRequest() override;
    void HandleResponse(const IPC::Message& message) override;

   private:
    const int guest_instance_id_;
    scoped_ptr<base::DictionaryValue> params_;
  };

  explicit ExtensionsGuestViewContainer(content::RenderFrame* render_frame);
  ~ExtensionsGuestViewContainer() override;

  static ExtensionsGuestViewContainer* FromID(int element_instance_id);

  void IssueRequest(linked_ptr<Request> request);

  // BrowserPluginDelegate implementation.
  void SetElementInstanceID(int element_instance_id) override;
  void Ready() override;

  // GuestViewContainer override.
  bool HandlesMessage(const IPC::Message& message) override;
  bool OnMessage(const IPC::Message& message) override;

 private:
  void OnHandleCallback(const IPC::Message& message);

  void EnqueueRequest(linked_ptr<Request> request);
  void PerformPendingRequest();
  void HandlePendingResponseCallback(const IPC::Message& message);

  bool ready_;

  std::deque<linked_ptr<Request> > pending_requests_;
  linked_ptr<Request> pending_response_;

  DISALLOW_COPY_AND_ASSIGN(ExtensionsGuestViewContainer);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_GUEST_VIEW_EXTENSIONS_GUEST_VIEW_CONTAINER_H_
