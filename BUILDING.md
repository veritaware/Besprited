# Building the source

## Table of contents

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
* [Generating icons](#generating-icons)
* [Installing](#installing)
* [Building distributable packages](#building-distributable-packages)
* [Static Code Analysis](#static-code-analysis)

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

You'll need the following dependencies to compile Besprited.

Besprited uses SDL3 by default. The older SDL2 backend is still available as a
deprecated fallback until the 1.27.03 release (March 2027); to use it, install the
SDL2 development packages (e.g. `libsdl2-dev libsdl2-image-dev` on Debian/Ubuntu)
instead of the SDL3 ones and configure with `-DUSE_SDL2_BACKEND=on`.

### Linux dependencies

Debian/Ubuntu/Mint:

    sudo apt-get install g++ cmake git libfreetype6-dev libgif-dev libgtest-dev libjpeg-dev libpixman-1-dev libpng-dev libsdl3-dev libsdl3-image-dev libtinyxml2-dev libnode-dev ninja-build zlib1g-dev libarchive-dev libssl-dev

Fedora:

    sudo dnf install g++ cmake git freetype-devel giflib-devel gtest-devel libjpeg-devel pixman-devel libpng-devel SDL3-devel SDL3_image-devel tinyxml2-devel zlib-devel ninja-build nodejs-devel libarchive-devel openssl-devel libXi-devel

Arch Linux:

    sudo pacman -Syu base-devel cmake ninja git sdl3 sdl3_image tinyxml2 freetype2 giflib gtest libjpeg-turbo pixman libpng zlib nodejs libarchive openssl
    

### Windows dependencies

To install the required dependencies with msys2, run the following in the MSYS2 UCRT64 shell:

    pacman -S base-devel mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-nodejs mingw-w64-ucrt-x86_64-openssl mingw-w64-ucrt-x86_64-freetype mingw-w64-ucrt-x86_64-giflib mingw-w64-ucrt-x86_64-libjpeg-turbo mingw-w64-ucrt-x86_64-libpng mingw-w64-ucrt-x86_64-libwebp mingw-w64-ucrt-x86_64-pixman mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-image mingw-w64-ucrt-x86_64-tinyxml2 mingw-w64-ucrt-x86_64-zlib mingw-w64-ucrt-x86_64-libarchive

### MacOS dependencies

On MacOS you will need Mac OS X 11.0 SDK and the corresponding Xcode.
In a terminal, install the dependencies using brew:

    brew install gnutls freetype jpeg webp pixman sdl3 sdl3_image tinyxml2 libarchive ninja zlib xmlto dylibbundler cmake

## Compiling

First, create the `build` directory with the following commands:

    cd Besprited
    mkdir build
    cd build

Then following the platform-specific instructions for compiling below.

The `build` directory will contain the results of the compilation process.
If you want to build a fresh copy of Besprited, remove the `build` directory
and recompile.

The executable is `bin/Besprited` (`Besprited.exe`) on Windows and macOS, and
`bin/besprited` elsewhere. The build target is `besprited` everywhere.

### Linux details

To compile Besprited, run the following commands:

    cmake -G Ninja ..
    ninja besprited

### Windows details

Run the following in the MSYS2 UCRT64 shell:

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

## Generating icons

The application icons are not stored in every size/format. They are derived from the
16x16 pixel-art sources in `packaging/desktop/icons/` (nearest-neighbour scaling), and
the generated files are git-ignored. This needs [ImageMagick](https://imagemagick.org)
(`magick`), plus [icoutils](https://www.nongnu.org/icoutils/) (`icotool`) for Windows.
The scripts can be run from any directory:

* **Windows** — required before building (the resource file and installer use the
  result). Run in the MSYS2 UCRT64 shell (`pacman -S mingw-w64-ucrt-x86_64-imagemagick mingw-w64-ucrt-x86_64-icoutils`):

      sh packaging/desktop/icons/generate_ico.sh   # writes data/icons/besprited.ico and doc.ico

* **Linux** — required before configuring with `-DWITH_DESKTOP_INTEGRATION=ON` or running
  `packaging/linux/package_linux.sh`:

      sh packaging/desktop/icons/generate_hicolor.sh   # writes packaging/desktop/icons/hicolor/<size>/apps/besprited.png

* **macOS** — `packaging/macos/package_macos.sh` generates the `.icns` itself (needs `imagemagick`).

## Installing

Once you've finished compiling, you can install Besprited by running the
following command from the `build` directory:

    ninja install

The `install()` rules honour `CMAKE_INSTALL_PREFIX` and, when configured with
`-DWITH_DESKTOP_INTEGRATION=ON`, also lay down the `.desktop` entry, icons,
MIME definitions and metainfo on Linux.

## Building distributable packages

The release artifacts on the [releases page](https://github.com/Veritaware/Besprited/releases)
are produced by the GitHub workflows under `.github/workflows/`, but every
packaging step can be reproduced locally from a configured `build/` tree.

### Linux `.tar.gz`

`packaging/linux/package_linux_tarball.sh` stages the install tree and bundles the
`install.sh` / `uninstall.sh` helpers at the archive root:

    cmake -G Ninja -DWITH_DESKTOP_INTEGRATION=ON ..
    ninja besprited
    ../packaging/linux/package_linux_tarball.sh  # writes besprited-<version>-linux-<arch>.tar.gz

### Linux `.deb` / `.rpm`

Configure with `-DENABLE_PACKAGING=ON` to wire up CPack, then invoke the
generator you need (build the `.rpm` on an actual RPM-based distro so the
auto-detected dependencies are correct):

    cmake -G Ninja -DWITH_DESKTOP_INTEGRATION=ON -DENABLE_PACKAGING=ON \
      -DBESPRITED_PACKAGE_VERSION=<version> ..
    ninja besprited
    cpack -G DEB --config CPackConfig.cmake
    cpack -G RPM --config CPackConfig.cmake

### macOS `.dmg`

`packaging/macos/package_macos.sh` builds the `.app` bundle, bundles the dylibs,
self-signs it and wraps it in a styled drag-to-install disk image (needs
`dylibbundler` and `dmgbuild`).

### Windows

`packaging/windows/package_win.js` copies the runtime DLLs next to `Besprited.exe`;
the portable archive is just those files zipped up. The installer is built
from `packaging/windows/WindowsInstaller.iss` with [Inno Setup](https://jrsoftware.org/isinfo.php):

    ISCC.exe /DMyAppVersion=<version> packaging/windows/WindowsInstaller.iss

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

    pacman -S mingw-w64-ucrt-x86_64-cppcheck mingw-w64-ucrt-x86_64-clang-tools-extra mingw-w64-ucrt-x86_64-gcc
