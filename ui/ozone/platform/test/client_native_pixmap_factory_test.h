// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_TEST_CLIENT_NATIVE_PIXMAP_FACTORY_TEST_H_
#define UI_OZONE_PLATFORM_TEST_CLIENT_NATIVE_PIXMAP_FACTORY_TEST_H_

namespace ui {

class ClientNativePixmapFactory;

// Constructor hook for use in constructor_list.cc
ClientNativePixmapFactory* CreateClientNativePixmapFactoryTest();

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_TEST_CLIENT_NATIVE_PIXMAP_FACTORY_TEST_H_
