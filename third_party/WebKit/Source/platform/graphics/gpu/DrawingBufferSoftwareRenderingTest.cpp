// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/graphics/gpu/DrawingBuffer.h"

#include "cc/resources/shared_bitmap_id_registrar.h"
#include "components/viz/common/resources/single_release_callback.h"
#include "components/viz/common/resources/transferable_resource.h"
#include "gpu/command_buffer/client/gles2_interface_stub.h"
#include "gpu/config/gpu_feature_info.h"
#include "platform/graphics/gpu/DrawingBufferTestHelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

// These unit tests are separate from DrawingBufferTests.cpp because they are
// built as a part of webkit_unittests instead blink_platform_unittests. This is
// because the software rendering mode has a dependency on the blink::Platform
// interface for buffer allocations.

namespace blink {
namespace {

class TestSharedBitmapIdRegistar : public cc::SharedBitmapIdRegistrar {
  virtual cc::SharedBitmapIdRegistration RegisterSharedBitmapId(
      const viz::SharedBitmapId& id,
      scoped_refptr<cc::CrossThreadSharedBitmap> bitmap) {
    return {};
  }
};

class DrawingBufferSoftwareCompositingTest : public testing::Test {
 protected:
  void SetUp() override {
    IntSize initial_size(kInitialWidth, kInitialHeight);
    auto gl = std::make_unique<GLES2InterfaceForTests>();
    auto provider =
        std::make_unique<WebGraphicsContext3DProviderForTests>(std::move(gl));
    GLES2InterfaceForTests* gl_ =
        static_cast<GLES2InterfaceForTests*>(provider->ContextGL());
    bool gpu_compositing = false;
    drawing_buffer_ = DrawingBufferForTests::Create(
        std::move(provider), gpu_compositing, gl_, initial_size,
        DrawingBuffer::kPreserve, kDisableMultisampling);
    CHECK(drawing_buffer_);
  }

  scoped_refptr<DrawingBufferForTests> drawing_buffer_;
  TestSharedBitmapIdRegistar test_shared_bitmap_id_registrar_;
};

TEST_F(DrawingBufferSoftwareCompositingTest, BitmapRecycling) {
  viz::TransferableResource resource;
  std::unique_ptr<viz::SingleReleaseCallback> release_callback1;
  std::unique_ptr<viz::SingleReleaseCallback> release_callback2;
  std::unique_ptr<viz::SingleReleaseCallback> release_callback3;
  IntSize initial_size(kInitialWidth, kInitialHeight);
  IntSize alternate_size(kInitialWidth, kAlternateHeight);

  drawing_buffer_->Resize(initial_size);
  drawing_buffer_->MarkContentsChanged();
  drawing_buffer_->PrepareTransferableResource(
      &test_shared_bitmap_id_registrar_, &resource,
      &release_callback1);  // create a bitmap.
  EXPECT_EQ(0, drawing_buffer_->RecycledBitmapCount());
  release_callback1->Run(
      gpu::SyncToken(),
      false /* lostResource */);  // release bitmap to the recycling queue
  EXPECT_EQ(1, drawing_buffer_->RecycledBitmapCount());
  drawing_buffer_->MarkContentsChanged();
  drawing_buffer_->PrepareTransferableResource(
      &test_shared_bitmap_id_registrar_, &resource,
      &release_callback2);  // recycle a bitmap.
  EXPECT_EQ(0, drawing_buffer_->RecycledBitmapCount());
  release_callback2->Run(
      gpu::SyncToken(),
      false /* lostResource */);  // release bitmap to the recycling queue
  EXPECT_EQ(1, drawing_buffer_->RecycledBitmapCount());
  drawing_buffer_->Resize(alternate_size);
  drawing_buffer_->MarkContentsChanged();
  // Regression test for crbug.com/647896 - Next line must not crash
  drawing_buffer_->PrepareTransferableResource(
      &test_shared_bitmap_id_registrar_, &resource,
      &release_callback3);  // cause recycling queue to be purged due to resize
  EXPECT_EQ(0, drawing_buffer_->RecycledBitmapCount());
  release_callback3->Run(gpu::SyncToken(), false /* lostResource */);
  EXPECT_EQ(1, drawing_buffer_->RecycledBitmapCount());

  drawing_buffer_->BeginDestruction();
}

TEST_F(DrawingBufferSoftwareCompositingTest, FramebufferBinding) {
  GLES2InterfaceForTests* gl_ = drawing_buffer_->ContextGLForTests();
  viz::TransferableResource resource;
  std::unique_ptr<viz::SingleReleaseCallback> release_callback;
  IntSize initial_size(kInitialWidth, kInitialHeight);
  GLint drawBinding = 0, readBinding = 0;

  GLuint draw_framebuffer_binding = 0xbeef3;
  GLuint read_framebuffer_binding = 0xbeef4;
  gl_->BindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_binding);
  gl_->BindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_binding);
  gl_->SaveState();
  drawing_buffer_->Resize(initial_size);
  drawing_buffer_->MarkContentsChanged();
  drawing_buffer_->PrepareTransferableResource(
      &test_shared_bitmap_id_registrar_, &resource, &release_callback);
  gl_->GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawBinding);
  gl_->GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readBinding);
  EXPECT_EQ(static_cast<GLint>(draw_framebuffer_binding), drawBinding);
  EXPECT_EQ(static_cast<GLint>(read_framebuffer_binding), readBinding);
  release_callback->Run(gpu::SyncToken(), false /* lostResource */);

  drawing_buffer_->BeginDestruction();
}

}  // unnamed namespace
}  // blink
