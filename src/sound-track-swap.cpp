#include <iostream>
#include <libgen.h>

using namespace std;

int main(int argc, char* argv[])
{
  cout << "usage:\n" 
       << "  " << basename(argv[0]) << " <video file> <audio file>" << endl;
  return 0;
}
