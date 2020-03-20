#include <OpenALRecorder.h>

#include <AL/alext.h>

#include <stdexcept>


OpenALRecorder::OpenALRecorder(const std::string &deviceName, int channels, int bits, int sampleRate) :
        mDataSizeOffset(0),
        mChannels(channels),
        mBits(bits),
        mSampleRate(sampleRate) {
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

void OpenALRecorder::recordInFile(float seconds, const std::string &fileName) {
    auto file = fopen(fileName.c_str(), "wb");
    if (!file)
        throw std::runtime_error("unable to open a file");

    openAndWriteWAVHeader(file);

    auto err = ALC_NO_ERROR;
    auto dataSize = 0;
    ALbyte *buffer = nullptr;
    ALsizei bufferSize = 0;

    alcCaptureStart(mDevice);

    while (static_cast<double>(dataSize) / static_cast<double>(mSampleRate) < seconds &&
           (err = alcGetError(mDevice)) == ALC_NO_ERROR && !ferror(file)) {
        ALCint count = 0;
        alcGetIntegerv(mDevice, ALC_CAPTURE_SAMPLES, 1, &count);

        if (count < 1) {
            al_nssleep(10000000);
            continue;
        }

        if (count > bufferSize) {
            auto data = static_cast<ALbyte *>(calloc(mFrameSize, static_cast<ALuint>(count)));
            free(buffer);
            buffer = data;
            bufferSize = count;
        }

        alcCaptureSamples(mDevice, buffer, count);
        dataSize += static_cast<ALuint>(fwrite(buffer, mFrameSize, static_cast<ALuint>(count), file));
    }

    alcCaptureStop(mDevice);

    if (err != ALC_NO_ERROR)
        throw std::runtime_error("device error while recording: " + std::string(alcGetString(mDevice, err)));

    free(buffer);

    auto total_size = ftell(file);
    if (fseek(file, mDataSizeOffset, SEEK_SET) == 0) {
        fwrite32le(dataSize * mFrameSize, file);
        if (fseek(file, 4, SEEK_SET) == 0)
            fwrite32le(static_cast<ALuint>(total_size) - 8, file);
    }

    fclose(file);
}

void OpenALRecorder::fwrite16le(ALushort val, FILE *f) {
    ALubyte data[2] = {static_cast<ALubyte>(val & 0xff), static_cast<ALubyte>((val >> 8) & 0xff)};
    fwrite(data, 1, 2, f);
}

void OpenALRecorder::fwrite32le(ALuint val, FILE *f) {
    ALubyte data[4] = {static_cast<ALubyte>(val & 0xff), static_cast<ALubyte>((val >> 8) & 0xff),
                       static_cast<ALubyte>((val >> 16) & 0xff), static_cast<ALubyte>((val >> 24) & 0xff)};
    fwrite(data, 1, 4, f);
}

void OpenALRecorder::al_nssleep(unsigned long nsec) {
    timespec ts{}, rem{};
    ts.tv_sec = static_cast<time_t>(nsec / 1000000000ul);
    ts.tv_nsec = static_cast<long>(nsec % 1000000000ul);
    while (nanosleep(&ts, &rem) == -1 && errno == EINTR)
        ts = rem;
}

void OpenALRecorder::openAndWriteWAVHeader(FILE *file) {
    fputs("RIFF", file);
    fwrite32le(0xFFFFFFFF, file); // 'RIFF' header len; filled in at close

    fputs("WAVE", file);

    fputs("fmt ", file);
    fwrite32le(18, file); // 'fmt' header len

    // 16-bit val, format type id (1 = integer PCM, 3 = float PCM)
    fwrite16le(mBits == 32 ? 0x0003 : 0x0001, file);
    // 16-bit val, channel count
    fwrite16le(static_cast<ALushort>(mChannels), file);
    // 32-bit val, frequency
    fwrite32le(mSampleRate, file);
    // 32-bit val, bytes per second
    fwrite32le(mSampleRate * mFrameSize, file);
    // 16-bit val, frame size
    fwrite16le(static_cast<ALushort>(mFrameSize), file);
    // 16-bit val, bits per sample
    fwrite16le(static_cast<ALushort>(mBits), file);
    // 16-bit val, extra byte count
    fwrite16le(0, file);

    fputs("data", file);
    fwrite32le(0xFFFFFFFF, file); // 'data' header len; filled in at close

    mDataSizeOffset = ftell(file) - 4;
    if (ferror(file) || mDataSizeOffset < 0) {
        fclose(file);
        throw std::runtime_error("unable to write header");
    }
}
