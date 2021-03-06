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


#include "FFmpegMuxer.hpp"
#include "FFmpegClasses.hpp"
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

pair<FFmpegAVFormatContextPtr, int>
openVideoMediaFile_(const string& filename)
{
    static auto InvalidRet = make_pair(FFmpegAVFormatContextPtr(), -1);

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
    for (unsigned int i = 0; i < numOfStreams; ++i) {
        if (fmtctx->streams) {
            AVStream* stream = fmtctx->streams[i];
            if (stream && stream->codecpar) {
                if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ) {
                    return make_pair(FFmpegAVFormatContextPtr(fmtctx), i);
                }
            }
        }
    }

    if (numOfStreams != 1) {
        cout << "input file must has exactly one stream: " << filename << " " << numOfStreams << endl;
        return InvalidRet;
    }

    if (fmtctx->streams) {
        AVStream* stream = fmtctx->streams[0];
        if (stream && stream->codecpar) {
            return make_pair(FFmpegAVFormatContextPtr(fmtctx), stream->codecpar->codec_type);
        }
    }
    return InvalidRet;
}

pair<FFmpegAVFormatContextPtr, int>
openAudioMediaFile_(const string& filename)
{
    static auto InvalidRet = make_pair(FFmpegAVFormatContextPtr(), -1);

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
        cout << "audio input file must has exactly one stream: " << filename << " " << numOfStreams << endl;
        return InvalidRet;
    }

    if (fmtctx->streams) {
        AVStream* stream = fmtctx->streams[0];
        if (stream && stream->codecpar && stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            return make_pair(FFmpegAVFormatContextPtr(fmtctx), 0);
        }
    }
    return InvalidRet;
}

int writeFrame_(AVFormatContext *ic, int inStreamIdx, AVFormatContext *oc, int outStreamIdx, int64_t& pts, bool isAudio = false)
{
    int numOfFramesWritten = 0;
    AVPacket avPkt;
    FFmpegAVPacketPtr pkt(&avPkt);

    AVStream *in_stream, *out_stream;

    do {
        auto ret = av_read_frame(ic, pkt);
        if (ret < 0) {
            cout << "error or end of file." << endl;
            return numOfFramesWritten;
        }
    } while (pkt->stream_index != inStreamIdx);

    in_stream  = ic->streams[inStreamIdx];

    pkt->stream_index = outStreamIdx;
    out_stream = oc->streams[outStreamIdx];

    // copy packet
    pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
    pkt->pos = -1;

    ++numOfFramesWritten;
    pts = pkt->pts;
    if (isAudio) {
        double tb = out_stream->time_base.num * 1.0 / out_stream->time_base.den;
        pkt->pts -= 82 / 25.0 / tb;
        pkt->dts -= 82 / 25.0 / tb;
    }
    cout << "write " << numOfFramesWritten << " frame(s) to stream " << outStreamIdx << " " << pts << endl;

    auto ret = av_interleaved_write_frame(oc, pkt);
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

    FFmpegAVFormatContextPtr oc(poc);
    fmt = oc->oformat;

    FFmpegAVFormatContextPtr inVideoFmtCtx;
    FFmpegAVFormatContextPtr inAudioFmtCtx;
    int inVideoStreamIdx = -1;

    auto fmtctx_idx = openVideoMediaFile_(inputFilename1);
    if (fmtctx_idx.second >= 0 ) {
        inVideoFmtCtx = fmtctx_idx.first;
        inVideoStreamIdx = fmtctx_idx.second;
    }
    else {
        cout << "input file " << inputFilename1 << " does not contais video stream." << endl;
        return 1;
    }
    fmtctx_idx = openAudioMediaFile_(inputFilename2);
    if (fmtctx_idx.second == 0) {
        inAudioFmtCtx = fmtctx_idx.first;
    }
    else {
        cout << "input file " << inputFilename2 << " must contain exactly one audio stream." << endl;
        return 1;
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
    FFmpegAVStreamPtr outVideoStream(avformat_new_stream(oc, nullptr));
    if (!outVideoStream) {
        cout << "Failed allocating output video stream." << endl;;
        return 1;
    }
    if (avcodec_parameters_copy(outVideoStream->codecpar, inVideoFmtCtx->streams[inVideoStreamIdx]->codecpar) < 0) {
        cout << "Failed to copy video codec parameters." << endl;
        return 1;
    }
    outVideoStream->id = oc->nb_streams - 1;
    outVideoStream->codecpar->codec_tag = 0;

    FFmpegAVStreamPtr outAudioStream(avformat_new_stream(oc, nullptr));
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
            auto numOfFrameWritten = writeFrame_(inVideoFmtCtx, inVideoStreamIdx, oc, outVideoStream->id, videoPts);
            if (numOfFrameWritten == 0) {
                isVideoDone = 1;
            }
            else if (numOfFrameWritten == -1) {
                return -1;
            }
        }
        else if (!isAudioDone) {
            // write audio
            auto numOfFrameWritten = writeFrame_(inAudioFmtCtx, 0, oc, outAudioStream->id, audioPts, true);
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
