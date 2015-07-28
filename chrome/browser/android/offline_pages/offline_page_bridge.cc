// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/offline_pages/offline_page_bridge.h"

#include "base/android/jni_string.h"
#include "chrome/browser/android/offline_pages/offline_page_model_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_android.h"
#include "components/offline_pages/offline_page_item.h"
#include "components/offline_pages/offline_page_model.h"
#include "content/public/browser/browser_context.h"
#include "jni/OfflinePageBridge_jni.h"

using base::android::ConvertUTF16ToJavaString;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaGlobalRef;

namespace offline_pages {
namespace android {

namespace {

void LoadAllPagesCallback(ScopedJavaGlobalRef<jobject>* j_callback_obj,
                          ScopedJavaGlobalRef<jobject>* j_result_obj,
                          OfflinePageModel::LoadResult result,
                          const std::vector<OfflinePageItem>& offline_pages) {
  JNIEnv* env = base::android::AttachCurrentThread();

  scoped_ptr<ScopedJavaGlobalRef<jobject>> j_callback_ptr(j_callback_obj);
  scoped_ptr<ScopedJavaGlobalRef<jobject>> j_result_ptr(j_result_obj);

  for (const OfflinePageItem& offline_page : offline_pages) {
    Java_OfflinePageBridge_createOfflinePageAndAddToList(
        env, j_result_ptr->obj(),
        ConvertUTF8ToJavaString(env, offline_page.url.spec()).obj(),
        ConvertUTF16ToJavaString(env, offline_page.title).obj(),
        ConvertUTF8ToJavaString(env, offline_page.GetOfflineURL().spec()).obj(),
        offline_page.file_size);
  }

  Java_OfflinePageCallback_onLoadAllPagesDone(env, j_callback_ptr->obj(),
                                              static_cast<int>(result),
                                              j_result_ptr->obj());
}

}  // namespace

OfflinePageBridge::OfflinePageBridge(JNIEnv* env,
                                     jobject obj,
                                     content::BrowserContext* browser_context)
    : weak_java_ref_(env, obj),
      offline_page_model_(OfflinePageModelFactory::GetForBrowserContext(
          browser_context)),
      browser_context_(browser_context) {
}

void OfflinePageBridge::Destroy(JNIEnv*, jobject) {
  delete this;
}

void OfflinePageBridge::LoadAllPages(JNIEnv* env,
                                     jobject obj,
                                     jobject j_callback_obj,
                                     jobject j_result_obj) {
  DCHECK(j_callback_obj);
  DCHECK(j_result_obj);

  scoped_ptr<ScopedJavaGlobalRef<jobject>> j_callback_obj_ptr(
      new ScopedJavaGlobalRef<jobject>());
  j_callback_obj_ptr->Reset(env, j_callback_obj);

  scoped_ptr<ScopedJavaGlobalRef<jobject>> j_result_obj_ptr(
      new ScopedJavaGlobalRef<jobject>());
  j_result_obj_ptr->Reset(env, j_result_obj);

  offline_page_model_->LoadAllPages(base::Bind(&LoadAllPagesCallback,
                                               j_callback_obj_ptr.release(),
                                               j_result_obj_ptr.release()));
}

static jlong Init(JNIEnv* env, jobject obj, jobject j_profile) {
  return reinterpret_cast<jlong>(new OfflinePageBridge(
      env, obj, ProfileAndroid::FromProfileAndroid(j_profile)));
}

bool RegisterOfflinePageBridge(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace android
}  // namespace offline_pages
