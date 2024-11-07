#pragma once

#include <vector>
#include <string>

// The Image class keeps track of the pixel data, filepath, width, height, and channels
// that are loaded from an STBI_LOAD.
class Image {
    private:
    unsigned char *pixel_data {};
    std::string *filepath {};
    int width {};
    int height {};
    int channels {};

    public:
    Image() {}

    Image(unsigned char* pixel_data, std::string *filepath, int width, int height, int channels): 
        pixel_data(pixel_data), 
        filepath(filepath), 
        width(width),
        height(height), 
        channels(channels) 
        {}

    // 
    ~Image() {
        if (pixel_data) {
            //WARNING: pixel_data is loaded using a malloc(), don't delete here!
            free(pixel_data);
            pixel_data = nullptr;
        }
        if (filepath) {
            delete filepath;
            filepath = nullptr;
        }
    }

    unsigned char* get_pixels() const {
        return pixel_data;
    }

    std::string* get_filepath() const {
        return filepath;
    }

    int get_width() const {
        return width;
    }

    int get_height() const {
        return height;
    }

    int get_channels() const {
        return channels;
    }
};

std::vector<Image*>* load_images_from_dir(const std::string &dir);
Image* load_image(const std::string &path);
int save_image(const Image &image, const std::string &target_dir);
