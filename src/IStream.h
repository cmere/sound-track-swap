#ifndef ISTREAM_H__
#define ISTREAM_H__

namespace Swaper {

/**
 *
 */
class IStream {
public:
  int fd() const = 0;
  int read(char* buffer) = 0;
  int write(const char* const data, int dataSize) = 0;
};

}

#endif

