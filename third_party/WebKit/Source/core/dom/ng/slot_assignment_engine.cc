// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/dom/ng/slot_assignment_engine.h"

#include "core/dom/ShadowRoot.h"
#include "core/dom/SlotAssignment.h"

namespace blink {

SlotAssignmentEngine::SlotAssignmentEngine() {}

void SlotAssignmentEngine::AddShadowRootNeedingRecalc(ShadowRoot& shadow_root) {
  DCHECK(shadow_root.isConnected());
  DCHECK(shadow_root.NeedsSlotAssignmentRecalc());
  shadow_roots_needing_recalc_.insert(&shadow_root);
}

void SlotAssignmentEngine::RemoveShadowRootNeedingRecalc(
    ShadowRoot& shadow_root) {
  DCHECK(shadow_root.isConnected());
  DCHECK(!shadow_root.NeedsSlotAssignmentRecalc());
  DCHECK(shadow_roots_needing_recalc_.Contains(&shadow_root));
  shadow_roots_needing_recalc_.erase(&shadow_root);
}

void SlotAssignmentEngine::Connected(ShadowRoot& shadow_root) {
  if (shadow_root.NeedsSlotAssignmentRecalc())
    AddShadowRootNeedingRecalc(shadow_root);
}

void SlotAssignmentEngine::Disconnected(ShadowRoot& shadow_root) {
  if (shadow_root.NeedsSlotAssignmentRecalc()) {
    DCHECK(shadow_roots_needing_recalc_.Contains(&shadow_root));
    shadow_roots_needing_recalc_.erase(&shadow_root);
  } else {
    DCHECK(!shadow_roots_needing_recalc_.Contains(&shadow_root));
  }
}

void SlotAssignmentEngine::RecalcSlotAssignments() {
  for (auto& shadow_root :
       HeapHashSet<WeakMember<ShadowRoot>>(shadow_roots_needing_recalc_)) {
    DCHECK(shadow_root->isConnected());
    DCHECK(shadow_root->NeedsSlotAssignmentRecalc());
    // SlotAssignment::RecalcAssignmentNg() will remove its shadow root from
    // shadow_roots_needing_recalc_.
    shadow_root->GetSlotAssignment().RecalcAssignmentNg();
  }
  DCHECK(shadow_roots_needing_recalc_.IsEmpty());
}

void SlotAssignmentEngine::Trace(blink::Visitor* visitor) {
  visitor->Trace(shadow_roots_needing_recalc_);
}

}  // namespace blink
