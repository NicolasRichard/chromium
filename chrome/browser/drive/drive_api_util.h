// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DRIVE_DRIVE_API_UTIL_H_
#define CHROME_BROWSER_DRIVE_DRIVE_API_UTIL_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "google_apis/drive/drive_api_error_codes.h"
#include "google_apis/drive/drive_common_callbacks.h"

class GURL;

namespace base {
class FilePath;
class Value;
}  // namespace base

namespace google_apis {
class ChangeList;
class ChangeResource;
class FileList;
class FileResource;
class ResourceEntry;
}  // namespace google_apis

namespace drive {
namespace util {

// Google Apps MIME types:
const char kGoogleDocumentMimeType[] = "application/vnd.google-apps.document";
const char kGoogleDrawingMimeType[] = "application/vnd.google-apps.drawing";
const char kGooglePresentationMimeType[] =
    "application/vnd.google-apps.presentation";
const char kGoogleSpreadsheetMimeType[] =
    "application/vnd.google-apps.spreadsheet";
const char kGoogleTableMimeType[] = "application/vnd.google-apps.table";
const char kGoogleFormMimeType[] = "application/vnd.google-apps.form";
const char kGoogleMapMimeType[] = "application/vnd.google-apps.map";
const char kDriveFolderMimeType[] = "application/vnd.google-apps.folder";

// Escapes ' to \' in the |str|. This is designed to use for string value of
// search parameter on Drive API v2.
// See also: https://developers.google.com/drive/search-parameters
std::string EscapeQueryStringValue(const std::string& str);

// Parses the query, and builds a search query for Drive API v2.
// This only supports:
//   Regular query (e.g. dog => fullText contains 'dog')
//   Conjunctions
//     (e.g. dog cat => fullText contains 'dog' and fullText contains 'cat')
//   Exclusion query (e.g. -cat => not fullText contains 'cat').
//   Quoted query (e.g. "dog cat" => fullText contains 'dog cat').
// See also: https://developers.google.com/drive/search-parameters
std::string TranslateQuery(const std::string& original_query);

// If |resource_id| is in the old resource ID format used by WAPI, converts it
// into the new format.
std::string CanonicalizeResourceId(const std::string& resource_id);

// Returns the (base-16 encoded) MD5 digest of the file content at |file_path|,
// or an empty string if an error is found.
std::string GetMd5Digest(const base::FilePath& file_path);

// Returns preferred file extension for hosted documents which have given mime
// type.
std::string GetHostedDocumentExtension(const std::string& mime_type);

// Returns true if the given mime type is corresponding to one of known hosted
// document types.
bool IsKnownHostedDocumentMimeType(const std::string& mime_type);

// Returns true if the given file path has an extension corresponding to one of
// hosted document types.
bool HasHostedDocumentExtension(const base::FilePath& path);

}  // namespace util
}  // namespace drive

#endif  // CHROME_BROWSER_DRIVE_DRIVE_API_UTIL_H_
