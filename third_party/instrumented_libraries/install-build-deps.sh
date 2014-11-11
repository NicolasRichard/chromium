#!/bin/bash -e

# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Script to install build dependencies of packages which we instrument.

# TODO(earthdok): find a way to pull the list from the build config.
common_packages="\
atk1.0 \
dee \
freetype \
libappindicator1 \
libasound2 \
libcairo2 \
libcap2 \
libcups2 \
libdbus-1-3 \
libdbus-glib-1-2 \
libdbusmenu \
libdbusmenu-glib4 \
libexpat1 \
libffi6 \
libfontconfig1 \
libgconf-2-4 \
libgcrypt11 \
libgdk-pixbuf2.0-0 \
libglib2.0-0 \
libgnome-keyring0 \
libgpg-error0 \
libgtk2.0-0 \
libnspr4 \
libp11-kit0 \
libpci3 \
libpcre3 \
libpixman-1-0 \
libpng12-0 \
libunity9 \
libx11-6 \
libxau6 \
libxcb1 \
libxcomposite1 \
libxcursor1 \
libxdamage1 \
libxdmcp6 \
libxext6 \
libxfixes3 \
libxi6 \
libxinerama1 \
libxrandr2 \
libxrender1 \
libxss1 \
libxtst6 \
nss \
overlay-scrollbar \
pango1.0 \
pulseaudio \
udev \
zlib1g"

precise_specific_packages="libtasn1-3"
trusty_specific_packages="libtasn1-6"

ubuntu_release=$(lsb_release -cs)

if test "$ubuntu_release" = "precise" ; then
  packages="$common_packages $precise_specific_packages"
else
  packages="$common_packages $trusty_specific_packages"
fi

echo $packages
sudo apt-get build-dep -y $packages

# Extra build deps for pulseaudio, which apt-get build-dep may fail to install
# for reasons which are not entirely clear. 
sudo apt-get install libltdl3-dev libjson0-dev \
         libsndfile1-dev libspeexdsp-dev \
         chrpath -y  # Chrpath is required by fix_rpaths.sh.
