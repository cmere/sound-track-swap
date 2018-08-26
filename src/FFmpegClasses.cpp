#include "FFmpegClasses.hpp"
#include <iostream>

using namespace std;

namespace Swaper {

template<>
FFmpegClass<AVFormatContext>::FFmpegClass(AVFormatContext* t) 
{
    t_.reset(t, [](AVFormatContext* p) { cout << "AVFormatContext dtor" << endl; if (p) avformat_free_context(p); });
}

template<>
FFmpegClass<AVStream>::FFmpegClass(AVStream* t) {
    t_.reset(t, [](AVStream* p) { cout << "AVStream dtor" << endl; });  // Deleter do nothing.
}

template<>
FFmpegClass<AVCodecContext>::FFmpegClass(AVCodecContext* t) 
{
    t_.reset(t, [](AVCodecContext* p) { cout << "AVCodecContext dtor" << endl; if (p) avcodec_free_context(&p); });
}

template<>
FFmpegClass<AVPacket>::FFmpegClass(AVPacket* t)
{
    t_.reset(t, [](AVPacket* p) { if (p) av_packet_unref(p); });
}

template<> 
FFmpegClass<AVCodecParameters>::FFmpegClass(AVCodecParameters* t)
{
    AVCodecParameters* dp = avcodec_parameters_alloc();
    cout << "AVCodecParameters ctor " << dp << endl;
    avcodec_parameters_copy(dp, t);
    t_.reset(dp, [](AVCodecParameters* p) { cout << "AVCodecParameters dtor" << endl; if (p) avcodec_parameters_free(&p); });
}

}
