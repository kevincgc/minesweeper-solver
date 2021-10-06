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

## Game Codes and Edit Mode

This includes "game codes", which is essentially a generated game grid represented in text form. This allows users to replay the same level or to share their level with others.

A game code is automatically generated whenever a new game is started (i.e. when the first cell is revealed and the timer is started). You can also press the "generate code" button to generate a code, which is mainly used when editting the game (see below).

Toggle the "Use Code" button if you wish to use the game code that's currently in the text box. If it is active and the game code is valid, the game will always reset to that game configuration.

**Edit Mode:** An edit mode is available if the user wishes to edit the game directly (i.e. set/unset tiles to be mines). Toggle the edit mode button to enter edit mode.
Once in edit mode, left click or right click to set/unset mines. Use "generate code" to generate a code for the editted game state.

## Notes

Due to limitations of GTKMM (GTK4), the game cannot fluidly process transition from one window to another window within this application. i.e. while one window is capturing mouse inputs, other windows within this application won't receive any mouse inputs. To move from window to window, use alt+tab or click on the title bar of the active window and then click on the title bar of the desired window.
