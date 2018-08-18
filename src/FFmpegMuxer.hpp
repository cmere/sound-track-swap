#ifndef FFmpegMuxer_H
#define FFmpegMuxer_H

#include <string>

namespace Swaper {

    class FFmpegMuxer
    {
    public:
        static int mux(const std::string& inputFilename1, const std::string& inputFilename2, const std::string& outFilename);

    private:

    };

}

#endif
