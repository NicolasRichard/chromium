#!/usr/bin/python
# Copyright (C) 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Unit tests for test_expectations.py."""

import os
import sys
import unittest

# We need following workaround hack to run this unit tests in stand-alone.
try:
    d = os.path.dirname(__file__)
except NameError:
    d = os.path.dirname(sys.argv[0])
sys.path.append(os.path.abspath(os.path.join(d, '../../../')))

from test_expectations import TestExpectationsChecker
from webkitpy.style_references import port
from webkitpy.style_references import test_expectations as test_expectations_style


class ErrorCollector(object):
    """An error handler class for unit tests."""

    def __init__(self):
        self._errors = []

    def __call__(self, lineno, category, confidence, message):
        self._errors.append('%s  [%s] [%d]' % (message, category, confidence))

    def get_errors(self):
        return ''.join(self._errors)

    def reset_errors(self):
        self._errors = []


class TestExpectationsTestCase(unittest.TestCase):
    """TestCase for test_expectations.py"""

    def setUp(self):
        self._error_collector = ErrorCollector()
        port_obj = port.get('test')
        self._test_file = os.path.join(port_obj.layout_tests_dir(), 'misc/passing.html')

    def process_expectations(self, expectations, overrides=None):
        self._checker = TestExpectationsChecker()

    def assert_lines_lint(self, lines, expected):
        self._error_collector.reset_errors()
        checker = TestExpectationsChecker('test/test_expectations.txt',
                                          self._error_collector)
        checker.check_test_expectations(expectations_str='\n'.join(lines),
                                        tests=[self._test_file],
                                        overrides=None)
        checker.check_tabs(lines)
        self.assertEqual(expected, self._error_collector.get_errors())

    def test_valid_expectations(self):
        self.assert_lines_lint(
            ["misc/passing.html = PASS"],
            "")
        self.assert_lines_lint(
            ["misc/passing.html = FAIL PASS"],
            "")
        self.assert_lines_lint(
            ["misc/passing.html = CRASH TIMEOUT FAIL PASS"],
            "")
        self.assert_lines_lint(
            ["BUG1234 TEST : misc/passing.html = PASS FAIL"],
            "")
        self.assert_lines_lint(
            ["SKIP BUG1234 : misc/passing.html = TIMEOUT PASS"],
            "")
        self.assert_lines_lint(
            ["BUG1234 DEBUG : misc/passing.html = TIMEOUT PASS"],
            "")
        self.assert_lines_lint(
            ["BUG1234 DEBUG SKIP : misc/passing.html = TIMEOUT PASS"],
            "")
        self.assert_lines_lint(
            ["BUG1234 TEST DEBUG SKIP : misc/passing.html = TIMEOUT PASS"],
            "")
        self.assert_lines_lint(
            ["BUG1234 DEBUG TEST : misc/passing.html = TIMEOUT PASS"],
            "")
        self.assert_lines_lint(
            ["SLOW DEFER BUG1234 : misc/passing.html = PASS"],
            "")
        self.assert_lines_lint(
            ["WONTFIX SKIP : misc/passing.html = TIMEOUT"],
            "")

    def test_valid_modifiers(self):
        self.assert_lines_lint(
            ["INVALID-MODIFIER : misc/passing.html = PASS"],
            "Invalid modifier for test: invalid-modifier "
            "misc/passing.html  [test/expectations] [5]")
        self.assert_lines_lint(
            ["SKIP : misc/passing.html = PASS"],
            "Test lacks BUG modifier. "
            "misc/passing.html  [test/expectations] [2]")
        self.assert_lines_lint(
            ["WONTFIX DEFER : misc/passing.html = PASS"],
            "Test cannot be both DEFER and WONTFIX. "
            "misc/passing.html  [test/expectations] [5]")

    def test_expectation_errors(self):
        self.assert_lines_lint(
            ["missing expectations"],
            "Missing expectations. ['missing expectations']  [test/expectations] [5]")
        self.assert_lines_lint(
            ["SLOW : misc/passing.html = TIMEOUT"],
            "A test can not be both slow and timeout. "
            "If it times out indefinitely, then it should be just timeout. "
            "misc/passing.html  [test/expectations] [5]")
        self.assert_lines_lint(
            ["does/not/exist.html = FAIL"],
            "Path does not exist. does/not/exist.html  [test/expectations] [2]")

    def test_parse_expectations(self):
        self.assert_lines_lint(
            ["misc/passing.html = PASS"],
            "")
        self.assert_lines_lint(
            ["misc/passing.html = UNSUPPORTED"],
            "Unsupported expectation: unsupported "
            "misc/passing.html  [test/expectations] [5]")
        self.assert_lines_lint(
            ["misc/passing.html = PASS UNSUPPORTED"],
            "Unsupported expectation: unsupported "
            "misc/passing.html  [test/expectations] [5]")

    def test_already_seen_test(self):
        self.assert_lines_lint(
            ["misc/passing.html = PASS",
             "misc/passing.html = TIMEOUT"],
            "Duplicate expectation. %s  [test/expectations] [5]" % self._test_file)

    def test_tab(self):
        self.assert_lines_lint(
            ["\tmisc/passing.html = PASS"],
            "Line contains tab character.  [whitespace/tab] [5]")

if __name__ == '__main__':
    unittest.main()
