#include <iostream>
#include <random>
#include <string>
#include <windows.h>
#include "Mutex.h"
#include "Config.h"

// Function to generate a random string of a given length
//std::string generateRandomString(int length) {
//    // Charset of characters from which the random string will be generated
//    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
//
//    // Initialize random device for non-deterministic random number generation
//    std::random_device rd;
//
//    // Mersenne Twister generator initialized with random device seed
//    std::mt19937 gen(rd());
//
//    // Create a uniform distribution to pick indices between 0 and the length of the charset
//    std::uniform_int_distribution<> dis(0, charset.length() - 1);
//
//    // String to store the randomly generated characters
//    std::string randomString;
//
//    // Loop through and generate the random string of specified length
//    for (int i = 0; i < length; ++i) {
//        // Add a random character from the charset to the result string
//        randomString += charset[dis(gen)];
//    }
//
//    // Return the generated random string
//    return randomString;
//}

// Function in the Mutex class that checks the mutex state and performs actions based on configuration
//void Mutex::Check()
//{
//    // Check if the Mutex configuration value is set to "1"
//    if (Config::Mutex == "1")
//    {
//        // Define the desired length for the random string
//        int length = 20;
//
//        // Generate a random string with the given length
//        std::string randomString = generateRandomString(length);
//    }
//
//    // Create a named mutex with the name "MyMutex"
//    hMutex = CreateMutex(NULL, TRUE, "MyMutex");
//    // Check if mutex creation failed or if the mutex already exists (i.e., another instance is running)
//    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
//
//        // Exit the process if mutex creation failed or already exists (i.e., another process is using it)
//        ExitProcess(0);
//    }
//}
