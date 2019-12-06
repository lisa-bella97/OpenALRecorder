#include <OpenALRecorder.h>

#include <AL/al.h>
#include <AL/alext.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <cmath>
#include <cstdio>
#include <cstdlib>


static void fwrite16le(ALushort val, std::ofstream& f) {
    ALubyte data[2] = { (ALubyte)(val&0xff), (ALubyte)((val>>8)&0xff) };
    for (const auto& i : data)
        f << i;
}

static void fwrite32le(ALuint val, std::ofstream& f) {
    ALubyte data[4] = { (ALubyte)(val&0xff), (ALubyte)((val>>8)&0xff), (ALubyte)((val>>16)&0xff),
                        (ALubyte)((val>>24)&0xff) };
    for (const auto& i : data)
        f << i;
}

void al_nssleep(unsigned long nsec) {
    timespec ts{}, rem{};
    ts.tv_sec = static_cast<time_t>(nsec / 1000000000ul);
    ts.tv_nsec = static_cast<long>(nsec % 1000000000ul);
    while (nanosleep(&ts, &rem) == -1 && errno == EINTR)
        ts = rem;
}


OpenALRecorder::OpenALRecorder(const std::string& deviceName) :
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
}

OpenALRecorder::~OpenALRecorder() {
    alcCaptureCloseDevice(mDevice);
}

std::string OpenALRecorder::getCaptureDeviceName() {
    return alcGetString(mDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
}

void OpenALRecorder::recordInFile(float seconds, const std::string& fileName) {
    std::ofstream file(fileName, std::ios::binary);
    if (!file)
        throw std::runtime_error("unable to open a file");

    openAndWriteWAVHeader(file);

    auto err = ALC_NO_ERROR;
    alcCaptureStart(mDevice);

    while (static_cast<double>(mDataSize) / static_cast<double>(mSampleRate) < seconds &&
           (err = alcGetError(mDevice)) == ALC_NO_ERROR && !file.fail()) {
        ALCint count = 0;
        alcGetIntegerv(mDevice, ALC_CAPTURE_SAMPLES, 1, &count);
        if (count < 1) {
            al_nssleep(10000000);
            //std::this_thread::sleep_for(std::chrono::nanoseconds(10000000));
            continue;
        }
        if (count > mBufferSize) {
            //auto data = new ALbyte[mFrameSize];
            auto data = (ALbyte*)calloc(mFrameSize, (ALuint)count);
            free(mBuffer);
            mBuffer = data;
            mBufferSize = count;
        }
        alcCaptureSamples(mDevice, mBuffer, count);

        for (auto i = 0; i < mBufferSize * mFrameSize; i++)
            file << mBuffer[i];
        mDataSize += mBufferSize * mFrameSize;
        //mDataSize += (ALuint)fwrite(mBuffer, mFrameSize, (ALuint)count, mFile);
    }

    alcCaptureStop(mDevice);

    if (err != ALC_NO_ERROR)
        throw std::runtime_error("device error while recording: " + std::string(alcGetString(mDevice, err)));

    free(mBuffer);
    mBuffer = nullptr;
    mBufferSize = 0;

    auto total_size = file.tellp();
    if (file.seekp(mDataSizeOffset, std::ios_base::beg)) {
        fwrite32le(mDataSize * mFrameSize, file);
        if(file.seekp(4, std::ios_base::beg))
            fwrite32le((ALuint)total_size - 8, file);
    }

    file.close();
}

void OpenALRecorder::openAndWriteWAVHeader(std::ofstream& file) {
    file << "RIFF";
    fwrite32le(0xFFFFFFFF, file); // 'RIFF' header len; filled in at close

    file << "WAVE";

    file << "fmt";
    fwrite32le(18, file); // 'fmt ' header len

    // 16-bit val, format type id (1 = integer PCM, 3 = float PCM)
    fwrite16le(mBits == 32 ? 0x0003 : 0x0001, file);
    // 16-bit val, channel count
    fwrite16le((ALushort)mChannels, file);
    // 32-bit val, frequency
    fwrite32le(mSampleRate, file);
    // 32-bit val, bytes per second
    fwrite32le(mSampleRate * mFrameSize, file);
    // 16-bit val, frame size
    fwrite16le((ALushort)mFrameSize, file);
    // 16-bit val, bits per sample
    fwrite16le((ALushort)mBits, file);
    // 16-bit val, extra byte count
    fwrite16le(0, file);

    file << "data";
    fwrite32le(0xFFFFFFFF, file); // 'data' header len; filled in at close

    mDataSizeOffset = file.tellp() - 4L;
    if (file.fail() || mDataSizeOffset < 0) {
        file.close();
        throw std::runtime_error("unable to write header");
    }
}
