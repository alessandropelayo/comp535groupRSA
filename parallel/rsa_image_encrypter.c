#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>  // Include math.h for pow()
#include <time.h>

// function to find primes within 250 stored in prime array
void primefiller(int prime[], int *primeCount) {
    bool seive[250];
    for (int i = 0; i < 250; i++) {
        seive[i] = true;
    }
    seive[0] = false; // 0 is not prime
    seive[1] = false; // 1 is not prime

    // Sieve of Eratosthenes
    for (int i = 2; i < 250; i++) {
        if (seive[i]) {
            for (int j = i * 2; j < 250; j += i) {
                seive[j] = false; // multiples of i are not prime
            }
        }
    }

    // collecting prime numbers
    *primeCount = 0;
    for (int i = 0; i < 250; i++) {
        if (seive[i]) {
            prime[*primeCount] = i;
            (*primeCount)++;
        }
    }
}

//function to pick a random prime from the prime array
int pickrandomprime(int prime[], int *primeCount) {
    if (*primeCount == 0) {
        return -1; // No primes available
    }

    int k = rand() % (*primeCount);
    int selected = prime[k];

    // remove the selected prime from prime array
    for (int i = k; i < *primeCount - 1; i++) {
        prime[i] = prime[i + 1];
    }
    (*primeCount)--;

    return selected;
}

// find greatest common divisor
int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

//function to create pubic and private keys
void setkeys(int primes[], int primeCount, long long *public_key, long long *private_key, long long *n) {
    int prime1 = pickrandomprime(primes, &primeCount); // first prime number
    int prime2 = pickrandomprime(primes, &primeCount); // second prime number
    printf("Prime P: %d\n", prime1);
    printf("Prime Q: %d\n", prime2);

    *n = prime1 * prime2;
    printf("N: %lld\n", *n);
    int fi = (prime1 - 1) * (prime2 - 1);
    
    // find public exponent e
    int e = 2;
    while (1) {
        if (gcd(e, fi) == 1) {
            break;
        }
        e++;
    }
    *public_key = e;

    // find private exponent d
    int d = 2;
    while (1) {
        if ((d * e) % fi == 1) {
            break;
        }
        d++;
    }
    *private_key = d;
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
    // Seed the random number generator
    srand(time(NULL));

    // Load image
    int width, height, channels;
    unsigned char *imageData = stbi_load("input_image.png", &width, &height, &channels, 0);
    if (imageData == NULL) {
        printf("Error in loading the image\n");
        return -1;
    }

    int primes[250];
    int primeCount;
    primefiller(primes, &primeCount);

    long long public_key, private_key, n;
    setkeys(primes, primeCount, &public_key, &private_key, &n);

    // Display keys
    printf("Public Key: e = %lld, n = %lld\n", public_key, n);
    printf("Private Key: d = %lld, n = %lld\n", private_key, n);

    // Prepare data for encryption
    int totalPixels = width * height * channels;
    long long *encryptedData = (long long *)malloc(totalPixels * sizeof(long long));
    unsigned char *decryptedData = (unsigned char *)malloc(totalPixels * sizeof(unsigned char));

    // Encrypt each pixel's color values
    for (int i = 0; i < totalPixels; i++) {
        encryptedData[i] = encrypt(imageData[i], public_key, n);
    }

    // Save the encrypted image
    stbi_write_png("encrypted_image.png", width, height, channels, encryptedData, width * channels);

    // Write encrypted data to a binary file
    writeEncryptedDataToFile("encrypted_image.bin", encryptedData, totalPixels);
    printf("Encrypted data written to 'encrypted_image.bin'.\n");

    // Display encrypted data (optional)
    // printf("Encrypted pixel values:\n");
    // for (int i = 0; i < totalPixels; i++) {
    //     printf("%lld ", encryptedData[i]);
    // }
    // printf("\n");

    // Read encrypted data back from the file (if needed)
    long long *readEncryptedData = (long long *)malloc(totalPixels * sizeof(long long));
    readEncryptedDataFromFile("encrypted_image.bin", readEncryptedData, totalPixels);

    // Decrypt each pixel's color values
    for (int i = 0; i < totalPixels; i++) {
        decryptedData[i] = decrypt(readEncryptedData[i], private_key, n);
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
