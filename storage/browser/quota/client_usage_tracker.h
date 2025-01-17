// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_QUOTA_CLIENT_USAGE_TRACKER_H_
#define STORAGE_BROWSER_QUOTA_CLIENT_USAGE_TRACKER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/sequence_checker.h"
#include "storage/browser/quota/quota_callbacks.h"
#include "storage/browser/quota/quota_client.h"
#include "storage/browser/quota/quota_task.h"
#include "storage/browser/quota/special_storage_policy.h"
#include "storage/browser/storage_browser_export.h"
#include "third_party/WebKit/public/mojom/quota/quota_types.mojom.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace storage {

class StorageMonitor;
class UsageTracker;

// This class holds per-client usage tracking information and caches per-host
// usage data.  An instance of this class is created per client.
class ClientUsageTracker : public SpecialStoragePolicy::Observer,
                           public base::SupportsWeakPtr<ClientUsageTracker> {
 public:
  using HostUsageAccumulator =
      base::RepeatingCallback<void(int64_t limited_usage,
                                   int64_t unlimited_usage)>;
  using OriginUsageAccumulator =
      base::RepeatingCallback<void(const GURL& origin, int64_t usage)>;
  using UsageAccumulator = base::RepeatingCallback<void(int64_t usage)>;
  using OriginSetByHost = std::map<std::string, std::set<GURL>>;

  using HostUsageCallback =
      base::OnceCallback<void(int64_t limited_usage, int64_t unlimited_usage)>;

  ClientUsageTracker(UsageTracker* tracker,
                     QuotaClient* client,
                     blink::mojom::StorageType type,
                     SpecialStoragePolicy* special_storage_policy,
                     StorageMonitor* storage_monitor);
  ~ClientUsageTracker() override;

  void GetGlobalLimitedUsage(UsageCallback callback);
  void GetGlobalUsage(GlobalUsageCallback callback);
  void GetHostUsage(const std::string& host, UsageCallback callback);
  void UpdateUsageCache(const GURL& origin, int64_t delta);
  int64_t GetCachedUsage() const;
  void GetCachedHostsUsage(std::map<std::string, int64_t>* host_usage) const;
  void GetCachedOriginsUsage(std::map<GURL, int64_t>* origin_usage) const;
  void GetCachedOrigins(std::set<GURL>* origins) const;
  bool IsUsageCacheEnabledForOrigin(const GURL& origin) const;
  void SetUsageCacheEnabled(const GURL& origin, bool enabled);

 private:
  using HostUsageAccumulatorMap =
      CallbackQueueMap<HostUsageCallback, std::string, int64_t, int64_t>;

  using HostSet = std::set<std::string>;
  using UsageMap = std::map<GURL, int64_t>;
  using HostUsageMap = std::map<std::string, UsageMap>;

  struct AccumulateInfo {
    int pending_jobs;
    int64_t limited_usage;
    int64_t unlimited_usage;

    AccumulateInfo()
        : pending_jobs(0), limited_usage(0), unlimited_usage(0) {}
  };

  void AccumulateLimitedOriginUsage(AccumulateInfo* info,
                                    UsageCallback callback,
                                    int64_t usage);
  void DidGetOriginsForGlobalUsage(GlobalUsageCallback callback,
                                   const std::set<url::Origin>& origins);
  void AccumulateHostUsage(AccumulateInfo* info,
                           GlobalUsageCallback callback,
                           int64_t limited_usage,
                           int64_t unlimited_usage);

  void DidGetOriginsForHostUsage(const std::string& host,
                                 const std::set<url::Origin>& origins);

  void GetUsageForOrigins(const std::string& host,
                          const std::set<url::Origin>& origins);
  void AccumulateOriginUsage(AccumulateInfo* info,
                             const std::string& host,
                             const GURL& origin,
                             int64_t usage);

  void DidGetHostUsageAfterUpdate(const GURL& origin, int64_t usage);

  // Methods used by our GatherUsage tasks, as a task makes progress
  // origins and hosts are added incrementally to the cache.
  void AddCachedOrigin(const GURL& origin, int64_t usage);
  void AddCachedHost(const std::string& host);

  int64_t GetCachedHostUsage(const std::string& host) const;
  int64_t GetCachedGlobalUnlimitedUsage();
  bool GetCachedOriginUsage(const GURL& origin, int64_t* usage) const;

  // SpecialStoragePolicy::Observer overrides
  void OnGranted(const GURL& origin, int change_flags) override;
  void OnRevoked(const GURL& origin, int change_flags) override;
  void OnCleared() override;

  void UpdateGlobalUsageValue(int64_t* usage_value, int64_t delta);

  bool IsStorageUnlimited(const GURL& origin) const;

  UsageTracker* tracker_;
  QuotaClient* client_;
  const blink::mojom::StorageType type_;
  StorageMonitor* storage_monitor_;

  int64_t global_limited_usage_;
  int64_t global_unlimited_usage_;
  bool global_usage_retrieved_;
  HostSet cached_hosts_;
  HostUsageMap cached_usage_by_host_;

  OriginSetByHost non_cached_limited_origins_by_host_;
  OriginSetByHost non_cached_unlimited_origins_by_host_;

  HostUsageAccumulatorMap host_usage_accumulators_;

  scoped_refptr<SpecialStoragePolicy> special_storage_policy_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(ClientUsageTracker);
};

}  // namespace storage

#endif  // STORAGE_BROWSER_QUOTA_CLIENT_USAGE_TRACKER_H_
