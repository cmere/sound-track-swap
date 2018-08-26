#include "AudioData.hpp"
#include <cmath>
#include <iostream>

using namespace std;

namespace 
{
using namespace Swaper;

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
    data_.reset(new uint8_t[bufLength_]);
}

unsigned int 
AudioData::appendData(uint8_t* bytes, int size)
{
    return size;
}

} // namespace Swaper
