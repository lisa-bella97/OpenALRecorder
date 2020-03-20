#pragma once

#include <AL/alc.h>
#include <AL/al.h>

#include <string>
#include <vector>


class OpenALRecorder {
public:
    explicit OpenALRecorder(const std::string &deviceName = "", int channels = 1, int bits = 16,
                            int sampleRate = 9600);

    ~OpenALRecorder();

    /**
     * Returns the capture device name
     */
    std::string getCaptureDeviceName();

    /**
     * Records audio from the capture device for seconds time and saves it to WAV file
     */
    void recordInFile(float seconds, const std::string &fileName);

private:
    static void fwrite16le(ALushort val, FILE *f);

    static void fwrite32le(ALuint val, FILE *f);

    static void al_nssleep(unsigned long nsec);

    void openAndWriteWAVHeader(FILE *file);

    ALCdevice *mDevice;
    long mDataSizeOffset;
    ALuint mChannels;
    ALuint mBits;
    ALuint mSampleRate;
    ALuint mFrameSize;
};
