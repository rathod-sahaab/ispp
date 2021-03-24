#include "processor.h"
#include <array>
namespace ISPP {

Processor::Processor(const char *filename) {
  if (std::filesystem::exists(filename)) {
    _imageProcessor.open_file(filename);

    fmt::print("Image read! dimensions {}x{}\n",
               _imageProcessor.imgdata.sizes.width,
               _imageProcessor.imgdata.sizes.height);

    _imageProcessor.unpack();
    _imageProcessor.raw2image();
  } else {
    fmt::print(stderr, "{}:{}:{}, FILE DOESN'T EXIST\n", __FILE_NAME__,
               __FUNCTION__, __LINE__);
  }
}

void Processor::correct_bad_pixels() {
  auto &image =
      _imageProcessor.imgdata.image; // ushort *[4] or [][4] : 4 channels
  // TODO: confirm if width may change
  auto &width = _imageProcessor.imgdata.sizes.width;
  auto &height = _imageProcessor.imgdata.sizes.height;

  const auto total_pixels = width * height;
}

void Processor::correct_black_level(
    std::array<std::pair<ushort, ushort>, 4> clip_range) {
  auto &image =
      _imageProcessor.imgdata.image; // ushort *[4] or [][4] : 4 channels
  // TODO: confirm if width/height may change
  auto &width = _imageProcessor.imgdata.sizes.width;
  auto &height = _imageProcessor.imgdata.sizes.height;

  const auto total_pixels = width * height;

  for (int i = 0; i < total_pixels; ++i) {
    for (int j = 0; j < 4; ++j) {
      // TODO: add logic here
    }
  }
}
} // namespace ISPP
