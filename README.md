# Minesweeper

This is an implementation of classic Minesweeper (i.e. version found on Windows XP) in C++ using GTKMM for the GUI. This is a CMake project directed for the Windows environment. Within the repo are:
- `Minesweeper_Release_Build.7z` 
- `Minesweeper_Release_Build.zip`

Both of  these are precompiled builds targetting Windows x86.

`src` contains the source code and `gresources` contain resource files used in the application. CMakeLists.txt is configured to automatically process `gresources` into a .c binary file that gets linked directly into the application.

If compiling on MS Visual Studio, I recommend using vcpkg to install GTKMM and required dependencies. However, the current version of vcpkg (at the time of writing) has a known bug where the GTKMM libraries cannot be included as-is - it requires additional configuration with pkgconf to setup the environment. I've written instructions for setting this up which can be found here: [VStudio GTKMM Install Guide](https://gist.github.com/jerrywang94/ffd370d2e42918817bbfb765def7d771).

## How to play

- **Left click** on an empty square to reveal it
- **Right click** to flag/unflag an unrevealed square
- **Left+Right click** a number to reveal all adjacent squares (if there is the correct number of adjacent squares flagged)
- **Space bar** while hovering over an unrevealed square to flag/unflag it, or over a number to reveal all adjacent squares

Game settings menu on the top left to configure the game.
