#include "AVFormatFile.h"

#include <iostream>

using namespace std;

namespace {

using namespace Swaper;

bool
readAudioStreamIndexes(AVFormatContext* fmtCtx, list<unsigned int>& indexes) 
{
    if (!fmtCtx) {
        return false;
    }

    int ret = avformat_find_stream_info(fmtCtx, nullptr);
    if (ret < 0) {
        cout << "failed to find stream info " << ret << endl;
        return false;
    }
    for (unsigned int i = 0; i < fmtCtx->nb_streams; ++i) {
        AVStream* stream = fmtCtx->streams[i];
        if (stream && stream->codecpar && stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            cout << "audio stream index " << i << endl;
            indexes.push_back(i);
        }
    }
    return true;
}

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

    if (!readAudioStreamIndexes(fmtCtx_, audioStreamIndexes_)) {
        return false;
    }

    return true;
}

}  // namespace Swaper
