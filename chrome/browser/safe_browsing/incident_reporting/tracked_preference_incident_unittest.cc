// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/safe_browsing/incident_reporting/tracked_preference_incident.h"

#include "base/memory/scoped_ptr.h"
#include "chrome/common/safe_browsing/csd.pb.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace safe_browsing {

namespace {

scoped_ptr<Incident> MakeIncident(bool changed, bool is_personal) {
  scoped_ptr<ClientIncidentReport_IncidentData_TrackedPreferenceIncident>
      incident(new ClientIncidentReport_IncidentData_TrackedPreferenceIncident);

  incident->set_path("foo");
  incident->set_atomic_value("bar");
  incident->set_value_state(
      changed
          ? ClientIncidentReport_IncidentData_TrackedPreferenceIncident_ValueState_CHANGED
          : ClientIncidentReport_IncidentData_TrackedPreferenceIncident_ValueState_CLEARED);
  return make_scoped_ptr(
      new TrackedPreferenceIncident(incident.Pass(), is_personal));
}

}  // namespace

TEST(TrackedPreferenceIncident, GetType) {
  ASSERT_EQ(IncidentType::TRACKED_PREFERENCE,
            MakeIncident(false, false)->GetType());
}

// Tests that GetKey returns the preference path.
TEST(TrackedPreferenceIncident, KeyIsPath) {
  ASSERT_EQ(std::string("foo"), MakeIncident(false, false)->GetKey());
}

// Tests that GetDigest returns the same value for the same incident.
TEST(TrackedPreferenceIncident, SameIncidentSameDigest) {
  ASSERT_EQ(MakeIncident(false, false)->ComputeDigest(),
            MakeIncident(false, false)->ComputeDigest());
}

// Tests that GetDigest returns a different value for different incidents.
TEST(TrackedPreferenceIncident, DifferentIncidentDifferentDigest) {
  ASSERT_NE(MakeIncident(false, false)->ComputeDigest(),
            MakeIncident(true, false)->ComputeDigest());
}

// Tests that values are removed for personal preferences.
TEST(TrackedPreferenceIncident, Filter) {
  scoped_ptr<ClientIncidentReport_IncidentData> impersonal(
      MakeIncident(false, false)->TakePayload());
  ASSERT_TRUE(impersonal->tracked_preference().has_atomic_value());

  scoped_ptr<ClientIncidentReport_IncidentData> personal(
      MakeIncident(false, true)->TakePayload());
  ASSERT_FALSE(personal->tracked_preference().has_atomic_value());
}

}  // namespace safe_browsing
