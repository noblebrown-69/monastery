#ifndef MONASTERYFRAME_H
#define MONASTERYFRAME_H

#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QResizeEvent>

class MonasteryEditor;

class MonasteryFrame : public QWidget {
    Q_OBJECT

public:
    MonasteryFrame(QWidget *parent = nullptr);
    ~MonasteryFrame();

protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onNew();
    void onOpen();
    void onSave();
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

    MonasteryEditor *m_editor;
    QTimer *m_autoSaveTimer;
    QString m_docsDir;

    // Actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
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
};

#endif // MONASTERYFRAME_H