// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/declarative_net_request/declarative_net_request_api.h"

#include <utility>

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/api/declarative_net_request/ruleset_manager.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/info_map.h"
#include "extensions/common/api/declarative_net_request.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/url_pattern.h"
#include "extensions/common/url_pattern_set.h"

namespace extensions {

DeclarativeNetRequestUpdateWhitelistedPagesFunction::
    DeclarativeNetRequestUpdateWhitelistedPagesFunction() = default;
DeclarativeNetRequestUpdateWhitelistedPagesFunction::
    ~DeclarativeNetRequestUpdateWhitelistedPagesFunction() = default;

ExtensionFunction::ResponseAction
DeclarativeNetRequestUpdateWhitelistedPagesFunction::UpdateWhitelistedPages(
    const std::vector<std::string>& patterns,
    Action action) {
  if (patterns.empty())
    return RespondNow(NoArguments());

  // It's ok to allow file access and to use SCHEME_ALL since this is not
  // actually granting any permissions to the extension. This will only be used
  // to whitelist requests.
  URLPatternSet delta;
  std::string error;
  if (!delta.Populate(patterns, URLPattern::SCHEME_ALL,
                      true /*allow_file_access*/, &error)) {
    return RespondNow(Error(error));
  }

  ExtensionPrefs* prefs = ExtensionPrefs::Get(browser_context());
  URLPatternSet current_set = prefs->GetDNRWhitelistedPages(extension_id());
  URLPatternSet new_set;
  switch (action) {
    case Action::ADD:
      new_set = URLPatternSet::CreateUnion(current_set, delta);
      break;
    case Action::REMOVE:
      new_set = URLPatternSet::CreateDifference(current_set, delta);
      break;
  }

  // TODO(crbug.com/811460): Impost a limit on the number of patterns that can
  // be whitelisted by an extension.

  // Persist |new_set| as part of preferences.
  prefs->SetDNRWhitelistedPages(extension_id(), new_set);

  return RespondNow(NoArguments());
}

DeclarativeNetRequestAddWhitelistedPagesFunction::
    DeclarativeNetRequestAddWhitelistedPagesFunction() = default;
DeclarativeNetRequestAddWhitelistedPagesFunction::
    ~DeclarativeNetRequestAddWhitelistedPagesFunction() = default;

ExtensionFunction::ResponseAction
DeclarativeNetRequestAddWhitelistedPagesFunction::Run() {
  using Params = api::declarative_net_request::AddWhitelistedPages::Params;

  std::unique_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  return UpdateWhitelistedPages(params->page_patterns, Action::ADD);
}

DeclarativeNetRequestRemoveWhitelistedPagesFunction::
    DeclarativeNetRequestRemoveWhitelistedPagesFunction() = default;
DeclarativeNetRequestRemoveWhitelistedPagesFunction::
    ~DeclarativeNetRequestRemoveWhitelistedPagesFunction() = default;

ExtensionFunction::ResponseAction
DeclarativeNetRequestRemoveWhitelistedPagesFunction::Run() {
  using Params = api::declarative_net_request::AddWhitelistedPages::Params;

  std::unique_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  return UpdateWhitelistedPages(params->page_patterns, Action::REMOVE);
}

DeclarativeNetRequestGetWhitelistedPagesFunction::
    DeclarativeNetRequestGetWhitelistedPagesFunction() = default;
DeclarativeNetRequestGetWhitelistedPagesFunction::
    ~DeclarativeNetRequestGetWhitelistedPagesFunction() = default;

ExtensionFunction::ResponseAction
DeclarativeNetRequestGetWhitelistedPagesFunction::Run() {
  const ExtensionPrefs* prefs = ExtensionPrefs::Get(browser_context());
  URLPatternSet current_set = prefs->GetDNRWhitelistedPages(extension_id());

  return RespondNow(ArgumentList(
      api::declarative_net_request::GetWhitelistedPages::Results::Create(
          *current_set.ToStringVector())));
}

}  // namespace extensions
