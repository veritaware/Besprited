#!/bin/sh
# Usage: sh packaging/linux/package_linux.sh [VERSION]
#
# VERSION is the release tag (e.g. "v1.26.09"). When given, the AppImage is
# built with embedded update information (gh-releases-zsync, resolved
# against the repo's GitHub releases) and a matching .zsync file is written
# alongside it, so it works with AppImageUpdate. Omit it for local/dev
# builds that won't be published as a GitHub release.
set -e

rootdir=$(pwd)
builddir="$rootdir"/build/bin/
APP="Besprited"
ARCH="$(uname -m)"
VERSION="$1"

cd "$builddir"
chmod +x besprited
mkdir -p Besprited/usr/bin Besprited/usr/lib
mv "$rootdir"/packaging/desktop/besprited.desktop Besprited/
cp "$rootdir"/packaging/desktop/icons/hicolor/256x256/apps/besprited.png Besprited/besprited.png
if ls ./*.so* >/dev/null 2>&1; then mv ./*.so* Besprited/usr/lib; fi

# Create AppImage with lib4bin and Sharun
(
cd Besprited
wget "https://raw.githubusercontent.com/VHSgunzo/sharun/refs/heads/main/lib4bin" -O ./lib4bin
chmod +x ./lib4bin

# Download sharun ourselves and pin SHARUN to its path. Without this,
# lib4bin's own auto-resolution (SHARUN="$(readlink -f "$(which_exe sharun)")")
# breaks when sharun isn't on PATH: which_exe returns empty, and
# `readlink -f ''` resolves to the current directory (a GNU coreutils quirk)
# instead of failing. Since we've already cd'd into Besprited, that CWD is
# the AppDir itself, which is a directory with the execute bit set, so
# lib4bin's `[ -x "$SHARUN" ]` check wrongly passes and it skips downloading
# sharun, then tries to cp the directory as if it were the binary and fails.
wget "https://github.com/VHSgunzo/sharun/releases/latest/download/sharun-$ARCH" -O ./sharun-tmp
chmod +x ./sharun-tmp
export SHARUN
SHARUN="$(pwd)/sharun-tmp"

# Use lib4bin's default static (ldd/patchelf) dependency scan instead of
# strace mode (-e): strace mode actually executes besprited under Xvfb to
# observe its library loads, which is fragile/crash-prone in headless CI
# and can silently leave sharun/AppRun uncreated.
./lib4bin -p -v -k -w \
  ../besprited \
  /usr/lib/libpthread.so* \
  /usr/lib/librt.so* \
  /usr/lib/libstdc++.so*
rm -f ./sharun-tmp
ln ./sharun ./AppRun
./sharun -g
)

# Fail loudly instead of letting appimagetool package an incomplete AppDir
for f in Besprited/sharun Besprited/AppRun; do
  if [ ! -e "$f" ]; then
    echo "package_linux.sh: expected file '$f' was not created by lib4bin" >&2
    exit 1
  fi
done

# Maybe the data folder is being read during initial run
# This lets the run complete with expected original locations and then
# copies it over afterwards using the below command
mv ./data ./Besprited/bin

wget "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-$ARCH.AppImage" -O appimagetool
chmod +x ./appimagetool

if [ -n "$VERSION" ]; then
  outname="$APP-$VERSION-anylinux-$ARCH.AppImage"
else
  outname="$APP-anylinux-$ARCH.AppImage"
fi

set -- --comp zstd --mksquashfs-opt -Xcompression-level --mksquashfs-opt 22
if [ -n "$VERSION" ]; then
  set -- "$@" -u "gh-releases-zsync|Veritaware|Besprited|latest|$APP-*-anylinux-$ARCH.AppImage.zsync"
fi
./appimagetool "$@" -n "$builddir"/Besprited "$builddir"/"$outname"
