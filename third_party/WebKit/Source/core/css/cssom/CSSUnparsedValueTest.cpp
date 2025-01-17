// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/CSSCustomPropertyDeclaration.h"
#include "core/css/cssom/CSSUnparsedValue.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(CSSUnparsedValueTest, FromCustomPropertyDeclarationWithCSSWideKeyword) {
  const auto* initial_value =
      CSSCustomPropertyDeclaration::Create("--var", CSSValueInitial);
  const auto* unparsed_value = CSSUnparsedValue::FromCSSValue(*initial_value);
  ASSERT_NE(nullptr, unparsed_value);
  ASSERT_EQ(1U, unparsed_value->length());

  const auto& item =
      unparsed_value->AnonymousIndexedGetter(0, ASSERT_NO_EXCEPTION);
  ASSERT_TRUE(item.IsString());
  EXPECT_EQ("initial", item.GetAsString());
}

}  // namespace blink
