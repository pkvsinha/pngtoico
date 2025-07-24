#include <iostream>
#include <vector>
#include <string>
#include "lodepng.h"
#include <emscripten/bind.h>

// By default, C++ structs are padded.
// We use #pragma pack to ensure the structs are laid out in memory
// exactly as the ICO file format requires, with no extra bytes.
#pragma pack(push, 1)

// ICO file header structure (6 bytes total)
struct ICONDIR {
    uint16_t idReserved; // Reserved (must be 0)
    uint16_t idType;     // Resource Type (1 for icons)
    uint16_t idCount;    // How many images?
};

// Image entry structure for each image in the file (16 bytes total)
struct ICONDIRENTRY {
    uint8_t  bWidth;        // Width, in pixels, of the image
    uint8_t  bHeight;       // Height, in pixels, of the image
    uint8_t  bColorCount;   // Number of colors in image (0 if more than 256)
    uint8_t  bReserved;     // Reserved (must be 0)
    uint16_t wPlanes;       // Color Planes (0 or 1)
    uint16_t wBitCount;     // Bits per pixel
    uint32_t dwBytesInRes;  // How many bytes in this resource?
    uint32_t dwImageOffset; // Where in the file is this image?
};

#pragma pack(pop)

// This is the function we will call from JavaScript.
// It takes a vector of JS Uint8Array objects. Emscripten handles the conversion.
emscripten::val generateIco(const std::vector<emscripten::val>& png_files) {
    if (png_files.empty()) {
        // In a real app, you might throw an error here.
        // Returning an empty object for simplicity.
        return emscripten::val::null();
    }
    
    // The final ICO file will be built in this vector of bytes.
    std::vector<char> ico_file_data;
    
    // 1. Prepare the main ICO header
    ICONDIR icon_dir = {0};
    icon_dir.idReserved = 0; // Must be 0
    icon_dir.idType = 1;     // 1 for Icon
    icon_dir.idCount = png_files.size(); // Number of images
    
    // Reserve space for the main header and the directory entries.
    // We will write the actual data later once we have the image sizes and offsets.
    size_t header_and_entries_size = sizeof(ICONDIR) + (png_files.size() * sizeof(ICONDIRENTRY));
    ico_file_data.resize(header_and_entries_size);
    
    // Copy the main header into our file buffer
    memcpy(ico_file_data.data(), &icon_dir, sizeof(ICONDIR));
    
    uint32_t current_image_offset = header_and_entries_size;
    std::vector<char> all_png_data;

    // 2. Process each PNG file
    for (int i = 0; i < png_files.size(); ++i) {
        // Convert the JavaScript Uint8Array into a C++ vector of characters
        std::vector<unsigned char> png_data_uc = emscripten::vecFromJSArray<unsigned char>(png_files[i]);

        // Decode the PNG to get its width and height
        unsigned int width, height;
        std::vector<unsigned char> decoded_image_data;
        unsigned error = lodepng::decode(decoded_image_data, width, height, png_data_uc);

        if (error) {
            std::cerr << "LodePNG decode error " << error << ": " << lodepng_error_text(error) << std::endl;
            // Handle error, maybe return null or throw.
            return emscripten::val::null();
        }

        // 3. Create the image directory entry for this PNG
        ICONDIRENTRY entry = {0};
        // ICO format uses 0 for 256px width/height
        entry.bWidth = (width == 256) ? 0 : width;
        entry.bHeight = (height == 256) ? 0 : height;
        entry.dwBytesInRes = png_data_uc.size();
        entry.dwImageOffset = current_image_offset;
        
        // Write this entry into the correct slot in the ICO file header
        size_t entry_offset = sizeof(ICONDIR) + (i * sizeof(ICONDIRENTRY));
        memcpy(ico_file_data.data() + entry_offset, &entry, sizeof(ICONDIRENTRY));
        
        // 4. Append the raw PNG data to our collection of image data
        all_png_data.insert(all_png_data.end(), (char*)png_data_uc.data(), (char*)png_data_uc.data() + png_data_uc.size());
        
        // Update the offset for the next image
        current_image_offset += png_data_uc.size();
    }
    
    // 5. Append all the collected PNG image data to the end of the ICO file
    ico_file_data.insert(ico_file_data.end(), all_png_data.begin(), all_png_data.end());

    // Return the final ICO file as a Uint8Array that JavaScript can use
    return emscripten::val(emscripten::typed_memory_view(ico_file_data.size(), ico_file_data.data()));
}

// Bind the C++ function `generateIco` so it can be called from JavaScript.
EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("generateIco", &generateIco);
    emscripten::register_vector<emscripten::val>("VectorVal");
}