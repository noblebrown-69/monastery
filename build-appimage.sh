#!/bin/bash
set -e

echo "=== Monastery AppImage Builder ==="
echo "Starting build process..."

echo "Running rebuild.sh to get latest binary..."
./rebuild.sh

# Download deployment tools only once
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

# Install Hunspell if not present (one-time)
echo "Ensuring Hunspell is installed..."
sudo apt-get update -qq
sudo apt-get install -y --no-install-recommends libhunspell-dev hunspell-en-us

echo "Cleaning up previous AppDir..."
rm -rf AppDir

# Prepare icon
echo "Preparing icon (forcing exact 512x512 square)..."
if command -v convert >/dev/null 2>&1; then
    convert monastery.png -resize 512x512^ -gravity center -extent 512x512 -background none monastery.png
    echo "Icon resized to perfect 512x512 square."
else
    echo "WARNING: ImageMagick not found. Install with: sudo apt install imagemagick"
fi

echo "Bundling Qt application + Hunspell into AppImage..."
./linuxdeploy-x86_64.AppImage --appdir AppDir \
    --plugin qt \
    --executable build/Monastery \
    --icon-file monastery.png \
    --desktop-file Monastery.desktop \
    --library /usr/lib/x86_64-linux-gnu/libhunspell-1.7.so.0 \
    --output appimage

# Manually copy dictionary files into the AppImage (Hunspell needs them)
mkdir -p AppDir/usr/share/hunspell
cp /usr/share/hunspell/en_US.aff AppDir/usr/share/hunspell/ 2>/dev/null || true
cp /usr/share/hunspell/en_US.dic AppDir/usr/share/hunspell/ 2>/dev/null || true

echo "Renaming to Monastery.AppImage..."
mv Monastery-x86_64.AppImage Monastery.AppImage 2>/dev/null || true

echo "✅ Success! Monastery.AppImage is ready with Hunspell spell checking."
ls -lh Monastery.AppImage
echo "Copy this to your Dropbox folder."