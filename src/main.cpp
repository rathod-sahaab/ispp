#include <filesystem>
#include <string>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"
#include "fmt/core.h"

#include "processor.h"

void process_file(char *filename);

int main(int argc, char **argv) {
  // --------------- vars -----------------------------
  std::string filename;
  std::string outputfile;

  // -------------- operations -----------------------
  bool correct_bad_pixels = false;

  bool correct_black_level = false;
  // arbitary values b g g r, g b r g, etc.
  constexpr const int BLACK_LEVEL[4] = {300, 300, 300, 300};
  constexpr const int WHITE_LEVEL[4] = {15520, 15520, 15520, 15520};

  // ---------------- CLI parsing ---------------------
  CLI::App app{"An app to apply image signal processing algorithm on \
  raw image files."};

  app.add_option("-f,--file", filename, "raw image file to work on")
      ->required();
  app.add_option("-o,--output", outputfile, "output file name/uri");
  app.add_flag("--bad-pixel", correct_bad_pixels, "Correct bad pixels");

  CLI11_PARSE(app, argc, argv);

  // --------------- processing/interpolation of cli --------------------
  if (outputfile.empty()) {
    outputfile = fmt::format("{}.out.raw", filename);
  }
  // ---------------- CLI parsing done ---------------------

  fmt::print("Selected file is: '{}'\n", filename);
  if (std::filesystem::exists(filename)) {
    ISPP::Processor processor(filename.c_str());

    if (correct_bad_pixels) {
      // arbitary value 10000
      processor.correct_bad_pixels(10'000, ISPP::BadPixelFilter::GRADIENT);
    }

  } else {
    fmt::print(stderr, "Selected file doesn't exists, exiting...\n");
  }

  return 0;
}
