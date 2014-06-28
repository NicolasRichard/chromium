// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/service_worker_manager.h"

#include "base/bind.h"
#include "base/callback.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/service_worker_context.h"
#include "content/public/browser/storage_partition.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/process_manager.h"
#include "extensions/common/manifest_handlers/background_info.h"

namespace extensions {

using base::Callback;
using base::Closure;
using base::WeakPtr;
using content::BrowserContext;
using content::BrowserThread;

ServiceWorkerManager::State::State()
    : registration(UNREGISTERED), outstanding_state_changes(0) {
}
ServiceWorkerManager::State::~State() {
}

ServiceWorkerManager::ServiceWorkerManager(BrowserContext* context)
    : context_(context), weak_this_factory_(this) {
}
ServiceWorkerManager::~ServiceWorkerManager() {
}

ServiceWorkerManager* ServiceWorkerManager::Get(
    content::BrowserContext* context) {
  return ServiceWorkerManagerFactory::GetForBrowserContext(context);
}

content::StoragePartition* ServiceWorkerManager::GetStoragePartition(
    const ExtensionId& ext_id) const {
  return content::BrowserContext::GetStoragePartitionForSite(
      context_, Extension::GetBaseURLFromExtensionId(ext_id));
}

content::ServiceWorkerContext* ServiceWorkerManager::GetSWContext(
    const ExtensionId& ext_id) const {
  return GetStoragePartition(ext_id)->GetServiceWorkerContext();
}

// alecflett says that if we send a series of RegisterServiceWorker and
// UnregisterServiceWorker calls on the same scope to a
// ServiceWorkerContextCore, we're guaranteed that the callbacks come back in
// the same order, and that the last one will be the final state.
void ServiceWorkerManager::RegisterExtension(const Extension* extension) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  CHECK(BackgroundInfo::HasServiceWorker(extension));
  State& ext_state = states_[extension->id()];
  if (ext_state.registration == REGISTERING ||
      ext_state.registration == REGISTERED)
    return;
  ext_state.registration = REGISTERING;
  ++ext_state.outstanding_state_changes;
  const GURL service_worker_script = extension->GetResourceURL(
      BackgroundInfo::GetServiceWorkerScript(extension));

  GetSWContext(extension->id())->RegisterServiceWorker(
      extension->GetResourceURL("/*"),
      service_worker_script,
      base::Bind(&ServiceWorkerManager::FinishRegistration,
                 WeakThis(),
                 extension->id()));
}

void ServiceWorkerManager::FinishRegistration(const ExtensionId& extension_id,
                                              bool success) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  State& ext_state = states_[extension_id];
  --ext_state.outstanding_state_changes;
  DCHECK_GE(ext_state.outstanding_state_changes, 0);
  if (ext_state.outstanding_state_changes > 0)
    return;

  DCHECK_EQ(ext_state.registration, REGISTERING);
  if (success) {
    ext_state.registration = REGISTERED;
    ext_state.registration_callbacks.RunSuccessCallbacksAndClear();
  } else {
    LOG(ERROR) << "Service Worker Registration failed for extension "
               << extension_id;
    ext_state.registration_callbacks.RunFailureCallbacksAndClear();
    states_.erase(extension_id);
  }
}

void ServiceWorkerManager::UnregisterExtension(const Extension* extension) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  CHECK(BackgroundInfo::HasServiceWorker(extension));

  base::hash_map<ExtensionId, State>::iterator it =
      states_.find(extension->id());
  if (it == states_.end()) {
    // Extension isn't registered.
    return;
  }
  State& ext_state = it->second;
  if (ext_state.registration == UNREGISTERING)
    return;

  ext_state.registration = UNREGISTERING;
  ++ext_state.outstanding_state_changes;

  GetSWContext(extension->id())->UnregisterServiceWorker(
      extension->GetResourceURL("/*"),
      base::Bind(&ServiceWorkerManager::FinishUnregistration,
                 WeakThis(),
                 extension->id()));
}

