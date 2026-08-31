#!/bin/sh
# Besprited | Copyright (C) 2026 Veritaware
# Licensed under the MIT License (https://opensource.org/licenses/MIT).
#
# Installs the bundled "usr/" tree into standard FHS locations.
#
#   ./install.sh                 install into /usr/local (needs root)
#   ./install.sh --user          install into $HOME/.local (no root needed)
#   ./install.sh --prefix DIR    install into DIR
#
# Run this from the root of the extracted tarball.

set -e

prefix=/usr/local
user_install=0

usage() {
  cat <<EOF
Usage: ./install.sh [options]

  --user          Install into \$HOME/.local (no root required)
  --prefix DIR    Install into DIR (default: /usr/local)
  -h, --help      Show this help
EOF
}

explicit_prefix=0

while [ $# -gt 0 ]; do
  case "$1" in
    --user) user_install=1; prefix="$HOME/.local" ;;
    --prefix) shift; [ $# -gt 0 ] || { echo "install.sh: --prefix needs an argument" >&2; exit 1; }; prefix="$1"; explicit_prefix=1 ;;
    --prefix=*) prefix="${1#*=}"; explicit_prefix=1 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "install.sh: unknown option '$1'" >&2; usage; exit 1 ;;
  esac
  shift
done

srcdir=$(cd "$(dirname "$0")" && pwd)

if [ ! -d "$srcdir/usr" ]; then
  echo "install.sh: '$srcdir/usr' not found - run this script from the extracted tarball root" >&2
  exit 1
fi

# Find the nearest existing ancestor of the prefix and check we can write to it.
probe="$prefix"
while [ ! -d "$probe" ]; do
  parent=$(dirname "$probe")
  [ "$parent" = "$probe" ] && break
  probe="$parent"
done
if [ ! -w "$probe" ]; then
  if [ "$explicit_prefix" -eq 1 ]; then
    echo "install.sh: $probe is not writable - re-run with sudo" >&2
  else
    echo "install.sh: writing to $prefix requires root - re-run with sudo, or pass --user" >&2
  fi
  exit 1
fi

echo "Installing Besprited into $prefix ..."
mkdir -p "$prefix"
cp -a "$srcdir/usr/." "$prefix/"

# Refresh the desktop/icon/mime caches so the launcher entry shows up right away.
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "$prefix/share/applications" >/dev/null 2>&1 || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t "$prefix/share/icons/hicolor" >/dev/null 2>&1 || true
fi
if command -v update-mime-database >/dev/null 2>&1 && [ -d "$prefix/share/mime" ]; then
  update-mime-database "$prefix/share/mime" >/dev/null 2>&1 || true
fi

# Don't try to auto-resolve system dependencies (too fragile across distros);
# just report any shared libraries the dynamic loader can't find.
bin="$prefix/bin/besprited"
if command -v ldd >/dev/null 2>&1 && [ -x "$bin" ]; then
  missing=$(ldd "$bin" 2>/dev/null | awk '/not found/ { print $1 }' || true)
  if [ -n "$missing" ]; then
    echo
    echo "Warning: these shared libraries could not be resolved:"
    echo "$missing" | sed 's/^/  /'
    echo "Install the matching packages for your distribution - see the"
    echo "'Linux dependencies' section of INSTALL.md:"
    echo "  https://github.com/Veritaware/Besprited/blob/trunk/INSTALL.md"
  fi
fi

echo
echo "Done."
if [ "$user_install" -eq 1 ]; then
  case ":$PATH:" in
    *":$prefix/bin:"*) ;;
    *) echo "Note: $prefix/bin is not on your PATH." ;;
  esac
fi
echo "You may need to log out and back in for the launcher entry to appear."
