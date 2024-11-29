#include "arg_handler.hpp"
#include "cuda_image_encrypter.cuh"
#include "rsa_keygen.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {

// Print out the help menu and usage details of the program.
void print_help() {
    std::cout <<
    "Usage\n"
    "\n"
    "./cuda_encrypter -d [PATH-TO-IMAGES] -t [DIRECTORY] -v\n"
    "./cuda_encrypter -f [PATH-TO-IMAGE]\n"
    "\n"
    "Options"
    "\n"
    "-d (set encryption directory) Select a directory and its subdirectories containing images to be decrypted.\n"
    "Arg: [PATH-TO-IMAGES]\n"

    "-f (set encryption image) Select a single image to be encrypted.\n"
    "Arg: [PATH-TO-IMAGE]\n"

    "-t (set target directory) Select the directory to save the encrypted image(s) to.\n"
    "Arg: [DIRECTORY]\n"

    "-v (verbose) Print out more information about the file being processed.\n\n";
}

void error(const std::string_view err_msg) {
    std::cerr << "Error: " << err_msg << "\n";
    std::exit(EXIT_FAILURE);
}

}

// Prints an error message to the terminal and exits the program


// Read flags and store the arguments provided by the user.
// Flags that take an argument have a basic check on them against the filesystem
// and return an error if it is invalid.
// Each flag flips a boolean that ensures no flag is used twice.
void Arg_Handler::read_args() {
    namespace fs = std::filesystem;
    int opt {};
    while ((opt = getopt(argc, argv, "d:f:t:vh")) != -1)
        switch (opt)
        {
            case 'd':
                if (args.contains(D_FLAG))
                    error("Duplicate flag -d.");

                if (!fs::is_directory(optarg))
                    error("Please enter a valid read directory.");

                args[D_FLAG] = optarg;
                break;

            case 'f':
                if (args.contains(F_FLAG))
                    error("Duplicate flag -f.");

                if (!fs::exists(optarg))
                    error("File does not exist.");

                args[F_FLAG] = optarg;
                break;

            case 't':
                if (args.contains(T_FLAG))
                    error("Duplicate flag -t.");

                if (!fs::is_directory(optarg))
                    error("Target directory does not exist.");

                args[T_FLAG] = optarg;
                break;

            case 'v':
                if (args.contains(V_FLAG))
                    error("Duplicate flag -v.");

                args[V_FLAG] = "";
                break;

            case 'h':
                if (args.contains(H_FLAG))
                    error("Duplicate flag -h.");

                args[H_FLAG] = "";
                break;
        }

    // If no parameters, or no required parameters are used, throw an error.
    if (optind == 1 || !(args.contains(D_FLAG) || args.contains(F_FLAG))) {
        print_help();
        error("Please enter a parameter.");
    }
}

// Loads a series of images from a directory and returns it as a vector.
std::vector<Image> Arg_Handler::load_images_from_dir(const std::string_view dir) {
    namespace fs = std::filesystem;
    std::vector<Image> images {};
    std::vector<std::string> filepaths {};

    // Grab every filepath from a directory and its subdirectories.
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (fs::is_directory(entry))
            continue;

        std::string filepath {entry.path().string()};
        filepaths.push_back(filepath);
    }

    // This loop is separated from the last for loop since
    // a recursive directory iterator can't be used with
    // parallel threads.
    // Constructing a single Image takes a really long time
    // so it's worth running two separate for loops here.
    #pragma omp parallel for
    for (std::string filepath: filepaths) {
        try {
            verbose("Loading file: " + filepath);
            Image image {filepath};
            #pragma omp critical
            images.push_back(image);
        }
        catch (const std::invalid_argument& e) {
            verbose("Invalid image file: " + filepath);
        }
    }

    // No images were loaded, so throw an error.
    if (images.empty())
        error("No valid image files found.");

    return images;
}

void Arg_Handler::process_args() {
    if (args.contains(H_FLAG)) {
        print_help();
        std::exit(EXIT_SUCCESS);
    }

    if (args.contains(V_FLAG)) {
        verbose_flag = true;
    }

    long long public_key {}, n {};
    generateKeys(&public_key, &n);

    if (args.contains(D_FLAG)) {
        std::cout << "Loading images...\n";
        std::vector<Image> images = load_images_from_dir(args[D_FLAG]);
        std::cout << "Image loading done!\n";

        std::cout << "Starting CUDA encryption...\n";
        #pragma omp parallel for
        for (Image& image : images) {
            verbose("Launching CUDA encryption on file: " + image.get_filepath());
            gpu_encrypt_image(image, public_key, n);
        }
        std::cout << "CUDA encryption done!\n";

        std::cout << "Saving images...\n";
        #pragma omp parallel for
        for (Image& image : images) {
            verbose("Saving image: " + image.get_filepath());
            if (!image.save_image(args[T_FLAG])) {
                std::cout << "Failed to save image " << image.get_filepath();
            }
        }
        std::cout << "Image saving done!\n";
    }

    if (args.contains(F_FLAG)) {
        verbose("Loading image: " + args[F_FLAG]);
        Image image {args[F_FLAG]};
        gpu_encrypt_image(image, public_key, n);
        verbose("Saving image: " + image.get_filepath());
        if (!image.save_image(args[T_FLAG])) {
            std::cout << "Failed to save image " << image.get_filepath();
        }
    }
}
