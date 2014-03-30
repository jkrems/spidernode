'use strict';

let foo = () => 'Hello';
let twice = x => 2 * x;

let mul = function({init}, ...more) {
  return more.reduce(
    (product, n) => product * n,
    init
  );
};

setTimeout(function() {
  let nowISO = (new Date()).toISOString();
  console.log("It is currently: " + nowISO);
  setTimeout(() => console.log('ok'), 100);
}, 10);
