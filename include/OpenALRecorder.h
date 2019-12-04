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
     * Records audio to the capture device for seconds time
     */
    void recordInFile(float seconds, const std::string& fileName);

    /**
     * Plays captured audio from the capture device
     */
    void play();

    /**
     * Saves captured audio to the wav file
     */


private:
    void openAndWriteWAVHeader(const std::string& fileName);

    ALCdevice* mDevice;

    FILE* mFile;
    long mDataSizeOffset;
    ALuint mDataSize;

    ALuint mChannels;
    ALuint mBits;
    ALuint mSampleRate;
    ALuint mFrameSize;
    ALbyte* mBuffer;
    ALsizei mBufferSize;
};
