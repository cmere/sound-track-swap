#ifndef SWAPER_AUDIOALIGNER_H
#define SWAPER_AUDIOALIGNER_H

#include <string>

namespace Swaper {

class AudioAligner
{
public:
    static double align(const std::string& baseFile, const std::string& newFile);
};

}

#endif
