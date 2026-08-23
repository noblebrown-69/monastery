#!/bin/bash
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")"

echo "=== Monastery AppImage Builder ==="
echo "Using existing Release binary (will not rebuild)."

BIN="build/Monastery"
if [ ! -x "$BIN" ]; then
    echo "ERROR: $BIN is missing or not executable. Build it first; this script does not call rebuild.sh."
    exit 1
fi

if [ ! -f monastery.png ]; then
    echo "ERROR: monastery.png is missing."
    exit 1
fi

if [ ! -f Monastery.desktop ]; then
    echo "ERROR: Monastery.desktop is missing."
    exit 1
fi

if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    echo "ERROR: linuxdeploy-x86_64.AppImage is missing."
    exit 1
fi
if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    echo "ERROR: linuxdeploy-plugin-qt-x86_64.AppImage is missing."
    exit 1
fi
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage

export QMAKE=/usr/bin/qmake6
export APPIMAGE_EXTRACT_AND_RUN=1
export PATH="$PWD:$PATH"
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu/qt6${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
# Encourage linuxdeploy-plugin-qt to pull extra WebEngine-related plugins
export EXTRA_QT_PLUGINS="${EXTRA_QT_PLUGINS:-webview;qmltooling;position}"

HUNSPELL_LIB="/usr/lib/x86_64-linux-gnu/libhunspell-1.7.so.0"
if [ ! -e "$HUNSPELL_LIB" ]; then
    echo "ERROR: $HUNSPELL_LIB not found."
    exit 1
fi

if [ ! -f /usr/share/hunspell/en_US.aff ] || [ ! -f /usr/share/hunspell/en_US.dic ]; then
    echo "Hunspell en_US dictionaries missing; installing hunspell-en-us..."
    sudo -n apt-get update -qq
    sudo -n apt-get install -y --no-install-recommends hunspell-en-us
fi

echo "Cleaning previous AppDir..."
rm -rf AppDir

run_linuxdeploy() {
    local -a cmd
    cmd=(./linuxdeploy-x86_64.AppImage)
    if [ "${1:-}" = "extract" ]; then
        cmd+=(--appimage-extract-and-run)
    fi
    cmd+=(--appdir AppDir
          --plugin qt
          --executable "$BIN"
          --icon-file monastery.png
          --desktop-file Monastery.desktop
          --library "$HUNSPELL_LIB"
          --output appimage)
    echo "Running: ${cmd[*]}"
    "${cmd[@]}"
}

echo "Bundling Qt application + Hunspell into AppImage..."
if ! run_linuxdeploy; then
    echo "linuxdeploy failed; retrying with --appimage-extract-and-run..."
    run_linuxdeploy extract
fi

copy_hunspell() {
    mkdir -p AppDir/usr/share/hunspell
    if [ -f /usr/share/hunspell/en_US.aff ]; then
        cp -a /usr/share/hunspell/en_US.aff AppDir/usr/share/hunspell/
    fi
    if [ -f /usr/share/hunspell/en_US.dic ]; then
        cp -a /usr/share/hunspell/en_US.dic AppDir/usr/share/hunspell/
    fi
}

copy_webengine() {
    echo "Copying Qt WebEngine process, resources, locales, and libraries into AppDir..."
    mkdir -p AppDir/usr/lib/qt6/libexec
    mkdir -p AppDir/usr/libexec
    mkdir -p AppDir/usr/bin
    mkdir -p AppDir/usr/share/qt6/resources
    mkdir -p AppDir/usr/share/qt6/translations/qtwebengine_locales
    mkdir -p AppDir/usr/lib
    mkdir -p AppDir/usr/lib/x86_64-linux-gnu

    if [ -x /usr/lib/qt6/libexec/QtWebEngineProcess ]; then
        cp -a /usr/lib/qt6/libexec/QtWebEngineProcess AppDir/usr/lib/qt6/libexec/
        cp -a /usr/lib/qt6/libexec/QtWebEngineProcess AppDir/usr/libexec/
        cp -a /usr/lib/qt6/libexec/QtWebEngineProcess AppDir/usr/bin/
    fi

    if [ -d /usr/share/qt6/resources ]; then
        cp -a /usr/share/qt6/resources/. AppDir/usr/share/qt6/resources/
    fi

    if [ -d /usr/share/qt6/translations/qtwebengine_locales ]; then
        cp -a /usr/share/qt6/translations/qtwebengine_locales/. AppDir/usr/share/qt6/translations/qtwebengine_locales/
    fi

    # Also place paks where some Qt builds look (next to libexec)
    mkdir -p AppDir/usr/lib/qt6/resources
    cp -a /usr/share/qt6/resources/. AppDir/usr/lib/qt6/resources/ 2>/dev/null || true

    local libdir="/usr/lib/x86_64-linux-gnu"
    local -a we_libs=(
        libQt6WebEngineWidgets.so.6
        libQt6WebEngineCore.so.6
        libQt6QuickWidgets.so.6
        libQt6Quick.so.6
        libQt6WebChannel.so.6
        libQt6Qml.so.6
        libQt6QmlModels.so.6
        libQt6Positioning.so.6
        libQt6PrintSupport.so.6
        libQt6OpenGL.so.6
        libQt6Network.so.6
    )
    local lib
    for lib in "${we_libs[@]}"; do
        if [ -e "$libdir/$lib" ]; then
            cp -a "$libdir/$lib"* AppDir/usr/lib/ 2>/dev/null || true
            if [ -d AppDir/usr/lib/x86_64-linux-gnu ]; then
                cp -a "$libdir/$lib"* AppDir/usr/lib/x86_64-linux-gnu/ 2>/dev/null || true
            fi
        fi
    done
}

has_webengine() {
    local proc pak
    proc="$(find AppDir -name QtWebEngineProcess -type f 2>/dev/null | head -n 1 || true)"
    pak="$(find AppDir -name qtwebengine_resources.pak -type f 2>/dev/null | head -n 1 || true)"
    [ -n "$proc" ] && [ -n "$pak" ]
}

copy_hunspell

if ! has_webengine; then
    echo "WARNING: QtWebEngineProcess or qtwebengine_resources.pak missing from AppDir."
    copy_webengine
    echo "Re-running linuxdeploy so the AppImage includes WebEngine files..."
    if ! run_linuxdeploy; then
        echo "linuxdeploy failed; retrying with --appimage-extract-and-run..."
        run_linuxdeploy extract
    fi
    copy_hunspell
    if ! has_webengine; then
        echo "WebEngine files still missing after re-run; copying again and rebuilding AppImage..."
        copy_webengine
        copy_hunspell
        if ! run_linuxdeploy; then
            run_linuxdeploy extract
        fi
        copy_hunspell
    fi
fi

# Rename linuxdeploy output (desktop Name=Monastery)
if [ -f Monastery-x86_64.AppImage ]; then
    mv -f Monastery-x86_64.AppImage Monastery.AppImage
elif ls Monastery-*.AppImage >/dev/null 2>&1; then
    shopt -s nullglob
    for f in Monastery-*.AppImage; do
        case "$f" in
            linuxdeploy*) continue ;;
        esac
        mv -f "$f" Monastery.AppImage
        break
    done
    shopt -u nullglob
fi

if [ ! -f Monastery.AppImage ]; then
    echo "ERROR: Monastery.AppImage was not produced."
    ls -lh ./*.AppImage 2>/dev/null || true
    exit 1
fi

echo ""
echo "=== AppDir sanity ==="
echo -n "Monastery binary: "
find AppDir -type f \( -path 'AppDir/usr/bin/Monastery' -o -name Monastery \) ! -name '*.desktop' | head -n 5
echo -n "libQt6WebEngineWidgets: "
find AppDir -name 'libQt6WebEngineWidgets.so*' | head -n 5
echo -n "QtWebEngineProcess: "
find AppDir -name QtWebEngineProcess
echo -n "qtwebengine_resources.pak: "
find AppDir -name qtwebengine_resources.pak

echo ""
echo "✅ Monastery.AppImage ready"
ls -lh Monastery.AppImage
