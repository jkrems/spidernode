'use strict';

let foo = () => 'Hello';
let twice = x => 2 * x;

let mul = function({init}, ...more) {
  return more.reduce(
    (product, n) => product * n,
    init
  );
};

let nowISO = (new Date()).toISOString();
// console.log('Hello World!');
// console.log("It is currently: " + nowISO);

setTimeout(function() {
  console.log("After some time.");
  setTimeout(function() {}, 100);
}, 0);
