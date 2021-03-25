#include <algorithm>
#include <array>
#include <cmath>
#include <execution>
#include <filesystem>

#include <fmt/core.h>
#include <numeric>

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
std::array<ushort *, 8> get_reflect_neighbors(const int index, const int width,
                                              const int height,
                                              ushort image[][4]) {
  // arrays can be replaced with vector for variable kernel size
  const int x = index / width;
  const int y = index % width;
  // TODO: Make it work for multiple kernel sizes
  std::array<std::pair<int, int>, 8> neighbor_indices{{
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

  for (auto &neighbor : neighbor_indices) {
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

  std::array<ushort *, 8> neighbor_pixels;
  // stores value of neighbor pointer to array

  std::transform(neighbor_indices.begin(), neighbor_indices.end(),
                 neighbor_pixels.begin(),
                 [image](int index) -> ushort * { return image[index]; });

  return neighbor_pixels;
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
  const auto [width, height, total_pixels] = image_dimensions();

  for (int i = 0; i < total_pixels; ++i) {
    const auto neighbor_pixels = get_reflect_neighbors(i, width, height, image);
    for (int j = 0; j < 4; ++j) {
      const auto curr_value = image[i][j];

      // check if all pixels are above threshold w.r.t neighbors
      // TODO: Use better bad pixel detection mechanism
      bool is_bad_pixel = std::all_of(
          neighbor_pixels.begin(), neighbor_pixels.end(),
          [curr_value, threshold, j](ushort neighbor_pixel[4]) -> bool {
            return abs(neighbor_pixel[j] - curr_value) > threshold;
          });

      if (is_bad_pixel) {
        auto new_value = curr_value;

        // transform neighbors to take values of pixel at indices they contain
        // of image they point to

        switch (filter) {
        case BadPixelFilter::MEAN:
          // average of all neighboring pixels
          new_value = std::accumulate(
              neighbor_pixels.begin(), neighbor_pixels.end(), 0,
              [j](const auto a, const auto b) { return a[j] + b[j]; });
          new_value /= neighbor_pixels.size();
          break;
        case BadPixelFilter::HALF_MEAN:
          // TODO: [KERNEL] depends on kernel size
          /*
           * Index map [multiple instances search by tag INDEX_MAP]
           *  0 1 2
           *  3   4
           *  5 6 7
           */
          new_value = neighbor_pixels[1][j] + neighbor_pixels[3][j] +
                      neighbor_pixels[4][j] + neighbor_pixels[6][j];
          new_value /= 4;
          break;
        case BadPixelFilter::GRADIENT: {
          // TODO: [KERNEL] depends on kernel size
          int twice_curr = 2 * curr_value;

          // d vertical, d horizontal, d digonal right, d digonal left
          int dv = twice_curr - neighbor_pixels[1][j] - neighbor_pixels[6][j];
          int dh = twice_curr - neighbor_pixels[3][j] - neighbor_pixels[4][j];
          int ddr = twice_curr - neighbor_pixels[0][j] - neighbor_pixels[7][j];
          int ddl = twice_curr - neighbor_pixels[2][j] - neighbor_pixels[5][j];

          int min_gradient = std::min({dv, dh, ddr, ddl});

          if (min_gradient == dv) {
            // mean along vertical gradient
            new_value = (neighbor_pixels[1][j] + neighbor_pixels[6][j] + 1) / 2;
          } else if (min_gradient == dh) {
            // horizontal
            new_value = (neighbor_pixels[3][j] + neighbor_pixels[4][j] + 1) / 2;
          } else if (min_gradient == dh) {
            // digonal right
            new_value = (neighbor_pixels[0][j] + neighbor_pixels[7][j] + 1) / 2;
          } else {
            // digonal left
            new_value = (neighbor_pixels[2][j] + neighbor_pixels[5][j] + 1) / 2;
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
  const auto [width, height, total_pixels] = image_dimensions();

  for (int i = 0; i < total_pixels; ++i) {
    for (int j = 0; j < 4; ++j) {
      // TODO: add logic here
    }
  }
}
std::tuple<ushort, ushort, int> Processor::image_dimensions() {

  const auto width = _imageProcessor.imgdata.sizes.width;
  const auto height = _imageProcessor.imgdata.sizes.height;
  const auto total_pixels = width * height;

  return {width, height, total_pixels};
}

void Processor::export_image(const char *filename) {}
} // namespace ISPP
