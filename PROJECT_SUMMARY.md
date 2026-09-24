# Monastery Word Processor - Project Summary

**Last Updated:** May 28, 2026

## Current State
- Successfully upgraded from QTextEdit to **QWebEngineView** with beautiful centered parchment page view (8.5x11in, leather theme preserved).
- Frameless leather UI, custom title bar, toolbar, menus, status bar, drag/resize all intact.
- Most core features working:
  - Typing, real WYSIWYG editing
  - Bold, Italic, Underline, Strikethrough
  - Alignment (Left/Center/Right/Justify)
  - Bullet & Numbered lists
  - Font & Size comboboxes (mostly)
  - Word count (live)
  - Save / Open / New
  - Auto-save (but needs fix)

## Remaining Issues / Tasks
1. **Auto-save** — Currently always writes to `Monastery_AutoSave.html` instead of respecting the user's saved filename.
2. **Print to PDF** — Crashes the application.
3. **Narrow Margins toggle** — Add option for 0.5in margins vs default 0.75in.

## Key Files
- `MonasteryFrame.cpp` — Main UI and actions
- `MonasteryEditor.cpp/h` — QWebEngineView wrapper
- `editor.html` — Embedded parchment page (in resources.qrc)
- `rebuild.sh`, `build-appimage.sh`

## Philosophy
- Keep code minimal, fast, old-school C++
- Beautiful leather/parchment aesthetic
- Single binary + Dropbox portable
- Future: Classic Blue & Matrix themes

---

**To continue development on any machine:**
1. Start Grok Build CLI in the project root.
2. Paste: `Read PROJECT_SUMMARY.md first for full context. Continue from current state.`
3. Then use targeted micro-prompts.

Status: **Editor core is solid. Polish phase.**
