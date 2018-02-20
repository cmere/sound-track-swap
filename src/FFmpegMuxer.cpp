/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "FFmpegMuxer.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <typeinfo>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern "C" {

#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

using namespace std;

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

    private:
        std::shared_ptr<T> t_;
};

typedef FFmpegClass<AVFormatContext> FFmpegAVFormatContext;
typedef FFmpegClass<AVStream> FFmpegAVStream;
typedef FFmpegClass<AVCodecContext> FFmpegAVCodecContext;
typedef FFmpegClass<AVPacket> FFmpegAVPacket;

template<>
FFmpegClass<AVFormatContext>::FFmpegClass(AVFormatContext* t) {
    t_.reset(t, [](AVFormatContext* p) { avformat_free_context(p); });
}
template<>
FFmpegClass<AVStream>::FFmpegClass(AVStream* t) {
    t_.reset(t, [](AVStream* p) { });  // Deleter do nothing.
}
template<>
FFmpegClass<AVCodecContext>::FFmpegClass(AVCodecContext* t) {
    t_.reset(t, [](AVCodecContext* p) { avcodec_free_context(&p); });
}
template<>
FFmpegClass<AVPacket>::FFmpegClass(AVPacket* t) {
    t_.reset(t, [](AVPacket* p) { av_packet_unref(p); });
}

pair<FFmpegAVFormatContext, AVMediaType>
openMediaFile_(const string& filename)
{
    static auto InvalidRet = make_pair(FFmpegAVFormatContext(), AVMEDIA_TYPE_UNKNOWN);

    AVFormatContext* fmtctx = nullptr;
    if (avformat_open_input(&fmtctx, filename.c_str(), 0, 0) < 0) {
        cout << "Could not open input file " << filename << endl;
        return InvalidRet;
    }

    if (avformat_find_stream_info(fmtctx, 0) < 0) {
        cout << "Failed to retrieve input stream information in file " << filename << endl;
        return InvalidRet;
    }

    unsigned int numOfStreams = fmtctx->nb_streams;
    if (numOfStreams != 1) {
        cout << "input file must has exactly one stream: " << filename << " " << numOfStreams << endl;
        return InvalidRet;
    }

    if (fmtctx->streams) {
        AVStream* stream = fmtctx->streams[0];
        if (stream && stream->codecpar) {
            return make_pair(FFmpegAVFormatContext(fmtctx), stream->codecpar->codec_type);
        }
    }
    return InvalidRet;
}

int writeFrame_(AVFormatContext *ic, AVFormatContext *oc, int outStreamIdx, int64_t& pts)
{
    int numOfFramesWritten = 0;
    AVPacket avPkt;
    FFmpegAVPacket pkt(&avPkt);

    AVStream *in_stream, *out_stream;

    auto ret = av_read_frame(ic, pkt);
    if (ret < 0) {
        cout << "error or end of file." << endl;
        return numOfFramesWritten;
    }

    in_stream  = ic->streams[0];
    if (pkt->stream_index != 0) {
        cout << "invalid packet in input stream." << endl;
        return -1;
    }

    pkt->stream_index = outStreamIdx;
    out_stream = oc->streams[outStreamIdx];

    // copy packet
    pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
    pkt->pos = -1;

    ++numOfFramesWritten;
    pts = pkt->pts;
    cout << "write " << numOfFramesWritten << " frame(s) to stream " << outStreamIdx << " " << pts << endl;

    ret = av_interleaved_write_frame(oc, pkt);
    if (ret < 0) {
        cout << "Error muxing packet." << endl;
        return -1;
    }

    return numOfFramesWritten;
}

