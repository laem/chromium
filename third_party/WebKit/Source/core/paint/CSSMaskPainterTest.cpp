// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/paint/CSSMaskPainter.h"

#include <gtest/gtest.h>
#include "core/testing/CoreUnitTestHelper.h"

namespace blink {
namespace {

using CSSMaskPainterTest = RenderingTest;

TEST_F(CSSMaskPainterTest, MaskBoundingBoxSVG) {
  SetBodyInnerHTML(R"HTML(
    <svg style="width:300px; height:200px;">
      <mask id="m">
        <rect style="x:75px; y:75px; width:100px; height:100px; fill:white;"/>
      </mask>
      <g id="masked" style="mask:url(#m);">
        <rect style="x:50px; y:100px; width:100px; height:100px; fill:green;"/>
        <rect style="x:100px; y:50px; width:100px; height:100px; fill:green;"/>
      </g>
    </svg>
  )HTML");
  auto& masked = *GetLayoutObjectByElementId("masked");
  Optional<IntRect> mask_bounding_box =
      CSSMaskPainter::MaskBoundingBox(masked, LayoutPoint());
  ASSERT_TRUE(mask_bounding_box.has_value());
  EXPECT_EQ(IntRect(75, 75, 100, 100), *mask_bounding_box);
}

TEST_F(CSSMaskPainterTest, MaskBoundingBoxCSSBlock) {
  SetBodyInnerHTML(R"HTML(
    <div id="masked" style="-webkit-mask:linear-gradient(black,transparent);
                            width:300px; height:200px;"></div>
  )HTML");
  auto& masked = *GetLayoutObjectByElementId("masked");
  Optional<IntRect> mask_bounding_box =
      CSSMaskPainter::MaskBoundingBox(masked, LayoutPoint(8, 8));
  ASSERT_TRUE(mask_bounding_box.has_value());
  EXPECT_EQ(IntRect(8, 8, 300, 200), *mask_bounding_box);
}

TEST_F(CSSMaskPainterTest, MaskBoundingBoxCSSMaskBoxImageOutset) {
  SetBodyInnerHTML(R"HTML(
    <div id="masked" style="
        -webkit-mask-box-image:linear-gradient(black,transparent);
        -webkit-mask-box-image-outset:10px; width:300px; height:200px;"></div>
  )HTML");
  auto& masked = *GetLayoutObjectByElementId("masked");
  Optional<IntRect> mask_bounding_box =
      CSSMaskPainter::MaskBoundingBox(masked, LayoutPoint(8, 8));
  ASSERT_TRUE(mask_bounding_box.has_value());
  EXPECT_EQ(IntRect(-2, -2, 320, 220), *mask_bounding_box);
}

TEST_F(CSSMaskPainterTest, MaskBoundingBoxCSSInline) {
  LoadAhem();
  SetBodyInnerHTML(R"HTML(
    <div style="font:10px Ahem; width:300px; height:200px;">
      <span id="masked" style="
          -webkit-mask:linear-gradient(black,transparent);">
        The quick brown fox jumped
        over the lazy dog.
      </span>
    </div>
  )HTML");
  auto& masked = *GetLayoutObjectByElementId("masked");
  Optional<IntRect> mask_bounding_box =
      CSSMaskPainter::MaskBoundingBox(masked, LayoutPoint(8, 8));
  ASSERT_TRUE(mask_bounding_box.has_value());
  EXPECT_EQ(IntRect(8, 8, 260, 20), *mask_bounding_box);
}

}  // unnamed namespace
}  // namespace blink
