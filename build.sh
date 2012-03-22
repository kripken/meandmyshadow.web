#rm -Rf CMakeFiles/ CMakeCache.txt
#~/Dev/emscripten/emconfigure cmake .
~/Dev/emscripten/emmake make VERBOSE=1 -j 4
mv meandmyshadow meandmyshadow.bc
~/Dev/emscripten/emcc meandmyshadow.bc -O0 -o mams.html --preload-file data --pre-js pre.js -s TOTAL_MEMORY=20971520

