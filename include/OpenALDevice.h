#pragma once

#include <string>
#include <vector>
#include <AL/alc.h>
#include <AL/al.h>


class OpenALDevice {
public:
    OpenALDevice();
    ~OpenALDevice();

    /**
     * Returns the capture device name
     */
    std::string getCaptureDeviceName();

    /**
     * Records audio from the device for seconds time
     */
    void record(int seconds);

    void play();

    void saveWAV(const std::string& fileName);

private:
    ALCdevice* mMainDevice;
    ALCdevice* mCaptureDevice;
    ALint mSamplesCaptured;
    ALubyte mCaptureBuffer[1048576];
};
