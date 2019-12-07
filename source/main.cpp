#include <OpenALRecorder.h>

#include <cstring>
#include <iostream>


int main(int argc, char** argv) {
    std::string deviceName;
    int channels = 1;
    int bits = 16;
    int sampleRate = 44100;
    int time = 4;
    std::string fileName = "../saved/default.wav";

    const std::string helpText = "Usage:\n" + std::string(argv[0]) + "\n"
                                 "[--device {device name}]\n"
                                 "[--channels {1 or 2}]\n"
                                 "[--bits {8, 16 or 32 bits}]\n"
                                 "[--rate {record rate in Hz}]\n"
                                 "[--time {record time in seconds}]\n"
                                 "[--file {file name (*.wav)}]\n";

    for (auto i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--device") == 0)
            deviceName = argv[i + 1];
        else if (std::strcmp(argv[i], "--channels") == 0)
            channels = std::atoi(argv[i + 1]);
        else if (std::strcmp(argv[i], "--bits") == 0)
            bits = std::atoi(argv[i + 1]);
        else if (std::strcmp(argv[i], "--rate") == 0)
            sampleRate = std::atoi(argv[i + 1]);
        else if (std::strcmp(argv[i], "--time") == 0)
            time = std::atoi(argv[i + 1]);
        else if (std::strcmp(argv[i], "--file") == 0)
            fileName = argv[i + 1];
        else if (std::strcmp(argv[i], "--help") == 0) {
            std::cout << helpText;
            return 0;
        } else {
            std::cout << "Wrong args." << std::endl << helpText;
            return -1;
        }
    }

    try {
        OpenALRecorder recorder(deviceName, channels, bits, sampleRate);
        std::cout << "Opened device: " << recorder.getCaptureDeviceName() << std::endl;
        recorder.recordInFile(time, fileName);
    } catch (const std::exception& e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}
