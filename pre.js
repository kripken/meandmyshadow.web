
Module['noExitRuntime'] = true;
Module['postRun'] = function() {
  var id;
  function doOne() {
    try {
      _OneMainLoopIteration();
    } catch(e) {
      console.log('exception in main loop, stopping looping');
      clearInterval(id);
      throw e;
    }
  }
  var id = setInterval(doOne, 1000);
};

