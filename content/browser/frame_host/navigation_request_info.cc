// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/frame_host/navigation_request_info.h"
#include "content/common/service_worker/service_worker_types.h"
#include "mojo/common/values_struct_traits.h"

namespace content {

NavigationRequestInfo::NavigationRequestInfo(
    const CommonNavigationParams& common_params,
    mojom::BeginNavigationParamsPtr begin_params,
    const GURL& site_for_cookies,
    bool is_main_frame,
    bool parent_is_main_frame,
    bool are_ancestors_secure,
    int frame_tree_node_id,
    bool is_for_guests_only,
    bool report_raw_headers,
    bool is_prerendering)
    : common_params(common_params),
      begin_params(std::move(begin_params)),
      site_for_cookies(site_for_cookies),
      is_main_frame(is_main_frame),
      parent_is_main_frame(parent_is_main_frame),
      are_ancestors_secure(are_ancestors_secure),
      frame_tree_node_id(frame_tree_node_id),
      is_for_guests_only(is_for_guests_only),
      report_raw_headers(report_raw_headers),
      is_prerendering(is_prerendering) {}

NavigationRequestInfo::NavigationRequestInfo(const NavigationRequestInfo& other)
    : common_params(other.common_params),
      begin_params(other.begin_params.Clone()),
      site_for_cookies(other.site_for_cookies),
      is_main_frame(other.is_main_frame),
      parent_is_main_frame(other.parent_is_main_frame),
      are_ancestors_secure(other.are_ancestors_secure),
      frame_tree_node_id(other.frame_tree_node_id),
      is_for_guests_only(other.is_for_guests_only),
      report_raw_headers(other.report_raw_headers),
      is_prerendering(other.is_prerendering) {}

NavigationRequestInfo::~NavigationRequestInfo() {}

}  // namespace content
