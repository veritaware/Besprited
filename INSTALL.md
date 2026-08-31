# Installation

## Table of contents

* [Platforms](#platforms)
* [Get the source code](#get-the-source-code)
* [Dependencies](#dependencies)
  * [Linux dependencies](#linux-dependencies)
  * [Windows dependencies](#windows-dependencies)
  * [MacOS dependencies](#macos-dependencies)
* [Compiling](#compiling)
  * [Linux details](#linux-details)
  * [Windows details](#windows-details)
  * [MacOS details](#macos-details)
  * [Android details](#android-details)
* [Installing](#installing)
* [Static Code Analysis](#static-code-analysis)

## Platforms

You can download installers from the [releases page](https://github.com/Veritaware/Besprited/releases).

On Linux you have two prebuilt options:

* **AppImage** – a single self-contained executable. You need `libfuse2` installed to run it.
* **`.tar.gz` package** – a portable FHS tree (`besprited-<version>-linux-<arch>.tar.gz`) with a
  bundled installer. Extract it and run the install script from the extracted directory:

      tar -xf besprited-*-linux-*.tar.gz
      cd besprited-*-linux-*/          # or wherever you extracted it

      sudo ./install.sh                # system-wide, into /usr/local
      ./install.sh --user              # per-user, into ~/.local (no sudo)

  Use `./uninstall.sh` (with the same `--user` / `--prefix` flag you installed with) to remove it.
  The installer does not pull in system libraries – if it reports missing `.so` files after
  installing, install the matching packages from the [Linux dependencies](#linux-dependencies)
  section below.

If you want to compile Besprited from source, continue reading.

You *should* be able to build the project on any relatively modern Linux, macOS or Windows machine.

To compile Besprited you will need:

* [CMake](http://www.cmake.org/) (4.1 or greater – you probably can get away with an older version if you edit it down in the CMakeLists, but we upped the version in the repo for the GitHub workflows reasons)
* [Ninja](https://ninja-build.org) (actually optional as you can choose Unix makefiles / Xcode as CMake output, but Ninja speeds up the building process a bit)
* [Msys2](https://www.msys2.org/) (Windows only)

## Get the source code

Clone the repository and its submodules using the following command:

    git clone --recursive https://github.com/Veritaware/Besprited

(You can use [Git for Windows](https://git-for-windows.github.io/) to
clone the repository on Windows.)

To update an existing clone, use the following commands:

    cd Besprited
    git pull
    git submodule update --init --recursive

## Dependencies

You'll need the following dependencies to compile Besprited:

### Linux dependencies

Debian/Ubuntu/Mint:

    sudo apt-get install g++ cmake git libcurl4-gnutls-dev libfreetype6-dev libgif-dev libgtest-dev libjpeg-dev libpixman-1-dev libpng-dev libsdl2-dev libsdl2-image-dev libtinyxml2-dev libnode-dev ninja-build zlib1g-dev libarchive-dev

Fedora:

    sudo dnf install g++ cmake git libcurl-devel freetype-devel giflib-devel gtest-devel libjpeg-devel pixman-devel libpng-devel SDL2-devel SDL2_image-devel tinyxml2-devel zlib-devel ninja-build nodejs-devel libarchive-devel libXi-devel

Arch Linux:

    sudo pacman -Syu base-devel cmake ninja git sdl3 sdl2-compat sdl2_image sdl2_ttf tinyxml2 freetype2 curl giflib gtest libjpeg-turbo pixman libpng zlib nodejs libarchive
    

### Windows dependencies

To install the required dependencies with msys2, run the following in mingw32:

    pacman -S base-devel mingw-w64-i686-gcc mingw-w64-i686-cmake mingw-w64-i686-make mingw-w64-i686-curl mingw-w64-i686-freetype mingw-w64-i686-giflib mingw-w64-i686-libjpeg-turbo mingw-w64-i686-libpng mingw-w64-i686-libwebp mingw-w64-i686-pixman mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_image mingw-w64-i686-tinyxml2 mingw-w64-i686-zlib mingw-w64-i686-libarchive

### MacOS dependencies

On MacOS you will need Mac OS X 11.0 SDK and the corresponding Xcode.
In a terminal, install the dependencies using brew:

    brew install gnutls freetype jpeg webp pixman sdl2 sdl2_image tinyxml2 libarchive ninja zlib xmlto dylibbundler cmake

## Compiling

First, create the `build` directory with the following commands:

    cd Besprited
    mkdir build
    cd build

Then following the platform-specific instructions for compiling below.

The `build` directory will contain the results of the compilation process.
If you want to build a fresh copy of Besprited, remove the `build` directory
and recompile.

### Linux details

To compile Besprited, run the following commands:

    cmake -G Ninja ..
    ninja besprited

### Windows details

Run the following in mingw32.exe:

    cmake -G Ninja ..

### MacOS details

To compile Besprited, run the following commands:
```
    cmake \
      -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk \
      -G Ninja \
      ..
    ninja besprited
```
### Android details

Before you can make an Android build, you must make a native build for your OS,
so follow the appropriate instructions above. Once that is done, download
https://github.com/LibreSprite/ls-android-deps as android/ in the Besprited
directory. Now you can open the android subdirectory in Android Studio and build
Besprited for Android.


## Installing

Once you've finished compiling, you can install Besprited by running the
following command from the `build` directory:

    ninja install

## Static Code Analysis

Besprited includes build targets for running static code analysis tools. These targets help maintain code quality by detecting potential bugs, security vulnerabilities, and code style issues.

### Available Analysis Targets

From the `build` directory, you can run the following analysis targets:

**cppcheck** - Runs cppcheck static analysis:

    cmake --build . --target cppcheck

Requires: [cppcheck](http://cppcheck.sourceforge.net/) to be installed on your system.

**clang-tidy** - Runs clang-tidy static analysis:

    cmake --build . --target clang-tidy

Requires: clang-tidy to be installed on your system.

**gcc-analyzer** - Builds with GCC's `-fanalyzer` flag:

    cmake --build . --target gcc-analyzer

Requires: GCC 10 or higher (which includes the `-fanalyzer` feature).

### Installing Analysis Tools

**Linux (Debian/Ubuntu):**

    sudo apt-get install cppcheck clang-tidy gcc-10 g++-10

**Linux (Fedora):**

    sudo dnf install cppcheck clang-tools-extra gcc

**MacOS:**

    brew install cppcheck llvm gcc

**Windows (MSYS2):**

    pacman -S mingw-w64-i686-cppcheck mingw-w64-i686-clang-tools-extra mingw-w64-i686-gcc
