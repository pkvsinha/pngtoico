import createPngToIcoModule from './dist/pngtoico.js';
// import wasmUrl from './dist/pngtoico.wasm';

let wasmModulePromise = null;

async function preparePngToIcoModule() {
    if (!wasmModulePromise) {
        wasmModulePromise = createPngToIcoModule(/*{
            locateFile: () => wasmUrl,
        }*/);
    }
    return wasmModulePromise;
}

export async function convertPngToIco(pngFiles) {
    if (!Array.isArray(pngFiles) || pngFiles.length === 0) {
        throw new Error("Input must be a non-empty array of PNG files.");
    }

    const wasmModule = await preparePngToIcoModule();

    const fileBuffers = await Promise.all(pngFiles.map(file => file.arrayBuffer()))

    const pngCppVector = new wasmModule.VectorVal();
    fileBuffers.forEach(buffer => pngCppVector.push_back(new Uint8Array(buffer)));
    
    const icoData = wasmModule.generateIco(pngCppVector);

    pngCppVector.delete();

    if (!icoData) {
        throw new Error("Failed to generate ICO data.");
    }
    // wasmModule._free(icoData.byteOffset); // Free the memory allocated for the ICO data

    return new Blob([icoData], { type: 'image/x-icon' });
}