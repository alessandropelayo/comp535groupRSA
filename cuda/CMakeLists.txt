cmake_minimum_required(VERSION 3.28)
project(Parallel_RSA VERSION 1.0.0)

# Specify to compile in C++ 17
set(CMAKE_CXX_STANDARD 17)

# Specify to build in a Release configuration
set(CMAKE_BUILD_TYPE Release)

enable_language(CUDA)

include_directories(header lib)

find_package(OpenMP REQUIRED)

# Set SOURCES variable to all source files in /src directory.
file(GLOB SOURCES "src/*.cpp" "src/*.c" "src/*.cu")

add_executable(cuda_encrypter ${SOURCES})

# Link the OpenMP library
target_link_libraries(cuda_encrypter PUBLIC OpenMP::OpenMP_CXX)

# Link the math library in src/rsa_keygen.c
target_link_libraries(cuda_encrypter PUBLIC m)
