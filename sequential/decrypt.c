#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include <limits.h> // For LLONG_MAX

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// find (base^exp) % mod
long long modExp(long long base, long long exp, long long mod) {
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

// find modular inverse of e under mod φ(n)
int modInverse(int e, int phi) {
    for (int x = 1; x < phi; x++) {
        if ((e * x) % phi == 1)
            return x;
    }
    return -1;
}

// Encrypt a pixel value using RSA
long long encrypt(unsigned char pixel, long long e, long long n) {
    return modExp((long long)pixel, e, n);
}

// Decrypt a pixel value using RSA
unsigned char decrypt(long long encryptedPixel, long long d, long long n) {
    return (unsigned char)modExp(encryptedPixel, d, n);
}

// Function to write encrypted data to a binary file
void writeEncryptedDataToFile(const char *filename, long long *encryptedData, int totalPixels) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error opening file for writing\n");
        return;
    }
    fwrite(encryptedData, sizeof(long long), totalPixels, file);
    fclose(file);
}

// Function to read encrypted data from a binary file
void readEncryptedDataFromFile(const char *filename, long long *encryptedData, int totalPixels) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file for reading\n");
        return;
    }
    fread(encryptedData, sizeof(long long), totalPixels, file);
    fclose(file);
}
// Function to merge 8 bytes into a long long value
long long mergeBytesIntoLongLong(const unsigned char *bytes) {
    long long value = 0;
    for (int i = 0; i < 8; i++) {
        value |= ((long long)bytes[i] << (i * 8));
    }
    return value;
}

int main() {
    // Load the massive encrypted image
    int encryptedImageWidth, encryptedImageHeight, channels;
    unsigned char *encryptedImageData = stbi_load("massive_encrypted_image.png", &encryptedImageWidth, &encryptedImageHeight, &channels, 0);
    if (encryptedImageData == NULL) {
        printf("Error in loading the encrypted image\n");
        return -1;
    }

    // Calculate the original image width based on the encrypted image width
    // Each pixel in the original image was expanded to 8 bytes in the encrypted image
    int bytesPerPixel = 8; // 8 bytes per pixel
    int originalChannels = channels; // The original image channels (e.g., 3 for RGB, 4 for RGBA)
    int originalWidth = encryptedImageWidth / (bytesPerPixel * originalChannels);
    int originalHeight = encryptedImageHeight;

    printf("Original Image Dimensions: %d x %d (Channels: %d)\n", originalWidth, originalHeight, originalChannels);

    // Prime numbers p and q (same as encryption)
    int p = 61;
    int q = 53;

    // Calculate n and φ(n) (same as encryption)
    int n = p * q;
    int phi = (p - 1) * (q - 1);

    // Public and private keys (same as encryption)
    int e = 17;
    while (gcd(e, phi) != 1) {
        e++;
    }
    int d = modInverse(e, phi);
    if (d == -1) {
        printf("Couldn't find the modInverse\n");
        return -1;
    }

    // Prepare memory for the decrypted data
    int totalPixels = originalWidth * originalHeight * originalChannels;
    unsigned char *decryptedData = (unsigned char *)malloc(totalPixels * sizeof(unsigned char));

    // Decrypt each encrypted pixel
    for (int i = 0; i < totalPixels; i++) {
        // Extract the next 8 bytes from the massive encrypted image
        long long encryptedValue = mergeBytesIntoLongLong(&encryptedImageData[i * bytesPerPixel]);

        // Decrypt the long long encrypted value
        decryptedData[i] = decrypt(encryptedValue, d, n);
    }

    // Save the decrypted image
    stbi_write_png("PNGdecrypted_image.png", originalWidth, originalHeight, 1, decryptedData, originalWidth * originalChannels);

    // Free allocated memory
    free(decryptedData);
    stbi_image_free(encryptedImageData);

    printf("Image decryption completed successfully. Decrypted image saved as 'PNGdecrypted_image.png'.\n");
    return 0;
}
