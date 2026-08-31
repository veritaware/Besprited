#!/bin/sh
# Besprited | Copyright (C) 2026 Veritaware
# Licensed under the MIT License (https://opensource.org/licenses/MIT).
#
# Builds a portable .tar.gz package for Linux from an already-configured and
# already-built CMake tree in ./build. The archive contains a standard FHS
# "usr/" tree (bin/, share/besprited/data, share/applications, share/icons, ...)
# plus install.sh / uninstall.sh at its root.
#
# Usage:
#   sh packaging/package_linux_tarball.sh [VERSION]
#
# VERSION defaults to `git describe` (falls back to "dev"). The resulting file
# is written to build/besprited-<version>-linux-<arch>.tar.gz.

set -e

rootdir=$(pwd)
builddir="$rootdir/build"
arch="$(uname -m)"
version="${1:-$(git -C "$rootdir" describe --tags --always 2>/dev/null || echo dev)}"

if [ ! -d "$builddir" ]; then
  echo "package_linux_tarball.sh: '$builddir' not found - configure and build first" >&2
  exit 1
fi

stagedir="$builddir/tarball-stage"
rm -rf "$stagedir"
mkdir -p "$stagedir"

# Reuse the existing install() rules (src/CMakeLists.txt and
# packaging/desktop/CMakeLists.txt) to stage the FHS tree under stage/usr.
cmake --install "$builddir" --prefix "$stagedir/usr"

# Bundle the self-install scripts at the archive root.
cp "$rootdir/packaging/install.sh" "$stagedir/install.sh"
cp "$rootdir/packaging/uninstall.sh" "$stagedir/uninstall.sh"
chmod +x "$stagedir/install.sh" "$stagedir/uninstall.sh"

tarball="besprited-$version-linux-$arch.tar.gz"
rm -f "$builddir/$tarball"
tar -czf "$builddir/$tarball" -C "$stagedir" .

echo "Created $builddir/$tarball"