int 
FFmpegMuxer::mux(const string& inputFilename1, const string& inputFilename2, const string& outFilename)
{
    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *poc;
    int ret;

    av_register_all();

    filename = outFilename.c_str();
    avformat_alloc_output_context2(&poc, NULL, NULL, filename);
    if (!poc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&poc, NULL, "mpeg", filename);
    }
    if (!poc)
        return 1;

    FFmpegAVFormatContext oc(poc);
    fmt = oc->oformat;

    FFmpegAVFormatContext inVideoFmtCtx;
    FFmpegAVFormatContext inAudioFmtCtx;

    auto fmtctx_type = openMediaFile_(inputFilename1);
    if (fmtctx_type.first) {
        if (fmtctx_type.second == AVMEDIA_TYPE_VIDEO) {
            inVideoFmtCtx = fmtctx_type.first;
        }
        else if (fmtctx_type.second == AVMEDIA_TYPE_AUDIO) {
            inAudioFmtCtx = fmtctx_type.first;
        }
        else {
            cout << "input file " << inputFilename1 << " has unsupport media type: " << av_get_media_type_string(fmtctx_type.second) << endl;
            return 1;
        }
    }
    fmtctx_type = openMediaFile_(inputFilename2);
    if (fmtctx_type.first) {
        if (fmtctx_type.second == AVMEDIA_TYPE_VIDEO) {
            inVideoFmtCtx = fmtctx_type.first;
        }
        else if (fmtctx_type.second == AVMEDIA_TYPE_AUDIO) {
            inAudioFmtCtx = fmtctx_type.first;
        }
        else {
            cout << "input file " << inputFilename2 << " has unsupport media type: " << av_get_media_type_string(fmtctx_type.second) << endl;
            return 1;
        }
    }
    if (!inVideoFmtCtx) {
        cout << "Cannot find video input." << endl;
        return 1;
    }
    if (!inAudioFmtCtx) {
        cout << "Cannot find audio input." << endl;
        return 1;
    }

    // create streams
    FFmpegAVStream outVideoStream(avformat_new_stream(oc, nullptr));
    if (!outVideoStream) {
        cout << "Failed allocating output video stream." << endl;;
        return 1;
    }
    if (avcodec_parameters_copy(outVideoStream->codecpar, inVideoFmtCtx->streams[0]->codecpar) < 0) {
        cout << "Failed to copy video codec parameters." << endl;
        return 1;
    }
    outVideoStream->id = oc->nb_streams - 1;
    outVideoStream->codecpar->codec_tag = 0;

    FFmpegAVStream outAudioStream(avformat_new_stream(oc, nullptr));
    if (!outAudioStream) {
        cout << "Failed allocating output audio stream." << endl;;
        return 1;
    }
    if (avcodec_parameters_copy(outAudioStream->codecpar, inAudioFmtCtx->streams[0]->codecpar) < 0) {
        cout << "Failed to copy audio codec parameters." << endl;
        return 1;
    }
    outAudioStream->id = oc->nb_streams - 1;
    outAudioStream->codecpar->codec_tag = 0;

    av_dump_format(oc, 0, filename, 1);
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            cout << "Could not open file " << filename << endl;
            return 1;
        }
    }
    // Write the stream header, if any.
    ret = avformat_write_header(oc, nullptr);
    if (ret < 0) {
        cout << "Error occurred when opening output file: " << filename << " " << ret << endl;
        return 1;
    }

    int isVideoDone = 0, isAudioDone = 0;
    int64_t videoPts = 0, audioPts = 0;
    while (!isVideoDone || !isAudioDone) {
        if (   !isVideoDone 
             && av_compare_ts(videoPts, outVideoStream->time_base, audioPts, outAudioStream->time_base) <= 0) {
            // write video
            auto numOfFrameWritten = writeFrame_(inVideoFmtCtx, oc, outVideoStream->id, videoPts);
            if (numOfFrameWritten == 0) {
                isVideoDone = 1;
            }
            else if (numOfFrameWritten == -1) {
                return -1;
            }
        }
        else if (!isAudioDone) {
            // write audio
            auto numOfFrameWritten = writeFrame_(inAudioFmtCtx, oc, outAudioStream->id, audioPts);
            if (numOfFrameWritten == 0) {
                isAudioDone = 1;
            }
            else if (numOfFrameWritten == -1) {
                return -1;
            }
        }
    }

    av_write_trailer(oc);
    if (!(fmt->flags & AVFMT_NOFILE))
        avio_closep(&oc->pb);

    return 0;
}

}  // namespace Swaper
