// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/media_controls/elements/MediaControlLoadingPanelElement.h"

#include "core/css/CSSStyleDeclaration.h"
#include "core/dom/ShadowRoot.h"
#include "core/dom/events/Event.h"
#include "core/event_type_names.h"
#include "core/html/media/HTMLMediaElement.h"
#include "core/html/media/HTMLVideoElement.h"
#include "core/html_names.h"
#include "core/input_type_names.h"
#include "core/testing/PageTestBase.h"
#include "modules/media_controls/MediaControlsImpl.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

class MediaControlLoadingPanelElementTest : public PageTestBase {
 public:
  void SetUp() final {
    // Create page and add a video element with controls.
    PageTestBase::SetUp();
    media_element_ = HTMLVideoElement::Create(GetDocument());
    media_element_->SetBooleanAttribute(HTMLNames::controlsAttr, true);
    GetDocument().body()->AppendChild(media_element_);

    // Create instance of MediaControlInputElement to run tests on.
    media_controls_ =
        static_cast<MediaControlsImpl*>(media_element_->GetMediaControls());
    ASSERT_NE(media_controls_, nullptr);
    loading_element_ = new MediaControlLoadingPanelElement(*media_controls_);
  }

 protected:
  void ExpectStateIsHidden() {
    EXPECT_EQ(MediaControlLoadingPanelElement::kHidden,
              loading_element_->state_);
    CheckIsHidden();
  }

  void ExpectStateIsPlaying() {
    EXPECT_EQ(MediaControlLoadingPanelElement::kPlaying,
              loading_element_->state_);
    CheckIsShown();
  }

  void ExpectStateIsCoolingDown() {
    EXPECT_EQ(MediaControlLoadingPanelElement::kCoolingDown,
              loading_element_->state_);
    CheckIsShown();
  }

  void SimulateLoadingMetadata() {
    SetMediaElementState(HTMLMediaElement::kHaveNothing,
                         HTMLMediaElement::kNetworkLoading);
    EXPECT_EQ(media_controls_->State(), MediaControlsImpl::kLoadingMetadata);
    loading_element_->UpdateDisplayState();
  }

  void SimulateStopped() {
    SetMediaElementState(HTMLMediaElement::kHaveCurrentData,
                         HTMLMediaElement::kNetworkIdle);
    EXPECT_EQ(media_controls_->State(), MediaControlsImpl::kStopped);
    loading_element_->UpdateDisplayState();
  }

  void SimulatePlaying() {
    SetMediaElementState(HTMLMediaElement::kHaveCurrentData,
                         HTMLMediaElement::kNetworkIdle, false);
    EXPECT_EQ(media_controls_->State(), MediaControlsImpl::kPlaying);
    loading_element_->UpdateDisplayState();
  }

  void SimulateNoSource() {
    SetMediaElementState(HTMLMediaElement::kHaveNothing,
                         HTMLMediaElement::kNetworkNoSource);
    EXPECT_EQ(media_controls_->State(), MediaControlsImpl::kNoSource);
    loading_element_->UpdateDisplayState();
  }

  void SimulateAnimationIterations(int count) {
    for (int i = 0; i < count; i++) {
      TriggerEvent(EventTypeNames::animationiteration);
    }
  }

  void ExpectAnimationIterationCount(const String& value) {
    ExpectAnimationIterationCount(loading_element_->mask1_background_, value);
    ExpectAnimationIterationCount(loading_element_->mask2_background_, value);
  }

  void ExpectAnimationIterationInfinite() {
    ExpectAnimationIterationCount("infinite");
  }

  void SimulateAnimationEnd() { TriggerEvent(EventTypeNames::animationend); }

  void SimulateControlsHidden() { loading_element_->OnControlsHidden(); }

  void SimulateControlsShown() { loading_element_->OnControlsShown(); }

