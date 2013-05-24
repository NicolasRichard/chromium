# Copyright (C) 2013 Google Inc. All rights reserved.
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
#     * Neither the Google name nor the names of its
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

"""Generates a fake TestExpectations file consisting of flaky tests from the bot
corresponding to the give port."""

import json
import logging
import os.path
import urllib
import urllib2

from webkitpy.layout_tests.port import builders
from webkitpy.layout_tests.models.test_expectations import TestExpectationLine


_log = logging.getLogger(__name__)


# results.json v4 format:
# {
#  'version': 4,
#  'builder name' : {
#     'fixableCounts': {},
#     'blinkRevision': [],
#     'tests': {
#       'directory' { # Each path component is a dictionary.
#          'testname.html': {
#             'expected' : 'FAIL', # expectation name
#             'results': [], # Run-length encoded result.
#             'times': [],
#             'bugs': [], # bug urls
#          }
#      }
#   }
#  'buildNumbers': [],
#  'secondsSinceEpoch': [],
#  'fixableCount': [],
#  'allFixableCount': [],
#  'chromeRevision': [],
#  'failure_map': { } # Map from letter code to expectation name.
# },
class ResultsJSON(object):
    TESTS_KEY = 'tests'
    FAILURE_MAP_KEY = 'failure_map'
    RESULTS_KEY = 'results'
    BUGS_KEY = 'bugs'
    RLE_LENGTH = 0
    RLE_VALUE = 1

    # results.json was originally designed to support
    # multiple builders in one json file, so the builder_name
    # is needed to figure out which builder this json file
    # refers to (and thus where the results are stored)
    def __init__(self, builder_name, json_dict):
        self.builder_name = builder_name
        self._json = json_dict

    def _walk_trie(self, trie, parent_path):
        for name, value in trie.items():
            full_path = os.path.join(parent_path, name)

            # FIXME: If we ever have a test directory self.RESULTS_KEY
            # ("results"), this logic will break!
            if self.RESULTS_KEY not in value:
                for path, results in self._walk_trie(value, full_path):
                    yield path, results
            else:
                yield full_path, value

    def walk_results(self, full_path=''):
        tests_trie = self._json[self.builder_name][self.TESTS_KEY]
        return self._walk_trie(tests_trie, parent_path='')

    def expectation_for_type(self, type_char):
        return self._json[self.builder_name][self.FAILURE_MAP_KEY][type_char]

    # Knowing how to parse the run-length-encoded values in results.json
    # is a detail of this class.
    def occurances_and_type_from_result_item(self, item):
        return item[self.RLE_LENGTH], item[self.RLE_VALUE]


class BotTestExpecationsFactory(object):
    RESULTS_URL_PREFIX = 'http://test-results.appspot.com/testfile?master=ChromiumWebkit&testtype=layout-tests&name=results-small.json&builder='

    def _results_json_for_port(self, port_name):
        builder_name = builders.builder_name_for_port_name(port_name)
        if not builder_name:
            return None
        results_url = self.RESULTS_URL_PREFIX + urllib.quote(builder_name)
        try:
            _log.debug('Fetching flakiness data from appengine.')
            return ResultsJSON(builder_name, json.load(urllib2.urlopen(results_url)))
        except urllib2.URLError as error:
            _log.warning('Could not retrieve flakiness data from the bot.  url: %s', results_url)
            _log.warning(error)

    def expectations_for_port(self, port_name):
        results_json = self._results_json_for_port(port_name)
        if not results_json:
            return None
        return BotTestExpectations(results_json)


class BotTestExpectations(object):
    # FIXME: Get this from the json instead of hard-coding it.
    RESULT_TYPES_TO_IGNORE = ['N', 'X', 'Y']

    def __init__(self, results_json):
        self.results_json = results_json

    def _line_from_test_and_flaky_types_and_bug_urls(self, test_path, flaky_types, bug_urls):
        line = TestExpectationLine()
        line.original_string = test_path
        line.name = test_path
        line.filename = test_path
        line.modifiers = bug_urls if bug_urls else ""
        line.expectations = sorted(map(self.results_json.expectation_for_type, flaky_types))
        return line

    def flakes_by_path(self, only_ignore_very_flaky):
        flakes_by_path = {}
        for test_path, entry in self.results_json.walk_results():
            results_dict = entry[self.results_json.RESULTS_KEY]
            flaky_types = self._flaky_types_in_results(results_dict, only_ignore_very_flaky)
            if len(flaky_types) <= 1:
                continue
            flakes_by_path[test_path] = sorted(map(self.results_json.expectation_for_type, flaky_types))
        return flakes_by_path

    def expectation_lines(self):
        lines = []
        for test_path, entry in self.results_json.walk_results():
            results_array = entry[self.results_json.RESULTS_KEY]
            flaky_types = self._flaky_types_in_results(results_array, False)
            if len(flaky_types) > 1:
                bug_urls = entry.get(self.results_json.BUGS_KEY)
                line = self._line_from_test_and_flaky_types_and_bug_urls(test_path, flaky_types, bug_urls)
                lines.append(line)
        return lines

    def _flaky_types_in_results(self, run_length_encoded_results, only_ignore_very_flaky=False):
        results_map = {}
        seen_results = {}

        for result_item in run_length_encoded_results:
            _, result_type = self.results_json.occurances_and_type_from_result_item(result_item)
            if result_type in self.RESULT_TYPES_TO_IGNORE:
                continue

            if only_ignore_very_flaky and result_type not in seen_results:
                # Only consider a short-lived result if we've seen it more than once.
                # Otherwise, we include lots of false-positives due to tests that fail
                # for a couple runs and then start passing.
                # FIXME: Maybe we should make this more liberal and consider it a flake
                # even if we only see that failure once.
                seen_results[result_type] = True
                continue

            results_map[result_type] = True

        return results_map.keys()
