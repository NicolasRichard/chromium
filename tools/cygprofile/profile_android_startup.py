# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Utility library for running a startup profile on an Android device.

Sets up a device for cygprofile, disables sandboxing permissions, and sets up
support for web page replay, device forwarding, and fake certificate authority
to make runs repeatable.
"""

import logging
import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.append(os.path.join(sys.path[0], '..', '..', 'build', 'android'))
from pylib import android_commands
from pylib import constants
from pylib import flag_changer
from pylib import forwarder
from pylib.device import device_errors
from pylib.device import device_utils
from pylib.device import intent

sys.path.append(os.path.join(sys.path[0], '..', '..', 'tools', 'telemetry'))
from telemetry.core import webpagereplay

sys.path.append(os.path.join(sys.path[0], '..', '..',
    'third_party', 'webpagereplay'))
import adb_install_cert
import certutils


class NoCyglogDataError(Exception):
  """An error used to indicate that no cyglog data was collected."""

  def __init__(self, value):
    super(NoCyglogDataError, self).__init__()
    self.value = value

  def __str__(self):
    return repr(self.value)


def _DownloadFromCloudStorage(bucket, sha1_file_name):
  """Download the given file based on a hash file."""
  cmd = ['download_from_google_storage', '--no_resume',
         '--bucket', bucket, '-s', sha1_file_name]
  print 'Executing command ' + ' '.join(cmd)
  process = subprocess.Popen(cmd)
  process.wait()
  if process.returncode != 0:
    raise Exception('Exception executing command %s' % ' '.join(cmd))


class WprManager(object):
  """A utility to download a WPR archive, host it, and forward device ports to
  it.
  """

  _WPR_BUCKET = 'chrome-partner-telemetry'

  def __init__(self, wpr_archive, device, cmdline_file):
    self._device = device
    self._wpr_archive = wpr_archive
    self._wpr_archive_hash = wpr_archive + '.sha1'
    self._cmdline_file = cmdline_file
    self._wpr_server = None
    self._wpr_ca_cert_path = None
    self._device_cert_util = None
    self._host_http_port = None
    self._host_https_port = None
    self._is_test_ca_installed = False
    self._flag_changer = None

  def Start(self):
    """Set up the device and host for WPR."""
    self.Stop()
    #TODO(azarchs): make self._InstallTestCa() work
    self._BringUpWpr()
    self._StartForwarder()

  def Stop(self):
    """Clean up the device and host's WPR setup."""
    self._StopForwarder()
    self._StopWpr()
    #TODO(azarchs): make self._RemoveTestCa() work

  def __enter__(self):
    self.Start()

  def __exit__(self, unused_exc_type, unused_exc_val, unused_exc_tb):
    self.Stop()

  def _InstallTestCa(self):
    """Generates and deploys a test certificate authority."""
    print 'Installing test certificate authority on device: %s' % (
        self._device.adb.GetDeviceSerial())
    self._wpr_ca_cert_path = os.path.join(tempfile.mkdtemp(), 'testca.pem')
    certutils.write_dummy_ca_cert(*certutils.generate_dummy_ca_cert(),
                                  cert_path=self._wpr_ca_cert_path)
    self._device_cert_util = adb_install_cert.AndroidCertInstaller(
        self._device.adb.GetDeviceSerial(), None, self._wpr_ca_cert_path)
    self._device_cert_util.install_cert(overwrite_cert=True)
    self._is_test_ca_installed = True

  def _RemoveTestCa(self):
    """Remove root CA generated by previous call to InstallTestCa().

    Removes the test root certificate from both the device and host machine.
    """
    print 'Cleaning up test CA...'
    if not self._wpr_ca_cert_path:
      return

    if self._is_test_ca_installed:
      try:
        self._device_cert_util.remove_cert()
      except Exception:
        # Best effort cleanup - show the error and continue.
        logging.error(
            'Error while trying to remove certificate authority: %s. '
            % self._adb.device_serial())
      self._is_test_ca_installed = False

    shutil.rmtree(os.path.dirname(self._wpr_ca_cert_path), ignore_errors=True)
    self._wpr_ca_cert_path = None
    self._device_cert_util = None

  def _BringUpWpr(self):
    """Start the WPR server on the host and the forwarder on the device."""
    print 'Starting WPR on host...'
    _DownloadFromCloudStorage(self._WPR_BUCKET, self._wpr_archive_hash)
    self._wpr_server = webpagereplay.ReplayServer(self._wpr_archive,
        '127.0.0.1', 0, 0, None,
        ['--should_generate_certs',
         '--https_root_ca_cert_path=' + self._wpr_ca_cert_path,
         '--use_closest_match'])
    ports = self._wpr_server.StartServer()[:-1]
    self._host_http_port = ports[0]
    self._host_https_port = ports[1]

  def _StopWpr(self):
    """ Stop the WPR and forwarder. """
    print 'Stopping WPR on host...'
    if self._wpr_server:
      self._wpr_server.StopServer()

  def _StartForwarder(self):
    """Sets up forwarding of device ports to the host, and configures chrome
    to use those ports.
    """
    print 'Starting device forwarder...'
    forwarder.Forwarder.Map([(0, self._host_http_port),
                             (0, self._host_https_port)],
                            self._device)
    device_http = forwarder.Forwarder.DevicePortForHostPort(
        self._host_http_port)
    device_https = forwarder.Forwarder.DevicePortForHostPort(
        self._host_https_port)
    self._flag_changer = flag_changer.FlagChanger(
        self._device, self._cmdline_file)
    self._flag_changer.AddFlags([
        '--host-resolver-rules="MAP * 127.0.0.1,EXCLUDE localhost"',
        '--testing-fixed-http-port=%s' % device_http,
        '--testing-fixed-https-port=%s' % device_https])

  def _StopForwarder(self):
    """Shuts down the port forwarding service."""
    print 'Stopping device forwarder...'
    if self._flag_changer:
      self._flag_changer.Restore()
      self._flag_changer = None
    forwarder.Forwarder.UnmapAllDevicePorts(self._device)


