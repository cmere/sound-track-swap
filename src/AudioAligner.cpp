#include "AudioAligner.h"
#include "AVFormatFile.h"

using namespace std;

namespace Swaper {

double 
AudioAligner::align(const std::string& fromFilename, const std::string& toFilename)
{
    AVFormatFile fromFile;
    if (!fromFile.openFile(fromFilename)) {
        return 0.0;
    }

    return 0.0;
}

}  // namespace Swaper
