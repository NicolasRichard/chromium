/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

function LayoutTestHistoryAnalyzer(builder) {
    this._builder = builder;
    this._cache = {};
    this._loader = new LayoutTestResultsLoader(builder);
}

LayoutTestHistoryAnalyzer.prototype = {
    /*
     * Preiodically calls callback until all current failures have been explained. Callback is
     * passed an object like the following:
     * {
     *     'r12347 (681)': {
     *         'tooManyFailures': false,
     *         'tests': {
     *             'css1/basic/class_as_selector2.html': 'fail',
     *         },
     *     },
     *     'r12346 (680)': {
     *         'tooManyFailures': false,
     *         'tests': {},
     *     },
     *     'r12345 (679)': {
     *         'tooManyFailures': false,
     *         'tests': {
     *             'css1/basic/class_as_selector.html': 'crash',
     *         },
     *     },
     * },
     * Each build contains just the failures that a) are still occuring on the bots, and b) were new
     * in that build.
     */
    start: function(callback) {
        var cacheKey = '_startFetchingBuildHistory';
        if (!(cacheKey in this._cache))
            this._cache[cacheKey] = {};

        var history = this._cache[cacheKey];

        var self = this;
        self._builder.getBuildNames(function(buildNames) {
            function inner(buildIndex) {
                self._incorporateBuildHistory(buildNames, buildIndex, history, function(callAgain) {
                    var nextIndex = buildIndex + 1;
                    if (nextIndex >= buildNames.length)
                        callAgain = false;
                    callback(history, callAgain);
                    if (!callAgain)
                        return;
                    setTimeout(function() { inner(nextIndex) }, 0);
                });
            }
            inner(0);
        });
    },

    _incorporateBuildHistory: function(buildNames, buildIndex, history, callback) {
        var previousBuildName = Object.keys(history).last();
        var nextBuildName = buildNames[buildIndex];

        this._loader.start(nextBuildName, function(tests, tooManyFailures) {
            history[nextBuildName] = {
                tooManyFailures: tooManyFailures,
                tests: {},
            };

            for (var testName in tests) {
                if (previousBuildName) {
                    if (!(testName in history[previousBuildName].tests))
                        continue;
                    delete history[previousBuildName].tests[testName];
                }
                history[nextBuildName].tests[testName] = tests[testName];
            }

            callback(Object.keys(history[nextBuildName].tests).length);
        },
        function(tests) {
            // Some tests failed, but we couldn't fetch results.html (perhaps because the test
            // run aborted early for some reason). Just skip this build entirely.
            callback(true);
        });
    },
};
