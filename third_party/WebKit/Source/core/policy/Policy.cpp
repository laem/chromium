// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/policy/Policy.h"

#include "core/dom/Document.h"
#include "core/inspector/ConsoleMessage.h"
#include "platform/feature_policy/FeaturePolicy.h"
#include "platform/weborigin/SecurityOrigin.h"
#include "platform/wtf/text/StringUTF8Adaptor.h"

namespace blink {

bool Policy::allowsFeature(const String& feature) const {
  if (GetDefaultFeatureNameMap().Contains(feature)) {
    return GetPolicy()->IsFeatureEnabled(
        GetDefaultFeatureNameMap().at(feature));
  }

  AddWarningForUnrecognizedFeature(feature);
  return false;
}

bool Policy::allowsFeature(const String& feature, const String& url) const {
  scoped_refptr<const SecurityOrigin> origin =
      SecurityOrigin::CreateFromString(url);
  if (!origin || origin->IsUnique()) {
    GetDocument()->AddConsoleMessage(ConsoleMessage::Create(
        kOtherMessageSource, kWarningMessageLevel,
        "Invalid origin url for feature '" + feature + "': " + url + "."));
    return false;
  }

  if (!GetDefaultFeatureNameMap().Contains(feature)) {
    AddWarningForUnrecognizedFeature(feature);
    return false;
  }

  return GetPolicy()->IsFeatureEnabledForOrigin(
      GetDefaultFeatureNameMap().at(feature), origin->ToUrlOrigin());
}

Vector<String> Policy::allowedFeatures() const {
  Vector<String> allowed_features;
  for (const auto& entry : GetDefaultFeatureNameMap()) {
    if (GetPolicy()->IsFeatureEnabled(entry.value))
      allowed_features.push_back(entry.key);
  }
  return allowed_features;
}

Vector<String> Policy::getAllowlistForFeature(const String& feature) const {
  if (GetDefaultFeatureNameMap().Contains(feature)) {
    const FeaturePolicy::Allowlist allowlist =
        GetPolicy()->GetAllowlistForFeature(
            GetDefaultFeatureNameMap().at(feature));
    if (allowlist.MatchesAll())
      return Vector<String>({"*"});
    Vector<String> result;
    for (const auto& origin : allowlist.Origins()) {
      result.push_back(WTF::String::FromUTF8(origin.Serialize().c_str()));
    }
    return result;
  }

  AddWarningForUnrecognizedFeature(feature);
  return Vector<String>();
}

void Policy::AddWarningForUnrecognizedFeature(const String& feature) const {
  GetDocument()->AddConsoleMessage(
      ConsoleMessage::Create(kOtherMessageSource, kWarningMessageLevel,
                             "Unrecognized feature: '" + feature + "'."));
}

void Policy::Trace(blink::Visitor* visitor) {
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
