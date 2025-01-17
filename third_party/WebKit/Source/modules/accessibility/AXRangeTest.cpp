// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/accessibility/AXRange.h"

#include "modules/accessibility/AXObject.h"
#include "modules/accessibility/AXPosition.h"
#include "modules/accessibility/testing/AccessibilityTest.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST_F(AccessibilityTest, CommonAncestorContainerOfRange) {
  SetBodyInnerHTML(R"HTML(<input id='input' type='text' value='value'>"
                   R"<p id='paragraph'>hello<br id='br'>there</p>"
                   R"<button id='button'>button</button>)HTML");

  const AXObject* root = GetAXRootObject();
  ASSERT_NE(nullptr, root);
  const AXObject* input = GetAXObjectByElementId("input");
  ASSERT_NE(nullptr, input);
  const AXObject* paragraph = GetAXObjectByElementId("paragraph");
  ASSERT_NE(nullptr, paragraph);
  const AXObject* text1 = paragraph->FirstChild();
  ASSERT_NE(nullptr, text1);
  ASSERT_EQ(AccessibilityRole::kStaticTextRole, text1->RoleValue());
  const AXObject* br = GetAXObjectByElementId("br");
  ASSERT_NE(nullptr, br);
  const AXObject* text2 = paragraph->LastChild();
  ASSERT_NE(nullptr, text2);
  ASSERT_EQ(AccessibilityRole::kStaticTextRole, text2->RoleValue());
  const AXObject* button = GetAXObjectByElementId("button");
  ASSERT_NE(nullptr, button);

  EXPECT_EQ(root,
            AXRange(AXPosition::CreateFirstPositionInContainerObject(*input),
                    AXPosition::CreateLastPositionInContainerObject(*button))
                .CommonAncestorContainer());
  EXPECT_EQ(root,
            AXRange(AXPosition::CreateFirstPositionInContainerObject(*br),
                    AXPosition::CreateFirstPositionInContainerObject(*button))
                .CommonAncestorContainer());
  EXPECT_EQ(paragraph, AXRange(AXPosition::CreatePositionBeforeObject(*text1),
                               AXPosition::CreatePositionAfterObject(*text2))
                           .CommonAncestorContainer());
}

TEST_F(AccessibilityTest, IsCollapsedRange) {
  SetBodyInnerHTML(R"HTML(<p id='paragraph'>hello there</p>)HTML");

  const AXObject* paragraph = GetAXObjectByElementId("paragraph");
  ASSERT_NE(nullptr, paragraph);
  const AXObject* text = paragraph->FirstChild();
  ASSERT_NE(nullptr, text);
  ASSERT_EQ(AccessibilityRole::kStaticTextRole, text->RoleValue());

  const AXRange paragraph_range(
      AXPosition::CreateLastPositionInContainerObject(*paragraph),
      AXPosition::CreateLastPositionInContainerObject(*paragraph));
  const AXRange text_range(
      AXPosition::CreateLastPositionInContainerObject(*text),
      AXPosition::CreateLastPositionInContainerObject(*text));
  EXPECT_TRUE(paragraph_range.IsCollapsed());
  EXPECT_TRUE(text_range.IsCollapsed());
  EXPECT_FALSE(AXRange::RangeOfContents(*paragraph).IsCollapsed());
}

TEST_F(AccessibilityTest, RangeOfContents) {
  SetBodyInnerHTML(R"HTML(<p id='paragraph'>hello there</p>)HTML");

  const AXObject* paragraph = GetAXObjectByElementId("paragraph");
  ASSERT_NE(nullptr, paragraph);

  const AXRange paragraph_range = AXRange::RangeOfContents(*paragraph);
  EXPECT_EQ(AXPosition::CreateFirstPositionInContainerObject(*paragraph),
            paragraph_range.Start());
  EXPECT_EQ(AXPosition::CreateLastPositionInContainerObject(*paragraph),
            paragraph_range.End());
}

}  // namespace blink
