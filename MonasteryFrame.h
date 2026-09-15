#ifndef MONASTERYFRAME_H
#define MONASTERYFRAME_H

#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QLabel>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QActionGroup>
#include "Theme.h"

class MonasteryEditor;
class QPlainTextEdit;
class QStackedWidget;
class QFontComboBox;
class QComboBox;
class QTemporaryFile;

class MonasteryFrame : public QWidget {
    Q_OBJECT

public:
    MonasteryFrame(QWidget *parent = nullptr);
    ~MonasteryFrame();
    static QString getRealAppDir();
    bool openPath(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onExit();
    void onAutoSave();
    void onBold();
    void onItalic();
    void onUnderline();
    void onAlignLeft();
    void onAlignCenter();
    void onAlignRight();
    void onJustify();
    void onBulletList();
    void onChecklist();
    void onNumberedList();
    void onFontChanged(const QString &font);
    void onSizeChanged(const QString &size);
    void onSelectionFontChanged(const QString &family, int pt);
    void onPrint();
    void onInsertPageBreak();
    void updateWordCount();
    void onToggleNarrowMargins();
    void onFind();
    void onFindNext();
    void onFindPrevious();
    void onToggleFocusMode();
    void maybeRestoreAutosave();
    void updateTitleBar();
    void applyTheme(ThemeId id);


private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDocsFolder();
    QIcon createToolbarIcon(const QString &symbol);
    void colorizeToolbarIcons(const Theme &theme);
    void applyUiFont(const Theme &theme);


    bool confirmProceedIfDirty();
    bool hasNamedDocument() const;
    QString documentDisplayName() const;
    QString autosaveSidecarPath() const;
    bool htmlLooksEmpty(const QString &html) const;
    bool wouldClobberManuscript(const QString &incoming) const;
    bool writeHtmlFile(const QString &path, const QString &html);
    bool persistDocument(const QString &path, const QString &html, bool markCleanAfter);
    bool ensureSavePath();
    bool saveNow();
    bool saveMarkdownNow();
    QString waitForEditorHtml();
    bool loadHtmlDocument(const QString &path);
    bool loadMarkdownDocument(const QString &path);
    bool isMarkdownSourcePath(const QString &path) const;
    bool isMarkdownMode() const;
    void setMarkdownMode(bool on);
    void setFormatActionsEnabled(bool on);
    int askRestoreAutosave();
    void emitListenHealth(const QString &openedPath, const QString &recovery);
    void dumpListenSelectionFont();
    void requestListenQuit();
    void onPdfPrintingFinished(const QString &path, bool success);
    bool maybeStartListenPrint();
    bool documentIsDirty() const;
    void setupFindDialog();
    void runFind(bool backward);

    MonasteryEditor *m_editor;
    QStackedWidget *m_editorStack = nullptr;
    QPlainTextEdit *m_mdEdit = nullptr;
    QTimer *m_autoSaveTimer;
    QTimer *m_wordCountPollTimer;
    bool m_narrowMargins = false;
    QString m_docsDir;
    QString m_currentFilePath;
    QRect m_normalGeometry;

    QWidget *m_titleBar = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_minBtn = nullptr;
    QPushButton *m_maxBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QMenuBar *m_menuBar = nullptr;
    QToolBar *m_toolBar = nullptr;
    QFontComboBox *m_fontCombo = nullptr;
    QComboBox *m_sizeCombo = nullptr;
    QStatusBar *m_statusBar = nullptr;
    QActionGroup *m_themeGroup = nullptr;
    Theme m_currentTheme;

    QPoint m_dragPosition;
    bool m_dragging = false;
    bool m_focusMode = false;
    bool m_didOfferRestore = false;
    bool m_restoreDialogUp = false;
    bool m_markdownMode = false;
    bool m_listenQuitArmed = false;
    bool m_listenPrintPending = false;
    QTemporaryFile *m_printTemp = nullptr;
    QString m_pendingLpPdf;
    QString m_pendingLpPrinter;
    int m_pendingLpCopies = 1;
    QString m_pendingOpenPath;

    QDialog *m_findDialog = nullptr;
    QLineEdit *m_findEdit = nullptr;

    // Resize handling
    bool m_resizing;
    QPoint m_resizeStartPos;
    QPoint m_resizeStartMousePos;
    QSize m_resizeStartSize;
    enum ResizeDirection { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeDirection m_resizeDirection;

    // Actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_saveAsAction;
    QAction *m_printAction;
    QAction *m_exitAction;
    QAction *m_boldAction;
    QAction *m_italicAction;
    QAction *m_underlineAction;
    QAction *m_alignLeftAction;
    QAction *m_alignCenterAction;
    QAction *m_alignRightAction;
    QAction *m_justifyAction;
    QAction *m_bulletAction;
    QAction *m_numberAction;
    QAction *m_checklistAction;
    QAction *m_pageBreakAction;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_narrowMarginsAction;
    QAction *m_findAction;
    QAction *m_focusModeAction;
    QLabel *m_wordCountLabel;
};

#endif // MONASTERYFRAME_H
