#rm -Rf CMakeFiles/ CMakeCache.txt
#~/Dev/emscripten/emconfigure cmake .
#~/Dev/emscripten/emmake make VERBOSE=1 -j 1
#mv meandmyshadow meandmyshadow.bc
~/Dev/emscripten/emcc meandmyshadow.bc -O2 -o mams.html --preload-file data --pre-js pre.js -s TOTAL_MEMORY=20971520 --compression /home/alon/Dev/emscripten/third_party/lzma.js/lzma-native,/home/alon/Dev/emscripten/third_party/lzma.js/lzma-decoder.js,LZMA.decompress -s "EXPORTED_FUNCTIONS=['_main', '_SDL_Quit']"
#~/Dev/emscripten/emcc meandmyshadow.bc -O2 -o mams.html --preload-file data --pre-js pre.js -s TOTAL_MEMORY=20971520 --minify 0

