#include <OpenALDevice.h>

#include <AL/al.h>
#include <AudioFile.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <cmath>


OpenALDevice::OpenALDevice() : mSamplesCaptured(0) {
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

void OpenALDevice::record(int seconds) {
    ALint samplesAvailable;
    ALubyte* captureBufPtr = mCaptureBuffer;
    auto start = std::chrono::system_clock::now();
    auto end = start;

    alcCaptureStart(mCaptureDevice);

    while (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() <= seconds) {
        // Get the number of samples available
        alcGetIntegerv(mCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

        // Copy the samples to our capture buffer
        if (samplesAvailable > 0) {
            alcCaptureSamples(mCaptureDevice, captureBufPtr, samplesAvailable);
            mSamplesCaptured += samplesAvailable;

            // Advance the buffer (two bytes per sample * number of samples)
            captureBufPtr += samplesAvailable * 2;
        }

        // Wait for a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Update the clock
        end = std::chrono::system_clock::now();
    }

    alcCaptureStop(mCaptureDevice);
}

void OpenALDevice::play() {
    ALuint buffer;
    ALuint source;

    // Generate an OpenAL buffer for the captured data
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);
    alBufferData(buffer, AL_FORMAT_MONO16, mCaptureBuffer, mSamplesCaptured * 2, 8000);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);

    // Wait for the source to stop playing
    auto playState = AL_PLAYING;
    while (playState == AL_PLAYING) {
        alGetSourcei(source, AL_SOURCE_STATE, &playState);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
}

void OpenALDevice::saveWAV(const std::string& fileName) {
    AudioFile<double> audioFile;

    audioFile.setSampleRate(8000);
    audioFile.setBitDepth(16);
    audioFile.setAudioBufferSize(1, mSamplesCaptured);

    AudioFile<double>::AudioBuffer buffer;
    // Set one channel
    buffer.resize(1);
    // Set number of samples per channel
    buffer[0].resize(mSamplesCaptured);
    // Fill the buffer with samples
    for (int i = 1; i < mSamplesCaptured * 2; i*=2)
        buffer[0][i] = mCaptureBuffer[i - 1] * std::pow(2, 8) + mCaptureBuffer[i];

    auto ok = audioFile.setAudioBuffer(buffer);
    if (!ok)
        throw std::runtime_error("unable to set audio buffer");

    ok = audioFile.save(fileName + ".wav", AudioFileFormat::Wave);
    if (!ok)
        throw std::runtime_error("unable to save audio buffer in file");
}
