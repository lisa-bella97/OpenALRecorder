#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <OpenALDevice.h>
#include <iostream>
#include <unistd.h>


int main() {
    try {
        OpenALDevice helper;
        time_t currentTime;
        time_t lastTime;

        std::cout << "Opened device: " << helper.getCaptureDeviceName() << std::endl;

        // Wait for three seconds to prompt the user
        for (auto i = 3; i > 0; i--)
        {
            printf("Starting capture in %d...\r", i);
            fflush(stdout);
            lastTime = time(NULL);
            currentTime = lastTime;
            while (currentTime == lastTime)
            {
                currentTime = time(NULL);
                usleep(100000);
            }
        }

        helper.record(5);
        printf("Done!\n");
        helper.saveWAV("../saved/2");
        printf("Starting playback...\n");
        helper.play();
    } catch (const std::exception& e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
        return -1;
    }
}
