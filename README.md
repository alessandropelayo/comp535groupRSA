# RSA encryption on images
Two implementations in C. 

## Sequential
Folder sequential contains a sequential implementation of the rsa encryption program

## Parallel
Folder parallel contains an OpenMP parallel implementation of the rsa encryption program

## How to use Sequential or Parallel
Begin by moving the data folder into parallel or sequential or both

To run either program in their respective folder
```
gcc rsa_image_encrypter.c -o rsa_image_encrypter -lm -fopenmp

gcc read_images_encrypt.c -o read_images_encrypt

./read_images_encrypt

```
## References
Sequential version implemented with assistence from https://chatgpt.com/ and https://www.geeksforgeeks.org/rsa-algorithm-cryptography/

Sieve of Eratosthenes referenced from https://www.geeksforgeeks.org/sieve-of-eratosthenes/
