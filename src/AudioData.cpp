#include "AudioData.hpp"
#include <cmath>
#include <iostream>

using namespace std;

namespace 
{
using namespace Swaper;

const float BUFFER_LENGTH_ESTIMATE_MULTIPLIER = 1.01;   // 1% more

uint32_t 
estimateSize_(const FFmpegAVStreamPtr& stream)
{
    uint32_t ret = 0;

    if (stream && stream->codecpar) {
        auto& timebase = stream->time_base;
        auto& duration = stream->duration;

        auto& codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            auto& sampleRate = stream->codecpar->sample_rate;
            auto& channels = stream->codecpar->channels;
            AVSampleFormat sampleFormat = (AVSampleFormat)stream->codecpar->format;
            int bytesPerSample = av_get_bytes_per_sample(sampleFormat);

            ret = ceil(duration * av_q2d(timebase) * sampleRate * bytesPerSample * channels);
            cout << av_q2d(timebase) << " " << duration << " " << sampleRate << " " << channels << " "<< bytesPerSample << " " << ret << endl;
        }
        else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            cout << "TODO: video stream" << endl;
        }
        else {
            cout << "not AV stream: " << av_get_media_type_string(codecpar->codec_type) << endl;
        }
    }

    return ret;
}

} // namespace anonymous

namespace Swaper {

AudioData::AudioData(const FFmpegAVStreamPtr& stream) 
{
    if (!stream || !stream->codecpar) {
        return;
    }

    codecParams_ = FFmpegAVCodecParametersPtr(stream->codecpar);

    bufLength_ = estimateSize_(stream);
    if (bufLength_ <= 0) {
        cout << "AV raw data has invalid estimate size: " << bufLength_ << endl;
        return;
    }
    bufLength_ *= BUFFER_LENGTH_ESTIMATE_MULTIPLIER;
    buf_.reset(new uint8_t[bufLength_]);
    for (int i = 0; i < numOfChannels_(); ++i) {
        channelStart_.push_back(buf_.get() + i * channelLength_());
    }
    channelSize_.resize(numOfChannels_());
}

int
AudioData::getSize() const
{
    int size = 0;
    for (const auto& i : channelSize_) {
        size += i;
    }
    return size;
}

int 
AudioData::numOfChannels_() const
{
    if (codecParams_) {
        return codecParams_->channels;
    }
    return 0;
}

int
AudioData::channelLength_() const
{
    if (numOfChannels_() > 0) {
        return bufLength_ / numOfChannels_();
    }
    return 0;
}

unsigned int 
AudioData::appendData(uint8_t* bytes, int size, int channel)
{
    if (!bytes || size <= 0 || channel < 0 || channel >= numOfChannels_()) {
        return 0;
    }

    if (channelSize_[channel] + size >= channelLength_()) {
        cout << "audio data buffer overflow, ignore. " << channel << " " << size << " " << channelSize_[channel] << " " << channelLength_() << endl;
        return 0;
    }

    auto* dst = channelStart_[channel] + channelSize_[channel];
    memcpy(dst, bytes, size);
    channelSize_[channel] += size;

    return size;
}

} // namespace Swaper
