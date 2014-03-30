// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

// Hello, and welcome to hacking node.js!
//
// This file is invoked by node::Load in src/node.cc, and responsible for
// bootstrapping the node.js core. Special caution is given to the performance
// of the startup process, so many dependencies are invoked lazily.
(function(process) {
  'use strict';

  let prefix = 'KNOT: ';

  process.moduleLoadList = [];

  this.global = this;
  global.process = process;

  process.env = [];
  process._asyncFlags = {};

  process.__defineGetter__('console', function() {
    return NativeModule.require('console');
  });


  function createWritableStdioStream(fd) {
    var stream;
    var tty_wrap = process.binding('tty_wrap');

    switch (tty_wrap.guessHandleType(fd)) {
      case 'TTY':
        var tty = NativeModule.require('tty');
        stream = new tty.WriteStream(fd);

        // Hack to have stream not keep the event loop alive.
        // See https://github.com/joyent/node/issues/1726
        // stream._handle.unref();
        break;

      default:
        // Probably an error on in uv_guess_handle()
        throw new Error('Implement me. Unknown stream file type!');
    }

    return stream;
  }

  function processStdio() {
    var stdin, stdout, stderr;

    process.__defineGetter__('stdout', function() {
      if (stdout) return stdout;
      stdout = createWritableStdioStream(1);
      // stdout.destroy = stdout.destroySoon = function(er) {
      //   er = er || new Error('process.stdout cannot be closed.');
      //   stdout.emit('error', er);
      // };
      // if (stdout.isTTY) {
      //   process.on('SIGWINCH', function() {
      //     stdout._refreshSize();
      //   });
      // }
      return stdout;
    });

    process.__defineGetter__('stderr', function() {
      if (stderr) return stderr;
      stderr = createWritableStdioStream(2);
      // stderr.destroy = stderr.destroySoon = function(er) {
      //   er = er || new Error('process.stderr cannot be closed.');
      //   stderr.emit('error', er);
      // };
      return stderr;
    });
  };


  function globalTimeouts() {
    global.setTimeout = function() {
      var t = NativeModule.require('timers');
      return t.setTimeout.apply(this, arguments);
    };

    global.setInterval = function() {
      var t = NativeModule.require('timers');
      return t.setInterval.apply(this, arguments);
    };

    global.clearTimeout = function() {
      var t = NativeModule.require('timers');
      return t.clearTimeout.apply(this, arguments);
    };

    global.clearInterval = function() {
      var t = NativeModule.require('timers');
      return t.clearInterval.apply(this, arguments);
    };

    global.setImmediate = function() {
      var t = NativeModule.require('timers');
      return t.setImmediate.apply(this, arguments);
    };

    global.clearImmediate = function() {
      var t = NativeModule.require('timers');
      return t.clearImmediate.apply(this, arguments);
    };
  };


  // Below you find a minimal module system, which is used to load the node
  // core modules found in lib/*.js. All core modules are compiled into the
  // node binary, so they can be loaded faster.

  // var ContextifyScript = process.binding('contextify').ContextifyScript;
  // function runInThisContext(code, options) {
  //   var script = new ContextifyScript(code, options);
  //   return script.runInThisContext();
  // }

  function NativeModule(id) {
    this.filename = id + '.js';
    this.id = id;
    this.exports = {};
    this.loaded = false;
  }

  NativeModule._source = process.binding('natives');
  NativeModule._cache = {};

  NativeModule.require = function(id) {
    if (id == 'native_module') {
      return NativeModule;
    }

    var cached = NativeModule.getCached(id);
    if (cached) {
      return cached.exports;
    }

    if (!NativeModule.exists(id)) {
      throw new Error('No such native module ' + id);
    }

    process.moduleLoadList.push('NativeModule ' + id);

    var nativeModule = new NativeModule(id);

    nativeModule.cache();
    nativeModule.compile();

    return nativeModule.exports;
  };

  NativeModule.getCached = function(id) {
    return NativeModule._cache[id];
  }

  NativeModule.exists = function(id) {
    return NativeModule._source.hasOwnProperty(id);
  }

  NativeModule.getSource = function(id) {
    return NativeModule._source[id];
  }

  NativeModule.wrap = function(script) {
    return NativeModule.wrapper[0] + script + NativeModule.wrapper[1];
  };

  NativeModule.wrapper = [
    '(function (exports, require, module, __filename, __dirname) { ',
    '\n});'
  ];

  NativeModule.prototype.compile = function() {
    var source = NativeModule.getSource(this.id);
    // source = NativeModule.wrap(source);

    var fn = new Function(
      'exports', 'require', 'module', '__filename', '__dirname',
      source
    );// runInThisContext(source, { filename: this.filename });
    fn(this.exports, NativeModule.require, this, this.filename);

    this.loaded = true;
  };

  NativeModule.prototype.cache = function() {
    NativeModule._cache[this.id] = this;
  };

  processStdio();
  globalTimeouts();
});
