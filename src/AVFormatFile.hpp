#ifndef SWAPER_AVFORMATFILE_H
#define SWAPER_AVFORMATFILE_H

#include "FFmpegClasses.hpp"
#include "AVRawData.hpp"
#include <list>
#include <string>
#include <vector>

namespace Swaper {

class AVFormatFile {
public:
    ~AVFormatFile();

    bool openFile(const std::string& filename);
    void readAudioData();

private:
    bool readStreamInfo_();
    void readStream_(int index);

private:
    AVFormatContext* fmtCtx_ = nullptr;
    std::list<int> audioStreamIndexes_;
    std::list<int> videoStreamIndexes_;
    std::list<int> otherStreamIndexes_;
    std::vector<std::shared_ptr<AVRawData>> data_;
    std::vector<FFmpegAVCodecContextPtr> codecContexts_;
};

}
#endif
