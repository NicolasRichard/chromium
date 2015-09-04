// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/shortcut_helper.h"

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/manifest/manifest_icon_downloader.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "jni/ShortcutHelper_jni.h"
#include "ui/gfx/android/java_bitmap.h"
#include "url/gurl.h"

using content::Manifest;

// static
void ShortcutHelper::AddShortcutInBackgroundWithSkBitmap(
    const ShortcutInfo& info,
    const std::string& webapp_id,
    const SkBitmap& icon_bitmap) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  // Send the data to the Java side to create the shortcut.
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> java_webapp_id =
      base::android::ConvertUTF8ToJavaString(env, webapp_id);
  ScopedJavaLocalRef<jstring> java_url =
      base::android::ConvertUTF8ToJavaString(env, info.url.spec());
  ScopedJavaLocalRef<jstring> java_user_title =
      base::android::ConvertUTF16ToJavaString(env, info.user_title);
  ScopedJavaLocalRef<jstring> java_name =
      base::android::ConvertUTF16ToJavaString(env, info.name);
  ScopedJavaLocalRef<jstring> java_short_name =
      base::android::ConvertUTF16ToJavaString(env, info.short_name);
  ScopedJavaLocalRef<jobject> java_bitmap;
  if (icon_bitmap.getSize())
    java_bitmap = gfx::ConvertToJavaBitmap(&icon_bitmap);

  Java_ShortcutHelper_addShortcut(
      env,
      base::android::GetApplicationContext(),
      java_webapp_id.obj(),
      java_url.obj(),
      java_user_title.obj(),
      java_name.obj(),
      java_short_name.obj(),
      java_bitmap.obj(),
      info.display == blink::WebDisplayModeStandalone,
      info.orientation,
      info.source,
      info.theme_color,
      info.background_color);
}

// static
void ShortcutHelper::FetchSplashScreenImage(
    content::WebContents* web_contents,
    const GURL& image_url,
    const int ideal_splash_image_size_in_dp,
    const std::string& webapp_id) {
  // This is a fire and forget task. It is not vital for the splash screen image
  // to be downloaded so if the downloader returns false there is no fallback.
  ManifestIconDownloader::Download(
      web_contents,
      image_url,
      ideal_splash_image_size_in_dp,
      base::Bind(&ShortcutHelper::StoreWebappData, webapp_id));
}

// static
void ShortcutHelper::StoreWebappData(
    const std::string& webapp_id,
    const SkBitmap& splash_image) {
  if (splash_image.drawsNothing())
    return;

  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> java_webapp_id =
      base::android::ConvertUTF8ToJavaString(env, webapp_id);
  ScopedJavaLocalRef<jobject> java_splash_image =
      gfx::ConvertToJavaBitmap(&splash_image);

  Java_ShortcutHelper_storeWebappData(
      env,
      base::android::GetApplicationContext(),
      java_webapp_id.obj(),
      java_splash_image.obj());
}

bool ShortcutHelper::RegisterShortcutHelper(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
