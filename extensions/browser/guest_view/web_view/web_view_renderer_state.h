// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// WebViewRendererState manages state data for WebView guest renderer processes.
//
// This class's data can be accessed via its methods from both the UI and IO
// threads, and uses locks to mediate this access. When making changes to this
// class, ensure that you avoid introducing any reentrant code in the methods,
// and that you always aquire the locks in the order |web_view_info_map_lock_|
// -> |web_view_partition_id_map_lock_| (if both are needed in one method).

#ifndef EXTENSIONS_BROWSER_GUEST_VIEW_WEB_VIEW_WEB_VIEW_RENDERER_STATE_H_
#define EXTENSIONS_BROWSER_GUEST_VIEW_WEB_VIEW_WEB_VIEW_RENDERER_STATE_H_

#include <map>
#include <set>
#include <string>
#include <utility>

#include "base/memory/singleton.h"

namespace extensions {

class WebViewGuest;

class WebViewRendererState {
 public:
  struct WebViewInfo {
    int embedder_process_id;
    int instance_id;
    int rules_registry_id;
    std::string partition_id;
    std::string owner_host;
    std::set<int> content_script_ids;

    WebViewInfo();
    ~WebViewInfo();
  };

  static WebViewRendererState* GetInstance();

  // Looks up the information for the embedder WebView for a RenderViewHost,
  // given its process and view ID. Returns true and writes the information to
  // |web_view_info| if found, otherwise returns false.
  bool GetInfo(int guest_process_id,
               int guest_routing_id,
               WebViewInfo* web_view_info) const;

  // Looks up the information for the owner of a WebView guest process, given
  // its process ID. Returns true and writes the info to |owner_process_id| and
  // |owner_host| if found, otherwise returns false.
  bool GetOwnerInfo(int guest_process_id,
                    int* owner_process_id,
                    std::string* owner_host) const;

  // Looks up the partition ID for a WebView guest process, given its
  // process ID. Returns true and writes the partition ID to |partition_id| if
  // found, otherwise returns false.
  bool GetPartitionID(int guest_process_id, std::string* partition_id) const;

  // Returns true if the renderer with process ID |render_process_id| is a
  // WebView guest process.
  bool IsGuest(int render_process_id) const;

  void AddContentScriptIDs(int embedder_process_id,
                           int view_instance_id,
                           const std::set<int>& script_ids);
  void RemoveContentScriptIDs(int embedder_process_id,
                              int view_instance_id,
                              const std::set<int>& script_ids);

 private:
  friend class WebViewGuest;
  friend struct base::DefaultSingletonTraits<WebViewRendererState>;

  using RenderId = std::pair<int, int>;
  using WebViewInfoMap = std::map<RenderId, WebViewInfo>;

  struct WebViewPartitionInfo {
    int web_view_count;
    std::string partition_id;
    WebViewPartitionInfo() {}
    WebViewPartitionInfo(int count, const std::string& partition)
        : web_view_count(count), partition_id(partition) {}
  };

  using WebViewPartitionIDMap = std::map<int, WebViewPartitionInfo>;

  WebViewRendererState();
  ~WebViewRendererState();

  // Adds/removes a WebView guest render process to/from the set.
  void AddGuest(int render_process_host_id, int routing_id,
                const WebViewInfo& web_view_info);
  void RemoveGuest(int render_process_host_id, int routing_id);

  // Locks are used to mediate access to these maps from both the UI and IO
  // threads.
  WebViewInfoMap web_view_info_map_;
  mutable base::Lock web_view_info_map_lock_;
  WebViewPartitionIDMap web_view_partition_id_map_;
  mutable base::Lock web_view_partition_id_map_lock_;

  DISALLOW_COPY_AND_ASSIGN(WebViewRendererState);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_GUEST_VIEW_WEB_VIEW_WEB_VIEW_RENDERER_STATE_H_
