#include <OpenALDevice.h>

#include <AL/al.h>

#include <cstring>
#include <stdexcept>


OpenALDevice::OpenALDevice() {
    mMainDevice = alcOpenDevice(nullptr);
    if (mMainDevice == nullptr)
        throw std::runtime_error("unable to open main device");

    auto context = alcCreateContext(mMainDevice, nullptr);
    if (context == nullptr)
        throw std::runtime_error("unable to create context");

    // Make the playback context current
    alcMakeContextCurrent(context);
    alcProcessContext(context);

    mCaptureDevice = alcCaptureOpenDevice(nullptr, 8000, AL_FORMAT_MONO16, 800);
    if (mCaptureDevice == nullptr)
        throw std::runtime_error("unable to open capture device");
}

OpenALDevice::~OpenALDevice() {
    alcMakeContextCurrent(nullptr);
    alcCloseDevice(mMainDevice);
    alcCaptureCloseDevice(mCaptureDevice);
}

std::string OpenALDevice::getCaptureDeviceName() {
    return alcGetString(mCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
}
