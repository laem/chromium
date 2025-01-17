// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ColorPropertyFunctions_h
#define ColorPropertyFunctions_h

#include "core/css/StyleColor.h"
#include "core/css_property_names.h"

namespace blink {

class ComputedStyle;
class CSSProperty;

struct OptionalStyleColor {
 public:
  OptionalStyleColor(std::nullptr_t) : is_null_(true) {}
  OptionalStyleColor(const StyleColor& style_color)
      : is_null_(false), style_color_(style_color) {}
  OptionalStyleColor(const Color& color)
      : is_null_(false), style_color_(color) {}

  bool IsNull() const { return is_null_; }
  const StyleColor& Access() const {
    DCHECK(!is_null_);
    return style_color_;
  }
  bool operator==(const OptionalStyleColor& other) const {
    return is_null_ == other.is_null_ && style_color_ == other.style_color_;
  }

 private:
  bool is_null_;
  StyleColor style_color_;
};

class ColorPropertyFunctions {
 public:
  static OptionalStyleColor GetInitialColor(const CSSProperty&);
  static OptionalStyleColor GetUnvisitedColor(const CSSProperty&,
                                              const ComputedStyle&);
  static OptionalStyleColor GetVisitedColor(const CSSProperty&,
                                            const ComputedStyle&);
  static void SetUnvisitedColor(const CSSProperty&,
                                ComputedStyle&,
                                const Color&);
  static void SetVisitedColor(const CSSProperty&, ComputedStyle&, const Color&);
};

}  // namespace blink

#endif  // ColorPropertyFunctions_h
