'use strict';

(function(process) {
  this.global = this;

  Object.defineProperty(global, 'console', {
    get: function() {
      return NativeModule.require('console');
    }
  });

  function NativeModule() {};

  NativeModule.require = function(moduleName) {
    return {
      log: function(message) {
        // throw new Error('Not implemented');
        return message;
      }
    };
  };
});
