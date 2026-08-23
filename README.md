# Monastery

A calm, portable Linux word processor. One AppImage. No install. Drop it in Dropbox and write.

Qt6 + C++. The page is a real sheet of parchment on a leather desk. Three other skins live under **Theme**.

![Monastery](https://github.com/noblebrown-69/monastery/raw/main/screenshot2.0.png)

## What's in 2.0

- **Parchment page** — 8.5 × 11 in Chromium editor, gold selection, leather chrome
- **Themes** — Leather (default), WordPerfect 5.1, Matrix, IBM Amber. Frame, menus, icons, and page all switch together. Last theme is remembered.
- **Writer-safe files** — Save and autosave will not write empty HTML over a manuscript. Named documents autosave to a sidecar `*_autosave.html`. Untitled work goes to `Docs/Monastery_AutoSave.html` and offers restore on launch.
- **Dirty documents** — Title bar shows the filename and a `*`. Close, New, and Open ask if the buffer changed. File → Exit goes through the same path.
- **Find** — Ctrl+F
- **Focus** — F11 hides the menu, toolbar, and status. Title bar stays so you can still drag the window.
- **PDF** — Letter, real inch margins, black type on white. The desk and parchment stay on screen, not in the export.
- **Narrow margins**, live word count, B/I/U, lists, alignment, page break
- **Portable AppImage** — Qt6, WebEngine, and Hunspell dictionaries are inside. No system Qt required.

HTML is the document format for now.

## Download

Get `Monastery.AppImage` from [Releases](https://github.com/noblebrown-69/monastery/releases).

```bash
chmod +x Monastery.AppImage
./Monastery.AppImage
```

Works on recent x86_64 Linux (glibc new enough for Ubuntu 24.04 builds). Very old distros will not run it.

Docs and autosaves land in a `Docs/` folder next to the AppImage.

## Build from source

```bash
sudo apt install -y cmake ninja-build g++ \
  qt6-base-dev qt6-webengine-dev libhunspell-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build
./build/Monastery
```

AppImage (uses the binary you just built; does not wipe `build/`):

```bash
./build-appimage.sh
```

## Theme notes

| Theme | Screen | Type |
| --- | --- | --- |
| Leather | Parchment sheet on leather | Georgia / Noto Serif |
| WordPerfect 5.1 | IBM text-mode blue `#0000AA`, grey menu | White on blue |
| Matrix | Black | `#00FF41` |
| IBM Amber | Black | P3 phosphor `#FFB000` |

PDF export is always ink on white, regardless of theme.

(icons: pictranoosa and Freepik via flaticon.com)
