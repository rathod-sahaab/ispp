#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <filesystem>

#include "fmt/core.h"
#include "libraw/libraw.h"

namespace ISPP {
class Processor {
public:
  // constructors, asssignment, destructor
  Processor(const char *filename);
  void correct_bad_pixels();
  void correct_black_level(std::array<std::pair<ushort, ushort>, 4> clip_range);
  void export_image(const char *filename);

private:
  LibRaw _imageProcessor;
};

} // namespace ISPP
#endif // PROCESSOR_HPP
