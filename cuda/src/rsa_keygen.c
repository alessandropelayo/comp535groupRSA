#include "rsa_keygen.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>  // Include math.h for pow()
#include <time.h>

// function to find primes within bound stored in prime array
void static primefiller(int bound, int prime[], int *primeCount) {
    // initialize the seive
    bool seive[bound];
    #pragma omp parallel for
    for (int i = 0; i < bound; i++) {
        seive[i] = true;
    }
    seive[0] = false; // 0 is not prime
    seive[1] = false; // 1 is not prime

    // Sieve of Eratosthenes
    #pragma omp parallel for
    for (int i = 2; i < bound; i++) {
        if (seive[i]) {
            for (int j = i * 2; j < bound; j += i) {
                #pragma omp critical
                {
                    seive[j] = false; // multiples of i are not prime
                }
            }
        }
    }

    // collecting prime numbers
    *primeCount = 0;
    #pragma omp parallel for
    for (int i = 0; i < bound; i++) {
        if (seive[i]) {
            #pragma omp critical
                {
                    prime[*primeCount] = i;
                    (*primeCount)++;
                }
        }
    }
}

//function to pick a random prime from the prime array
int static pickrandomprime(int prime[], int *primeCount) {
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
int static gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

//function to create pubic and private keys
void static setkeys(int primes[], int primeCount, long long *public_key, long long *private_key, long long *n) {
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

// find modular inverse of e under mod φ(n)
int static modInverse(int e, int phi) {
    for (int x = 1; x < phi; x++) {
        if ((e * x) % phi == 1)
            return x;
    }
    return -1;
}

void generateKeys(long long* public_key, long long* n) {
    printf("Generating keys...\n");
    srand(time(NULL));

    int bound = 250;
    int primes[bound];
    int primeCount;
    primefiller(bound, primes, &primeCount);

    long long private_key;
    setkeys(primes, primeCount, public_key, &private_key, n);

    // Display keys
    printf("Public Key: e = %lld, n = %lld\n", *public_key, *n);
    printf("Private Key: d = %lld, n = %lld\n", private_key, *n);
    printf("------------------------------\n");
}
