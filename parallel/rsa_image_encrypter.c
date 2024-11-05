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
#include <omp.h>
#include <sys/stat.h>
#include <sys/types.h>

// function to find primes within bound stored in prime array
void primefiller(int bound, int prime[], int *primeCount) {
    // initialize the seive
    bool seive[bound];
    for (int i = 0; i < bound; i++) {
        seive[i] = true;
    }
    seive[0] = false; // 0 is not prime
    seive[1] = false; // 1 is not prime

    // Sieve of Eratosthenes
    #pragma omp parallel
    {
        // int num_threads = omp_get_num_threads();
        // int thread_id = omp_get_thread_num();
        #pragma omp for schedule(dynamic)
        for (int i = 2; i < bound; i++) {
            if (seive[i]) {
                for (int j = i * 2; j < bound; j += i) {
                    seive[j] = false;
                }
            }
        }
    }

    // collecting prime numbers
    *primeCount = 0;
    for (int i = 0; i < bound; i++) {
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
    // printf("Prime P: %d\n", prime1);
    // printf("Prime Q: %d\n", prime2);

    *n = prime1 * prime2;
    // printf("N: %lld\n", *n);
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

void create_folders() {
    // Directory names
    const char *encrypted_dir = "encrypted";
    const char *decrypted_dir = "decrypted";

    // Create the "encrypted" directory
    if (mkdir(encrypted_dir, 0777) == -1) {
        // perror("Error creating 'encrypted' directory");
    } else {
        // printf("Directory 'encrypted' created successfully.\n");
    }

    // Create the "decrypted" directory
    if (mkdir(decrypted_dir, 0777) == -1) {
        // perror("Error creating 'decrypted' directory");
    } else {
        // printf("Directory 'decrypted' created successfully.\n");
    }
}

void encryption_mapping(long long *encryptTable, long long e, long long n) {
    // #pragma omp parallel for
    for (int i = 0; i < 256; i++) {
        encryptTable[i] = encrypt((unsigned char)i, e, n);
    }
}
void decryption_mapping(unsigned char *decryptTable ,long long d, long long n) {
    // #pragma omp parallel for
    for (int i = 0; i < 256; i++) {
        decryptTable[i] = decrypt((long long)i, d, n);
    }
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Use program: %s <image_file>\n", argv[0]);
        return -1;
    }

    // Seed the random number generator
    srand(time(NULL));
    
    // get the number of threads
    #pragma omp parallel
    {
        int num_threads = omp_get_num_threads();
        int thread_id = omp_get_thread_num();

        // printf("Thread %d of %d\n", thread_id, num_threads);
    }

    // Load image
    int width, height, channels;
    unsigned char *imageData = stbi_load(argv[1], &width, &height, &channels, 0);
    if (imageData == NULL) {
        printf("Error in loading the image\n");
        return -1;
    }

    int bound = 100000;
    int primes[bound];
    int primeCount;
    primefiller(bound, primes, &primeCount);

    long long public_key, private_key, n;
    setkeys(primes, primeCount, &public_key, &private_key, &n);

    // Display keys
    // printf("Public Key: e = %lld, n = %lld\n", public_key, n);
    // printf("Private Key: d = %lld, n = %lld\n", private_key, n);

    long long encrypt_table[256];
    unsigned char decrypt_table[256];
    encryption_mapping(encrypt_table, public_key, n);
    decryption_mapping(decrypt_table, private_key, n);

    // Prepare data for encryption
    int total_pixels = width * height * channels;
    long long *encryptedData = (long long *)malloc(total_pixels * sizeof(long long));
    unsigned char *decryptedData = (unsigned char *)malloc(total_pixels * sizeof(unsigned char));

    // Encrypt each pixel's color values
    #pragma omp parallel for
    for (int i = 0; i < total_pixels; i++) {
        encryptedData[i] = encrypt_table[imageData[i]];
    }

    // Save the encrypted image
    char encryptedFileName[256];
    const char *filePath = argv[1];

    const char *fileName = strrchr(filePath, '/');
    if (fileName) {
        fileName++; // Move past the '/'
    } else {
        fileName = filePath; // No '/' found, use the full string
    }
    
    create_folders();
    // Extract the filename from the path
    snprintf(encryptedFileName, sizeof(encryptedFileName), "./encrypted/%s_encrypted.png", fileName);
    //snprintf(encryptedFileName, sizeof(encryptedFileName), "encrypted/%s_encrypted.png", fileName);
    stbi_write_png(encryptedFileName, width, height, channels, encryptedData, width * channels);
    // printf("Encrypted image saved to '%s'.\n", encryptedFileName);

    // Write encrypted data to a binary file
    // writeEncryptedDataToFile("encrypted_image.bin", encryptedData, totalPixels);
    // printf("Encrypted data written to 'encrypted_image.bin'.\n");

    // // Read encrypted data back from the file (if needed)
    // long long *readEncryptedData = (long long *)malloc(totalPixels * sizeof(long long));
    // readEncryptedDataFromFile("encrypted_image.bin", readEncryptedData, totalPixels);

    // Decrypt each pixel's color values
    #pragma omp parallel for
    for (int i = 0; i < total_pixels; i++) {
        decryptedData[i] = decrypt_table[imageData[i]];
    }

    // Save the decrypted image
    char decryptedFileName[256];
    snprintf(decryptedFileName, sizeof(decryptedFileName), "./decrypted/%s_decrypted.png", fileName);
    //snprintf(decryptedFileName, sizeof(decryptedFileName), "decrypted/%s_decrypted.png", fileName);
    stbi_write_png(decryptedFileName, width, height, channels, decryptedData, width * channels);
    // printf("Decrypted image saved to '%s'.\n", decryptedFileName);

    // Free allocated memory
    free(encryptedData);
    free(decryptedData);
    // free(readEncryptedData);
    stbi_image_free(imageData);

    // printf("Image encryption and decryption completed successfully.\n");
    return 0;
}
