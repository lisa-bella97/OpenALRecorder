#include <OpenALRecorder.h>

#include <AL/al.h>
#include <AL/alext.h>
#include <AudioFile.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>


static void fwrite16le(ALushort val, FILE *f)
{
    ALubyte data[2] = { (ALubyte)(val&0xff), (ALubyte)((val>>8)&0xff) };
    fwrite(data, 1, 2, f);
}

static void fwrite32le(ALuint val, FILE *f)
{
    ALubyte data[4] = { (ALubyte)(val&0xff), (ALubyte)((val>>8)&0xff), (ALubyte)((val>>16)&0xff),
                        (ALubyte)((val>>24)&0xff) };
    fwrite(data, 1, 4, f);
}

void al_nssleep(unsigned long nsec)
{
    struct timespec ts{}, rem{};
    ts.tv_sec = (time_t)(nsec / 1000000000ul);
    ts.tv_nsec = (long)(nsec % 1000000000ul);
    while(nanosleep(&ts, &rem) == -1 && errno == EINTR)
        ts = rem;
}


OpenALRecorder::OpenALRecorder(const std::string& deviceName) :
        mFile(nullptr),
        mDataSizeOffset(0),
        mDataSize(0),
        mChannels(1),
        mBits(16),
        mSampleRate(44100),
        mBuffer(nullptr),
        mBufferSize(0) {
    mFrameSize = mChannels * mBits / 8;

    auto format = AL_NONE;
    if (mChannels == 1) {
        if (mBits == 8)
            format = AL_FORMAT_MONO8;
        else if (mBits == 16)
            format = AL_FORMAT_MONO16;
        else if (mBits == 32)
            format = AL_FORMAT_MONO_FLOAT32;
    } else if (mChannels == 2) {
        if (mBits == 8)
            format = AL_FORMAT_STEREO8;
        else if (mBits == 16)
            format = AL_FORMAT_STEREO16;
        else if (mBits == 32)
            format = AL_FORMAT_STEREO_FLOAT32;
    }

    mDevice = alcCaptureOpenDevice(deviceName.empty() ? nullptr : deviceName.c_str(), mSampleRate, format, 32768);
    if (!mDevice)
        throw std::runtime_error("unable to open device");

    /*mMainDevice = alcOpenDevice(nullptr);
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
        throw std::runtime_error("unable to open capture device");*/
}

OpenALRecorder::~OpenALRecorder() {
    //alcMakeContextCurrent(nullptr);
    alcCaptureCloseDevice(mDevice);
}

std::string OpenALRecorder::getCaptureDeviceName() {
    return alcGetString(mDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
}

void OpenALRecorder::recordInFile(float seconds, const std::string& fileName) {
    openAndWriteWAVHeader(fileName);

    auto err = ALC_NO_ERROR;
    alcCaptureStart(mDevice);

    while ((double)mDataSize / (double)mSampleRate < seconds &&
           (err = alcGetError(mDevice)) == ALC_NO_ERROR && !ferror(mFile)) {
        ALCint count = 0;
        alcGetIntegerv(mDevice, ALC_CAPTURE_SAMPLES, 1, &count);
        if (count < 1) {
            al_nssleep(10000000);
            continue;
        }
        if (count > mBufferSize) {
            auto data = (ALbyte*)calloc(mFrameSize, (ALuint)count);
            free(mBuffer);
            mBuffer = data;
            mBufferSize = count;
        }
        alcCaptureSamples(mDevice, mBuffer, count);

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
        /* Byteswap multibyte samples on big-endian systems (wav needs little-
         * endian, and OpenAL gives the system's native-endian).
         */
        if(recorder.mBits == 16)
        {
            ALCint i;
            for(i = 0;i < count*recorder.mChannels;i++)
            {
                ALbyte b = recorder.mBuffer[i*2 + 0];
                recorder.mBuffer[i*2 + 0] = recorder.mBuffer[i*2 + 1];
                recorder.mBuffer[i*2 + 1] = b;
            }
        }
        else if(recorder.mBits == 32)
        {
            ALCint i;
            for(i = 0;i < count*recorder.mChannels;i++)
            {
                ALbyte b0 = recorder.mBuffer[i*4 + 0];
                ALbyte b1 = recorder.mBuffer[i*4 + 1];
                recorder.mBuffer[i*4 + 0] = recorder.mBuffer[i*4 + 3];
                recorder.mBuffer[i*4 + 1] = recorder.mBuffer[i*4 + 2];
                recorder.mBuffer[i*4 + 2] = b1;
                recorder.mBuffer[i*4 + 3] = b0;
            }
        }
#endif
        mDataSize += (ALuint)fwrite(mBuffer, mFrameSize, (ALuint)count, mFile);
    }

    alcCaptureStop(mDevice);

    if (err != ALC_NO_ERROR)
        throw std::runtime_error("device error while recording: " + std::string(alcGetString(mDevice, err)));

    free(mBuffer);
    mBuffer = nullptr;
    mBufferSize = 0;

    auto total_size = ftell(mFile);
    if (fseek(mFile, mDataSizeOffset, SEEK_SET) == 0) {
        fwrite32le(mDataSize * mFrameSize, mFile);
        if(fseek(mFile, 4, SEEK_SET) == 0)
            fwrite32le((ALuint)total_size - 8, mFile);
    }

    fclose(mFile);
    mFile = nullptr;
}

void OpenALRecorder::play() {
    /*ALuint buffer;
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
    alDeleteBuffers(1, &buffer);*/
}

void OpenALRecorder::openAndWriteWAVHeader(const std::string& fileName) {
    mFile = fopen(fileName.c_str(), "wb");
    if (!mFile)
        throw std::runtime_error("unable to open a file");

    fputs("RIFF", mFile);
    fwrite32le(0xFFFFFFFF, mFile); // 'RIFF' header len; filled in at close

    fputs("WAVE", mFile);

    fputs("fmt ", mFile);
    fwrite32le(18, mFile); // 'fmt ' header len

    // 16-bit val, format type id (1 = integer PCM, 3 = float PCM)
    fwrite16le(mBits == 32 ? 0x0003 : 0x0001, mFile);
    // 16-bit val, channel count
    fwrite16le((ALushort)mChannels, mFile);
    // 32-bit val, frequency
    fwrite32le(mSampleRate, mFile);
    // 32-bit val, bytes per second
    fwrite32le(mSampleRate * mFrameSize, mFile);
    // 16-bit val, frame size
    fwrite16le((ALushort)mFrameSize, mFile);
    // 16-bit val, bits per sample
    fwrite16le((ALushort)mBits, mFile);
    // 16-bit val, extra byte count
    fwrite16le(0, mFile);

    fputs("data", mFile);
    fwrite32le(0xFFFFFFFF, mFile); // 'data' header len; filled in at close

    mDataSizeOffset = ftell(mFile) - 4;
    if (ferror(mFile) || mDataSizeOffset < 0) {
        fclose(mFile);
        throw std::runtime_error("unable to write header");
    }
}
