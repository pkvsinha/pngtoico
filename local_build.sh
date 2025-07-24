#!/bin/bash

mkdir -p dist

emcc cpp/local_converter.cpp cpp/lodepng.cpp \
    -o dist/local_pngtoico.js \
    -std=c++17 \
    -O3 \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createConverterModule" \
    -s "EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -I ./cpp/include \
    --bind