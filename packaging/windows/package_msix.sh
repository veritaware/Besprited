#!/usr/bin/env bash
# Copyright (C) 2026 Veritaware
#
# Builds an (unsigned) MSIX package from an already built and packaged
# build/bin directory (see package_win.js).
#
# Usage: package_msix.sh <release-version|tag> [bin-dir] [output.msix]
#   <release-version>  e.g. 1.26.6, v1.27.03 or 1.27.0-beta; mapped to the
#                      MSIX four-part version A.B.C.0 (last part must be 0
#                      for Store submissions).
set -euo pipefail

VERSION_IN="${1:?usage: $0 <release-version|tag> [bin-dir] [output.msix]}"
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN_DIR="${2:-$HERE/../../build/bin}"
OUT="${3:-$HERE/besprited-v${VERSION_IN#v}-windows-x86_64.msix}"

# Map the release tag to a four-part MSIX version.
# Strips a leading 'v' and any '-suffix', drops leading zeros of each part
# (MSIX does not accept them, e.g. 1.27.03 -> 1.27.3) and pads to 4 parts
# with the last one forced to 0.
map_version() {
  local v="${1#v}" parts=() out=()
  v="${v%%[-+]*}"
  IFS=. read -r -a parts <<< "$v"
  [[ ${#parts[@]} -ge 1 && ${#parts[@]} -le 4 ]] || return 1
  for p in "${parts[@]}"; do
    [[ "$p" =~ ^[0-9]+$ ]] || return 1
    out+=("$((10#$p))")
  done
  while [[ ${#out[@]} -lt 3 ]]; do out+=(0); done
  echo "${out[0]}.${out[1]}.${out[2]}.0"
}

MSIX_VERSION="$(map_version "$VERSION_IN")" || {
  echo "ERROR: cannot map '$VERSION_IN' to an MSIX version" >&2
  exit 1
}
echo "MSIX version: $MSIX_VERSION"

[[ -f "$BIN_DIR/besprited.exe" ]] || { echo "ERROR: $BIN_DIR/besprited.exe not found" >&2; exit 1; }
[[ -d "$BIN_DIR/data" ]] || { echo "ERROR: $BIN_DIR/data not found" >&2; exit 1; }

# Locate makeappx.exe (Windows SDK) unless already in PATH.
MAKEAPPX="$(command -v makeappx.exe || true)"
if [[ -z "$MAKEAPPX" ]]; then
  MAKEAPPX="$(ls -d /c/Program\ Files\ \(x86\)/Windows\ Kits/10/bin/*/x64/makeappx.exe 2>/dev/null | sort -V | tail -n1 || true)"
fi
[[ -n "$MAKEAPPX" ]] || { echo "ERROR: makeappx.exe not found (install the Windows SDK)" >&2; exit 1; }

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

cp "$BIN_DIR/besprited.exe" "$STAGE/"
cp "$BIN_DIR"/*.dll "$STAGE/"
cp -r "$BIN_DIR/data" "$STAGE/data"

# Manifest with the release version substituted in <Identity>.
sed -E 's/^([[:space:]]*)Version="[0-9.]+"/\1Version="'"$MSIX_VERSION"'"/' \
  "$HERE/AppxManifest.xml" > "$STAGE/AppxManifest.xml"
grep -q "Version=\"$MSIX_VERSION\"" "$STAGE/AppxManifest.xml" \
  || { echo "ERROR: failed to set version in manifest" >&2; exit 1; }

# Copy only the assets referenced by the manifest.
mkdir "$STAGE/Assets"
missing=0
while IFS= read -r asset; do
  if [[ -f "$HERE/Assets/$asset" ]]; then
    cp "$HERE/Assets/$asset" "$STAGE/Assets/"
  else
    echo "ERROR: manifest references missing asset: Assets/$asset" >&2
    missing=1
  fi
done < <(grep -oE 'Assets\\[A-Za-z0-9_.-]+' "$HERE/AppxManifest.xml" | sed 's/^Assets\\//' | sort -u)
[[ $missing -eq 0 ]] || exit 1

rm -f "$OUT"
MSYS_NO_PATHCONV=1 "$MAKEAPPX" pack /o /d "$(cygpath -w "$STAGE")" /p "$(cygpath -w "$OUT")"
echo "Created $OUT"
