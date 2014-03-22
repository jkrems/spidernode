'use strict';

let foo = () => 'Hello';
let twice = x => 2 * x;

let mul = function({init}, ...more) {
  return more.reduce(
    (product, n) => product * n,
    init
  );
};

// export const Console = function(stdout, stderr) {
//   if (!stdout || !isFunction(stdout.write)) {
//     throw new TypeError('Console expects a writable stream instance');
//   }
// };

let nowISO = (new Date()).toISOString();

console.log(mul({ init: 4 }, 2, 3) + ' world, it is currently ' + nowISO);
