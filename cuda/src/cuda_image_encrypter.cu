#include "image_handler.hpp"
#include "util.hpp"
#include <cuda.h>
#include <iostream>

__device__ long long modExp(long long base, long long exp, long long mod) {
    long long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1)
            result = (result * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;    
}

__global__ void encrypt_pixel(unsigned char* pixel_data, size_t size, long long public_key, long long n) {
    size_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < size) {
        pixel_data[i] = static_cast<unsigned char>(modExp(pixel_data[i], public_key, n));
    }
}

__host__ void setup_kernel(Image &image, long long public_key, long long n) {
    // Length of the image array being processed.
    size_t pixel_data_len = static_cast<size_t>(image.get_height()) 
                          * static_cast<size_t>(image.get_width()) 
                          * static_cast<size_t>(image.get_channels());
                          
    // Size of the image array for memcpy.
    size_t pixel_data_size = pixel_data_len * sizeof(unsigned char);

    // 512 threads per block.
    dim3 block_size (512);

    // Basically just dividing the size of the array against the number of threads per block.
    // The formula is for rounding up in integer division.
    dim3 grid_size ((pixel_data_size + block_size.x - 1) / block_size.x);

    unsigned char* pixel_data;
    cudaMalloc(&pixel_data, pixel_data_size);
    cudaMemcpy(pixel_data, 
               image.get_pixels(),
               pixel_data_size,
               cudaMemcpyHostToDevice);


    verbose("Launching CUDA encryption on file: " + *image.get_filepath());
    encrypt_pixel<<<grid_size, block_size>>>(pixel_data, pixel_data_len, public_key, n);
    cudaDeviceSynchronize();

    cudaMemcpy(image.get_pixels(), 
            pixel_data,
            pixel_data_size,
            cudaMemcpyDeviceToHost);
    cudaFree(pixel_data);        
}
