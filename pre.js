
Module['noExitRuntime'] = true;
Module['postRun'] = function() {
  function doOne() {
    //console.log('zz doOne ' + Date.now());
    _OneMainLoopIteration();
    setTimeout(doOne, 1000/30);
  }
  setTimeout(doOne, 1000/30);
};

