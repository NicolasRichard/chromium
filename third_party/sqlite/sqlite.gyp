# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'use_system_sqlite%': 0,
    'required_sqlite_version': '3.6.1',
  },
  'target_defaults': {
    'defines': [
      'SQLITE_CORE',
      'SQLITE_ENABLE_FTS3',
      'SQLITE_ENABLE_ICU',
      'SQLITE_ENABLE_MEMORY_MANAGEMENT',
      'SQLITE_SECURE_DELETE',
      'SQLITE_SEPARATE_CACHE_POOLS',
      'THREADSAFE',
      '_HAS_EXCEPTIONS=0',
    ],
  },
  'targets': [
    {
      'target_name': 'sqlite',
      'conditions': [
        [ 'chromeos==1' , {
            'defines': [
                # Despite obvious warnings about not using this flag
                # in deployment, we are turning off sync in ChromeOS
                # and relying on the underlying journaling filesystem
                # to do error recovery properly.  It's much faster.
                'SQLITE_NO_SYNC',
                ],
          },
        ],
        ['use_system_sqlite', {
          'type': 'none',
          'direct_dependent_settings': {
            'defines': [
              'USE_SYSTEM_SQLITE',
            ],
          },

          'conditions': [
            ['OS == "ios"', {
              'dependencies': [
                'sqlite_regexp',
              ],
              'link_settings': {
                'libraries': [
                  '$(SDKROOT)/usr/lib/libsqlite3.dylib',
                ],
              },
            }],
            ['os_posix == 1 and OS != "mac" and OS != "ios" and OS != "android"', {
              'direct_dependent_settings': {
                'cflags': [
                  # This next command produces no output but it it will fail
                  # (and cause GYP to fail) if we don't have a recent enough
                  # version of sqlite.
                  '<!@(pkg-config --atleast-version=<(required_sqlite_version) sqlite3)',

                  '<!@(pkg-config --cflags sqlite3)',
                ],
              },
              'link_settings': {
                'ldflags': [
                  '<!@(pkg-config --libs-only-L --libs-only-other sqlite3)',
                ],
                'libraries': [
                  '<!@(pkg-config --libs-only-l sqlite3)',
                ],
              },
            }],
          ],
        }, { # !use_system_sqlite
          'product_name': 'sqlite3',
          'type': 'static_library',
          'sources': [
            'amalgamation/sqlite3.h',
            'amalgamation/sqlite3.c',
          ],

          # TODO(shess): Previously fts1 and rtree files were
          # explicitly excluded from the build.  Make sure they are
          # logically still excluded.

          # TODO(shess): Should all of the sources be listed and then
          # excluded?  For editing purposes?

          'include_dirs': [
            'amalgamation',
          ],
          'dependencies': [
            '../icu/icu.gyp:icui18n',
            '../icu/icu.gyp:icuuc',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '.',
              '../..',
            ],
          },
          'msvs_disabled_warnings': [
            4018, 4244, 4267,
          ],
          'variables': {
            'clang_warning_flags': [
              # sqlite does `if (*a++ && *b++);` in a non-buggy way.
              '-Wno-empty-body',
              # sqlite has some `unsigned < 0` checks.
              '-Wno-tautological-compare',
              # Needed because we don't have this commit yet:
              # https://github.com/mackyle/sqlite/commit/25df0fa050dcc9be7fb937b8e25be24049b3fef0
              '-Wno-pointer-bool-conversion',
            ],
          },
          'conditions': [
            ['OS=="linux"', {
              'link_settings': {
                'libraries': [
                  '-ldl',
                ],
              },
            }],
            ['OS == "mac" or OS == "ios"', {
              'link_settings': {
                'libraries': [
                  '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
                ],
              },
            }],
            ['OS == "android"', {
              'defines': [
                'HAVE_USLEEP=1',
                'SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT=1048576',
                'SQLITE_DEFAULT_AUTOVACUUM=1',
                'SQLITE_TEMP_STORE=3',
                'SQLITE_ENABLE_FTS3_BACKWARDS',
                'SQLITE_DEFAULT_FILE_FORMAT=4',
              ],
            }],
            ['os_posix == 1 and OS != "mac" and OS != "android"', {
              'cflags': [
                # SQLite doesn't believe in compiler warnings,
                # preferring testing.
                #   http://www.sqlite.org/faq.html#q17
                '-Wno-int-to-pointer-cast',
                '-Wno-pointer-to-int-cast',
              ],
            }],
            # Enable feedback-directed optimisation for sqlite when building in android.
            ['android_webview_build == 1', {
              'aosp_build_settings': {
                'LOCAL_FDO_SUPPORT': 'true',
              },
            }],
            ['sqlite_enable_fts2', {
              'defines': [
                'SQLITE_ENABLE_BROKEN_FTS2',
                'SQLITE_ENABLE_FTS2',
              ],
              'sources': [
                # fts2.c currently has a lot of conflicts when added to
                # the amalgamation.  It is probably not worth fixing that.
                'src/ext/fts2/fts2.c',
                'src/ext/fts2/fts2.h',
                'src/ext/fts2/fts2_hash.c',
                'src/ext/fts2/fts2_hash.h',
                'src/ext/fts2/fts2_icu.c',
                'src/ext/fts2/fts2_porter.c',
                'src/ext/fts2/fts2_tokenizer.c',
                'src/ext/fts2/fts2_tokenizer.h',
                'src/ext/fts2/fts2_tokenizer1.c',
              ],
              'include_dirs': [
                'src/src',
              ],
            }],
          ],
        }],
      ],
      'includes': [
        # Disable LTO due to ELF section name out of range
        # crbug.com/422251
        '../../build/android/disable_lto.gypi',
      ],
    },
  ],
  'conditions': [
    ['os_posix == 1 and OS != "mac" and OS != "ios" and OS != "android" and not use_system_sqlite', {
      'targets': [
        {
          'target_name': 'sqlite_shell',
          'type': 'executable',
          'dependencies': [
            '../icu/icu.gyp:icuuc',
            'sqlite',
          ],
          'sources': [
            'src/src/shell.c',
            'src/src/shell_icu_linux.c',
            # Include a dummy c++ file to force linking of libstdc++.
            'build_as_cpp.cc',
          ],
        },
      ],
    },],
    ['OS == "ios"', {
      'targets': [
        {
          'target_name': 'sqlite_regexp',
          'type': 'static_library',
          'dependencies': [
            '../icu/icu.gyp:icui18n',
            '../icu/icu.gyp:icuuc',
          ],
          'sources': [
            'src/ext/icu/icu.c',
          ],
        },
      ],
    }],
  ],
}
