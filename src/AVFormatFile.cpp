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
    for (unsigned int i = 0; i < fmtCtx_->nb_streams; ++i) {
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
            dataByIndex_[i].reset(new AVRawData(stream));
        }
    }
    return true;
}

}  // namespace Swaper
