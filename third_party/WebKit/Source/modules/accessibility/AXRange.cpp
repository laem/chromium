// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/accessibility/AXRange.h"

#include "core/dom/Document.h"
#include "modules/accessibility/AXObject.h"

namespace blink {

AXRange::AXRange(const AXPosition& start, const AXPosition& end)
    : start_(start), end_(end) {
  DCHECK(start.IsValid());
  DCHECK(end.IsValid());
  DCHECK_LE(start, end);

  const Document* document = start.ContainerObject()->GetDocument();
  DCHECK(document);
  DCHECK(document->IsActive());
  DCHECK(!document->NeedsLayoutTreeUpdate());
  DCHECK_EQ(end.ContainerObject()->GetDocument(), document);
#if DCHECK_IS_ON()
  dom_tree_version_ = document->DomTreeVersion();
  style_version_ = document->StyleVersion();
#endif  // DCHECK_IS_ON()
}

AXObject* AXRange::CommonAncestorContainer() const {
  if (!IsValid())
    return nullptr;
  int start_index, end_index;
  return const_cast<AXObject*>(AXObject::LowestCommonAncestor(
      *start_.ContainerObject(), *end_.ContainerObject(), &start_index,
      &end_index));
}

bool AXRange::IsCollapsed() const {
  return IsValid() && start_ == end_;
}

bool AXRange::IsValid() const {
  if (!start_.IsValid() || !end_.IsValid())
    return false;

  // We don't support ranges that span across documents.
  if (start_.ContainerObject()->GetDocument() !=
      end_.ContainerObject()->GetDocument()) {
    return false;
  }

  DCHECK(!start_.ContainerObject()->GetDocument()->NeedsLayoutTreeUpdate());
#if DCHECK_IS_ON()
  DCHECK_EQ(start_.ContainerObject()->GetDocument()->DomTreeVersion(),
            dom_tree_version_);
  DCHECK_EQ(start_.ContainerObject()->GetDocument()->StyleVersion(),
            style_version_);
#endif  // DCHECK_IS_ON()
  return true;
}

// static
AXRange AXRange::RangeOfContents(const AXObject& container) {
  return AXRange(AXPosition::CreateFirstPositionInContainerObject(container),
                 AXPosition::CreateLastPositionInContainerObject(container));
}

bool operator==(const AXRange& a, const AXRange& b) {
  DCHECK(a.IsValid() && b.IsValid());
  return a.Start() == b.Start() && a.End() == b.End();
}

bool operator!=(const AXRange& a, const AXRange& b) {
  return !(a == b);
}

std::ostream& operator<<(std::ostream& ostream, const AXRange& range) {
  if (!range.IsValid())
    return ostream << "Invalid AXRange";
  return ostream << "AXRange from " << range.Start() << " to " << range.End();
}

}  // namespace blink
