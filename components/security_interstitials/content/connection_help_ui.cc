// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/security_interstitials/content/connection_help_ui.h"

#include "components/grit/components_resources.h"
#include "components/security_interstitials/content/urls.h"
#include "components/strings/grit/components_strings.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/base/l10n/l10n_util.h"

namespace security_interstitials {

ConnectionHelpUI::ConnectionHelpUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::Create(kChromeUIConnectionHelpHost);

  html_source->AddLocalizedString("connectionHelpTitle",
                                  IDS_CONNECTION_HELP_TITLE);
  html_source->AddLocalizedString("connectionHelpHeading",
                                  IDS_CONNECTION_HELP_HEADING);
  html_source->AddLocalizedString("connectionHelpGeneralHelp",
                                  IDS_CONNECTION_HELP_GENERAL_HELP);
  html_source->AddLocalizedString("connectionHelpSpecificErrorHeading",
                                  IDS_CONNECTION_HELP_SPECIFIC_ERROR_HEADING);
  html_source->AddLocalizedString(
      "connectionHelpConnectionNotPrivateTitle",
      IDS_CONNECTION_HELP_CONNECTION_NOT_PRIVATE_TITLE);
  html_source->AddLocalizedString(
      "connectionHelpConnectionNotPrivateDetails",
      IDS_CONNECTION_HELP_CONNECTION_NOT_PRIVATE_DETAILS);
  html_source->AddLocalizedString("connectionHelpConnectToNetworkTitle",
                                  IDS_CONNECTION_HELP_CONNECT_TO_NETWORK_TITLE);
  html_source->AddLocalizedString(
      "connectionHelpConnectToNetworkDetails",
      IDS_CONNECTION_HELP_CONNECT_TO_NETWORK_DETAILS);
  html_source->AddLocalizedString("connectionHelpIncorrectClockTitle",
                                  IDS_CONNECTION_HELP_INCORRECT_CLOCK_TITLE);
  html_source->AddLocalizedString("connectionHelpIncorrectClockDetails",
                                  IDS_CONNECTION_HELP_INCORRECT_CLOCK_DETAILS);
  html_source->AddLocalizedString("connectionHelpMitmSoftwareTitle",
                                  IDS_CONNECTION_HELP_MITM_SOFTWARE_TITLE);
  html_source->AddLocalizedString("connectionHelpMitmSoftwareDetails",
                                  IDS_CONNECTION_HELP_MITM_SOFTWARE_DETAILS);
  html_source->AddLocalizedString("connectionHelpShowMore",
                                  IDS_CONNECTION_HELP_SHOW_MORE);
  html_source->AddLocalizedString("connectionHelpShowLess",
                                  IDS_CONNECTION_HELP_SHOW_LESS);

  html_source->SetJsonPath("strings.js");

  html_source->AddResourcePath("interstitial_core.css",
                               IDR_SECURITY_INTERSTITIAL_CORE_CSS);
  html_source->AddResourcePath("interstitial_common.css",
                               IDR_SECURITY_INTERSTITIAL_COMMON_CSS);
  html_source->AddResourcePath("connection_help.css",
                               IDR_SECURITY_INTERSTITIAL_CONNECTION_HELP_CSS);
  html_source->AddResourcePath("connection_help.js",
                               IDR_SECURITY_INTERSTITIAL_CONNECTION_HELP_JS);
  html_source->SetDefaultResource(
      IDR_SECURITY_INTERSTITIAL_CONNECTION_HELP_HTML);
  html_source->UseGzip();

  content::BrowserContext* browser_context =
      web_ui->GetWebContents()->GetBrowserContext();
  content::WebUIDataSource::Add(browser_context, html_source);
}

ConnectionHelpUI::~ConnectionHelpUI() {}

}  // namespace security_interstitials
