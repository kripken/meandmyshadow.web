# rm -Rf CMakeFiles/ CMakeCache.txt
~/Dev/emscripten/emconfigure cmake .
~/Dev/emscripten/emmake make
~/Dev/llvm-emscripten/cbuild/bin/opt meandmyshadow -strip-debug -o=meandmyshadow.slim

