// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/editing/markers/GrammarMarker.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

const char* const kDescription = "Test description";

class GrammarMarkerTest : public testing::Test {};

TEST_F(GrammarMarkerTest, MarkerType) {
  DocumentMarker* marker = new GrammarMarker(0, 1, kDescription);
  EXPECT_EQ(DocumentMarker::kGrammar, marker->GetType());
}

TEST_F(GrammarMarkerTest, IsSpellCheckMarker) {
  DocumentMarker* marker = new GrammarMarker(0, 1, kDescription);
  EXPECT_TRUE(IsSpellCheckMarker(*marker));
}

TEST_F(GrammarMarkerTest, ConstructorAndGetters) {
  GrammarMarker* marker = new GrammarMarker(0, 1, kDescription);
  EXPECT_EQ(kDescription, marker->Description());
}

}  // namespace blink
