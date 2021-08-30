# CMakeList.txt : CMake project for Minesweeper, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Minesweeper "Minesweeper.cpp" "Minesweeper.h")

set(PKG_CONFIG_EXECUTABLE "C:/src/vcpkg/installed/x86-windows/tools/pkgconf/pkgconf.exe")
find_package(PkgConfig)
pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

target_link_libraries(Minesweeper PRIVATE PkgConfig::GTKMM_VARS)

# TODO: Add tests and install targets if needed.
