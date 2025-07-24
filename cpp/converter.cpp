#include <iostream>
#include <string>
#include <vector>
#include "lodepng.h" 
#include <emscripten/bind.h>

#pragma pack(push, 1)

struct ICONDIR {
    uint16_t reserved;
    uint16_t type;
    uint16_t count;
};

struct ICONDIRENTRY {
    uint8_t width;
    uint8_t height;
    uint8_t colorCount;
    uint8_t reserved;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t bytesInRes;
    uint32_t imageOffset;
};

#pragma pack(pop)

emscripten::val generateIco(const std::vector<emscripten::val>& png_files) {
    if (png_files.empty()) {
        return emscripten::val::null();
    }

    std::vector<char> ico_file_data;

    ICONDIR iconDir = {0, 1, static_cast<uint16_t>(png_files.size())};

    size_t header_plus_entries_size = sizeof(ICONDIR) + sizeof(ICONDIRENTRY) * png_files.size();

    ico_file_data.resize(header_plus_entries_size);

    // first copy the header
    memcpy(ico_file_data.data(), &iconDir, sizeof(ICONDIR));

    // iconir + n pngs * direntry metadata + n * image data
    // <-      header_plus_entries_size    ^ offset
    uint32_t curr_image_offset = header_plus_entries_size;

    std::vector<char> all_png_data;
    for (int i=0; i < png_files.size(); ++i) {
        // Convert the JavaScript Uint8Array into a C++ vector of characters
        std::vector<unsigned char> png_data_uc = emscripten::vecFromJSArray<unsigned char>(png_files[i]);

        unsigned int width, height;
        // unsigned error = lodepng::decode(nullptr, width, height, uc_png_data);
        // lodepng_decode(&uc_png_data.data(), &width, &height, nullptr, 
        //                reinterpret_cast<const unsigned char*>(uc_png_data.data()), uc_png_data.size());

        std::vector<unsigned char> decoded_image_data;
        unsigned error = lodepng::decode(decoded_image_data, width, height, uc_png_data);

        if (error) {
            std::cerr << "Error decoding PNG: " << lodepng_error_text(error) << std::endl;
            return emscripten::val::null();
        }

        ICONDIRENTRY entry = {0};
        entry.width = (width == 256) ? 0 : static_cast<uint8_t>(width);
        entry.height = (height == 256) ? 0 : static_cast<uint8_t>(height);
        entry.bytesInRes = uc_png_data.size();
        entry.imageOffset = curr_image_offset;

        // copy the nth PNG entry (meta) into the ICO file data
        size_t entry_offset = sizeof(ICONDIR) + i * sizeof(ICONDIRENTRY);
        memcpy(ico_file_data.data() + entry_offset, &entry, sizeof(ICONDIRENTRY));

        // copy the PNG data into the ICO file data
        all_png_data.insert(all_png_data.end(), uc_png_data.begin(), uc_png_data.end() + uc_png_data.size());
        
        curr_image_offset += uc_png_data.size();
    }

    ico_file_data.insert(ico_file_data.end(), all_png_data.begin(), all_png_data.end());

    return emscripten::val(emscripten::typed_memory_view(ico_file_data.size(), ico_file_data.data()));

}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("generateIco", &generateIco);
    emscripten::register_vector<emscripten::val>("VectorVal");
}