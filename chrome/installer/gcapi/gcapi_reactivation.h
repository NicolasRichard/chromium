// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_INSTALLER_GCAPI_GCAPI_REACTIVATION_H_
#define CHROME_INSTALLER_GCAPI_GCAPI_REACTIVATION_H_
#pragma once

#include <string>
#include <vector>

bool HasBeenReactivatedByBrandCodes(
    const std::vector<std::wstring>& brand_codes);

bool SetReactivationBrandCode(const std::wstring& brand_code);

#endif  // CHROME_INSTALLER_GCAPI_GCAPI_REACTIVATION_H_
