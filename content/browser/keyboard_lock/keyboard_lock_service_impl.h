// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_KEYBOARD_LOCK_KEYBOARD_LOCK_SERVICE_IMPL_H_
#define CONTENT_BROWSER_KEYBOARD_LOCK_KEYBOARD_LOCK_SERVICE_IMPL_H_

#include <string>
#include <vector>

#include "content/common/content_export.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "third_party/WebKit/public/platform/modules/keyboard_lock/keyboard_lock.mojom.h"

namespace content {

class RenderFrameHost;
class RenderFrameHostImpl;

class CONTENT_EXPORT KeyboardLockServiceImpl
    : public blink::mojom::KeyboardLockService {
 public:
  explicit KeyboardLockServiceImpl(RenderFrameHost* render_frame_host);
  ~KeyboardLockServiceImpl() override;

  static void CreateMojoService(
      RenderFrameHost* render_frame_host,
      blink::mojom::KeyboardLockServiceRequest request);

  // blink::mojom::KeyboardLockService implementation.
  void RequestKeyboardLock(const std::vector<std::string>& key_codes,
                           RequestKeyboardLockCallback callback) override;
  void CancelKeyboardLock() override;

 private:
  RenderFrameHostImpl* const render_frame_host_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_KEYBOARD_LOCK_KEYBOARD_LOCK_SERVICE_IMPL_H_
