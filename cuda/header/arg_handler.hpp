#pragma once

#include "image.hpp"

#include <string_view>
#include <unordered_map>
#include <vector>
#include <iostream>

class Arg_Handler {
public:
    Arg_Handler() = delete;

    Arg_Handler(int argc, char** argv):
    argv(argv),
    argc(argc)
    {}

    void execute() {
        if (args.empty()) {
            read_args();
            process_args();
        }
    }

private:
    enum Flags {
        D_FLAG, //File directory to encrypt flag: -d
        F_FLAG, //Single file to encrypt flag: -f
        T_FLAG, //Target directory to save images in: -t
        V_FLAG, //Verbose flag: -v
        H_FLAG,  //Help flag: -h
    };

    std::unordered_map<Flags, std::string> args {};
    char** argv {};
    int argc {};
    bool verbose_flag {};

    void read_args();
    void process_args();
    std::vector<Image> load_images_from_dir(const std::string_view dir);

    void verbose(const std::string_view msg) {
        if (verbose_flag)
            std::cout << msg << "\n";
    }
};