  void RunPlayingTestCycle() {
    ExpectStateIsHidden();

    // Show the panel when we are loading metadata.
    SimulateLoadingMetadata();
    ExpectStateIsPlaying();

    // Simulate some animations.
    SimulateAnimationIterations(3);
    ExpectAnimationIterationInfinite();

    // Transition the media controls to a playing state and expect the loading
    // panel to start cooling down.
    SimulatePlaying();
    ExpectStateIsCoolingDown();
    ExpectAnimationIterationCount("4");

    // Simulate the animations ending.
    SimulateAnimationEnd();

    // Make sure the loading panel is hidden now.
    ExpectStateIsHidden();
  }

 private:
  void SetMediaElementState(HTMLMediaElement::ReadyState ready_state,
                            HTMLMediaElement::NetworkState network_state,
                            bool paused = true) {
    media_element_->ready_state_ = ready_state;
    media_element_->network_state_ = network_state;
    media_element_->paused_ = paused;
  }

  void CheckIsHidden() {
    EXPECT_FALSE(loading_element_->IsWanted());
    EXPECT_FALSE(loading_element_->GetShadowRoot()->HasChildren());
  }

  void CheckIsShown() {
    EXPECT_TRUE(loading_element_->IsWanted());
    EXPECT_TRUE(loading_element_->GetShadowRoot()->HasChildren());
  }

  void ExpectAnimationIterationCount(Element* element, const String& value) {
    EXPECT_EQ(value,
              element->style()->getPropertyValue("animation-iteration-count"));
  }

  void TriggerEvent(const AtomicString& name) {
    Event* event = Event::Create(name);
    loading_element_->mask1_background_->DispatchEvent(event);
  }

  Persistent<HTMLMediaElement> media_element_;
  Persistent<MediaControlsImpl> media_controls_;
  Persistent<MediaControlLoadingPanelElement> loading_element_;
};

TEST_F(MediaControlLoadingPanelElementTest, StateTransitions_ToPlaying) {
  RunPlayingTestCycle();
}

TEST_F(MediaControlLoadingPanelElementTest, StateTransitions_ToStopped) {
  ExpectStateIsHidden();

  // Show the panel when we are loading metadata.
  SimulateLoadingMetadata();
  ExpectStateIsPlaying();

  // Simulate some animations.
  SimulateAnimationIterations(5);
  ExpectAnimationIterationInfinite();

  // Transition the media controls to a stopped state and expect the loading
  // panel to start cooling down.
  SimulateStopped();
  ExpectStateIsCoolingDown();
  ExpectAnimationIterationCount("6");

  // Simulate the animations ending.
  SimulateAnimationEnd();

  // Make sure the loading panel is hidden now.
  ExpectStateIsHidden();
}

TEST_F(MediaControlLoadingPanelElementTest, Reset_AfterComplete) {
  RunPlayingTestCycle();

  // Reset to kNoSource.
  SimulateNoSource();
  RunPlayingTestCycle();
}

TEST_F(MediaControlLoadingPanelElementTest, Reset_DuringCycle) {
  ExpectStateIsHidden();

  // Show the panel when we are loading metadata.
  SimulateLoadingMetadata();
  ExpectStateIsPlaying();

  // Reset to kNoSource.
  SimulateNoSource();
  ExpectStateIsCoolingDown();

  // Start loading metadata again before we have hidden.
  SimulateLoadingMetadata();
  SimulateAnimationEnd();

  // We should now be showing the controls again.
  ExpectStateIsPlaying();
  ExpectAnimationIterationInfinite();

  // Now move to playing.
  SimulatePlaying();
  SimulateAnimationEnd();
  ExpectStateIsHidden();
}

TEST_F(MediaControlLoadingPanelElementTest, SkipLoadingMetadata) {
  ExpectStateIsHidden();
  SimulatePlaying();
  ExpectStateIsHidden();
}

TEST_F(MediaControlLoadingPanelElementTest, AnimationHiddenWhenControlsHidden) {
  // Animation doesn't start when Media Controls are already hidden.
  SimulateControlsHidden();
  SimulateLoadingMetadata();
  ExpectStateIsHidden();

  // Animation appears once Media Controls are shown.
  SimulateControlsShown();
  ExpectStateIsPlaying();

  // Animation is hidden when Media Controls are hidden again.
  SimulateControlsHidden();
  ExpectStateIsHidden();
}

}  // namespace blink
