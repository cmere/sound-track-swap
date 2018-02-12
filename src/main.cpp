#include <iostream>
#include <libgen.h>
#include "FFmpegDemuxer.h"

using namespace std;
using namespace Swaper;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    cout << "usage:\n" 
         << "  " << basename(argv[0]) << " <video file>" << endl;
    return 1;
  }

  FFmpegDemuxer::stripStream(argv[1]);

  return 0;
}
