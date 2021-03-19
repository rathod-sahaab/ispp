#include <string>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"
#include "fmt/core.h"

int main(int argc, char **argv) {
  // --------------- vars -----------------------------
  std::string filename = "default";
  std::string outputfile;

  // ---------------- CLI parsing ---------------------
  CLI::App app{"An app to apply image signal processing algorithm on \
  raw image files."};

  app.add_option("-f,--file", filename, "raw image file to work on")
      ->required();
  app.add_option("-o,--output", outputfile, "output file name/uri");

  CLI11_PARSE(app, argc, argv);
  // ---------------- CLI parsing done ---------------------

  // ---------------- Functinal code -----------------------
  fmt::print("Selected file is: '{}'\n", filename);
  return 0;
}
