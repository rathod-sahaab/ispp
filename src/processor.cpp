#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>

#include <fmt/core.h>

#include "processor.h"

namespace {
// anonymous namespace for private functions

/**
 * TODO: move to utility
 * Reflect for edges and corner
 * 1 2 3
 * 4 5 6
 * 7 8 9
 *    to
 * 5 4 5 6 5
 * 2 1 2 3 2
 * 5 4 5 6 5
 * 8 7 8 9 8
 * 5 4 5 6 5
 *
 * however this doesn't pad just returns the padded neighbors
 * ex: the neighbors of '1' are indices below:
 * (-1, -1), (0, -1), (1, -1)
 * (-1,  0),          (1,  0)
 * (-1,  1), (0,  1), (1,  1)
 *
 * This will returns neighbors
 * (1, 1), (0, 1), (1, -1)
 * (1, 0),         (1,  0)
 * (1, 1), (0, 1), (1,  1)
 *
 * The neighbors will be in linear system as that is required
 */
std::array<int, 8> get_reflect_neighbours(const int index, const int width,
                                          const int height) {
  const int x = index / width;
  const int y = index % width;
  // TODO: Make it work for multiple kernel sizes
  std::array<std::pair<int, int>, 8> neighbors{{
      // clang-format off
      {x - 1, y - 1}, {x, y - 1}, {x + 1, y - 1},
      { x - 1, y},		      {x + 1, y},
      {x - 1, y + 1}, {x, y + 1}, {x + 1, y + 1},
      // clang-format on
      /*
       * Index map [multiple instances search by tag INDEX_MAP]
       *  0 1 2
       *  3   4
       *  5 6 7
       */
  }};

  for (auto &neighbor : neighbors) {
    if (neighbor.first < 0) {
      neighbor.first = abs(neighbor.first);
    } else if (neighbor.first >= width) {
      neighbor.first = width - 1 - (neighbor.first - width);
    }

    if (neighbor.second < 0) {
      neighbor.second = 1;
    } else if (neighbor.second >= height) {
      neighbor.second = height - 2; // 0 based indexing
    }
  }

  std::array<int, 8> result;

  // transform from 2D to 1D
  std::transform(neighbors.begin(), neighbors.end(), result.begin(),
                 [width, height](std::pair<int, int> neighbor) -> int {
                   return neighbor.first * width + neighbor.second;
                 });

  return result;
}
} // namespace

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

void Processor::correct_bad_pixels(const int threshold,
                                   const BadPixelFilter filter) {
  auto &image =
      _imageProcessor.imgdata.image; // ushort *[4] or [][4] : 4 channels
  // TODO: confirm if width may change
  auto &width = _imageProcessor.imgdata.sizes.width;
  auto &height = _imageProcessor.imgdata.sizes.height;

  const auto total_pixels = width * height;

  for (int i = 0; i < total_pixels; ++i) {
    const auto neighbors = get_reflect_neighbours(i, width, height);
    for (int j = 0; j < 4; ++j) {

      // check if all pixel are above threshold
      bool is_bad_pixel = std::all_of(
          neighbors.begin(), neighbors.end(),
          [&image, j, threshold, i](int neighbor) -> bool {
            return abs(image[neighbor][j] - image[i][j]) > threshold;
          });

      if (is_bad_pixel) {
        auto curr_value = image[i][j];
        auto new_value = curr_value;

        switch (filter) {
        case BadPixelFilter::MEAN:
          // TODO: [KERNEL] depends on kernel size
          /*
           * Index map [multiple instances search by tag INDEX_MAP]
           *  0 1 2
           *  3   4
           *  5 6 7
           */
          new_value = neighbors[1] + neighbors[3] + neighbors[4] + neighbors[6];
          new_value /= 4;
          break;
        case BadPixelFilter::GRADIENT: {
          // TODO: [KERNEL] depends on kernel size
          int twice_curr = 2 * curr_value;

          // d vertical, d horizontal, d digonal right, d digonal left
          int dv = twice_curr - neighbors[1] - neighbors[6];
          int dh = twice_curr - neighbors[3] - neighbors[4];
          int ddr = twice_curr - neighbors[0] - neighbors[7];
          int ddl = twice_curr - neighbors[2] - neighbors[5];

          int min_gradient = std::min({dv, dh, ddr, ddl});

          if (min_gradient == dv) {
            // mean along vertical gradient
            new_value = (neighbors[1] + neighbors[6] + 1) / 2;
          } else if (min_gradient == dh) {
            // horizontal
            new_value = (neighbors[3] + neighbors[4] + 1) / 2;
          } else if (min_gradient == dh) {
            // digonal right
            new_value = (neighbors[0] + neighbors[7] + 1) / 2;
          } else {
            // digonal left
            new_value = (neighbors[2] + neighbors[5] + 1) / 2;
          }

        } break;
        default:
          break;
        }
        image[i][j] = new_value;
      }
    }
  }
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

void Processor::export_image(const char *filename) {}
} // namespace ISPP