void ServiceWorkerManager::FinishUnregistration(const ExtensionId& extension_id,
                                                bool success) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  State& ext_state = states_[extension_id];
  --ext_state.outstanding_state_changes;
  DCHECK_GE(ext_state.outstanding_state_changes, 0);
  if (ext_state.outstanding_state_changes > 0)
    return;

  DCHECK_EQ(ext_state.registration, UNREGISTERING);
  if (success) {
    ext_state.unregistration_callbacks.RunSuccessCallbacksAndClear();
    states_.erase(extension_id);
  } else {
    LOG(ERROR) << "Service Worker Unregistration failed for extension "
               << extension_id;
    ext_state.registration = REGISTERED;
    ext_state.unregistration_callbacks.RunFailureCallbacksAndClear();
  }
}

void ServiceWorkerManager::WhenRegistered(
    const Extension* extension,
    const tracked_objects::Location& from_here,
    const base::Closure& success,
    const base::Closure& failure) {
  base::hash_map<ExtensionId, State>::iterator it =
      states_.find(extension->id());
  if (it == states_.end()) {
    base::MessageLoop::current()->PostTask(from_here, failure);
    return;
  }

  State& state = it->second;
  switch (state.registration) {
    case UNREGISTERED:
    case UNREGISTERING:
      base::MessageLoop::current()->PostTask(from_here, failure);
      break;
    case REGISTERED:
      base::MessageLoop::current()->PostTask(from_here, success);
      break;
    case REGISTERING:
      state.registration_callbacks.push_back(
          SuccessFailureClosurePair(success, failure));
      break;
  }
}

void ServiceWorkerManager::WhenUnregistered(
    const Extension* extension,
    const tracked_objects::Location& from_here,
    const base::Closure& success,
    const base::Closure& failure) {
  base::hash_map<ExtensionId, State>::iterator it =
      states_.find(extension->id());
  if (it == states_.end()) {
    base::MessageLoop::current()->PostTask(from_here, success);
    return;
  }

  State& state = it->second;
  switch (state.registration) {
    case REGISTERED:
    case REGISTERING:
      base::MessageLoop::current()->PostTask(from_here, failure);
      break;
    case UNREGISTERED:
      base::MessageLoop::current()->PostTask(from_here, success);
      break;
    case UNREGISTERING:
      state.unregistration_callbacks.push_back(
          SuccessFailureClosurePair(success, failure));
      break;
  }
}

WeakPtr<ServiceWorkerManager> ServiceWorkerManager::WeakThis() {
  return weak_this_factory_.GetWeakPtr();
}

ServiceWorkerManager::SuccessFailureClosurePair::SuccessFailureClosurePair(
    base::Closure success,
    base::Closure failure)
    : success(success), failure(failure) {
}

ServiceWorkerManager::SuccessFailureClosurePair::~SuccessFailureClosurePair() {
}

void ServiceWorkerManager::VectorOfClosurePairs::RunSuccessCallbacksAndClear() {
  std::vector<SuccessFailureClosurePair> swapped_callbacks;
  swap(swapped_callbacks);
  for (size_t i = 0; i < swapped_callbacks.size(); ++i) {
    swapped_callbacks[i].success.Run();
  }
}

void ServiceWorkerManager::VectorOfClosurePairs::RunFailureCallbacksAndClear() {
  std::vector<SuccessFailureClosurePair> swapped_callbacks;
  swap(swapped_callbacks);
  for (size_t i = 0; i < swapped_callbacks.size(); ++i) {
    swapped_callbacks[i].failure.Run();
  }
}

// ServiceWorkerManagerFactory

ServiceWorkerManager* ServiceWorkerManagerFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<ServiceWorkerManager*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

ServiceWorkerManagerFactory* ServiceWorkerManagerFactory::GetInstance() {
  return Singleton<ServiceWorkerManagerFactory>::get();
}

void ServiceWorkerManagerFactory::SetInstanceForTesting(
    content::BrowserContext* context,
    ServiceWorkerManager* manager) {
  Associate(context, manager);
}

ServiceWorkerManagerFactory::ServiceWorkerManagerFactory()
    : BrowserContextKeyedServiceFactory(
          "ServiceWorkerManager",
          BrowserContextDependencyManager::GetInstance()) {
}

ServiceWorkerManagerFactory::~ServiceWorkerManagerFactory() {
}

KeyedService* ServiceWorkerManagerFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new ServiceWorkerManager(context);
}

// TODO(jyasskin): Deal with incognito mode.
content::BrowserContext* ServiceWorkerManagerFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return ExtensionsBrowserClient::Get()->GetOriginalContext(context);
}

}  // namespace extensions
