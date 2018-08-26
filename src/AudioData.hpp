#ifndef SWAPER_AUDIODATA_H
#define SWAPER_AUDIODATA_H

#include "FFmpegClasses.hpp"
#include <cstdint>
#include <list>
#include <memory>

namespace Swaper {

/**
 * In demux, buffer that hold decoded audio data.
 *
 * Buffer size is estimated based on duration, sample rate, etc. 
 */
class AudioData {
public:
    AudioData(const FFmpegAVStreamPtr&);

    unsigned int size() const { return size_; }
    FFmpegAVCodecParametersPtr getCodecParams() const { return codecParams_; }

    unsigned int appendData(uint8_t* bytes, int size);

private:
    unsigned int size_ = 0;
    unsigned int bufLength_ = 0; 
    std::shared_ptr<uint8_t> buf_;
    FFmpegAVCodecParametersPtr codecParams_;
};

}  // namespace Swaper

#endif
