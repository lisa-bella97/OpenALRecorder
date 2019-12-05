#include <OpenALRecorder.h>
#include <iostream>

int main() {
    try {
        OpenALRecorder recorder;
        std::cout << "Opened device: " << recorder.getCaptureDeviceName() << std::endl;
        recorder.recordInFile(4, "../saved/5.wav");
    } catch (const std::exception& e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
