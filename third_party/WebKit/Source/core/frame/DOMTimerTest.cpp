// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/frame/DOMTimer.h"

#include <vector>

#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"
#include "bindings/core/v8/ScriptController.h"
#include "bindings/core/v8/ScriptSourceCode.h"
#include "bindings/core/v8/V8BindingForCore.h"
#include "core/dom/Document.h"
#include "core/testing/CoreUnitTestHelper.h"
#include "platform/testing/TestingPlatformSupportWithMockScheduler.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::DoubleNear;
using testing::ElementsAreArray;
using testing::Matcher;

namespace blink {

namespace {

// The resolution of performance.now is 5us, so the threshold for time
// comparison is 6us to account for rounding errors.
const double kThreshold = 0.006;

class DOMTimerTest : public RenderingTest {
 public:
  ScopedTestingPlatformSupport<TestingPlatformSupportWithMockScheduler>
      platform_;

  // Expected time between each iterator for setInterval(..., 1) or nested
  // setTimeout(..., 1) are 1, 1, 1, 1, 4, 4, ... as a minimum clamp of 4ms
  // is applied from the 5th iteration onwards.
  const std::vector<Matcher<double>> kExpectedTimings = {
      DoubleNear(1., kThreshold), DoubleNear(1., kThreshold),
      DoubleNear(1., kThreshold), DoubleNear(1., kThreshold),
      DoubleNear(4., kThreshold), DoubleNear(4., kThreshold),
  };

  void SetUp() override {
    platform_->SetAutoAdvanceNowToPendingTasks(true);
    // Advance timer manually as RenderingTest expects the time to be non-zero.
    platform_->AdvanceClockSeconds(1.);
    RenderingTest::SetUp();
    GetDocument().GetSettings()->SetScriptEnabled(true);
  }

  v8::Local<v8::Value> EvalExpression(const char* expr) {
    return GetDocument()
        .GetFrame()
        ->GetScriptController()
        .ExecuteScriptInMainWorldAndReturnValue(ScriptSourceCode(expr));
  }

  Vector<double> ToDoubleArray(v8::Local<v8::Value> value,
                               v8::HandleScope& scope) {
    NonThrowableExceptionState exception_state;
    return NativeValueTraits<IDLSequence<IDLDouble>>::NativeValue(
        scope.GetIsolate(), value, exception_state);
  }

  double ToDoubleValue(v8::Local<v8::Value> value, v8::HandleScope& scope) {
    NonThrowableExceptionState exceptionState;
    return ToDouble(scope.GetIsolate(), value, exceptionState);
  }

  void ExecuteScriptAndWaitUntilIdle(const char* script_text) {
    ScriptSourceCode script(script_text);
    GetDocument().GetFrame()->GetScriptController().ExecuteScriptInMainWorld(
        script);
    platform_->RunUntilIdle();
  }
};

const char* const kSetTimeout0ScriptText =
    "var last = performance.now();"
    "var elapsed;"
    "function setTimeoutCallback() {"
    "  var current = performance.now();"
    "  elapsed = current - last;"
    "}"
    "setTimeout(setTimeoutCallback, 0);";

TEST_F(DOMTimerTest, DISABLED_setTimeout_ZeroIsNotClampedToOne) {
  v8::HandleScope scope(v8::Isolate::GetCurrent());

  ExecuteScriptAndWaitUntilIdle(kSetTimeout0ScriptText);

  double time = ToDoubleValue(EvalExpression("elapsed"), scope);

  EXPECT_THAT(time, DoubleNear(0., kThreshold));
}

const char* const kSetTimeoutNestedScriptText =
    "var last = performance.now();"
    "var times = [];"
    "function nestSetTimeouts() {"
    "  var current = performance.now();"
    "  var elapsed = current - last;"
    "  last = current;"
    "  times.push(elapsed);"
    "  if (times.length < 6) {"
    "    setTimeout(nestSetTimeouts, 1);"
    "  }"
    "}"
    "setTimeout(nestSetTimeouts, 1);";

TEST_F(DOMTimerTest, setTimeout_ClampsAfter4Nestings) {
  v8::HandleScope scope(v8::Isolate::GetCurrent());

  ExecuteScriptAndWaitUntilIdle(kSetTimeoutNestedScriptText);

  auto times(ToDoubleArray(EvalExpression("times"), scope));

  EXPECT_THAT(times, ElementsAreArray(kExpectedTimings));
}

const char* const kSetIntervalScriptText =
    "var last = performance.now();"
    "var times = [];"
    "var id = setInterval(function() {"
    "  var current = performance.now();"
    "  var elapsed = current - last;"
    "  last = current;"
    "  times.push(elapsed);"
    "  if (times.length > 5) {"
    "    clearInterval(id);"
    "  }"
    "}, 1);";

TEST_F(DOMTimerTest, setInterval_ClampsAfter4Iterations) {
  v8::HandleScope scope(v8::Isolate::GetCurrent());

  ExecuteScriptAndWaitUntilIdle(kSetIntervalScriptText);

  auto times(ToDoubleArray(EvalExpression("times"), scope));

  EXPECT_THAT(times, ElementsAreArray(kExpectedTimings));
}

TEST_F(DOMTimerTest, setInterval_NestingResetsForLaterCalls) {
  v8::HandleScope scope(v8::Isolate::GetCurrent());

  ExecuteScriptAndWaitUntilIdle(kSetIntervalScriptText);

  // Run the setIntervalScript again to verify that the clamp imposed for
  // nesting beyond 4 levels is reset when setInterval is called again in the
  // original scope but after the original setInterval has completed.
  ExecuteScriptAndWaitUntilIdle(kSetIntervalScriptText);

  auto times(ToDoubleArray(EvalExpression("times"), scope));

  EXPECT_THAT(times, ElementsAreArray(kExpectedTimings));
}

}  // namespace

}  // namespace blink
