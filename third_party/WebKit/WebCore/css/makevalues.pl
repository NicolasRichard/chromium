#! /usr/bin/perl
#
#   This file is part of the WebKit project
#
#   Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
#   Copyright (C) 2007 Apple Inc. All rights reserved.
#   Copyright (C) 2007 Trolltech ASA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public License
#   along with this library; see the file COPYING.LIB.  If not, write to
#   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#   Boston, MA 02111-1307, USA.
use strict;
use warnings;

open NAMES, "<CSSValueKeywords.in" || die "Could not open CSSValueKeywords.in";
my @names = ();
while (<NAMES>) {
  next if (m/#/);
  chomp $_;
  next if ($_ eq "");
  push @names, $_;
}
close(NAMES);

open GPERF, ">CSSValueKeywords.gperf" || die "Could not open CSSValueKeywords.gperf for writing";
print GPERF << "EOF";
%{
/* This file is automatically generated from CSSValueKeywords.in by makevalues, do not edit */

#include \"CSSValueKeywords.h\"
%}
struct css_value {
    const char* name;
    int id;
};
%%
EOF

foreach my $name (@names) {
  my $id = $name;
  $id =~ s/-/_/g;
  print GPERF $name . ", CSS_VAL_" . uc($id) . "\n";
}
print GPERF "%%\n";
close GPERF;

open HEADER, ">CSSValueKeywords.h" || die "Could not open CSSValueKeywords.h for writing";
print HEADER << "EOF";
/* This file is automatically generated from CSSValueKeywords.in by makevalues, do not edit */

#ifndef CSSVALUES_H
#define CSSVALUES_H

WebCore::String getValueName(unsigned short id);

#define CSS_VAL_INVALID 0
#define CSS_VAL_MIN 1
EOF

my $i = 1;
my $maxLen = 0;
foreach my $name (@names) {
  my $id = $name;
  $id =~ s/-/_/g;
  print HEADER "#define CSS_VAL_" . uc($id) . " " . $i . "\n";
  $i = $i + 1;
  if (length($name) > $maxLen) {
    $maxLen = length($name);
  }
}
print HEADER "#define CSS_VAL_TOTAL " . $i . "\n";
print HEADER "#endif\n";
close HEADER;

system("gperf -L ANSI-C -E -C -n -o -t --key-positions=\"*\" -NfindValue -Hhash_val -Wwordlist_value -D CSSValueKeywords.gperf > CSSValueKeywords.c");

open C, ">>CSSValueKeywords.c" || die "Could not open CSSValueKeywords.c for writing";
print C  "static const char * const valueList[] = {\n";
print C  "\"\",\n";
foreach my $name (@names) {
  print C  "\"" . $name . "\", \n";
}
print C << "EOF";
    0
};
String getValueName(unsigned short id)
{
    if (id >= CSS_VAL_TOTAL || id <= 0)
        return String();
    return String(valueList[id]);
}
EOF

close C;

