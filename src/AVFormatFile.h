#ifndef SWAPER_AVFORMATFILE_H
#define SWAPER_AVFORMATFILE_H

#include "FFmpegClasses.h"
#include "AVRawData.h"
#include <list>
#include <map>
#include <string>

namespace Swaper {

class AVFormatFile {
public:
    ~AVFormatFile();
    bool openFile(const std::string& filename);

private:
    bool readStreamInfo_();

private:
    AVFormatContext* fmtCtx_ = nullptr;
    std::list<unsigned int> audioStreamIndexes_;
    std::list<unsigned int> videoStreamIndexes_;
    std::list<unsigned int> otherStreamIndexes_;
    std::map<unsigned int, std::shared_ptr<AVRawData>> dataByIndex_;
};

}
#endif
