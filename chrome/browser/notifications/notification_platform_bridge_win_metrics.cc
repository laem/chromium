// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/notifications/notification_platform_bridge_win_metrics.h"

#include "base/metrics/histogram_macros.h"

namespace notifications_uma {

void LogDisplayHistogram(DisplayStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.DisplayStatus", status,
                            DisplayStatus::COUNT);
}

void LogCloseHistogram(CloseStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.CloseStatus", status,
                            CloseStatus::COUNT);
}

void LogHistoryHistogram(HistoryStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.HistoryStatus", status,
                            HistoryStatus::COUNT);
}

void LogGetDisplayedStatus(GetDisplayedStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.GetDisplayedStatus", status,
                            GetDisplayedStatus::COUNT);
}

void LogGetDisplayedLaunchIdStatus(GetDisplayedLaunchIdStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.GetDisplayedLaunchIdStatus",
                            status, GetDisplayedLaunchIdStatus::COUNT);
}

void LogGetNotificationLaunchIdStatus(GetNotificationLaunchIdStatus status) {
  UMA_HISTOGRAM_ENUMERATION(
      "Notifications.Windows.GetNotificationLaunchIdStatus", status,
      GetNotificationLaunchIdStatus::COUNT);
}

void LogHandleEventStatus(HandleEventStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.HandleEventStatus", status,
                            HandleEventStatus::COUNT);
}

void LogActivationStatus(ActivationStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.ActivationStatus", status,
                            ActivationStatus::COUNT);
}

void LogSetReadyCallbackStatus(SetReadyCallbackStatus status) {
  UMA_HISTOGRAM_ENUMERATION("Notifications.Windows.SetReadyCallbackStatus",
                            status, SetReadyCallbackStatus::COUNT);
}

}  // namespace notifications_uma
