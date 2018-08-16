#ifndef SWAPER_FFMPEGCLASSES_H
#define SWAPER_FFMPEGCLASSES_H

extern "C" {

#include <libavformat/avformat.h>

}

#include <memory>

namespace Swaper {

template<typename T>
class FFmpegClass {
    public:
        FFmpegClass() { }

        FFmpegClass(T* t);
        //{
        //    static_assert(false, "Need template specialization in constructor ");
        //}

        operator T*() const { return t_.get(); }
        T* operator->() const { return t_.get(); }
        void reset() { t_.reset(); }

    private:
        std::shared_ptr<T> t_;
};

template<> FFmpegClass<AVFormatContext>::FFmpegClass(AVFormatContext* t);
template<> FFmpegClass<AVStream>::FFmpegClass(AVStream* t);
template<> FFmpegClass<AVCodecContext>::FFmpegClass(AVCodecContext* t);
template<> FFmpegClass<AVPacket>::FFmpegClass(AVPacket* t); 

typedef FFmpegClass<AVFormatContext> FFmpegAVFormatContextPtr;
typedef FFmpegClass<AVStream> FFmpegAVStreamPtr;
typedef FFmpegClass<AVCodecContext> FFmpegAVCodecContextPtr;
typedef FFmpegClass<AVPacket> FFmpegAVPacketPtr;

}

#endif
