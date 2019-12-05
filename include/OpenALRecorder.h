#pragma once

#include <string>
#include <vector>
#include <AL/alc.h>
#include <AL/al.h>


class OpenALRecorder {
public:
    OpenALRecorder(const std::string& deviceName = "");

    ~OpenALRecorder();

    /**
     * Returns the capture device name
     */
    std::string getCaptureDeviceName();

    /**
     * Records audio from the capture device for seconds time and saves it to WAV file
     */
    void recordInFile(float seconds, const std::string& fileName);

private:
    void openAndWriteWAVHeader(std::ifstream& file);

    ALCdevice* mDevice;

    //std::ifstream mFile;

    //FILE* mFile;
    long mDataSizeOffset;
    ALuint mDataSize;

    ALuint mChannels;
    ALuint mBits;
    ALuint mSampleRate;
    ALuint mFrameSize;
    ALbyte* mBuffer;
    ALsizei mBufferSize;
};
