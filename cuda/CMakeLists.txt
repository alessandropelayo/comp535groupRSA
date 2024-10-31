cmake_minimum_required(VERSION 3.28)
project(Parallel_RSA VERSION 1.0.0)

enable_language(CUDA)

include_directories(header lib)

find_package(OpenMP REQUIRED)

# Set SOURCES variable to all source files in /src directory.
file(GLOB SOURCES "src/*.cpp" "src/*.c" "src/*.cu")

add_executable(img_handler ${SOURCES})

# Link the OpenMP library
target_link_libraries(img_handler PUBLIC OpenMP::OpenMP_CXX)

# Link the math library in src/rsa_keygen.c
target_link_libraries(img_handler PUBLIC m)
