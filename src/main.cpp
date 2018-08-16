#include <iostream>
#include <libgen.h>
#include "FFmpegDemuxer.h"
#include "FFmpegMuxer.h"
#include "AudioAligner.h"

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

  AudioAligner::align(filename1, filename2);

  /*
    // Initialise the factory
    if ( mlt_factory_init( NULL ) == 0 )
    {
        cout << "failed to init mlt factory." << endl;
        return EXIT_FAILURE;
    }

    Mlt::Profile profile;

    Mlt::Producer prod_1(profile, argv[1]);
    Mlt::Producer prod_2(profile, argv[2]);


  AudioEnvelope audioEnv_1(QString(filename1.c_str()), &prod_1);
  audioEnv_1.loadEnvelope();
  AudioEnvelope audioEnv_2(QString(filename2.c_str()), &prod_2);
  audioEnv_2.loadEnvelope();

  AudioCorrelation audioCorr(&audioEnv_1);
  audioCorr.addChild(&audioEnv_2);

  FFmpegMuxer::mux(filename1, filename2, filename3);
  */

  return 0;
}
