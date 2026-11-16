#!/bin/sh
# Generates data/icons/besprited.ico and data/icons/doc.ico (used by the
# Windows resource file and installer) from the 16x16 sources.
# Requires ImageMagick and icoutils. Can be run from any directory.
set -eu

for tool in magick icotool; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "$tool not found" >&2
        exit 127
    fi
done

icondir=$(cd "$(dirname "$0")" && pwd)
outdir="$icondir/../../../data/icons"
mkdir -p "$outdir"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

# make_ico <source-16x16.png> <output.ico>
make_ico() {
    src=$1
    out=$2
    cp "$src" "$tmpdir/16.png"
    for size in 32 48 64 128 256; do
        magick "$src" -filter Point -resize "${size}x${size}" "$tmpdir/$size.png"
    done
    icotool -c -o "$out" "$tmpdir/16.png" "$tmpdir/32.png" "$tmpdir/48.png" \
        "$tmpdir/64.png" "$tmpdir/128.png" "$tmpdir/256.png"
}

make_ico "$icondir/besprited16.png" "$outdir/besprited.ico"
make_ico "$icondir/doc16.png" "$outdir/doc.ico"
