// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_GEOFENCING_GEOFENCING_DISPATCHER_HOST_H_
#define CONTENT_BROWSER_GEOFENCING_GEOFENCING_DISPATCHER_HOST_H_

#include "content/common/geofencing_status.h"
#include "content/public/browser/browser_message_filter.h"

namespace blink {
struct WebCircularGeofencingRegion;
}

namespace content {

class BrowserContext;

class GeofencingDispatcherHost : public BrowserMessageFilter {
 public:
  explicit GeofencingDispatcherHost(BrowserContext* browser_context);

 private:
  virtual ~GeofencingDispatcherHost();

  // BrowserMessageFilter implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) override;

  void OnRegisterRegion(int thread_id,
                        int request_id,
                        const std::string& region_id,
                        const blink::WebCircularGeofencingRegion& region);
  void OnUnregisterRegion(int thread_id,
                          int request_id,
                          const std::string& region_id);
  void OnGetRegisteredRegions(int thread_id, int request_id);

  void RegisterRegionCompleted(int thread_id,
                               int request_id,
                               GeofencingStatus result);
  void UnregisterRegionCompleted(int thread_id,
                                 int request_id,
                                 GeofencingStatus result);

  BrowserContext* browser_context_;
  base::WeakPtrFactory<GeofencingDispatcherHost> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(GeofencingDispatcherHost);
};

}  // namespace content

#endif  // CONTENT_BROWSER_GEOFENCING_GEOFENCING_DISPATCHER_HOST_H_
