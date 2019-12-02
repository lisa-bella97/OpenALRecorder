#pragma once

#include <string>
#include <vector>
#include <AL/alc.h>


class OpenALDevice {
public:
    OpenALDevice();
    ~OpenALDevice();

    /**
     * Returns the device name
     */
    std::string getCaptureDeviceName();

private:
    ALCdevice* mMainDevice;
    ALCdevice* mCaptureDevice;
};
