#include "StartDelay.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

// Define the minimum sleep time in seconds (0 seconds)
const int sleepMin = 0;

// Define the maximum sleep time in seconds (10 seconds)
const int sleepMax = 11;

// Function to implement the delay or sleep mechanism
void StartDelay::Run()
{
    // Seed the random number generator with the current time, ensuring different random values each time
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Generate a random sleep time in seconds between sleepMin and sleepMax (inclusive)
    // The modulus operator (%) is used to get a value between 0 and (sleepMax - sleepMin)
    // Adding sleepMin ensures the value is within the range of [sleepMin, sleepMax]
    auto sleepTime = std::rand() % (sleepMax - sleepMin + 1) + sleepMin;

    // Convert the random sleep time from seconds to milliseconds by multiplying by 1000
    sleepTime *= 1000;

    // Log the sleep time to the console for debugging purposes
    //std::cout << "StartDelay: Sleeping " << sleepTime << " ms" << std::endl;

    // Pause the program execution for the calculated sleep time (in milliseconds)
    // std::this_thread::sleep_for takes a duration of type std::chrono::milliseconds
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
}
