#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image_handler.hpp"
#include "util.hpp"
#include <omp.h>
#include <string>
#include <filesystem>
#include <vector>

// Loads a series of images from a directory and returns it as a vector.
std::vector<Image*>* load_images_from_dir(const std::string &dir) {
    namespace fs = std::filesystem;
    std::vector<Image*> *images = new std::vector<Image*>();
    int width {}, height {}, channels {};

    // Grab every file from a directory and its subdirectories.
    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        std::string *filepath = new std::string(entry.path().string());
        verbose("Loading file: " + *filepath);
        // Load pixel information
        unsigned char* pixel_data = stbi_load(filepath->c_str(), &width, &height, &channels, 0);

        // Check if pixel information failed to load, or not an image file.
        if (pixel_data) {
            Image *image = new Image(pixel_data, filepath, width, height, channels);
            images->push_back(image);
        }   
        else {
            verbose("Invalid image file: " + *filepath);
        }
    }

    // No images were loaded, so throw an error.
    if (images->size() == 0)
        error("No valid image files found.");

    return images;
}

// Loads a singular image and returns it as an Image.
Image* load_image(const std::string &filepath) {
    int width {}, height {}, channels {};
    verbose("Loading file: " + filepath);
    //Load pixel information
    unsigned char* pixel_data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    // Image failed to load or not an image, throw an error.
    if (!pixel_data)
        error("Not a valid image file.");
    
    Image *image = new Image(pixel_data, new std::string(filepath), width, height, channels);
    return image;
}

int save_image(const Image &image, const std::string &target_dir) {
    namespace fs = std::filesystem;
    fs::path filepath = *image.get_filepath();
    // Append RSA_ to the prefix of the filename
    filepath.replace_filename("RSA_" + filepath.filename().string());
    // If target directory was provided, switch 
    if (!target_dir.empty()) 
        filepath = fs::path(target_dir) / filepath.filename();
    
    verbose("Saving image: " + filepath.string());

    // Determine type of image and save as that type.
    if (filepath.extension() == ".png")
        return stbi_write_png(filepath.c_str(), 
                              image.get_width(), 
                              image.get_height(),
                              image.get_channels(), 
                              image.get_pixels(),
                              image.get_width() * image.get_channels());
    else if (filepath.extension() == ".bmp")
        return stbi_write_bmp(filepath.c_str(), 
                              image.get_width(), 
                              image.get_height(),
                              image.get_channels(), 
                              image.get_pixels());
    else if (filepath.extension() == ".tga")
        return stbi_write_tga(filepath.c_str(), 
                              image.get_width(), 
                              image.get_height(),
                              image.get_channels(), 
                              image.get_pixels());
    else if (filepath.extension() == ".jpg")
        return stbi_write_jpg(filepath.c_str(), 
                              image.get_width(), 
                              image.get_height(),
                              image.get_channels(), 
                              image.get_pixels(),
                              100);    

    return -1;                 
}            
