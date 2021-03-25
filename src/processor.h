#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <libraw/libraw.h>

namespace ISPP {
enum class BadPixelFilter { MEAN, GRADIENT };
class Processor {
public:
  // constructors, asssignment, destructor
  Processor(const char *filename);
  void correct_bad_pixels(const int threshold,
                          const BadPixelFilter filter = BadPixelFilter::MEAN);
  void correct_black_level(std::array<std::pair<ushort, ushort>, 4> clip_range);
  void export_image(const char *filename);

private:
  LibRaw _imageProcessor;
};

} // namespace ISPP
#endif // PROCESSOR_HPP
