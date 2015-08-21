# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A library for cross-platform browser tests."""

import sys

# Ensure Python >= 2.7.
if sys.version_info < (2, 7):
  print >> sys.stderr, 'Need Python 2.7 or greater.'
  sys.exit(-1)

from telemetry.internal.util import global_hooks
global_hooks.InstallHooks()

# Add depdendencies into our path.
from telemetry.core import util
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'mock')
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'pexpect')
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'png')
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'pyserial')
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'webpagereplay')
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'websocket-client')
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'modulegraph')
util.AddDirToPythonPath(util.GetTelemetryThirdPartyDir(), 'altgraph')

util.AddDirToPythonPath(util.GetChromiumSrcDir(), 'build', 'android')
util.AddDirToPythonPath(util.GetChromiumSrcDir(), 'third_party', 'catapult')
