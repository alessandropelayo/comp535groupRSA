#pragma once

#include <string>

// The Image class keeps track of the pixel data, filepath, width, height, and channels
// that are loaded from an STBI_LOAD.
class Image {
public:
    Image() = delete;

    explicit Image(const std::string& filepath);

    ~Image() {
        free_pixel_data();
    }

    Image(const Image& other);

    Image& operator=(const Image& other);

    Image(Image&& other) noexcept;

    Image& operator=(Image&& other) noexcept;

    const unsigned char* get_pixels() const {
        return pixel_data;
    }

    const std::string& get_filepath() const {
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

    void set_pixels(unsigned char* pixel_data) {
        free_pixel_data();
        this->pixel_data = pixel_data;
    }

    int save_image(const std::string& target_dir="") const;

private:
    std::string filepath {};
    unsigned char* pixel_data {};
    int width {};
    int height {};
    int channels {};

    void free_pixel_data();

    void copy_pixel_data(const Image& other);
};
