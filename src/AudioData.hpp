#ifndef SWAPER_AUDIODATA_H
#define SWAPER_AUDIODATA_H

#include "FFmpegClasses.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace Swaper {

/**
 * In demux, buffer that hold decoded audio data.
 *
 * Buffer size is estimated based on duration, sample rate, etc. 
 */
class AudioData {
public:
    AudioData(const FFmpegAVStreamPtr&);

    int getSize() const;
    //int getBufferLength() const { return bufLength_; }

    FFmpegAVCodecParametersPtr getCodecParams() const { return codecParams_; }

    unsigned int appendData(uint8_t* bytes, int size, int channel);

private:
    int numOfChannels_() const;
    int channelLength_() const;

private:
    unsigned int sizePerChannel_ = 0;
    unsigned int bufLength_ = 0;    // estimate size * 1.1 (10% more)
    std::shared_ptr<uint8_t> buf_;  // |channel 1 data|padding|channel 2 data|padding|channel 3 data|padding......
    std::vector<uint8_t*> channelStart_;
    std::vector<int> channelSize_;

    FFmpegAVCodecParametersPtr codecParams_;
};

}  // namespace Swaper

#endif
