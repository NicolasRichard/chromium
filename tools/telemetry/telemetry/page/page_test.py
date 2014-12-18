# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from telemetry.page import test_expectations
from telemetry.page.actions import action_runner as action_runner_module


class TestNotSupportedOnPlatformError(Exception):
  """PageTest Exception raised when a required feature is unavailable.

  The feature required to run the test could be part of the platform,
  hardware configuration, or browser.
  """


class MultiTabTestAppCrashError(Exception):
  """PageTest Exception raised after browser or tab crash for multi-tab tests.

  Used to abort the test rather than try to recover from an unknown state.
  """


class Failure(Exception):
  """PageTest Exception raised when an undesired but designed-for problem."""


class MeasurementFailure(Failure):
  """PageTest Exception raised when an undesired but designed-for problem."""


class PageTest(object):
  """A class styled on unittest.TestCase for creating page-specific tests.

  Test should override ValidateAndMeasurePage to perform test
  validation and page measurement as necessary.

     class BodyChildElementMeasurement(PageTest):
       def ValidateAndMeasurePage(self, page, tab, results):
         body_child_count = tab.EvaluateJavaScript(
             'document.body.children.length')
         results.AddValue(scalar.ScalarValue(
             page, 'body_children', 'count', body_child_count))

  Args:
    action_name_to_run: This is the method name in telemetry.page.Page
        subclasses to run.
    discard_first_run: Discard the first run of this page. This is
        usually used with page_repeat and pageset_repeat options.
    is_action_name_to_run_optional: Determines what to do if
        action_name_to_run is not empty but the page doesn't have that
        action. The page will run (without any action) if
        is_action_name_to_run_optional is True, otherwise the page
        will fail.
  """

  def __init__(self,
               action_name_to_run='',
               needs_browser_restart_after_each_page=False,
               discard_first_result=False,
               clear_cache_before_each_run=False,
               is_action_name_to_run_optional=False):
    super(PageTest, self).__init__()

    self.options = None
    if action_name_to_run:
      assert action_name_to_run.startswith('Run') \
          and '_' not in action_name_to_run, \
          ('Wrong way of naming action_name_to_run. By new convention,'
           'action_name_to_run must start with Run- prefix and in CamelCase.')
    self._action_name_to_run = action_name_to_run
    self._needs_browser_restart_after_each_page = (
        needs_browser_restart_after_each_page)
    self._discard_first_result = discard_first_result
    self._clear_cache_before_each_run = clear_cache_before_each_run
    self._close_tabs_before_run = True
    self._is_action_name_to_run_optional = is_action_name_to_run_optional

  @property
  def is_multi_tab_test(self):
    """Returns True if the test opens multiple tabs.

    If the test overrides TabForPage, it is deemed a multi-tab test.
    Multi-tab tests do not retry after tab or browser crashes, whereas,
    single-tab tests too. That is because the state of multi-tab tests
    (e.g., how many tabs are open, etc.) is unknown after crashes.
    """
    return self.TabForPage.__func__ is not PageTest.TabForPage.__func__

  @property
  def discard_first_result(self):
    """When set to True, the first run of the test is discarded.  This is
    useful for cases where it's desirable to have some test resource cached so
    the first run of the test can warm things up. """
    return self._discard_first_result

  @discard_first_result.setter
  def discard_first_result(self, discard):
    self._discard_first_result = discard

  @property
  def clear_cache_before_each_run(self):
    """When set to True, the browser's disk and memory cache will be cleared
    before each run."""
    return self._clear_cache_before_each_run

  @property
  def close_tabs_before_run(self):
    """When set to True, all tabs are closed before running the test for the
    first time."""
    return self._close_tabs_before_run

  @close_tabs_before_run.setter
  def close_tabs_before_run(self, close_tabs):
    self._close_tabs_before_run = close_tabs

  def RestartBrowserBeforeEachPage(self):
    """ Should the browser be restarted for the page?

    This returns true if the test needs to unconditionally restart the
    browser for each page. It may be called before the browser is started.
    """
    return self._needs_browser_restart_after_each_page

  def StopBrowserAfterPage(self, browser, page):  # pylint: disable=W0613
    """Should the browser be stopped after the page is run?

    This is called after a page is run to decide whether the browser needs to
    be stopped to clean up its state. If it is stopped, then it will be
    restarted to run the next page.

    A test that overrides this can look at both the page and the browser to
    decide whether it needs to stop the browser.
    """
    return False

  def CustomizeBrowserOptions(self, options):
    """Override to add test-specific options to the BrowserOptions object"""

  def CustomizeBrowserOptionsForSinglePage(self, page, options):
    """Set options specific to the test and the given page.

    This will be called with the current page when the browser is (re)started.
    Changing options at this point only makes sense if the browser is being
    restarted for each page. Note that if page has a startup_url, the browser
    will always be restarted for each run.
    """
    if page.startup_url:
      options.browser_options.startup_url = page.startup_url

  def WillStartBrowser(self, platform):
    """Override to manipulate the browser environment before it launches."""

  def DidStartBrowser(self, browser):
    """Override to customize the browser right after it has launched."""

  def CanRunForPage(self, page):  # pylint: disable=W0613
    """Override to customize if the test can be ran for the given page."""
    if self._action_name_to_run and not self._is_action_name_to_run_optional:
      return hasattr(page, self._action_name_to_run)
    return True

  def SetOptions(self, options):
    """Sets the BrowserFinderOptions instance to use."""
    self.options = options

  def DidRunTest(self, browser, results): # pylint: disable=W0613
    """Override to do operations after all page set(s) are completed.

    This will occur before the browser is torn down.
    """
    self.options = None

  def WillNavigateToPage(self, page, tab):
    """Override to do operations before the page is navigated, notably Telemetry
    will already have performed the following operations on the browser before
    calling this function:
    * Ensure only one tab is open.
    * Call WaitForDocumentReadyStateToComplete on the tab."""

  def DidNavigateToPage(self, page, tab):
    """Override to do operations right after the page is navigated and after
    all waiting for completion has occurred."""

  def WillRunActions(self, page, tab):
    """Override to do operations before running the actions on the page."""

  def DidRunActions(self, page, tab):
    """Override to do operations after running the actions on the page."""

  def CleanUpAfterPage(self, page, tab):
    """Called after the test run method was run, even if it failed."""

  def CreateExpectations(self, page_set):   # pylint: disable=W0613
    """Override to make this test generate its own expectations instead of
    any that may have been defined in the page set."""
    return test_expectations.TestExpectations()

  def TabForPage(self, page, browser):   # pylint: disable=W0613
    """Override to select a different tab for the page.  For instance, to
    create a new tab for every page, return browser.tabs.New()."""
    return browser.tabs[0]

  def ValidatePageSet(self, page_set):
    """Override to examine the page set before the test run.  Useful for
    example to validate that the pageset can be used with the test."""

  def ValidateAndMeasurePage(self, page, tab, results):
    """Override to check test assertions and perform measurement.

    When adding measurement results, call results.AddValue(...) for
    each result. Raise an exception or add a failure.FailureValue on
    failure. page_test.py also provides several base exception classes
    to use.

    Prefer metric value names that are in accordance with python
    variable style. e.g., metric_name. The name 'url' must not be used.

    Put together:
      def ValidateAndMeasurePage(self, page, tab, results):
        res = tab.EvaluateJavaScript('2+2')
        if res != 4:
          raise Exception('Oh, wow.')
        results.AddValue(scalar.ScalarValue(
            page, 'two_plus_two', 'count', res))

    Args:
      page: A telemetry.page.Page instance.
      tab: A telemetry.core.Tab instance.
      results: A telemetry.results.PageTestResults instance.
    """
    raise NotImplementedError

  def RunPage(self, page, tab, results):
    # Run actions.
    interactive = self.options and self.options.interactive
    action_runner = action_runner_module.ActionRunner(
        tab, skip_waits=page.skip_waits)
    self.WillRunActions(page, tab)
    if interactive:
      action_runner.PauseInteractive()
    else:
      self._RunMethod(page, self._action_name_to_run, action_runner)
    self.DidRunActions(page, tab)

    self.ValidateAndMeasurePage(page, tab, results)

  def _RunMethod(self, page, method_name, action_runner):
    if hasattr(page, method_name):
      run_method = getattr(page, method_name)
      run_method(action_runner)

  def RunNavigateSteps(self, page, tab):
    """Navigates the tab to the page URL attribute.

    Runs the 'navigate_steps' page attribute as a compound action.
    """
    action_runner = action_runner_module.ActionRunner(
        tab, skip_waits=page.skip_waits)
    page.RunNavigateSteps(action_runner)

  @property
  def action_name_to_run(self):
    return self._action_name_to_run
