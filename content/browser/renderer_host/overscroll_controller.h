// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_OVERSCROLL_CONTROLLER_H_
#define CONTENT_BROWSER_RENDERER_HOST_OVERSCROLL_CONTROLLER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "cc/input/overscroll_behavior.h"
#include "content/common/content_export.h"
#include "third_party/WebKit/public/platform/WebGestureEvent.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
#include "ui/events/blink/did_overscroll_params.h"

namespace content {

class OverscrollControllerDelegate;
class OverscrollControllerTest;
class RenderWidgetHostViewAuraOverscrollTest;

// Indicates the direction that the scroll is heading in relative to the screen,
// with the top being NORTH.
enum OverscrollMode {
  OVERSCROLL_NONE,
  OVERSCROLL_NORTH,
  OVERSCROLL_SOUTH,
  OVERSCROLL_WEST,
  OVERSCROLL_EAST
};

// Indicates the source device that was used to trigger the overscroll gesture.
enum class OverscrollSource {
  NONE,
  TOUCHPAD,
  TOUCHSCREEN,
};

// When a page is scrolled beyond the scrollable region, it will trigger an
// overscroll gesture. This controller receives the events that are dispatched
// to the renderer, and the ACKs of events, and updates the overscroll gesture
// status accordingly.
// Exported for testing.
class CONTENT_EXPORT OverscrollController {
 public:
  OverscrollController();
  virtual ~OverscrollController();

  // This must be called when dispatching any event from the
  // RenderWidgetHostView so that the state of the overscroll gesture can be
  // updated properly. Returns true if the event was handled, in which case
  // further processing should cease.
  bool WillHandleEvent(const blink::WebInputEvent& event);

  // This is called whenever an overscroll event is generated on the renderer
  // side. This is called before ReceivedEventAck. The params contains an
  // OverscrollBehavior that can prevent overscroll navigation.
  void OnDidOverscroll(const ui::DidOverscrollParams& params);

  // This must be called when the ACK for any event comes in. This updates the
  // overscroll gesture status as appropriate.
  // Virtual for testing.
  virtual void ReceivedEventACK(const blink::WebInputEvent& event,
                                bool processed);

  OverscrollMode overscroll_mode() const { return overscroll_mode_; }

  void set_delegate(OverscrollControllerDelegate* delegate) {
    delegate_ = delegate;
  }

  // Resets internal states.
  void Reset();

  // Cancels any in-progress overscroll (and calls OnOverscrollModeChange on the
  // delegate if necessary), and resets internal states.
  void Cancel();

 private:
  friend class OverscrollControllerTest;
  friend class RenderWidgetHostViewAuraOverscrollTest;

  // Different scrolling states.
  enum class ScrollState {
    NONE,

    // Either a mouse-wheel or a gesture-scroll-update event is consumed by the
    // renderer in which case no overscroll should be initiated until the end of
    // the user interaction.
    CONTENT_CONSUMING,

    // Overscroll controller has initiated overscrolling and will consume all
    // subsequent gesture-scroll-update events, preventing them from being
    // forwarded to the renderer.
    OVERSCROLLING,
  };

  // Returns true if the event indicates that the in-progress overscroll gesture
  // can now be completed.
  bool DispatchEventCompletesAction(
      const blink::WebInputEvent& event) const;

  // Returns true to indicate that dispatching the event should reset the
  // overscroll gesture status.
  bool DispatchEventResetsState(const blink::WebInputEvent& event) const;

  // Processes an event to update the internal state for overscroll. Returns
  // true if the state is updated, false otherwise.
  bool ProcessEventForOverscroll(const blink::WebInputEvent& event);

  // Processes horizontal overscroll. This can update both the overscroll mode
  // and the over scroll amount (i.e. |overscroll_mode_|, |overscroll_delta_x_|
  // and |overscroll_delta_y_|). Returns true if overscroll was handled by the
  // delegate.
  bool ProcessOverscroll(float delta_x, float delta_y, bool is_touchpad);

  // Completes the desired action from the current gesture.
  void CompleteAction();

  // Sets the overscroll mode and triggers callback in the delegate when
  // appropriate. When a new overscroll is started (i.e. when |new_mode| is not
  // equal to OVERSCROLL_NONE), |source| will be set to the device that
  // triggered the overscroll gesture.
  void SetOverscrollMode(OverscrollMode new_mode, OverscrollSource source);

  // Whether this event should be processed or not handled by the controller.
  bool ShouldProcessEvent(const blink::WebInputEvent& event);

  // Helper function to reset |scroll_state_| and |locked_mode_|.
  void ResetScrollState();

  // Current value of overscroll-behavior CSS property for the root element of
  // the page.
  cc::OverscrollBehavior behavior_;

  // The current state of overscroll gesture.
  OverscrollMode overscroll_mode_ = OVERSCROLL_NONE;

  // When set to something other than OVERSCROLL_NONE, the overscroll cannot
  // switch to any other mode, except to OVERSCROLL_NONE. This is set when an
  // overscroll is started until the touch sequence is completed.
  OverscrollMode locked_mode_ = OVERSCROLL_NONE;

  // Source of the current overscroll gesture.
  OverscrollSource overscroll_source_ = OverscrollSource::NONE;

  // Current scrolling state.
  ScrollState scroll_state_ = ScrollState::NONE;

  // The amount of overscroll in progress. These values are invalid when
  // |overscroll_mode_| is set to OVERSCROLL_NONE.
  float overscroll_delta_x_ = 0.f;
  float overscroll_delta_y_ = 0.f;

  // The delegate that receives the overscroll updates. The delegate is not
  // owned by this controller.
  OverscrollControllerDelegate* delegate_ = nullptr;

  bool wheel_scroll_latching_enabled_;

  DISALLOW_COPY_AND_ASSIGN(OverscrollController);
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_OVERSCROLL_CONTROLLER_H_
