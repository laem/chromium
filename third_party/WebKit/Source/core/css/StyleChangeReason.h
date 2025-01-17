// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef StyleChangeReason_h
#define StyleChangeReason_h

#include "core/dom/QualifiedName.h"
#include "platform/wtf/text/AtomicString.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class QualifiedName;

namespace StyleChangeReason {
extern const char kActiveStylesheetsUpdate[];
extern const char kAnimation[];
extern const char kAttribute[];
extern const char kCleanupPlaceholderStyles[];
extern const char kControlValue[];
extern const char kControl[];
extern const char kDeclarativeContent[];
extern const char kDesignMode[];
extern const char kFontSizeChange[];
extern const char kFonts[];
extern const char kFullScreen[];
extern const char kInheritedStyleChangeFromParentFrame[];
extern const char kInline[];
extern const char kInlineCSSStyleMutated[];
extern const char kInspector[];
extern const char kLanguage[];
extern const char kLinkColorChange[];
extern const char kPlatformColorChange[];
extern const char kPropagateInheritChangeToDistributedNodes[];
extern const char kPropertyRegistration[];
extern const char kPropertyUnregistration[];
extern const char kPseudoClass[];
extern const char kSVGContainerSizeChange[];
extern const char kSVGCursor[];
extern const char kSettings[];
extern const char kShadow[];
extern const char kStyleInvalidator[];
extern const char kStyleSheetChange[];
extern const char kViewportUnits[];
extern const char kVisitedLink[];
extern const char kVisuallyOrdered[];
extern const char kWritingModeChange[];
extern const char kZoom[];
}  // namespace StyleChangeReason
typedef const char StyleChangeReasonString[];

namespace StyleChangeExtraData {
extern const AtomicString& g_active;
extern const AtomicString& g_disabled;
extern const AtomicString& g_drag;
extern const AtomicString& g_focus;
extern const AtomicString& g_focus_visible;
extern const AtomicString& g_focus_within;
extern const AtomicString& g_hover;
extern const AtomicString& g_past;
extern const AtomicString& g_unresolved;

void Init();
}  // namespace StyleChangeExtraData

// |StyleChangeReasonForTracing| is used to trace the reason a
// |Node::setNeedsStyleRecalc| call was made to show it in DevTools or in
// about:tracing.
// |StyleChangeReasonForTracing| is strictly only for the tracing purpose as
// described above. Blink logic must not depend on this value.
class StyleChangeReasonForTracing {
  DISALLOW_NEW();

 public:
  static StyleChangeReasonForTracing Create(
      StyleChangeReasonString reason_string) {
    return StyleChangeReasonForTracing(reason_string, g_null_atom);
  }

  static StyleChangeReasonForTracing CreateWithExtraData(
      StyleChangeReasonString reason_string,
      const AtomicString& extra_data) {
    return StyleChangeReasonForTracing(reason_string, extra_data);
  }

  static StyleChangeReasonForTracing FromAttribute(
      const QualifiedName& attribute_name) {
    return StyleChangeReasonForTracing(StyleChangeReason::kAttribute,
                                       attribute_name.LocalName());
  }

  String ReasonString() const { return String(reason_); }
  const AtomicString& GetExtraData() const { return extra_data_; }

 private:
  StyleChangeReasonForTracing(StyleChangeReasonString reason_string,
                              const AtomicString& extra_data)
      : reason_(reason_string), extra_data_(extra_data) {}

  // disable comparisons
  void operator==(const StyleChangeReasonForTracing&) const {}
  void operator!=(const StyleChangeReasonForTracing&) const {}

  const char* reason_;
  AtomicString extra_data_;
};

}  // namespace blink

#endif  // StyleChangeReason_h
