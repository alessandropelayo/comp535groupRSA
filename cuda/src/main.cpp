#include "image_handler.hpp"
#include "util.hpp"
#include "cuda_image_encrypter.cuh"
#include "rsa_keygen.h"
#include <omp.h>
#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <iomanip>

// Print out the help menu and usage details of the program.
void printHelp() {
    std::cout <<
    "Usage\n"
    "\n"
    "./cuda_encrypter -d [PATH-TO-IMAGES] -t [DIRECTORY] -v\n"
    "./cuda_encrypter -f [PATH-TO-IMAGE]\n"
    "\n"
    "Options"
    "\n"
    "-d (set encryption directory) Select a directory and its subdirectories containing "
    "images to be decrypted.\n"
    "Arg: [PATH-TO-IMAGES]\n"

    "-f (set encryption image) Select a single image to be encrypted.\n"
    "Arg: [PATH-TO-IMAGE]\n"

    "-t (set target directory) Select the directory to save the encrypted image(s) to. "
    ""
    "Arg: [DIRECTORY]\n"

    "-v (verbose) Print out more information about the file being processed.\n"
    << "\n";
}

int main(int argc, char **argv) {
    namespace fs = std::filesystem;
    char opt {};
    std::string d_read_dir {};    
    std::string f_img_path {};
    std::string t_target_dir {};
    bool d_flag {false};
    bool f_flag {false};
    bool t_flag {false};

    // Read flags and store the arguments provided by the user.
    // Flags that take an argument have a basic check on them against the filesystem
    // and return an error if it is invalid.
    // Each flag flips a boolean that ensures no flag is used twice.
    while ((opt = getopt(argc, argv, "d:f:t:vh")) != -1)
        switch (opt)
        {
        case 'd':
            if (d_flag)
                error("Duplicate flag -d.");

            if (!fs::is_directory(optarg))
                error("Please enter a valid read directory.");

            d_read_dir = optarg;
            d_flag = true;
            break;

        case 'f':
            if (f_flag)
                error("Duplicate flag -f.");  
            
            if (!fs::exists(optarg))
                error("File does not exist.");

            f_img_path = optarg;
            f_flag = true;
            break;

        case 't':
            if (t_flag)
                error("Duplicate flag -t.");

            if (!fs::is_directory(optarg))
                error("Target directory does not exist.");

            t_target_dir = optarg;
            t_flag = true;
            break;

        case 'v':
            if (v_flag)
                error("Duplicate flag -v.");

            v_flag = true;
            break;

        case 'h':
            printHelp();
            return 0;
            break;
        }


    // If no parameters, or no required parameters are used, throw an erorr.
    if (optind == 1 || !(d_flag || f_flag)) {
        printHelp();
        error("Please enter a parameter.");
        
    }   

    std::cout << "Generating keys..." << "\n";
    long long public_key {}, n {};
    generateKeys(&public_key, &n);
    std::cout << "------------------------------" << "\n";

    if (d_flag) {
        std::cout << "Loading images..." << "\n";
        std::vector<Image*> *images = load_images_from_dir(d_read_dir);
        std::cout << "Image loading done!" << "\n";

        std::cout << "Starting CUDA encryption..." << "\n";
        #pragma omp parallel for
        for (Image *image : *images) {
            setup_kernel(*image, public_key, n);
        }
        std::cout << "CUDA encryption done!" << "\n";

        std::cout << "Saving images..." << "\n";
        #pragma omp parallel for
        for (Image *image : *images) {
            save_image(*image, t_target_dir);
            delete image;            
        }
        std::cout << "Image saving done!" << "\n";
    }
    if (f_flag) {
        Image *image = load_image(f_img_path);
        setup_kernel(*image, public_key, n);
        save_image(*image, t_target_dir);
        delete image;
    }

    std::cout << "Operation completed successfully." << "\n";
}
