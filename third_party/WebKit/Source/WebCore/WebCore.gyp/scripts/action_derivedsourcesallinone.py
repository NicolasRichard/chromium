#!/usr/bin/python
#
# Copyright (C) 2009 Google Inc. All rights reserved.
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
#
# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# action_derivedsourcesallinone.py generates a single cpp file that includes
# all v8 bindings cpp files generated from idls. Files can be assigned into
# multiple output files, to reduce maximum compilation unit size and allow
# parallel compilation.
#
# usage: action_derivedsourcesallinone.py IDL_FILES_LIST -- OUTPUT_FILE1 OUTPUT_FILE2 ...
#
# Note that IDL_FILES_LIST is a text file containing the IDL file paths.

import errno
import os
import os.path
import re
import subprocess
import sys

# A regexp for finding Conditional attributes in interface definitions.
conditionalPattern = re.compile('interface[\s]*\[[^\]]*Conditional=([\_0-9a-zA-Z&|]*)')

copyrightTemplate = """/*
 * THIS FILE WAS AUTOMATICALLY GENERATED, DO NOT EDIT.
 *
 * This file was generated by the make_jni_lists.py script.
 *
 * Copyright (C) 2009 Google Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
"""


# Wraps conditional with ENABLE() and replace '&','|' with '&&','||' if more than one conditional is specified.
def formatConditional(conditional):
    def wrapWithEnable(s):
        if re.match('[|&]$', s):
            return s * 2
        return 'ENABLE(' + s + ')'
    return ' '.join(map(wrapWithEnable, conditional))


# Find the conditional interface attribute.
def extractConditional(idlFilePath):
    conditional = None

    # Read file and look for "interface [ Conditional=XXX ]".
    idlFile = open(idlFilePath)
    idlContents = idlFile.read().replace('\n', '')
    idlFile.close()

    match = conditionalPattern.search(idlContents)
    if match:
        conditional = match.group(1)
        conditional = re.split('([|&])', conditional)

    return conditional

# Extracts conditional and interface name from each IDL file.
def extractMetaData(filePaths):
    metaDataList = []

    for f in filePaths:
        metaData = {}
        if len(f) == 0:
            continue
        if not os.path.exists(f):
            print 'WARNING: file not found: "%s"' % f
            continue

        # Extract type name from file name
        (parentPath, fileName) = os.path.split(f)
        (interfaceName, ext) = os.path.splitext(fileName)

        if not ext == '.idl':
            continue

        metaData = {
            'conditional': extractConditional(f),
            'name': interfaceName,
        }

        metaDataList.append(metaData)

    return metaDataList


def generateContent(filesMetaData, partition, totalPartitions):
    # Sort files by conditionals.
    filesMetaData.sort()

    output = []

    # Add fixed content.
    output.append(copyrightTemplate)
    output.append('#define NO_IMPLICIT_ATOMICSTRING\n\n')

    # List all includes segmented by if and endif.
    prevConditional = None
    for metaData in filesMetaData:
        name = metaData['name']
        if (hash(name) % totalPartitions) != partition:
            continue
        conditional = metaData['conditional']

        if prevConditional and prevConditional != conditional:
            output.append('#endif\n')
        if conditional and prevConditional != conditional:
            output.append('\n#if %s\n' % formatConditional(conditional))

        output.append('#include "bindings/V8%s.cpp"\n' % name)

        prevConditional = conditional

    if prevConditional:
        output.append('#endif\n')

    return ''.join(output)


def writeContent(content, outputFileName):
    (parentPath, fileName) = os.path.split(outputFileName)
    if not os.path.exists(parentPath):
        print parentPath
        os.mkdir(parentPath)
    f = open(outputFileName, 'w')
    f.write(content)
    f.close()


def resolveCygpath(cygdriveNames):
    cmd = ['cygpath', '-f', '-', '-wa']
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    idlFileNames = []
    for fileName in cygdriveNames:
        process.stdin.write("%s\n" % fileName)
        process.stdin.flush()
        idlFileNames.append(process.stdout.readline().rstrip())
    process.stdin.close()
    process.wait()
    return idlFileNames


def main(args):
    assert(len(args) > 3)
    inOutBreakIndex = args.index('--')
    inputFileName = args[1]
    outputFileNames = args[inOutBreakIndex+1:]

    inputFile = open(inputFileName, 'r')
    idlFileNames = []
    cygdriveNames = []
    for line in inputFile:
        idlFileName = line.rstrip().split(' ')[0]
        if idlFileName.startswith("/cygdrive"):
            cygdriveNames.append(idlFileName)
        else:
            idlFileNames.append(idlFileName)

    if cygdriveNames:
        idlFileNames.extend(resolveCygpath(cygdriveNames))
    inputFile.close()

    filesMetaData = extractMetaData(idlFileNames)
    for fileName in outputFileNames:
        partition = outputFileNames.index(fileName)
        fileContents = generateContent(filesMetaData, partition, len(outputFileNames))
        writeContent(fileContents, fileName)

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
