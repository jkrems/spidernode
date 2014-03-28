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
      ],
      'include_dirs': [
        'deps/spidermonkey/src/build-release/dist/include',
        'deps/libuv/include',
        '<(SHARED_INTERMEDIATE_DIR)', # for knot_natives.h
      ],
      'library_dirs': [
        'deps/spidermonkey/src/build-release/dist/lib',
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
