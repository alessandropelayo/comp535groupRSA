#include "arg_handler.hpp"
#include <iostream>

int main(int argc, char** argv) {
    Arg_Handler handler {argc, argv};
    handler.execute();
    std::cout << "Operation completed successfully.\n";
}
