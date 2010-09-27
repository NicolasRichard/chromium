#!/usr/bin/perl -w
# Copyright (C) 2010 Andras Becsi (abecsi@inf.u-szeged.hu), University of Szeged
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# A script which searches for headers included by WebKit2 files
# and generates forwarding headers for these headers.

use strict;
use Cwd qw(abs_path realpath);
use File::Find;
use File::Basename;
use File::Spec::Functions;

my $srcRoot = realpath(File::Spec->catfile(dirname(abs_path($0)), ".."));
my @platformPrefixes = ("android", "brew", "cf", "chromium", "curl", "efl", "gtk", "haiku", "mac", "qt", "soup", "v8", "win", "wx");
my @frameworks = ( "JavaScriptCore", "WebCore", "WebKit2");
my @skippedPrefixes;
my @frameworkHeaders;
my $framework;
my %neededHeaders;

my $outputDirectory = $ARGV[0];
shift;
my $platform  = $ARGV[0];

foreach my $prefix (@platformPrefixes) {
    push(@skippedPrefixes, $prefix) unless ($prefix =~ $platform);
}

foreach (@frameworks) {
    $framework = $_;
    find(\&collectNeededHeaders, File::Spec->catfile($srcRoot, "WebKit2"));
    find(\&collectFameworkHeaderPaths, File::Spec->catfile($srcRoot, $framework));
    createForwardingHeadersForFramework();
}

sub collectNeededHeaders {
    my $filePath = $File::Find::name;
    my $file = $_;
    if ($filePath =~ '\.h$|\.cpp$') {
        open(FILE, "<$file") or die "Could not open $filePath.\n";
        while (<FILE>) {
           if (m/^#.*<$framework\/(.*\.h)/) {
               $neededHeaders{$1} = 1;
           }
        }
        close(FILE);
    }
}

sub collectFameworkHeaderPaths {
    my $filePath = $File::Find::name;
    my $file = $_;
    if ($filePath =~ '\.h$' && $filePath !~ "ForwardingHeaders" && grep{$file eq $_} keys %neededHeaders) {
        my $headerPath = substr($filePath, length("$srcRoot/$framework/"));
        push(@frameworkHeaders, $headerPath) unless (grep($headerPath =~ "$_/", @skippedPrefixes));
    }
}

sub createForwardingHeadersForFramework {
    foreach my $header (@frameworkHeaders) {
        my $forwardingHeaderPath = File::Spec->catfile($outputDirectory, $framework, basename($header));
        my $expectedIncludeStatement = "#include \"$header\"";
        my $foundIncludeStatement = 0;
        $foundIncludeStatement = <EXISTING_HEADER> if open(EXISTING_HEADER, "<$forwardingHeaderPath");
        chomp($foundIncludeStatement);
        if (! $foundIncludeStatement || $foundIncludeStatement ne $expectedIncludeStatement) {
            print "[Creating forwarding header for $framework/$header]\n";
            open(FORWARDING_HEADER, ">$forwardingHeaderPath") or die "Could not open $forwardingHeaderPath.";
            print FORWARDING_HEADER "$expectedIncludeStatement\n";
            close(FORWARDING_HEADER);
        }
        close(EXISTING_HEADER);
    }
}

