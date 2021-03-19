cmake_minimum_required(VERSION 3.10)
project(ispp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
conan_basic_setup()  # Prepares the CMakeList.txt for Conan.

# $source_files is a space-delimited list of filenames.
add_executable(ispp src/main.cpp)  # Specifies the executable to build.
target_link_libraries(ispp ${CONAN_LIBS})  # Specifies what libraries to link, using Conan.
