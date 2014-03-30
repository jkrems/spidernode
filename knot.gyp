{
  'variables': {
    'library_files': [
      'src/knot.js',
      'lib/util.js',
      'lib/console.js',
      'lib/tty.js',
      'lib/net.js',
      'lib/events.js',
      'lib/stream.js',
      'lib/_stream_duplex.js',
      'lib/_stream_passthrough.js',
      'lib/_stream_readable.js',
      'lib/_stream_transform.js',
      'lib/_stream_writable.js',
      'lib/timers.js',
      'lib/_linklist.js',
      'lib/assert.js',
    ],
  },


  'target_defaults': {
    'default_configuration': 'Debug',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'cflags': [ '-g', '-O0' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1, # static debug
            'Optimization': 0, # /Od, no optimization
            'MinimalRebuild': 'false',
            'OmitFramePointers': 'false',
            'BasicRuntimeChecks': 3, # /RTC1
          },
          'VCLinkerTool': {
            'LinkIncremental': 2, # enable incremental linking
          },
        },
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0', # stop gyp from defaulting to -Os
        },
      },
    },
  },


  'targets': [
    {
      'target_name': 'knot',
      'type': 'executable',
      'xcode_settings': {
        'CLANG_CXX_LANGUAGE_STANDARD': 'gnu++0x',
        'GCC_WARN_ABOUT_INVALID_OFFSETOF_MACRO': 'NO',
      },
      'sources': [
        'src/knot.cc',
        'src/tty_wrap.cc',
        'src/timer_wrap.cc',
      ],
      'include_dirs': [
        'deps/gecko/js/src/build-debug/dist/include',
        'deps/libuv/include',
        '<(SHARED_INTERMEDIATE_DIR)', # for knot_natives.h
      ],
      'library_dirs': [
        'deps/gecko/js/src/build-debug/dist/lib',
        'deps/libuv/out/Debug',
      ],
      'libraries': [
        '-ljs_static',
        '-luv',
        '-lz',
      ],
    }, # end knot
    {
      'target_name': 'knot_js2c',
      'type': 'none',
      'toolsets': ['host'],
      'actions': [
        {
          'action_name': 'knot_js2c',
          'inputs': [
            '<@(library_files)',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/knot_natives.h',
          ],
          'action': [
            'python',
            'tools/js2c.py',
            '<@(_outputs)',
            '<@(_inputs)',
          ],
        },
      ],
    }, # end knot_js2c
  ]
}
