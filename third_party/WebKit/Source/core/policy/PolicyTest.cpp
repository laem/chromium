// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/policy/DocumentPolicy.h"
#include "core/policy/IFramePolicy.h"

#include "core/dom/Document.h"
#include "platform/feature_policy/FeaturePolicy.h"
#include "platform/weborigin/SecurityOrigin.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

namespace {
constexpr char kSelfOrigin[] = "https://selforigin.com";
constexpr char kOriginA[] = "https://example.com";
constexpr char kOriginB[] = "https://example.net";
}  // namespace

using testing::UnorderedElementsAre;

class PolicyTest : public testing::Test {
 public:
  void SetUp() override {
    document_ = Document::CreateForTest();
    document_->SetSecurityOrigin(SecurityOrigin::CreateFromString(kSelfOrigin));
    document_->ApplyFeaturePolicyFromHeader(
        "fullscreen *; payment 'self'; midi 'none'; camera 'self' "
        "https://example.com https://example.net");
  }

  Policy* GetPolicy() const { return policy_; }

 protected:
  Persistent<Document> document_;
  Persistent<Policy> policy_;
};

class DocumentPolicyTest : public PolicyTest {
 public:
  void SetUp() override {
    PolicyTest::SetUp();
    policy_ = new DocumentPolicy(document_);
  }
};

class IFramePolicyTest : public PolicyTest {
 public:
  void SetUp() override {
    PolicyTest::SetUp();
    policy_ = new IFramePolicy(document_, {},
                               SecurityOrigin::CreateFromString(kSelfOrigin));
  }
};

TEST_F(DocumentPolicyTest, TestAllowsFeature) {
  EXPECT_FALSE(GetPolicy()->allowsFeature("badfeature"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("midi"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("midi", kSelfOrigin));
  EXPECT_TRUE(GetPolicy()->allowsFeature("fullscreen"));
  EXPECT_TRUE(GetPolicy()->allowsFeature("fullscreen", kOriginA));
  EXPECT_TRUE(GetPolicy()->allowsFeature("payment"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("payment", kOriginA));
  EXPECT_FALSE(GetPolicy()->allowsFeature("payment", kOriginB));
  EXPECT_TRUE(GetPolicy()->allowsFeature("camera"));
  EXPECT_TRUE(GetPolicy()->allowsFeature("camera", kOriginA));
  EXPECT_TRUE(GetPolicy()->allowsFeature("camera", kOriginB));
  EXPECT_FALSE(GetPolicy()->allowsFeature("camera", "https://badorigin.com"));
  EXPECT_TRUE(GetPolicy()->allowsFeature("geolocation", kSelfOrigin));
}

TEST_F(DocumentPolicyTest, TestGetAllowList) {
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("camera"),
              UnorderedElementsAre(kSelfOrigin, kOriginA, kOriginB));
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("payment"),
              UnorderedElementsAre(kSelfOrigin));
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("geolocation"),
              UnorderedElementsAre(kSelfOrigin));
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("fullscreen"),
              UnorderedElementsAre("*"));
  EXPECT_TRUE(GetPolicy()->getAllowlistForFeature("badfeature").IsEmpty());
  EXPECT_TRUE(GetPolicy()->getAllowlistForFeature("midi").IsEmpty());
}

TEST_F(DocumentPolicyTest, TestAllowedFeatures) {
  Vector<String> allowed_features = GetPolicy()->allowedFeatures();
  EXPECT_TRUE(allowed_features.Contains("fullscreen"));
  EXPECT_TRUE(allowed_features.Contains("payment"));
  EXPECT_TRUE(allowed_features.Contains("camera"));
  // "geolocation" has default policy as allowed on self origin.
  EXPECT_TRUE(allowed_features.Contains("geolocation"));
  EXPECT_FALSE(allowed_features.Contains("badfeature"));
  EXPECT_FALSE(allowed_features.Contains("midi"));
}

TEST_F(IFramePolicyTest, TestAllowsFeature) {
  EXPECT_FALSE(GetPolicy()->allowsFeature("badfeature"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("midi"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("midi", kSelfOrigin));
  EXPECT_TRUE(GetPolicy()->allowsFeature("fullscreen"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("fullscreen", kOriginA));
  EXPECT_TRUE(GetPolicy()->allowsFeature("fullscreen", kSelfOrigin));
  EXPECT_TRUE(GetPolicy()->allowsFeature("payment"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("payment", kOriginA));
  EXPECT_FALSE(GetPolicy()->allowsFeature("payment", kOriginB));
  EXPECT_TRUE(GetPolicy()->allowsFeature("camera"));
  EXPECT_FALSE(GetPolicy()->allowsFeature("camera", kOriginA));
  EXPECT_FALSE(GetPolicy()->allowsFeature("camera", kOriginB));
  EXPECT_FALSE(GetPolicy()->allowsFeature("camera", "https://badorigin.com"));
  EXPECT_TRUE(GetPolicy()->allowsFeature("geolocation", kSelfOrigin));
}

TEST_F(IFramePolicyTest, TestGetAllowList) {
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("camera"),
              UnorderedElementsAre(kSelfOrigin));
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("payment"),
              UnorderedElementsAre(kSelfOrigin));
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("geolocation"),
              UnorderedElementsAre(kSelfOrigin));
  EXPECT_THAT(GetPolicy()->getAllowlistForFeature("fullscreen"),
              UnorderedElementsAre(kSelfOrigin));
  EXPECT_TRUE(GetPolicy()->getAllowlistForFeature("badfeature").IsEmpty());
  EXPECT_TRUE(GetPolicy()->getAllowlistForFeature("midi").IsEmpty());
}

TEST_F(IFramePolicyTest, TestAllowedFeatures) {
  Vector<String> allowed_features = GetPolicy()->allowedFeatures();
  EXPECT_TRUE(allowed_features.Contains("fullscreen"));
  EXPECT_TRUE(allowed_features.Contains("payment"));
  EXPECT_TRUE(allowed_features.Contains("camera"));
  // "geolocation" has default policy as allowed on self origin.
  EXPECT_TRUE(allowed_features.Contains("geolocation"));
  EXPECT_FALSE(allowed_features.Contains("badfeature"));
  EXPECT_FALSE(allowed_features.Contains("midi"));
}

TEST_F(IFramePolicyTest, TestCombinedPolicy) {
  ParsedFeaturePolicy container_policy = ParseFeaturePolicyAttribute(
      "geolocation 'src'; payment 'none'; midi; camera 'src'",
      SecurityOrigin::CreateFromString(kSelfOrigin),
      SecurityOrigin::CreateFromString(kOriginA), nullptr, nullptr);
  GetPolicy()->UpdateContainerPolicy(
      container_policy, SecurityOrigin::CreateFromString(kOriginA));
  Vector<String> allowed_features = GetPolicy()->allowedFeatures();
  EXPECT_TRUE(allowed_features.Contains("fullscreen"));
  EXPECT_FALSE(allowed_features.Contains("payment"));
  EXPECT_TRUE(allowed_features.Contains("geolocation"));
  EXPECT_FALSE(allowed_features.Contains("midi"));
  EXPECT_TRUE(allowed_features.Contains("camera"));
  // "geolocation" has default policy as allowed on self origin.
  EXPECT_FALSE(allowed_features.Contains("badfeature"));
}

}  // namespace blink
