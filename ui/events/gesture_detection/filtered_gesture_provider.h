// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_FILTERED_GESTURE_PROVIDER_H_
#define UI_EVENTS_GESTURE_DETECTION_FILTERED_GESTURE_PROVIDER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/events/gesture_detection/gesture_event_data_packet.h"
#include "ui/events/gesture_detection/gesture_provider.h"
#include "ui/events/gesture_detection/touch_disposition_gesture_filter.h"

namespace ui {

// Provides filtered gesture detection and dispatch given a sequence of touch
// events and touch event acks.
class GESTURE_DETECTION_EXPORT FilteredGestureProvider
    : public ui::TouchDispositionGestureFilterClient,
      public ui::GestureProviderClient {
 public:
  // |client| will be offered all gestures detected by the |gesture_provider_|
  // and allowed by the |gesture_filter_|.
  FilteredGestureProvider(const GestureProvider::Config& config,
                          GestureProviderClient* client);

  struct TouchHandlingResult {
    TouchHandlingResult();

    // True if |event| was both valid and successfully handled by the
    // gesture provider. Otherwise, false, in which case the caller should drop
    // |event| and cease furthe propagation.
    bool succeeded;

    // Whether |event| produced scrolling motion, either the start of a scroll,
    // subsequent scroll movement or a fling event.
    // TODO(jdduke): Figure out a way to guarantee that this bit propagates with
    // the processed touch event as it moves downstream.
    bool did_generate_scroll;
  };
  TouchHandlingResult OnTouchEvent(const MotionEvent& event) WARN_UNUSED_RESULT;

  // To be called upon asynchronous ack of an event that was forwarded
  // after a successful call to |OnTouchEvent()|.
  void OnAsyncTouchEventAck(bool event_consumed);

  // To be called upon synchronous ack of an event that was forwarded
  // after a successful call to |OnTouchEvent()|.
  void OnSyncTouchEventAck(bool event_consumed);

  // Methods delegated to |gesture_provider_|.
  void ResetDetection();
  void SetMultiTouchZoomSupportEnabled(bool enabled);
  void SetDoubleTapSupportForPlatformEnabled(bool enabled);
  void SetDoubleTapSupportForPageEnabled(bool enabled);
  const ui::MotionEvent* GetCurrentDownEvent() const;

 private:
  // GestureProviderClient implementation.
  void OnGestureEvent(const ui::GestureEventData& event) override;

  // TouchDispositionGestureFilterClient implementation.
  void ForwardGestureEvent(const ui::GestureEventData& event) override;

  GestureProviderClient* const client_;

  ui::GestureProvider gesture_provider_;
  ui::TouchDispositionGestureFilter gesture_filter_;

  bool handling_event_;
  bool last_touch_event_did_generate_scroll_;
  ui::GestureEventDataPacket pending_gesture_packet_;

  DISALLOW_COPY_AND_ASSIGN(FilteredGestureProvider);
};

}  // namespace ui

#endif  // UI_EVENTS_GESTURE_DETECTION_FILTERED_GESTURE_PROVIDER_H_
