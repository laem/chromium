// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/graphics/paint/FloatClipRect.h"

#include "platform/geometry/FloatRect.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

class FloatClipRectTest : public testing::Test {
 public:
};

TEST_F(FloatClipRectTest, InfinitRect) {
  FloatClipRect rect;
  EXPECT_TRUE(rect.IsInfinite());
  EXPECT_FALSE(rect.HasRadius());
  EXPECT_TRUE(rect.IsTight());

  FloatClipRect rect2((FloatRect(1, 2, 3, 4)));
  EXPECT_FALSE(rect2.IsInfinite());
  EXPECT_FALSE(rect.HasRadius());
  EXPECT_TRUE(rect.IsTight());
}

TEST_F(FloatClipRectTest, MoveBy) {
  FloatClipRect rect;
  rect.MoveBy(FloatPoint(1, 2));
  EXPECT_EQ(rect.Rect(), FloatClipRect().Rect());
  EXPECT_TRUE(rect.IsInfinite());
  EXPECT_FALSE(rect.HasRadius());
  EXPECT_TRUE(rect.IsTight());

  FloatClipRect rect2((FloatRect(1, 2, 3, 4)));
  rect2.SetHasRadius();
  rect2.MoveBy(FloatPoint(5, 6));
  EXPECT_EQ(FloatRect(6, 8, 3, 4), rect2.Rect());
  EXPECT_TRUE(rect2.HasRadius());
  EXPECT_FALSE(rect2.IsTight());
}

TEST_F(FloatClipRectTest, Intersect) {
  FloatClipRect rect;
  FloatClipRect rect1(FloatRect(1, 2, 3, 4));
  FloatClipRect rect2(FloatRect(3, 4, 5, 6));
  rect2.SetHasRadius();

  rect.Intersect(rect1);
  EXPECT_FALSE(rect.IsInfinite());
  EXPECT_EQ(FloatRect(1, 2, 3, 4), rect.Rect());
  EXPECT_FALSE(rect.HasRadius());
  EXPECT_TRUE(rect.IsTight());

  rect.Intersect(rect2);
  EXPECT_FALSE(rect.IsInfinite());
  EXPECT_EQ(FloatRect(3, 4, 1, 2), rect.Rect());
  EXPECT_TRUE(rect.HasRadius());
  EXPECT_FALSE(rect.IsTight());
}

TEST_F(FloatClipRectTest, SetHasRadius) {
  FloatClipRect rect;
  rect.SetHasRadius();
  EXPECT_FALSE(rect.IsInfinite());
  EXPECT_TRUE(rect.HasRadius());
  EXPECT_FALSE(rect.IsTight());
}

TEST_F(FloatClipRectTest, ClearIsTight) {
  FloatClipRect rect;
  rect.ClearIsTight();
  EXPECT_TRUE(rect.IsInfinite());
  EXPECT_FALSE(rect.HasRadius());
  EXPECT_FALSE(rect.IsTight());
}

TEST_F(FloatClipRectTest, Map) {
  FloatClipRect rect;
  TransformationMatrix identity;
  TransformationMatrix translation = TransformationMatrix().Translate(10, 20);
  TransformationMatrix rotate = TransformationMatrix().Rotate(45);

  rect.Map(rotate);
  EXPECT_TRUE(rect.IsInfinite());
  EXPECT_FALSE(rect.IsTight());

  FloatClipRect rect2(FloatRect(1, 2, 3, 4));
  rect2.Map(identity);
  EXPECT_EQ(FloatRect(1, 2, 3, 4), rect2.Rect());
  EXPECT_TRUE(rect2.IsTight());

  rect2.Map(translation);
  EXPECT_EQ(FloatRect(11, 22, 3, 4), rect2.Rect());
  EXPECT_TRUE(rect2.IsTight());

  rect2.Map(rotate);
  EXPECT_EQ(rotate.MapRect(FloatRect(11, 22, 3, 4)), rect2.Rect());
  EXPECT_FALSE(rect2.IsTight());

  rect2.Map(identity);
  EXPECT_FALSE(rect2.IsTight());
}

}  // namespace blink
