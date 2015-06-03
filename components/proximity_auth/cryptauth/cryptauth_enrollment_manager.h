// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PROXIMITY_AUTH_CRYPTAUTH_CRYPTAUTH_ENROLLMENT_MANAGER_H
#define COMPONENTS_PROXIMITY_AUTH_CRYPTAUTH_CRYPTAUTH_ENROLLMENT_MANAGER_H

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/time/clock.h"
#include "base/time/time.h"
#include "components/proximity_auth/cryptauth/cryptauth_client.h"
#include "components/proximity_auth/cryptauth/cryptauth_enroller.h"
#include "components/proximity_auth/cryptauth/secure_message_delegate.h"
#include "components/proximity_auth/cryptauth/sync_scheduler.h"

class PrefRegistrySimple;
class PrefService;

namespace proximity_auth {

// This class manages the device's enrollment with CryptAuth, periodically
// re-enrolling to keep the state on the server fresh. If an enrollment fails,
// the manager will schedule the next enrollment more aggressively to recover
// from the failure.
class CryptAuthEnrollmentManager : public SyncScheduler::Delegate {
 public:
  class Observer {
   public:
    // Called when an enrollment attempt is started.
    virtual void OnEnrollmentStarted() = 0;

    // Called when an enrollment attempt finishes with the |success| of the
    // attempt.
    virtual void OnEnrollmentFinished(bool success) = 0;

    virtual ~Observer() {}
  };

  // Creates the manager:
  // |clock|: Used to determine the time between sync attempts.
  // |enroller_factory|: Creates CryptAuthEnroller instances to perform each
  //                     enrollment attempt.
  // |device_info|: Contains information about the local device that will be
  //                uploaded to CryptAuth with each enrollment request.
  CryptAuthEnrollmentManager(
      scoped_ptr<base::Clock> clock,
      scoped_ptr<CryptAuthEnrollerFactory> enroller_factory,
      const cryptauth::GcmDeviceInfo& device_info);

  ~CryptAuthEnrollmentManager() override;

  // Registers the prefs used by this class to the given |pref_service|.
  static void RegisterPrefs(PrefRegistrySimple* registry);

  // Starts the manager backed by a |pref_service|, which shall already
  // have registered the prefs through RegisterPrefs().
  // This function begins scheduling periodic enrollment attempts.
  void Start(PrefService* pref_service);

  // Adds an observer.
  void AddObserver(Observer* observer);

  // Removes an observer.
  void RemoveObserver(Observer* observer);

  // Skips the waiting period and forces an enrollment immediately. If an
  // enrollment is already in progress, this function does nothing.
  // |invocation_reason| specifies the reason that the enrollment was triggered,
  // which is upload to the server.
  void ForceEnrollmentNow(cryptauth::InvocationReason invocation_reason);

  // Returns true if a successful enrollment has been recorded and this
  // enrollment has not expired.
  bool IsEnrollmentValid() const;

  // Returns the timestamp of the last successful enrollment. If no enrollment
  // has ever been made, then a null base::Time object will be returned.
  base::Time GetLastEnrollmentTime() const;

  // Returns the time to the next enrollment attempt.
  base::TimeDelta GetTimeToNextAttempt() const;

  // Returns true if an enrollment attempt is currently in progress.
  bool IsEnrollmentInProgress() const;

  // Returns true if the last enrollment failed and the manager is now
  // scheduling enrollments more aggressively to recover. If no enrollment has
  // ever been recorded, then this function will also return true.
  bool IsRecoveringFromFailure() const;

 protected:
  // Creates a new SyncScheduler instance. Exposed for testing.
  virtual scoped_ptr<SyncScheduler> CreateSyncScheduler();

 private:
  // SyncScheduler::Delegate:
  void OnSyncRequested(
      scoped_ptr<SyncScheduler::SyncRequest> sync_request) override;

  // Callback when |cryptauth_enroller_| completes.
  void OnEnrollmentFinished(bool success);

  // Used to determine the time.
  scoped_ptr<base::Clock> clock_;

  // Creates CryptAuthEnroller instances for each enrollment attempt.
  scoped_ptr<CryptAuthEnrollerFactory> enroller_factory_;

  // The local device information to upload to CryptAuth.
  const cryptauth::GcmDeviceInfo device_info_;

  // Contains perferences that outlive the lifetime of this object and across
  // process restarts.
  // Not owned and must outlive this instance.
  PrefService* pref_service_;

  // Schedules the time between enrollment attempts.
  scoped_ptr<SyncScheduler> scheduler_;

  // Contains the SyncRequest that |scheduler_| requests when an enrollment
  // attempt is made.
  scoped_ptr<SyncScheduler::SyncRequest> sync_request_;

  // The CryptAuthEnroller instance for the current enrollment attempt. A new
  // instance will be created for each individual attempt.
  scoped_ptr<CryptAuthEnroller> cryptauth_enroller_;

  // List of observers.
  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<CryptAuthEnrollmentManager> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(CryptAuthEnrollmentManager);
};

}  // namespace proximity_auth

#endif  // COMPONENTS_PROXIMITY_CRYPTAUTH_CRYPTAUTH_ENROLLMENT_MANAGER_H
