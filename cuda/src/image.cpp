#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <cstring>
#include <filesystem>
#include <string>

// Loads a singular image and returns it as an Image.
Image::Image(const std::string& filepath): filepath(filepath) {
    //Load pixel information
    unsigned char* pixel_data {stbi_load(filepath.c_str(), &width, &height, &channels, 0)};

    // Image failed to load or not an image, throw an error.
    if (!pixel_data)
        throw std::invalid_argument(filepath);

    this->pixel_data = pixel_data;
}

Image::Image(const Image& other):
filepath(other.filepath),
width(other.width),
height(other.height),
channels(other.channels) {
    copy_pixel_data(other);
}

Image& Image::operator=(const Image& other) {
    if (this != &other) {
        free_pixel_data();
        filepath = other.filepath;
        width = other.width;
        height = other.height;
        channels = other.channels;
        copy_pixel_data(other);
    }
    return *this;
}

Image::Image(Image&& other) noexcept:
filepath(std::move(other.filepath)),
pixel_data(other.pixel_data),
width(other.width),
height(other.height),
channels(other.channels) {
    other.pixel_data = nullptr;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        free_pixel_data();
        pixel_data = other.pixel_data;
        other.pixel_data = nullptr;
        filepath = std::move(other.filepath);
        width = other.width;
        height = other.height;
        channels = other.channels;
    }
    return *this;
}

void Image::free_pixel_data() {
    if (pixel_data) {
        //WARNING: pixel_data is loaded using a malloc(), don't delete here!
        free(pixel_data);
        pixel_data = nullptr;
    }
}

void Image::copy_pixel_data(const Image& other) {
    if (other.pixel_data) {
        const size_t data_size {(width * height * channels) * sizeof(unsigned char)};
        pixel_data = static_cast<unsigned char*>(malloc(data_size));
        if (pixel_data) {
            std::memcpy(pixel_data, other.pixel_data, data_size);
        }
    }
}

int Image::save_image(const std::string& target_dir) const {
    namespace fs = std::filesystem;
    fs::path filepath {get_filepath()};
    // Append RSA_ to the prefix of the filename
    filepath.replace_filename("RSA_" + filepath.filename().string());
    // If target directory was provided, switch
    if (!target_dir.empty())
        filepath = fs::path(target_dir) / filepath.filename();

    // Determine type of image and save as that type.
    if (filepath.extension() == ".png")
        return stbi_write_png(filepath.c_str(),
                              get_width(),
                              get_height(),
                              get_channels(),
                              get_pixels(),
                              get_width() * get_channels());
    else if (filepath.extension() == ".bmp")
        return stbi_write_bmp(filepath.c_str(),
                              get_width(),
                              get_height(),
                              get_channels(),
                              get_pixels());
    else if (filepath.extension() == ".tga")
        return stbi_write_tga(filepath.c_str(),
                              get_width(),
                              get_height(),
                              get_channels(),
                              get_pixels());
    else if (filepath.extension() == ".jpg")
        return stbi_write_jpg(filepath.c_str(),
                              get_width(),
                              get_height(),
                              get_channels(),
                              get_pixels(),
                              100);

    return -1;
}
