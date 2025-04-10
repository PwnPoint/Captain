#include "time.h"
#include <windows.h>
#include <cstdlib>
#include <ctime>

// Function to generate a random delay between min and max milliseconds
void Time::randomDelay() {
    int minMs = 0, maxMs = 100;
    // Seed the random number generator (this should be done once)
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(0)));
        seeded = true;
    }

    // Generate a random number between minMs and maxMs
    int delayMs = minMs + rand() % (maxMs - minMs + 1);

    // Sleep for the random delay
    Sleep(delayMs);
}
