// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/crostini/crostini_registry_service.h"

#include <stddef.h>

#include "base/macros.h"
#include "chrome/test/base/testing_profile.h"
#include "components/crx_file/id_util.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {

class CrostiniRegistryServiceTest : public testing::Test {
 public:
  CrostiniRegistryServiceTest() = default;

  // testing::Test:
  void SetUp() override {
    service_ = std::make_unique<CrostiniRegistryService>(&profile_);
  }

 protected:
  std::string GenerateAppId(const std::string& desktop_file_id) {
    return crx_file::id_util::GenerateId("crostini:" + desktop_file_id);
  }

  CrostiniRegistryService* service() { return service_.get(); }

 private:
  content::TestBrowserThreadBundle thread_bundle_;
  TestingProfile profile_;

  std::unique_ptr<CrostiniRegistryService> service_;

  DISALLOW_COPY_AND_ASSIGN(CrostiniRegistryServiceTest);
};

TEST_F(CrostiniRegistryServiceTest, SetAndGetRegistration) {
  std::string desktop_file_id = "vim";
  std::string name = "Vim";
  std::string app_id = GenerateAppId(desktop_file_id);
  EXPECT_EQ(nullptr, service()->GetRegistration(app_id));

  CrostiniRegistryService::Registration registration(desktop_file_id, name);
  service()->SetRegistration(registration);
  auto result = service()->GetRegistration(app_id);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->desktop_file_id, desktop_file_id);
  EXPECT_EQ(result->name, name);
}

}  // namespace chromeos
