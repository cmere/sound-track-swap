#include <iostream>
#include <libgen.h>
#include "FFmpegDemuxer.h"
#include "FFmpegMuxer.h"

using namespace std;
using namespace Swaper;

int main(int argc, char* argv[])
{
  if (argc != 4) {
    cout << "usage:\n" 
         << "  " << basename(argv[0]) << " <audio file> <video file> <output file>" << endl;
    return 1;
  }

  string filename1 = argv[1];
  string filename2 = argv[2];
  string filename3 = argv[3];

  FFmpegMuxer::mux(filename1, filename2, filename3);

  return 0;
}