class AndroidProfileTool(object):
  """A utility for generating cygprofile data for chrome on andorid.

  Runs cygprofile_unittest found in output_directory, does profiling runs,
  and pulls the data to the local machine in output_directory/cyglog_data.
  """

  _DEVICE_CYGLOG_DIR = '/data/local/tmp/chrome/cyglog'

  # TEST_URL must be a url in the WPR_ARCHIVE.
  _TEST_URL = 'https://www.google.com/#hl=en&q=science'
  _WPR_ARCHIVE = os.path.join(
      constants.DIR_SOURCE_ROOT, 'tools', 'perf', 'page_sets', 'data',
      'top_10_mobile_002.wpr')


  def __init__(self, output_directory):
    devices = android_commands.GetAttachedDevices()
    self._device = device_utils.DeviceUtils(devices[0])
    self._cygprofile_tests = os.path.join(
        output_directory, 'cygprofile_unittests')
    self._host_cyglog_dir = os.path.join(
        output_directory, 'cyglog_data')
    self._SetUpDevice()

  def RunCygprofileTests(self):
    """Run the cygprofile unit tests suite on the device.

    Args:
      path_to_tests: The location on the host machine with the compiled
                     cygprofile test binary.
    Returns:
      The exit code for the tests.
    """
    device_path = '/data/local/tmp/cygprofile_unittests'
    self._device.old_interface.PushIfNeeded(
        self._cygprofile_tests, device_path)
    (exit_code, _) = (
        self._device.old_interface.GetShellCommandStatusAndOutput(
            command=device_path, log_result=True))
    return exit_code

  def CollectProfile(self, apk, package_info):
    """Run a profile and collect the log files.

    Args:
      apk: The location of the chrome apk to profile.
      package_info: A PackageInfo structure describing the chrome apk,
                    as from pylib/constants.
    Returns:
      A list of cygprofile data files.
    Raises:
      NoCyglogDataError: No data was found on the device.
    """
    self._Install(apk, package_info)

    try:
      changer = self._SetChromeFlags(package_info)
      self._SetUpDeviceFolders()
      # Start up chrome once with a blank page, just to get the one-off
      # activities out of the way such as apk resource extraction and profile
      # creation.
      self._StartChrome(package_info, 'about:blank')
      time.sleep(15)
      self._KillChrome(package_info)
      self._SetUpDeviceFolders()
      with WprManager(self._WPR_ARCHIVE, self._device,
                      package_info.cmdline_file):
        self._StartChrome(package_info, self._TEST_URL)
        time.sleep(90)
        self._KillChrome(package_info)
    finally:
      self._RestoreChromeFlags(changer)

    data = self._PullCyglogData()
    self._DeleteDeviceData()
    return data

  def Cleanup(self):
    """Delete all local and device files left over from profiling. """
    self._DeleteDeviceData()
    self._DeleteHostData()

  def _Install(self, apk, package_info):
    """Installs Chrome.apk on the device.
    Args:
      apk: The location of the chrome apk to profile.
      package_info: A PackageInfo structure describing the chrome apk,
                    as from pylib/constants.
    """
    print 'Installing apk...'
    self._device.old_interface.ManagedInstall(apk, package_info.package)

  def _SetUpDevice(self):
    """When profiling, files are output to the disk by every process.  This
    means running without sandboxing enabled.
    """
    # We need to have adb root in order to pull cyglog data
    try:
      print 'Enabling root...'
      self._device.EnableRoot()
      # SELinux need to be in permissive mode, otherwise the process cannot
      # write the log files.
      print 'Putting SELinux in permissive mode...'
      self._device.old_interface.RunShellCommand('setenforce 0')
    except device_errors.CommandFailedError as e:
      # TODO(jbudorick) Handle this exception appropriately once interface
      #                 conversions are finished.
      logging.error(str(e))

  def _SetChromeFlags(self, package_info):
    print 'Setting Chrome flags...'
    changer = flag_changer.FlagChanger(
        self._device, package_info.cmdline_file)
    changer.AddFlags(['--no-sandbox', '--disable-fre'])
    return changer

  def _RestoreChromeFlags(self, changer):
    print 'Restoring Chrome flags...'
    if changer:
      changer.Restore()

  def _SetUpDeviceFolders(self):
    """Creates folders on the device to store cyglog data. """
    print 'Setting up device folders...'
    self._DeleteDeviceData()
    self._device.old_interface.RunShellCommand(
        'mkdir -p %s' % self._DEVICE_CYGLOG_DIR)

  def _DeleteDeviceData(self):
    """Clears out cyglog storage locations on the device. """
    self._device.old_interface.RunShellCommand(
        'rm -rf %s' % self._DEVICE_CYGLOG_DIR)

  def _StartChrome(self, package_info, url):
    print 'Launching chrome...'
    self._device.StartActivity(
        intent.Intent(package=package_info.package,
                      activity=package_info.activity,
                      data=url,
                      extras={'create_new_tab' : True}),
        blocking=True, force_stop=True)

  def _KillChrome(self, package_info):
    self._device.KillAll(package_info.package)

  def _DeleteHostData(self):
    """Clears out cyglog storage locations on the host."""
    shutil.rmtree(self._host_cyglog_dir, ignore_errors=True)

  def _SetUpHostFolders(self):
    self._DeleteHostData()
    os.mkdir(self._host_cyglog_dir)

  def _PullCyglogData(self):
    """Pull the cyglog data off of the device.

    Returns:
      A list of cyglog data files which were pulled.
    Raises:
      NoCyglogDataError: No data was found on the device.
    """
    print 'Pulling cyglog data...'
    self._SetUpHostFolders()
    self._device.old_interface.Adb().Pull(
        self._DEVICE_CYGLOG_DIR, self._host_cyglog_dir)
    files = os.listdir(self._host_cyglog_dir)

    if len(files) == 0:
      raise NoCyglogDataError('No cyglog data was collected')

    return [os.path.join(self._host_cyglog_dir, x) for x in files]
