@echo off

if not exist dist (
    mkdir dist
)

emcc cpp/local_converter.cpp cpp/lodepng.cpp ^
    -o ./dist/local_pngtoico.js ^
    -std=c++17 ^
    -O3 ^
    --bind ^
    -s MODULARIZE=1 -s ^
    "EXPORT_NAME='createConverterModule'"