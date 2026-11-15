#!/bin/sh
# Generates the hicolor icon theme PNGs from besprited16.png.
# Requires ImageMagick. Can be run from any directory.
set -eu

# ImageMagick 6 (e.g. Ubuntu 24.04) only provides "convert".
if command -v magick >/dev/null 2>&1; then
    im=magick
elif command -v convert >/dev/null 2>&1; then
    im=convert
else
    echo "imagemagick not found" >&2
    exit 127
fi

icondir=$(cd "$(dirname "$0")" && pwd)

for size in 16 32 48 64 256; do
    outdir="$icondir/hicolor/${size}x${size}/apps"
    mkdir -p "$outdir"
    "$im" "$icondir/besprited16.png" -filter Point -resize "${size}x${size}" "$outdir/besprited.png"
done
