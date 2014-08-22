// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_TEST_ASYNC_FILE_TEST_HELPER_H_
#define CONTENT_PUBLIC_TEST_ASYNC_FILE_TEST_HELPER_H_

#include "base/basictypes.h"
#include "webkit/browser/fileapi/file_system_operation.h"
#include "webkit/common/fileapi/file_system_types.h"
#include "webkit/common/quota/quota_status_code.h"

namespace storage {
class QuotaManager;
}

namespace storage {
class FileSystemContext;
class FileSystemURL;
}

namespace content {

// A helper class to perform async file operations in a synchronous way.
class AsyncFileTestHelper {
 public:
  typedef storage::FileSystemOperation::FileEntryList FileEntryList;
  typedef storage::FileSystemOperation::CopyProgressCallback
      CopyProgressCallback;

  static const int64 kDontCheckSize;

  // Performs Copy from |src| to |dest| and returns the status code.
  static base::File::Error Copy(storage::FileSystemContext* context,
                                const storage::FileSystemURL& src,
                                const storage::FileSystemURL& dest);

  // Same as Copy, but this supports |progress_callback|.
  static base::File::Error CopyWithProgress(
      storage::FileSystemContext* context,
      const storage::FileSystemURL& src,
      const storage::FileSystemURL& dest,
      const CopyProgressCallback& progress_callback);

  // Performs Move from |src| to |dest| and returns the status code.
  static base::File::Error Move(storage::FileSystemContext* context,
                                const storage::FileSystemURL& src,
                                const storage::FileSystemURL& dest);

  // Removes the given |url|.
  static base::File::Error Remove(storage::FileSystemContext* context,
                                  const storage::FileSystemURL& url,
                                  bool recursive);

  // Performs ReadDirectory on |url|.
  static base::File::Error ReadDirectory(storage::FileSystemContext* context,
                                         const storage::FileSystemURL& url,
                                         FileEntryList* entries);

  // Creates a directory at |url|.
  static base::File::Error CreateDirectory(storage::FileSystemContext* context,
                                           const storage::FileSystemURL& url);

  // Creates a file at |url|.
  static base::File::Error CreateFile(storage::FileSystemContext* context,
                                      const storage::FileSystemURL& url);

  // Creates a file at |url| and fills with |buf|.
  static base::File::Error CreateFileWithData(
      storage::FileSystemContext* context,
      const storage::FileSystemURL& url,
      const char* buf,
      int buf_size);

  // Truncates the file |url| to |size|.
  static base::File::Error TruncateFile(storage::FileSystemContext* context,
                                        const storage::FileSystemURL& url,
                                        size_t size);

  // Retrieves File::Info for |url| and populates |file_info|.
  static base::File::Error GetMetadata(storage::FileSystemContext* context,
                                       const storage::FileSystemURL& url,
                                       base::File::Info* file_info);

  // Retrieves FilePath for |url| and populates |platform_path|.
  static base::File::Error GetPlatformPath(storage::FileSystemContext* context,
                                           const storage::FileSystemURL& url,
                                           base::FilePath* platform_path);

  // Returns true if a file exists at |url| with |size|. If |size| is
  // kDontCheckSize it doesn't check the file size (but just check its
  // existence).
  static bool FileExists(storage::FileSystemContext* context,
                         const storage::FileSystemURL& url,
                         int64 size);

  // Returns true if a directory exists at |url|.
  static bool DirectoryExists(storage::FileSystemContext* context,
                              const storage::FileSystemURL& url);

  // Returns usage and quota. It's valid to pass NULL to |usage| and/or |quota|.
  static storage::QuotaStatusCode GetUsageAndQuota(
      storage::QuotaManager* quota_manager,
      const GURL& origin,
      storage::FileSystemType type,
      int64* usage,
      int64* quota);
};

}  // namespace content

#endif  // CONTENT_PUBLIC_TEST_ASYNC_FILE_TEST_HELPER_H_
