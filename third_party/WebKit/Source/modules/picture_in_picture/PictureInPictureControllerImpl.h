// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PictureInPictureControllerImpl_h
#define PictureInPictureControllerImpl_h

#include "core/frame/PictureInPictureController.h"

namespace blink {

class HTMLVideoElement;
class PictureInPictureWindow;

// The PictureInPictureControllerImpl is keeping the state and implementing the
// logic around the Picture-in-Picture feature. It is meant to be used as well
// by the Picture-in-Picture Web API and internally (eg. media controls). All
// consumers inside Blink modules/ should use this class to access
// Picture-in-Picture. In core/, they should use PictureInPictureController.
// PictureInPictureControllerImpl instance is associated to a Document. It is
// supplement and therefore can be lazy-initiated. Callers should consider
// whether they want to instantiate an object when they make a call.
class PictureInPictureControllerImpl : public PictureInPictureController {
  USING_GARBAGE_COLLECTED_MIXIN(PictureInPictureControllerImpl);
  WTF_MAKE_NONCOPYABLE(PictureInPictureControllerImpl);

 public:
  ~PictureInPictureControllerImpl() override;

  // Meant to be called internally by PictureInPictureController::From()
  // through ModulesInitializer.
  static PictureInPictureControllerImpl* Create(Document&);

  // Gets, or creates, PictureInPictureControllerImpl supplement on Document.
  // Should be called before any other call to make sure a document is attached.
  static PictureInPictureControllerImpl& From(Document&);

  // Returns whether system allows Picture-in-Picture feature or not for
  // the associated document.
  bool PictureInPictureEnabled() const;

  // Returns whether the document associated with the controller is allowed to
  // request Picture-in-Picture.
  Status IsDocumentAllowed() const;

  // Implementation of PictureInPictureController.
  Status IsElementAllowed(const HTMLVideoElement&) const override;

  // Meant to be called by HTMLVideoElementPictureInPicture and DOM objects
  // but not internally.
  void SetPictureInPictureElement(HTMLVideoElement&);

  // Meant to be called by DocumentPictureInPicture,
  // HTMLVideoElementPictureInPicture, and DOM objects but not internally.
  void UnsetPictureInPictureElement();

  // Returns element currently in Picture-in-Picture if any. Null otherwise.
  Element* PictureInPictureElement(TreeScope&) const;

  // Meant to be called by HTMLVideoElementPictureInPicture, and DOM objects but
  // not internally. It closes the current Picture-in-Picture window if any.
  PictureInPictureWindow* CreatePictureInPictureWindow(int width, int height);

  // Meant to be called by DocumentPictureInPicture,
  // HTMLVideoElementPictureInPicture, and DOM objects but not internally.
  void OnClosePictureInPictureWindow();

  void Trace(blink::Visitor*) override;

 private:
  explicit PictureInPictureControllerImpl(Document&);

  // The Picture-in-Picture element for the associated document.
  Member<HTMLVideoElement> picture_in_picture_element_;

  // The Picture-in-Picture window for the associated document.
  Member<PictureInPictureWindow> picture_in_picture_window_;
};

}  // namespace blink

#endif  // PictureInPictureControllerImpl_h
