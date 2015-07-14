// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Simple success test: we want content-script APIs to be available (like
// sendRequest), but other APIs to be undefined or throw exceptions on access.

chrome.test.getConfig(function(config) {
  'use strict';

  let success = true;
  let isSitePerProcess = config.sitePerProcess;

  // chrome.storage should exist, since the extension has the permission, and
  // the storage api is allowed in content scripts.
  if (!chrome.storage) {
    console.log("Error: chrome.storage doesn't exist; it should");
    success = false;
  }

  let checkPrivilegedApi = function(api, name) {
    if (api && !isSitePerProcess) {
      console.log("Error: " + name +
                  " exists, but shouldn't without site-per-process.");
      return false;
    }
    if (!api && isSitePerProcess) {
      console.log("Error: " + name +
                  " doesn't exist, but should with site-per-process.");
      return false;
    }
    return true;
  };

  // For other permissions, they should only be available if this is a trusted
  // extension process - which is the case only with site-per-process.
  // The whole of chrome.bookmarks (arbitrary unprivileged) is unavailable
  // without site-per-process.
  success = success && checkPrivilegedApi(chrome.bookmarks, 'chrome.bookmarks');
  // We test chrome.tabs as a special case, because it's a dependency of the
  // partially unprivileged chrome.extension.
  success = success && checkPrivilegedApi(chrome.tabs, 'chrome.tabs');
  // And parts of chrome.extension should be unavailable to unprivileged
  // contexts.
  success = success && checkPrivilegedApi(chrome.extension.getViews,
                                          'chrome.extension.getViews');

  chrome.extension.sendRequest({success: success});
});
