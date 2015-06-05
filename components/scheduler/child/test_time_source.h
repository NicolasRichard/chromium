// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SCHEDULER_CHILD_TEST_TIME_SOURCE_H_
#define COMPONENTS_SCHEDULER_CHILD_TEST_TIME_SOURCE_H_

#include "base/memory/ref_counted.h"
#include "base/time/tick_clock.h"

namespace cc {
class TestNowSource;
}

namespace scheduler {

class TestTimeSource : public base::TickClock {
 public:
  explicit TestTimeSource(scoped_refptr<cc::TestNowSource> time_source);
  ~TestTimeSource() override;

  base::TimeTicks NowTicks() override;

 private:
  scoped_refptr<cc::TestNowSource> time_source_;

  DISALLOW_COPY_AND_ASSIGN(TestTimeSource);
};

}  // namespace scheduler

#endif  // COMPONENTS_SCHEDULER_CHILD_TEST_TIME_SOURCE_H_
