// NOTES: 
// To encrypt and decrypt we just do one call to modExp for each pixel and its color values,
// so we can consider those independent tasks and split them up into threads.
// (we could have a thread per color value of pixels, so like 3 * pixel count different tasks)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// find greatest common divisor
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

int main() {
    // Load image
    int width, height, channels;
    unsigned char *imageData = stbi_load("input_image.png", &width, &height, &channels, 0);
    if (imageData == NULL) {
        printf("Error in loading the image\n");
        return -1;
    }

    // Prime numbers p and q
    int p = 61;
    int q = 53;

    // Calculate n and φ(n)
    int n = p * q;
    int phi = (p - 1) * (q - 1);

    // e is the public key, (1 < e < φ(n) and gcd(e, φ(n)) = 1)
    int e = 17;
    while (gcd(e, phi) != 1) {
        e++;
    }

    // Calculate private key 'd' (d = modInverse(e, φ(n)))
    int d = modInverse(e, phi);
    if (d == -1) {
        printf("Couldn't find the modInverse\n");
        return -1;
    }

    // Display keys
    printf("Public Key: e = %d, n = %d\n", e, n);
    printf("Private Key: d = %d, n = %d\n", d, n);

    // Prepare data for encryption
    int totalPixels = width * height * channels;
    long long *encryptedData = (long long *)malloc(totalPixels * sizeof(long long));
    unsigned char *decryptedData = (unsigned char *)malloc(totalPixels * sizeof(unsigned char));

    // Encrypt each pixel's color values
    for (int i = 0; i < totalPixels; i++) {
        encryptedData[i] = encrypt(imageData[i], e, n);
    }

    // Write encrypted data to a binary file
    writeEncryptedDataToFile("encrypted_image.bin", encryptedData, totalPixels);
    printf("Encrypted data written to 'encrypted_image.bin'.\n");

    // Display encrypted data (optional)
    printf("Encrypted pixel values:\n");
    for (int i = 0; i < totalPixels; i++) {
        printf("%lld ", encryptedData[i]);
    }
    printf("\n");

    // Read encrypted data back from the file (if needed)
    long long *readEncryptedData = (long long *)malloc(totalPixels * sizeof(long long));
    readEncryptedDataFromFile("encrypted_image.bin", readEncryptedData, totalPixels);

    // Decrypt each pixel's color values
    for (int i = 0; i < totalPixels; i++) {
        decryptedData[i] = decrypt(readEncryptedData[i], d, n);
    }

    // Save the decrypted image
    stbi_write_png("decrypted_image.png", width, height, channels, decryptedData, width * channels);

    // Free allocated memory
    free(encryptedData);
    free(decryptedData);
    free(readEncryptedData);
    stbi_image_free(imageData);

    printf("Image encryption and decryption completed successfully.\n");
    return 0;
}
