#ifndef SWAPER_AVFORMATFILE_H
#define SWAPER_AVFORMATFILE_H

#include "FFmpegClasses.h"
#include <list>
#include <string>

namespace Swaper {

class AVFormatFile {
public:
    ~AVFormatFile();
    bool openFile(const std::string& filename);

private:
    AVFormatContext* fmtCtx_ = nullptr;
    std::list<unsigned int> audioStreamIndexes_;
};

}
#endif
