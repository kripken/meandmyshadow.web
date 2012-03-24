
Module['noExitRuntime'] = true;
Module['postRun'] = function() {
  function doOne() {
    //console.log('zz doOne ' + Date.now());
    _OneMainLoopIteration();
    setTimeout(doOne, 1000/40); // Game expects to run at 40fps
  }
  doOne();
};

