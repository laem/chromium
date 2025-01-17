// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/supervised_user/supervised_user_error_page_controller.h"

#include "base/strings/utf_string_conversions.h"
#include "chrome/renderer/supervised_user/supervised_user_error_page_controller_delegate.h"
#include "content/public/renderer/render_frame.h"
#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "third_party/WebKit/public/web/WebKit.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"

gin::WrapperInfo SupervisedUserErrorPageController::kWrapperInfo = {
    gin::kEmbedderNativeGin};

void SupervisedUserErrorPageController::Install(
    content::RenderFrame* render_frame,
    base::WeakPtr<SupervisedUserErrorPageControllerDelegate> delegate) {
  v8::Isolate* isolate = blink::MainThreadIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context =
      render_frame->GetWebFrame()->MainWorldScriptContext();
  if (context.IsEmpty())
    return;

  v8::Context::Scope context_scope(context);

  gin::Handle<SupervisedUserErrorPageController> controller = gin::CreateHandle(
      isolate, new SupervisedUserErrorPageController(delegate, render_frame));
  if (controller.IsEmpty())
    return;

  v8::Local<v8::Object> global = context->Global();
  global->Set(gin::StringToV8(isolate, "supervisedUserErrorPageController"),
              controller.ToV8());
}

SupervisedUserErrorPageController::SupervisedUserErrorPageController(
    base::WeakPtr<SupervisedUserErrorPageControllerDelegate> delegate,
    content::RenderFrame* render_frame)
    : delegate_(delegate), render_frame_(render_frame), weak_factory_(this) {}

SupervisedUserErrorPageController::~SupervisedUserErrorPageController() {}

void SupervisedUserErrorPageController::GoBack() {
  if (delegate_)
    delegate_->GoBack();
}

void SupervisedUserErrorPageController::RequestPermission() {
  if (delegate_) {
    delegate_->RequestPermission(base::BindOnce(
        &SupervisedUserErrorPageController::RequestPermissionCallback,
        weak_factory_.GetWeakPtr()));
  }
}

void SupervisedUserErrorPageController::Feedback() {
  if (delegate_)
    delegate_->Feedback();
}

void SupervisedUserErrorPageController::RequestPermissionCallback(
    bool success) {
  std::string result = success ? "true" : "false";
  std::string js = "setRequestStatus(" + result + ");";
  render_frame_->ExecuteJavaScript(base::ASCIIToUTF16(js));
}

gin::ObjectTemplateBuilder
SupervisedUserErrorPageController::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<SupervisedUserErrorPageController>::
      GetObjectTemplateBuilder(isolate)
          .SetMethod("goBack", &SupervisedUserErrorPageController::GoBack)
          .SetMethod("requestPermission",
                     &SupervisedUserErrorPageController::RequestPermission)
          .SetMethod("feedback", &SupervisedUserErrorPageController::Feedback);
}
