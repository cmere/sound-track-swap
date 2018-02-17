/*
 * Copyright (c) 2012 Stefano Sabatini
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

#include "FFmpegDemuxer.h"
#include <fstream>
#include <iostream>
#include <string>
#include <boost/filesystem/convenience.hpp>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

using namespace std;

namespace Swaper {

int
FFmpegDemuxer::stripStream(const string& filename)
{
    const char* in_filename = filename.c_str();
    string *out_filename = NULL;
    AVFormatContext *ifmt_ctx = NULL;
    AVFormatContext **ofmt_ctxs = NULL;

    AVPacket pkt;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;

    int nb_streams = 0;

    auto releaseResources = [&]() {
        avformat_close_input(&ifmt_ctx);
        for (i = 0; i < nb_streams; ++i) {
            /* close output */
            if (ofmt_ctxs[i] && !(ofmt_ctxs[i]->oformat->flags & AVFMT_NOFILE))
                avio_closep(&ofmt_ctxs[i]->pb);
            avformat_free_context(ofmt_ctxs[i]);
        }

        if (ofmt_ctxs) 
            delete[] ofmt_ctxs;
        if (out_filename) 
            delete[] out_filename;
        if (stream_mapping) 
            delete[] stream_mapping;
    };

    av_register_all();

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        releaseResources();
        return 1;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        releaseResources();
        return 1;
    }


    nb_streams = ifmt_ctx->nb_streams;
    stream_mapping = new int[nb_streams];
    out_filename = new string[nb_streams];
    ofmt_ctxs = new AVFormatContext*[nb_streams];
    for (i = 0; i < nb_streams; ++i) {
        ofmt_ctxs[i] = NULL;
    }

    cout << "number of streams: " << nb_streams << endl;

    for (i = 0; i < nb_streams; i++) {
      cout << "==================== intput stream " << i << " ====================" << endl;
      av_dump_format(ifmt_ctx, i, in_filename, 0);
      cout << "=========================================================" << endl;

      AVStream *in_stream = ifmt_ctx->streams[i];
      AVCodecParameters *in_codecpar = in_stream->codecpar;

      if (   in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO
          && in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
          stream_mapping[i] = -1;
          cout << "ignore non auido/video stream " << i << ": type=" << in_codecpar->codec_type << endl;
          continue;
      }

      out_filename[i] = boost::filesystem::basename(filename) 
                        + '.' + to_string(i) + '-' 
                        + av_get_media_type_string(in_codecpar->codec_type) 
                        + boost::filesystem::extension(filename);
      avformat_alloc_output_context2(&ofmt_ctxs[i], NULL, NULL, out_filename[i].c_str());
      if (!ofmt_ctxs[i]) {
          fprintf(stderr, "Could not create output context\n");
          ret = AVERROR_UNKNOWN;
          releaseResources();
          return 1;
      }

      AVStream *out_stream;
      stream_mapping[i] = stream_index++;

      out_stream = avformat_new_stream(ofmt_ctxs[i], NULL);
      if (!out_stream) {
          fprintf(stderr, "Failed allocating output stream\n");
          ret = AVERROR_UNKNOWN;
          releaseResources();
          return 1;
      }

      ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
      if (ret < 0) {
          fprintf(stderr, "Failed to copy codec parameters\n");
          releaseResources();
          return 1;
      }
      out_stream->codecpar->codec_tag = 0;

      cout << "==================== output stream " << i << " ====================" << endl;
      av_dump_format(ofmt_ctxs[i], 0, out_filename[i].c_str(), 1);
      cout << "=========================================================" << endl;

      if (!(ofmt_ctxs[i]->oformat->flags & AVFMT_NOFILE)) {
          ret = avio_open(&ofmt_ctxs[i]->pb, out_filename[i].c_str(), AVIO_FLAG_WRITE);
          if (ret < 0) {
              fprintf(stderr, "Could not open output file '%s'", out_filename[i].c_str());
              releaseResources();
              return 1;
          }
      }

      ret = avformat_write_header(ofmt_ctxs[i], NULL);
      if (ret < 0) {
          cout << "Error occurred when opening output file " << out_filename[i] << endl;;
          releaseResources();
          return 1;
      }
    }

    while (1) {
        AVStream *in_stream, *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index >= nb_streams||
            stream_mapping[pkt.stream_index] < 0) {
            cout << "ignore packet in stream " << in_stream << endl;
            av_packet_unref(&pkt);
            continue;
        }

        int stream_index = pkt.stream_index;
        pkt.stream_index = 0;
        out_stream = ofmt_ctxs[stream_index]->streams[0];
        //log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        //log_packet(ofmt_ctx, &pkt, "out");

        ret = av_interleaved_write_frame(ofmt_ctxs[stream_index], &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    for (i = 0; i < nb_streams; ++i) {
        if (ofmt_ctxs[i]) {
            av_write_trailer(ofmt_ctxs[i]);
        }
    }

    releaseResources();

    if (ret < 0 && ret != AVERROR_EOF) {
        cout << "Error occurred: " << endl;;
        return 1;
    }

    cout << "done." <<endl;

    return 0;
}

int
FFmpegDemuxer::mux(const string& videoFilename, const string& audioFilename)
{
    return 0;
}

}  // namespace Swaper
