// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OffscreenCanvasRenderingContext2D_h
#define OffscreenCanvasRenderingContext2D_h

#include <memory>
#include "core/html/canvas/CanvasContextCreationAttributesCore.h"
#include "core/html/canvas/CanvasRenderingContext.h"
#include "core/html/canvas/CanvasRenderingContextFactory.h"
#include "modules/canvas/canvas2d/BaseRenderingContext2D.h"

namespace blink {

class CanvasResourceProvider;
class Font;
class TextMetrics;

class MODULES_EXPORT OffscreenCanvasRenderingContext2D final
    : public CanvasRenderingContext,
      public BaseRenderingContext2D {
  DEFINE_WRAPPERTYPEINFO();
  USING_GARBAGE_COLLECTED_MIXIN(OffscreenCanvasRenderingContext2D);

 public:
  class Factory : public CanvasRenderingContextFactory {
   public:
    Factory() = default;
    ~Factory() override = default;

    CanvasRenderingContext* Create(
        CanvasRenderingContextHost* host,
        const CanvasContextCreationAttributesCore& attrs) override {
      DCHECK(host->IsOffscreenCanvas());
      return new OffscreenCanvasRenderingContext2D(
          static_cast<OffscreenCanvas*>(host), attrs);
    }

    CanvasRenderingContext::ContextType GetContextType() const override {
      return CanvasRenderingContext::kContext2d;
    }
  };

  OffscreenCanvas* offscreenCanvasForBinding() const {
    DCHECK(!Host() || Host()->IsOffscreenCanvas());
    return static_cast<OffscreenCanvas*>(Host());
  }
  ScriptPromise commit(ScriptState*, ExceptionState&);

  // CanvasRenderingContext implementation
  ~OffscreenCanvasRenderingContext2D() override;
  ContextType GetContextType() const override { return kContext2d; }
  bool Is2d() const override { return true; }
  bool IsComposited() const override { return false; }
  bool IsAccelerated() const override;
  void SetOffscreenCanvasGetContextResult(OffscreenRenderingContext&) final;
  void SetIsHidden(bool) final { NOTREACHED(); }
  void Stop() final { NOTREACHED(); }
  void SetCanvasGetContextResult(RenderingContext&) final {}
  void ClearRect(double x, double y, double width, double height) override {
    BaseRenderingContext2D::clearRect(x, y, width, height);
  }
  scoped_refptr<StaticBitmapImage> GetImage(AccelerationHint) const final;
  void Reset() override;
  void RestoreCanvasMatrixClipStack(PaintCanvas* c) const override {
    RestoreMatrixClipStack(c);
  }

  String font() const;
  void setFont(const String&) override;

  String direction() const;
  void setDirection(const String&);

  void fillText(const String& text, double x, double y);
  void fillText(const String& text, double x, double y, double max_width);
  void strokeText(const String& text, double x, double y);
  void strokeText(const String& text, double x, double y, double max_width);
  TextMetrics* measureText(const String& text);

  // BaseRenderingContext2D implementation
  bool OriginClean() const final;
  void SetOriginTainted() final;
  bool WouldTaintOrigin(CanvasImageSource*, ExecutionContext*) final;

  int Width() const final;
  int Height() const final;

  bool CanCreateCanvas2dResourceProvider() const final;
  CanvasResourceProvider* GetCanvasResourceProvider() const;

  bool ParseColorOrCurrentColor(Color&, const String& color_string) const final;

  PaintCanvas* DrawingCanvas() const final;
  PaintCanvas* ExistingDrawingCanvas() const final;
  void DisableDeferral(DisableDeferralReason) final;

  void DidDraw(const SkIRect& dirty_rect) final;  // overrides
                                                  // BaseRenderingContext2D and
                                                  // CanvasRenderingContext

  bool StateHasFilter() final;
  sk_sp<PaintFilter> StateGetFilter() final;
  void SnapshotStateForFilter() final {}

  void ValidateStateStack() const final;

  bool HasAlpha() const final { return CreationAttributes().alpha; }
  bool isContextLost() const override;

  ImageBitmap* TransferToImageBitmap(ScriptState*) final;

  virtual void Trace(blink::Visitor*);

 protected:
  OffscreenCanvasRenderingContext2D(
      OffscreenCanvas*,
      const CanvasContextCreationAttributesCore& attrs);

  virtual void NeedsFinalizeFrame() {
    CanvasRenderingContext::NeedsFinalizeFrame();
  }

  CanvasColorParams ColorParams() const override;
  bool WritePixels(const SkImageInfo& orig_info,
                   const void* pixels,
                   size_t row_bytes,
                   int x,
                   int y) override;

 private:
  bool IsPaintable() const final;
  bool IsCanvas2DBufferValid() const override;

  void DrawTextInternal(const String&,
                        double,
                        double,
                        CanvasRenderingContext2DState::PaintType,
                        double* max_width = nullptr);
  const Font& AccessFont();

  scoped_refptr<StaticBitmapImage> TransferToStaticBitmapImage();

  String ColorSpaceAsString() const override;
  CanvasPixelFormat PixelFormat() const override;
  SkIRect dirty_rect_for_commit_;
};

DEFINE_TYPE_CASTS(OffscreenCanvasRenderingContext2D,
                  CanvasRenderingContext,
                  context,
                  context->Is2d() && context->Host(),
                  context.Is2d() && context.Host());

}  // namespace blink

#endif  // OffscreenCanvasRenderingContext2D_h
