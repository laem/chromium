// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/usb/web_usb_chooser_service_desktop.h"

#include <utility>

#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/chrome_bubble_manager.h"
#include "chrome/browser/ui/permission_bubble/chooser_bubble_delegate.h"
#include "chrome/browser/usb/usb_chooser_controller.h"
#include "components/bubble/bubble_controller.h"
#include "content/public/browser/web_contents.h"

WebUsbChooserServiceDesktop::WebUsbChooserServiceDesktop(
    content::RenderFrameHost* render_frame_host)
    : WebUsbChooserService(render_frame_host) {}

WebUsbChooserServiceDesktop::~WebUsbChooserServiceDesktop() {
  for (const auto& bubble : bubbles_) {
    if (bubble)
      bubble->CloseBubble(BUBBLE_CLOSE_FORCED);
  }
}

void WebUsbChooserServiceDesktop::ShowChooser(
    std::unique_ptr<UsbChooserController> controller) {
  auto delegate = std::make_unique<ChooserBubbleDelegate>(
      render_frame_host(), std::move(controller));
  auto* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host());
  Browser* browser = chrome::FindBrowserWithWebContents(web_contents);
  BubbleReference bubble_reference =
      browser->GetBubbleManager()->ShowBubble(std::move(delegate));
  bubbles_.push_back(bubble_reference);
}
