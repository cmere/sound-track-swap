#include "AVFormatFile.hpp"

#include <iostream>

using namespace std;

namespace {

using namespace Swaper;


}  // namespace anonymous

namespace Swaper {

AVFormatFile::~AVFormatFile() 
{
    if (fmtCtx_) {
        avformat_close_input(&fmtCtx_);
        fmtCtx_ = nullptr;
    }
}

bool 
AVFormatFile::openFile(const string& filename) 
{
    av_register_all();

    int ret = 0;
    ret = avformat_open_input(&fmtCtx_, filename.c_str(), nullptr, nullptr);
    if (ret < 0) {
        cout << "failed to open file " << filename << " " << ret << endl;
        return false;
    }

    if (!readStreamInfo_()) {
        return false;
    }

    return true;
}

bool
AVFormatFile::readStreamInfo_() 
{
    if (!fmtCtx_) {
        return false;
    }

    int ret = avformat_find_stream_info(fmtCtx_, nullptr);
    if (ret < 0) {
        cout << "failed to find stream info " << ret << endl;
        return false;
    }
    auto numOfStreams = fmtCtx_->nb_streams;
    data_.resize(numOfStreams);
    codecContexts_.resize(numOfStreams);
    for (unsigned int i = 0; i < numOfStreams; ++i) {
        AVStream* stream = fmtCtx_->streams[i];
        if (stream && stream->codecpar) {
            if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioStreamIndexes_.push_back(i);
            }
            else if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndexes_.push_back(i);
            }
            else {
                cout << "non audio/video stream: " << i << endl;
                otherStreamIndexes_.push_back(i);
            }

            data_[i] = make_shared<AVRawData>(stream);
            if (auto decoder = avcodec_find_decoder(stream->codecpar->codec_id)) {
                codecContexts_[i] = FFmpegAVCodecContextPtr(avcodec_alloc_context3(decoder));
                if (avcodec_open2(codecContexts_[i], decoder, nullptr) < 0) {
                    cout << "error in open code context for stream index " << i << endl;
                }
            }
        }
    }
    return true;
}

void
AVFormatFile::readAudioData()
{
    if (!fmtCtx_) {
        return;
    }
    for (const int i : audioStreamIndexes_) {
        readStream_(i);
    }
}

void
AVFormatFile::readStream_(int index)
{
    auto codecContext = codecContexts_[index];
    if (!codecContext) {
        cout << "failed to read stream due to invalid code context." << endl;
        return;
    }

    AVStream* stream = fmtCtx_->streams[index];
    if (stream) {
        int numOfPackets = 0, numOfFrames = 0;
        
        while (true) {
            FFmpegAVPacketPtr packet(new AVPacket);
            if (av_read_frame(fmtCtx_, packet) < 0) {
                break;  // eof or fail
            }

            if (packet->stream_index != index) {
                continue;
            }
            ++numOfPackets;

            int ret = avcodec_send_packet(codecContext, packet);
            if (ret < 0) {
                cout << "failed to send packet to decoder." << ret << endl;
                break;
            }

            AVFrame* frame = av_frame_alloc();
            while (ret == 0) {
                ret = avcodec_receive_frame(codecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                else if (ret < 0){ 
                    cout << "failed to receive frame from decoder. " << ret << endl;
                    break;
                }
                ++numOfFrames;
                for (int channel = 0; channel < frame->channels; ++channel) {
                    AVBufferRef* buf = frame->buf[channel];
                    data_[i].appendData(buf->data, buf->size);
                }
            }
            av_frame_free(&frame);
        }
        cout << "stream " << index << " read " << numOfPackets << " packets " << numOfFrames << " frames." << endl;
    }
}

}  // namespace Swaper
