// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/layout/ng/geometry/ng_physical_offset_rect.h"

#include "platform/geometry/FloatRect.h"
#include "platform/geometry/LayoutRect.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

bool NGPhysicalOffsetRect::operator==(const NGPhysicalOffsetRect& other) const {
  return other.offset == offset && other.size == size;
}

NGPhysicalOffsetRect NGPhysicalOffsetRect::operator+(
    const NGPhysicalOffset& offset) const {
  return {this->offset + offset, size};
}

void NGPhysicalOffsetRect::Unite(const NGPhysicalOffsetRect& other) {
  if (other.IsEmpty())
    return;
  if (IsEmpty()) {
    *this = other;
    return;
  }

  LayoutUnit left = std::min(offset.left, other.offset.left);
  LayoutUnit top = std::min(offset.top, other.offset.top);
  LayoutUnit right = std::max(Right(), other.Right());
  LayoutUnit bottom = std::max(Bottom(), other.Bottom());
  offset = {left, top};
  size = {right - left, bottom - top};
}

NGPhysicalOffsetRect::NGPhysicalOffsetRect(const LayoutRect& source)
    : NGPhysicalOffsetRect({source.X(), source.Y()},
                           {source.Width(), source.Height()}) {}

LayoutRect NGPhysicalOffsetRect::ToLayoutRect() const {
  return {offset.left, offset.top, size.width, size.height};
}

FloatRect NGPhysicalOffsetRect::ToFloatRect() const {
  return {offset.left, offset.top, size.width, size.height};
}

String NGPhysicalOffsetRect::ToString() const {
  return String::Format("%s,%s %sx%s", offset.left.ToString().Ascii().data(),
                        offset.top.ToString().Ascii().data(),
                        size.width.ToString().Ascii().data(),
                        size.height.ToString().Ascii().data());
}

std::ostream& operator<<(std::ostream& os, const NGPhysicalOffsetRect& value) {
  return os << value.ToString();
}

}  // namespace blink
