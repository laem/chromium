// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MockFetchContext_h
#define MockFetchContext_h

#include "platform/exported/WrappedResourceRequest.h"
#include "platform/loader/fetch/FetchContext.h"
#include "platform/loader/fetch/FetchParameters.h"
#include "platform/loader/fetch/ResourceTimingInfo.h"
#include "platform/scheduler/public/frame_scheduler.h"
#include "platform/scheduler/test/fake_frame_scheduler.h"
#include "platform/scheduler/test/fake_task_runner.h"
#include "platform/wtf/Optional.h"
#include "public/platform/Platform.h"
#include "public/platform/WebURLLoaderFactory.h"

#include <memory>

namespace blink {

class KURL;
class ResourceRequest;
struct ResourceLoaderOptions;

// Mocked FetchContext for testing.
class MockFetchContext : public FetchContext {
 public:
  enum LoadPolicy {
    kShouldLoadNewResource,
    kShouldNotLoadNewResource,
  };
  static MockFetchContext* Create(LoadPolicy load_policy,
                                  scoped_refptr<base::SingleThreadTaskRunner>
                                      loading_task_runner = nullptr) {
    return new MockFetchContext(load_policy, std::move(loading_task_runner));
  }

  ~MockFetchContext() override = default;

  void SetLoadComplete(bool complete) { complete_ = complete; }
  long long GetTransferSize() const { return transfer_size_; }

  const SecurityOrigin* GetSecurityOrigin() const override {
    return security_origin_.get();
  }

  void SetSecurityOrigin(scoped_refptr<const SecurityOrigin> security_origin) {
    security_origin_ = security_origin;
  }

  // The last ResourceRequest passed to DispatchWillSendRequest.
  WTF::Optional<ResourceRequest> RequestFromWillSendRequest() const {
    return will_send_request_;
  }

  // FetchContext:
  void DispatchWillSendRequest(
      unsigned long identifier,
      ResourceRequest& request,
      const ResourceResponse& redirect_response,
      Resource::Type,
      const FetchInitiatorInfo& = FetchInitiatorInfo()) override {
    will_send_request_ = request;
  }
  bool AllowImage(bool images_enabled, const KURL&) const override {
    return true;
  }
  ResourceRequestBlockedReason CanRequest(
      Resource::Type,
      const ResourceRequest&,
      const KURL&,
      const ResourceLoaderOptions&,
      SecurityViolationReportingPolicy,
      FetchParameters::OriginRestriction,
      ResourceRequest::RedirectStatus redirect_status) const override {
    return ResourceRequestBlockedReason::kNone;
  }
  ResourceRequestBlockedReason CheckCSPForRequest(
      WebURLRequest::RequestContext,
      const KURL& url,
      const ResourceLoaderOptions& options,
      SecurityViolationReportingPolicy reporting_policy,
      ResourceRequest::RedirectStatus redirect_status) const override {
    return ResourceRequestBlockedReason::kNone;
  }
  virtual ResourceRequestBlockedReason CheckResponseNosniff(
      WebURLRequest::RequestContext,
      const ResourceResponse&) const {
    return ResourceRequestBlockedReason::kNone;
  }
  bool ShouldLoadNewResource(Resource::Type) const override {
    return load_policy_ == kShouldLoadNewResource;
  }
  bool IsLoadComplete() const override { return complete_; }
  void AddResourceTiming(
      const ResourceTimingInfo& resource_timing_info) override {
    transfer_size_ = resource_timing_info.TransferSize();
  }

  std::unique_ptr<WebURLLoader> CreateURLLoader(
      const ResourceRequest& request,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner,
      const ResourceLoaderOptions&) override {
    if (!url_loader_factory_) {
      url_loader_factory_ =
          Platform::Current()->CreateDefaultURLLoaderFactory();
    }
    WrappedResourceRequest wrapped(request);
    return url_loader_factory_->CreateURLLoader(wrapped, task_runner);
  }

  ResourceLoadScheduler::ThrottlingPolicy InitialLoadThrottlingPolicy()
      const override {
    return ResourceLoadScheduler::ThrottlingPolicy::kTight;
  }

  FrameScheduler* GetFrameScheduler() const override {
    return frame_scheduler_.get();
  }

  scoped_refptr<base::SingleThreadTaskRunner> GetLoadingTaskRunner() override {
    return frame_scheduler_->GetTaskRunner(TaskType::kInternalTest);
  }

 private:
  class MockFrameScheduler final : public scheduler::FakeFrameScheduler {
   public:
    explicit MockFrameScheduler(
        scoped_refptr<base::SingleThreadTaskRunner> runner)
        : runner_(std::move(runner)) {}
    scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner(
        TaskType) override {
      return runner_;
    }

   private:
    scoped_refptr<base::SingleThreadTaskRunner> runner_;
  };

  MockFetchContext(
      LoadPolicy load_policy,
      scoped_refptr<base::SingleThreadTaskRunner> loading_task_runner)
      : load_policy_(load_policy),
        runner_(loading_task_runner
                    ? std::move(loading_task_runner)
                    : base::MakeRefCounted<scheduler::FakeTaskRunner>()),
        security_origin_(SecurityOrigin::CreateUnique()),
        frame_scheduler_(new MockFrameScheduler(runner_)),
        complete_(false),
        transfer_size_(-1) {}

  enum LoadPolicy load_policy_;
  scoped_refptr<base::SingleThreadTaskRunner> runner_;
  scoped_refptr<const SecurityOrigin> security_origin_;
  std::unique_ptr<FrameScheduler> frame_scheduler_;
  std::unique_ptr<WebURLLoaderFactory> url_loader_factory_;
  bool complete_;
  long long transfer_size_;
  WTF::Optional<ResourceRequest> will_send_request_;
};

}  // namespace blink

#endif
