// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/media/AutoplayUmaHelper.h"

#include "core/dom/Document.h"
#include "core/html/media/AutoplayPolicy.h"
#include "core/html/media/HTMLMediaElement.h"
#include "core/html/media/HTMLVideoElement.h"
#include "core/testing/PageTestBase.h"

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

using testing::Invoke;

class MockAutoplayUmaHelper : public AutoplayUmaHelper {
 public:
  MockAutoplayUmaHelper(HTMLMediaElement* element)
      : AutoplayUmaHelper(element) {
    ON_CALL(*this, HandleContextDestroyed())
        .WillByDefault(
            Invoke(this, &MockAutoplayUmaHelper::ReallyHandleContextDestroyed));
  }

  void HandlePlayingEvent() { AutoplayUmaHelper::HandlePlayingEvent(); }

  MOCK_METHOD0(HandleContextDestroyed, void());

  // Making this a wrapper function to avoid calling the mocked version.
  void ReallyHandleContextDestroyed() {
    AutoplayUmaHelper::HandleContextDestroyed();
  }
};

class AutoplayUmaHelperTest : public PageTestBase {
 protected:
  HTMLMediaElement& MediaElement() {
    Element* element = GetDocument().getElementById("video");
    DCHECK(element);
    return ToHTMLVideoElement(*element);
  }

  MockAutoplayUmaHelper& UmaHelper() { return *uma_helper_; }

 private:
  void SetUp() override {
    PageTestBase::SetUp();
    GetDocument().documentElement()->SetInnerHTMLFromString(
        "<video id=video></video>", ASSERT_NO_EXCEPTION);
    HTMLMediaElement& element = MediaElement();
    uma_helper_ = new MockAutoplayUmaHelper(&element);
    element.autoplay_policy_->autoplay_uma_helper_ = uma_helper_;
    testing::Mock::AllowLeak(&UmaHelper());
  }

  void TearDown() override { uma_helper_.Clear(); }

  Persistent<MockAutoplayUmaHelper> uma_helper_;
};

TEST_F(AutoplayUmaHelperTest, VisibilityChangeWhenUnload) {
  EXPECT_CALL(UmaHelper(), HandleContextDestroyed());

  MediaElement().setMuted(true);
  UmaHelper().OnAutoplayInitiated(AutoplaySource::kMethod);
  UmaHelper().HandlePlayingEvent();
  PageTestBase::TearDown();
  testing::Mock::VerifyAndClear(&UmaHelper());
}

}  // namespace blink
