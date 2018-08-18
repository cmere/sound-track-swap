#ifndef SWAPER_AVRAWDATA_H
#define SWAPER_AVRAWDATA_H

#include "FFmpegClasses.hpp"
#include <cstdint>
#include <list>
#include <memory>

namespace Swaper {

class AVRawData {
public:
    AVRawData(const FFmpegAVStreamPtr&);

    unsigned int appendData(const char* bytes, unsigned int size);
    unsigned int length() const { return length_; }

private:
    unsigned int length_ = 0;
    unsigned int capacity_ = 0; 
    std::shared_ptr<char> data_;
    FFmpegAVCodecParametersPtr codecParams_;
};

}  // namespace Swaper

#endif
