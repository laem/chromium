// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/filesystem/DOMFileSystemBase.h"

#include "core/fileapi/File.h"
#include "platform/testing/UnitTestHelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

class DOMFileSystemBaseTest : public testing::Test {
 public:
  DOMFileSystemBaseTest() {
    file_path_ = test::BlinkRootDir();
    file_path_.append("/Source/modules/filesystem/DOMFileSystemBaseTest.cpp");
    GetFileMetadata(file_path_, file_metadata_);
    file_metadata_.platform_path = file_path_;
  }

 protected:
  String file_path_;
  FileMetadata file_metadata_;
};

TEST_F(DOMFileSystemBaseTest, externalFilesystemFilesAreUserVisible) {
  KURL root_url = DOMFileSystemBase::CreateFileSystemRootURL(
      "http://chromium.org/", kFileSystemTypeExternal);

  File* file = DOMFileSystemBase::CreateFile(file_metadata_, root_url,
                                             kFileSystemTypeExternal,
                                             "DOMFileSystemBaseTest.cpp");
  EXPECT_TRUE(file);
  EXPECT_TRUE(file->HasBackingFile());
  EXPECT_EQ(File::kIsUserVisible, file->GetUserVisibility());
  EXPECT_EQ("DOMFileSystemBaseTest.cpp", file->name());
  EXPECT_EQ(file_path_, file->GetPath());
}

TEST_F(DOMFileSystemBaseTest, temporaryFilesystemFilesAreNotUserVisible) {
  KURL root_url = DOMFileSystemBase::CreateFileSystemRootURL(
      "http://chromium.org/", kFileSystemTypeTemporary);

  File* file = DOMFileSystemBase::CreateFile(file_metadata_, root_url,
                                             kFileSystemTypeTemporary,
                                             "UserVisibleName.txt");
  EXPECT_TRUE(file);
  EXPECT_TRUE(file->HasBackingFile());
  EXPECT_EQ(File::kIsNotUserVisible, file->GetUserVisibility());
  EXPECT_EQ("UserVisibleName.txt", file->name());
  EXPECT_EQ(file_path_, file->GetPath());
}

TEST_F(DOMFileSystemBaseTest, persistentFilesystemFilesAreNotUserVisible) {
  KURL root_url = DOMFileSystemBase::CreateFileSystemRootURL(
      "http://chromium.org/", kFileSystemTypePersistent);

  File* file = DOMFileSystemBase::CreateFile(file_metadata_, root_url,
                                             kFileSystemTypePersistent,
                                             "UserVisibleName.txt");
  EXPECT_TRUE(file);
  EXPECT_TRUE(file->HasBackingFile());
  EXPECT_EQ(File::kIsNotUserVisible, file->GetUserVisibility());
  EXPECT_EQ("UserVisibleName.txt", file->name());
  EXPECT_EQ(file_path_, file->GetPath());
}

}  // namespace blink
