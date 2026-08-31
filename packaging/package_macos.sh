#!/usr/bin/env bash

rm -rf bundle
rm -f build/bin/clip_image_tests build/bin/clip_text_tests build/bin/clip_user_format_tests build/bin/gen
mkdir besprited.iconset
sips -z 16 16     data/icons/ase64.png --out besprited.iconset/icon_16x16.png
sips -z 32 32     data/icons/ase64.png --out besprited.iconset/icon_16x16@2x.png
sips -z 32 32     data/icons/ase64.png --out besprited.iconset/icon_32x32.png
sips -z 64 64     data/icons/ase64.png --out besprited.iconset/icon_32x32@2x.png
sips -z 128 128   data/icons/ase64.png --out besprited.iconset/icon_128x128.png
sips -z 256 256   data/icons/ase64.png --out besprited.iconset/icon_128x128@2x.png
sips -z 256 256   data/icons/ase64.png --out besprited.iconset/icon_256x256.png
sips -z 512 512   data/icons/ase64.png --out besprited.iconset/icon_256x256@2x.png
sips -z 512 512   data/icons/ase64.png --out besprited.iconset/icon_512x512.png
sips -z 1024 1024 data/icons/ase64.png --out besprited.iconset/icon_512x512@2x.png
iconutil -c icns -o besprited.icns besprited.iconset
rm -R besprited.iconset
mkdir bundle/
mkdir bundle/besprited.app
mkdir bundle/besprited.app/Contents
mkdir bundle/besprited.app/Contents/libs
mkdir bundle/besprited.app/Contents/MacOS
mkdir bundle/besprited.app/Contents/Resources
cp ./build/bin/besprited ./bundle/besprited.app/Contents/MacOS/
cp -r ./build/bin/data/ ./bundle/besprited.app/Contents/Resources/data/
cp ./besprited.icns ./bundle/besprited.app/Contents/Resources/
cp ./desktop/Info.plist ./bundle/besprited.app/Contents/
chmod +x ./bundle/besprited.app/Contents/MacOS/besprited
dylibbundler -od -b -ns -x ./bundle/besprited.app/Contents/MacOS/besprited -d ./bundle/besprited.app/Contents/libs/

# Check current RPATH entries after dylibbundler
echo "RPATH entries after dylibbundler:"
otool -l ./bundle/besprited.app/Contents/MacOS/besprited | grep -A2 LC_RPATH

# Clean up any duplicate RPATH entries from the main executable 
# Use otool to check current RPATHs and remove duplicates
CURRENT_RPATHS=$(otool -l ./bundle/besprited.app/Contents/MacOS/besprited | grep -A2 LC_RPATH | grep path | awk '{print $2}' | sort | uniq)
echo "Found RPATH entries: $CURRENT_RPATHS"
for rpath in $CURRENT_RPATHS; do
    echo "Removing RPATH: $rpath"
    install_name_tool -delete_rpath "$rpath" ./bundle/besprited.app/Contents/MacOS/besprited 2>/dev/null || true
done

# Add our RPATH cleanly
echo "Adding single RPATH: @executable_path/../libs/"
install_name_tool -add_rpath "@executable_path/../libs/" ./bundle/besprited.app/Contents/MacOS/besprited 2>/dev/null || true

# Verify final RPATH
echo "Final RPATH entries:"
otool -l ./bundle/besprited.app/Contents/MacOS/besprited | grep -A2 LC_RPATH

# Debug: List what libraries we have
echo "Libraries found:"
ls -la ./bundle/besprited.app/Contents/libs/ || echo "No libs directory found"

# Check RPATH entries in V8 libraries specifically
echo "Checking V8 library RPATH entries:"
if [ -f "./bundle/besprited.app/Contents/libs/libv8_libbase.dylib" ]; then
    echo "libv8_libbase.dylib RPATH entries:"
    otool -l ./bundle/besprited.app/Contents/libs/libv8_libbase.dylib | grep -A2 LC_RPATH || echo "No RPATH entries found"

    # Clean up any RPATH entries in libv8_libbase.dylib
    V8BASE_RPATHS=$(otool -l ./bundle/besprited.app/Contents/libs/libv8_libbase.dylib | grep -A2 LC_RPATH | grep path | awk '{print $2}' | sort | uniq)
    for rpath in $V8BASE_RPATHS; do
        echo "Removing RPATH from libv8_libbase.dylib: $rpath"
        install_name_tool -delete_rpath "$rpath" ./bundle/besprited.app/Contents/libs/libv8_libbase.dylib 2>/dev/null || true
    done
fi
if [ -f "./bundle/besprited.app/Contents/libs/libv8.dylib" ]; then
    echo "libv8.dylib RPATH entries:"
    otool -l ./bundle/besprited.app/Contents/libs/libv8.dylib | grep -A2 LC_RPATH || echo "No RPATH entries found"

    # Clean up any RPATH entries in libv8.dylib
    V8_RPATHS=$(otool -l ./bundle/besprited.app/Contents/libs/libv8.dylib | grep -A2 LC_RPATH | grep path | awk '{print $2}' | sort | uniq)
    for rpath in $V8_RPATHS; do
        echo "Removing RPATH from libv8.dylib: $rpath"
        install_name_tool -delete_rpath "$rpath" ./bundle/besprited.app/Contents/libs/libv8.dylib 2>/dev/null || true
    done
fi

# Clean up RPATH entries in ALL bundled libraries to be thorough
echo "Cleaning RPATH entries from all bundled libraries:"
for dylib in ./bundle/besprited.app/Contents/libs/*.dylib; do
    if [ -f "$dylib" ]; then
        LIB_RPATHS=$(otool -l "$dylib" | grep -A2 LC_RPATH | grep path | awk '{print $2}' | sort | uniq)
        if [ ! -z "$LIB_RPATHS" ]; then
            echo "Cleaning RPATHs from $(basename "$dylib"): $LIB_RPATHS"
            for rpath in $LIB_RPATHS; do
                install_name_tool -delete_rpath "$rpath" "$dylib" 2>/dev/null || true
            done
        fi
    fi
done

# Remove all existing signatures before re-signing
find ./bundle/besprited.app -type f \( -name "*.dylib" -o -name "besprited" \) -exec codesign --remove-signature {} \; 2>/dev/null || true

# Ad-hoc sign without hardened runtime (--options=runtime triggers CS_RUNTIME which causes
# macOS 27's code signing monitor to enforce strict page-level validation on every loaded
# dylib — validation that ad-hoc signatures produced by older runners fail. Without
# CS_RUNTIME, library validation is not kernel-enforced and all dylibs load freely.
# This also allows V8's JIT, which requires writable+executable memory that hardened
# runtime blocks.)
find ./bundle/besprited.app/Contents/libs -name "*.dylib" | while read -r dylib; do
    echo "Signing: $dylib"
    codesign --force -s - "$dylib" && echo "✓ $dylib" || echo "✗ $dylib"
done

codesign --force -s - ./bundle/besprited.app/Contents/MacOS/besprited && \
    echo "✓ Signed: besprited" || echo "✗ Failed: besprited"

codesign --force -s - ./bundle/besprited.app && \
    echo "✓ Signed: besprited.app" || echo "✗ Failed: besprited.app"
rm besprited.icns

# Styled "drag to install" DMG (background image, Applications shortcut).
# Requires dmgbuild: pip3 install dmgbuild
dmgbuild --detach-retries 5 -Dapp=bundle/besprited.app -s packaging/macos/dmg_settings.py "Besprited" besprited.dmg