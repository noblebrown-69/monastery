#!/bin/bash
set -e

echo "=== Monastery AppImage Builder ==="
echo "Starting build process..."

echo "Running rebuild.sh to get latest binary..."
./rebuild.sh

# Download deployment tools only once (safe in root, never deleted)
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    echo "Downloading linuxdeploy-x86_64.AppImage..."
    wget -q --show-progress https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
fi
if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    echo "Downloading linuxdeploy-plugin-qt-x86_64.AppImage..."
    wget -q --show-progress https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
fi

echo "Making deployment tools executable..."
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage

echo "Cleaning up previous AppDir..."
rm -rf AppDir

# Auto-fix icon: force exact 512x512 square
echo "Preparing icon (forcing exact 512x512 square)..."
if command -v convert >/dev/null 2>&1; then
    convert monastery.png -resize 512x512^ -gravity center -extent 512x512 -background none monastery.png
    echo "Icon resized to perfect 512x512 square."
else
    echo "WARNING: ImageMagick (convert) not found."
    echo "Please install once with: sudo apt install imagemagick"
    echo "Then run this script again."
fi

echo "Bundling Qt application into AppImage (with icon + desktop)..."
./linuxdeploy-x86_64.AppImage --appdir AppDir \
    --plugin qt \
    --executable build/Monastery \
    --icon-file monastery.png \
    --desktop-file Monastery.desktop \
    --output appimage

# Final rename so we always get the clean name we expect
echo "Renaming to Monastery.AppImage for clean Dropbox use..."
mv Monastery-x86_64.AppImage Monastery.AppImage 2>/dev/null || true

echo "✅ Success! Monastery.AppImage is ready."
ls -lh Monastery.AppImage
echo "Copy this to your Dropbox folder and test resize/maximize + icon on any Linux box."