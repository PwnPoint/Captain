#include "rc4.h"  // Include the header file where the RC4-related functions and constants are declared

#define N 256   // Define N as 256, representing the size of the permutation array (S), used in the RC4 algorithm

// Function to swap two unsigned characters (used during key-scheduling and pseudo-random generation)
void swap(unsigned char* a, unsigned char* b)
{
	int tmp = *a;  // Store the value of 'a' in a temporary variable
	*a = *b;       // Assign the value of 'b' to 'a'
	*b = tmp;      // Assign the value of 'tmp' (original value of 'a') to 'b', effectively swapping the two
}

// Key-Scheduling Algorithm (KSA)  Initializes the permutation array (S) using the provided key
int KSA(char* key, unsigned char* S)
{
	int len = strlen(key);  // Get the length of the key
	int j = 0;              // Initialize j to 0 (used for mixing the key with the array S)

	// Initialize the permutation array S to the identity permutation (S[i] = i for all i)
	for (int i = 0; i < N; i++)
		S[i] = i;

	// Use the key to generate a permutation of the array S
	for (int i = 0; i < N; i++) {
		j = (j + S[i] + key[i % len]) % N;  // Update j using the current state of S and the key (key[i % len] loops over key)

		swap(&S[i], &S[j]);  // Swap the values of S[i] and S[j] to create a pseudo-random permutation
	}

	return 0;  // Return 0 to indicate successful execution
}

// Pseudo-Random Generation Algorithm (PRGA)  Encrypts/Decrypts the input plaintext
unsigned char* PRGA(unsigned char* S, unsigned char* plaintext, int length) {
	int i = 0;  // Initialize index i to 0 (used to generate the pseudo-random sequence)
	int j = 0;  // Initialize index j to 0 (used in conjunction with i to select random elements of S)

	// Allocate memory for the ciphertext, ensuring enough space to hold the result (including the null terminator)
	unsigned char* ciphertext = (unsigned char*)malloc(length + 1);
	memset(ciphertext, 0, length + 1);  // Initialize the allocated memory to zero

	// Iterate through the plaintext characters
	for (size_t n = 0, len = length; n < len; n++)
	{
		i = (i + 1) % N;  // Increment i and wrap around to stay within the bounds of the array S
		j = (j + S[i]) % N;  // Update j based on the current value of S[i] and wrap around using modulo N

		swap(&S[i], &S[j]);  // Swap the values of S[i] and S[j] to further randomize the sequence
		int rnd = S[(S[i] + S[j]) % N];  // Generate a pseudo-random byte by using the sum of S[i] and S[j]

		ciphertext[n] = rnd ^ plaintext[n];  // XOR the pseudo-random byte with the plaintext character to get the ciphertext
	}

	return ciphertext;  // Return the ciphertext
}

// RC4 encryption/decryption function  takes a key and plaintext and returns the encrypted/decrypted data
unsigned char* RC4(char* key, unsigned char* plaintext, int length) {

	unsigned char S[N];  // Declare the permutation array S of size N (256)
	KSA(key, S);         // Call the Key-Scheduling Algorithm (KSA) to initialize the array S based on the provided key

	// Call the Pseudo-Random Generation Algorithm (PRGA) to perform the actual encryption or decryption
	return PRGA(S, plaintext, length);
}