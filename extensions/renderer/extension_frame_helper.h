// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_EXTENSION_FRAME_HELPER_H_
#define EXTENSIONS_RENDERER_EXTENSION_FRAME_HELPER_H_

#include "content/public/common/console_message_level.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"

struct ExtensionMsg_ExternalConnectionInfo;
struct ExtensionMsg_TabConnectionInfo;

namespace extensions {

class Dispatcher;
struct Message;

// RenderFrame-level plumbing for extension features.
class ExtensionFrameHelper
    : public content::RenderFrameObserver,
      public content::RenderFrameObserverTracker<ExtensionFrameHelper> {
 public:
  ExtensionFrameHelper(content::RenderFrame* render_frame,
                       Dispatcher* extension_dispatcher);
  ~ExtensionFrameHelper() override;

  int tab_id() const { return tab_id_; }
  const std::string& tab_extension_owner_id() const {
    return tab_extension_owner_id_;
  }

 private:
  // RenderFrameObserver implementation.
  void DidCreateScriptContext(v8::Local<v8::Context>,
                              int extension_group,
                              int world_id) override;
  void WillReleaseScriptContext(v8::Local<v8::Context>, int world_id) override;
  bool OnMessageReceived(const IPC::Message& message) override;

  // IPC handlers.
  void OnAddMessageToConsole(content::ConsoleMessageLevel level,
                             const std::string& message);
  void OnExtensionDispatchOnConnect(
      int target_port_id,
      const std::string& channel_name,
      const ExtensionMsg_TabConnectionInfo& source,
      const ExtensionMsg_ExternalConnectionInfo& info,
      const std::string& tls_channel_id);
  void OnExtensionDeliverMessage(int target_port_id,
                                 const Message& message);
  void OnExtensionDispatchOnDisconnect(int port_id,
                                       const std::string& error_message);
  void OnExtensionSetTabId(int tab_id);
  void OnSetTabExtensionOwner(const std::string& extension_id);

  // The id of the tab the render frame is attached to.
  int tab_id_;

  // The id of the extension that "owns" the tab if this is a chrome-extension
  // page. If it's not a chrome-extension page, |tab_extension_owner_id_| is
  // empty.
  std::string tab_extension_owner_id_;

  Dispatcher* extension_dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(ExtensionFrameHelper);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_EXTENSION_FRAME_HELPER_H_
