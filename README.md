
# Me & My Shadow - Web Port

This is a port of

[https://acmepjz.github.io/meandmyshadow/](https://acmepjz.github.io/meandmyshadow/)

using Emscripten.

## Building

* [Get and install the emsdk](https://emscripten.org/docs/getting_started/downloads.html)
* Run cmake: `emcmake cmake . -G Ninja`
* Run ninja: `ninja`

## Running

* Run a webserver, e.g. `python -m http.server`
* Browse to `http://localhost:8000/meandmyshadow.html` (adjust the port if you
  use a different server)
