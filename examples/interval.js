/**
 * Timers/setInterval
 */
const interval = setInterval(function() {
  clearInterval(interval);
  console.log('ok');
}, 50);
