#!/bin/sh
# Besprited | Copyright (C) 2026 Veritaware
# Licensed under the MIT License (https://opensource.org/licenses/MIT).
#
# Removes a Besprited install that was created by install.sh. It deletes the
# files listed in the bundled "usr/" tree from the given prefix, so run it from
# the root of the same tarball you installed from.
#
#   ./uninstall.sh                 remove from /usr/local (needs root)
#   ./uninstall.sh --user          remove from $HOME/.local
#   ./uninstall.sh --prefix DIR    remove from DIR

set -e

prefix=/usr/local
user_install=0

usage() {
  cat <<EOF
Usage: ./uninstall.sh [options]

  --user          Remove from \$HOME/.local
  --prefix DIR    Remove from DIR (default: /usr/local)
  -h, --help      Show this help
EOF
}

explicit_prefix=0

while [ $# -gt 0 ]; do
  case "$1" in
    --user) user_install=1; prefix="$HOME/.local" ;;
    --prefix) shift; [ $# -gt 0 ] || { echo "uninstall.sh: --prefix needs an argument" >&2; exit 1; }; prefix="$1"; explicit_prefix=1 ;;
    --prefix=*) prefix="${1#*=}"; explicit_prefix=1 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "uninstall.sh: unknown option '$1'" >&2; usage; exit 1 ;;
  esac
  shift
done

srcdir=$(cd "$(dirname "$0")" && pwd)

if [ ! -d "$srcdir/usr" ]; then
  echo "uninstall.sh: '$srcdir/usr' not found - run this script from the extracted tarball root" >&2
  exit 1
fi

if [ -d "$prefix" ] && [ ! -w "$prefix" ]; then
  if [ "$explicit_prefix" -eq 1 ]; then
    echo "uninstall.sh: $prefix is not writable - re-run with sudo" >&2
  else
    echo "uninstall.sh: writing to $prefix requires root - re-run with sudo, or pass --user" >&2
  fi
  exit 1
fi

echo "Removing Besprited from $prefix ..."

# Delete every file/symlink that the bundled tree would have installed.
( cd "$srcdir/usr" && find . \( -type f -o -type l \) -print ) | while IFS= read -r rel; do
  rm -f "$prefix/${rel#./}"
done

# Prune directories that are now empty (deepest first).
( cd "$srcdir/usr" && find . -depth -type d -print ) | while IFS= read -r rel; do
  rmdir "$prefix/${rel#./}" 2>/dev/null || true
done

if command -v update-desktop-database >/dev/null 2>&1 && [ -d "$prefix/share/applications" ]; then
  update-desktop-database "$prefix/share/applications" >/dev/null 2>&1 || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1 && [ -d "$prefix/share/icons/hicolor" ]; then
  gtk-update-icon-cache -f -t "$prefix/share/icons/hicolor" >/dev/null 2>&1 || true
fi
if command -v update-mime-database >/dev/null 2>&1 && [ -d "$prefix/share/mime" ]; then
  update-mime-database "$prefix/share/mime" >/dev/null 2>&1 || true
fi

echo "Done."
