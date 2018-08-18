#ifndef FFmpegDemuxer_H
#define FFmpegDemuxer_H

#include <string>

namespace Swaper {

  class FFmpegDemuxer
  {
    public:
      static int stripStream(const std::string& filename);
      static int mux(const std::string& videoFilename, const std::string& audioFilename);

    private:

  };

}

#endif
