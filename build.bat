@echo off

if not exist dist (
    mkdir dist
)

emcc cpp/converter.cpp cpp/lodepng.cpp ^
    -o dist/pngtoico.js ^
    -std=c++17 ^
    -O3 ^
    --bind ^
    -s MODULARIZE=1 ^
    -s ENVIRONMENT=web ^
    -s "EXPORT_NAME='createPngToIcoModule'"

echo ✅ WASM compilation complete. Output files are in the /dist folder.