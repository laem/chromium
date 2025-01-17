// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BlinkLeakDetector_h
#define BlinkLeakDetector_h

#include "core/CoreExport.h"
#include "platform/Timer.h"

namespace blink {

class BlinkLeakDetectorClient;
class WebFrame;
class ResourceFetcher;

// This class is responsible for stabilizing the detection results which are
// InstanceCounter values by 1) preparing for leak detection and 2) operating
// garbage collections before leak detection.
// This class is implemented as singleton, but there needs to be only one user
// at a time.
class CORE_EXPORT BlinkLeakDetector {
 public:
  static BlinkLeakDetector& Instance();
  ~BlinkLeakDetector();

  void PrepareForLeakDetection(WebFrame*);
  void CollectGarbage();
  void SetClient(BlinkLeakDetectorClient*);

  // This registration function needs to be called whenever ResourceFetcher is
  // created (i.e. ResourceFetcher::Create is called).
  void RegisterResourceFetcher(ResourceFetcher*);

 private:
  BlinkLeakDetector();
  void TimerFiredGC(TimerBase*);

  TaskRunnerTimer<BlinkLeakDetector> delayed_gc_timer_;
  int number_of_gc_needed_;
  BlinkLeakDetectorClient* client_;
  PersistentHeapHashSet<WeakMember<ResourceFetcher>> resource_fetchers_;
};
}  // namespace blink

#endif  // LeakDetector_h
