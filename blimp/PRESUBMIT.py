# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Top-level presubmit script for blimp.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

import re

BLIMP_SOURCE_FILES=(r'^blimp[\\/].*\.(cc|h)$',)

def CheckChangeLintsClean(input_api, output_api):
  source_filter = lambda x: input_api.FilterSourceFile(
    x, white_list=BLIMP_SOURCE_FILES, black_list=None)

  return input_api.canned_checks.CheckChangeLintsClean(
      input_api, output_api, source_filter, lint_filters=[], verbose_level=1)

def CheckChangeOnUpload(input_api, output_api):
  results = []
  results += CheckChangeLintsClean(input_api, output_api)
  return results
