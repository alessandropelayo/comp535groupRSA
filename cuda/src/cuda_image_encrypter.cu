#include "cuda_image_encrypter.cuh"
#include "image.hpp"

namespace {

__device__ long long mod_exp(long long base, long long exp, const long long mod) {
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

__global__ void encrypt_pixel(unsigned char* pixel_data, const size_t size,
                              const long long public_key, const long long n) {
    const size_t i {blockIdx.x * blockDim.x + threadIdx.x};
    if (i < size) {
        pixel_data[i] = static_cast<unsigned char>(mod_exp(pixel_data[i], public_key, n));
    }
}

}

__host__ void gpu_encrypt_image(Image& image, const long long public_key, const long long n) {
    // Length of the image array being processed.
    const size_t pixel_data_len { static_cast<size_t>(image.get_height())
                                * static_cast<size_t>(image.get_width())
                                * static_cast<size_t>(image.get_channels()) };

    // Size of the image array for memcpy.
    const size_t pixel_data_size {pixel_data_len * sizeof(unsigned char)};

    // 512 threads per block
    constexpr dim3 block_size {512};

    // Basically just dividing the size of the array against the number of threads per block.
    // The formula is for rounding up in integer division.
    const dim3 grid_size {static_cast<unsigned int>((pixel_data_len + block_size.x - 1) / block_size.x) };

    unsigned char* cuda_pixel_data;
    cudaMalloc(&cuda_pixel_data, pixel_data_size);
    cudaMemcpy(cuda_pixel_data,
               image.get_pixels(),
               pixel_data_size,
               cudaMemcpyHostToDevice);

    encrypt_pixel<<<grid_size, block_size>>>(cuda_pixel_data, pixel_data_len, public_key, n);
    cudaDeviceSynchronize();

    unsigned char* host_pixel_data {static_cast<unsigned char*>(malloc(pixel_data_size))};
    cudaMemcpy(host_pixel_data,
               cuda_pixel_data,
               pixel_data_size,
               cudaMemcpyDeviceToHost);
    image.set_pixels(host_pixel_data);
    cudaFree(cuda_pixel_data);
}
