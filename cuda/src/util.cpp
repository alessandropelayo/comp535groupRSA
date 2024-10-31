#include <iostream>
#include <cstdlib>
#include <string>

bool v_flag = false;

// Prints an error message to the terminal and exits the program
void error(const std::string &err_msg) {
    std::cerr << "Error: " << err_msg << "\n";
    std::exit(EXIT_FAILURE);
}

// If the verbose flag is set, print out a message
void verbose(const std::string &msg) {
    if (v_flag)
        std::cout << msg << "\n";
}
