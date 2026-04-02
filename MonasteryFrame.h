#ifndef MONASTERYFRAME_H
#define MONASTERYFRAME_H

#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QLabel>

class MonasteryEditor;

class MonasteryFrame : public QWidget {
    Q_OBJECT

public:
    MonasteryFrame(QWidget *parent = nullptr);
    ~MonasteryFrame();
    static QString getRealAppDir();

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
    void onNumberedList();
    void onFontChanged(const QString &font);
    void onSizeChanged(const QString &size);
    void onPrint();
    void onInsertPageBreak();
    void updateWordCount();

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDocsFolder();
    QIcon createToolbarIcon(const QString &symbol);

    MonasteryEditor *m_editor;
    QTimer *m_autoSaveTimer;
    QString m_docsDir;
    QString m_currentFilePath;
    QRect m_normalGeometry;

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
    QAction *m_pageBreakAction;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QLabel *m_wordCountLabel;
};

#endif // MONASTERYFRAME_H