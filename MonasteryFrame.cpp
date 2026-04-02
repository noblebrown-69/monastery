#include "MonasteryFrame.h"
#include "MonasteryEditor.h"
#include <QApplication>
#include <algorithm>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QFontComboBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <QCloseEvent>
#include <QPixmap>
#include <QIcon>
#include <QRegularExpression>
#include <QTextEdit>
#include <QPrinter>
#include <QPrintDialog>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QPainter>

// Member variables
QWidget *m_titleBar;
QMenuBar *m_menuBar;
QToolBar *m_toolBar;
QStatusBar *m_statusBar;
QPoint m_dragPosition;
bool m_dragging;
MonasteryEditor *m_editor;
QTimer *m_autoSaveTimer;
QString m_docsDir;
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
QAction *m_undoAction;
QAction *m_redoAction;
QAction *m_pageBreakAction;

// Embedded XPM icons for classic Word 6.0 look
static const char *bold_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"B c Black",
"BBBB............",
"B...B...........",
"B...B...........",
"BBBB............",
"B...B...........",
"B...B...........",
"BBBB............",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................","................",nullptr
};

static const char *italic_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"I c Black",
"........I.......",
".........I......",
"..........I.....",
"...........I....",
"............I...",
".............I..",
"..............I.",
"...............I",
"..............I.",
".............I..",
"............I...",
"...........I....",
"..........I.....",
".........I......",
"........I.......",
"................",
nullptr
};

static const char *underline_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"U c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"UUUUUUUUUUUUUUUU",
nullptr
};

static const char *alignleft_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"L c Black",
"   L............",
"  L.............",
" L..............",
"L...............",
" L..............",
"  L.............",
"   L............",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
nullptr
};

static const char *aligncenter_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"C c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
".....CCCCCCCC...",
nullptr
};

static const char *alignright_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"R c Black",
"............R   ",
".............R  ",
"..............R ",
"...............R",
"..............R ",
".............R  ",
"............R   ",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
nullptr
};

static const char *justify_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"J c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"JJJJJJJJJJJJJJJJ",
nullptr
};

static const char *bullet_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"o c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"......ooo.......",
nullptr
};

static const char *number_xpm[] = {
"16 16 3 1",
"  c None",
". c None",
"1 c Black",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"......1.........",
nullptr
};

MonasteryFrame::MonasteryFrame(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    setStyleSheet("QWidget { background-color: #3C2F2F; }");  // Match title bar color for borders
    setFont(QFont("Noto Serif", 12));
    setGeometry(100, 100, 800, 600);

    createDocsFolder();
    m_currentFilePath.clear();
    m_dragging = false;
    m_resizing = false;
    m_resizeDirection = None;

    createActions();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // titleBar - dark leather like Aureus
    m_titleBar = new QWidget;
    m_titleBar->setStyleSheet("background-color: #3C2F2F;");
    m_titleBar->setFixedHeight(30);
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(10,0,10,0);

    QPushButton *minBtn = new QPushButton("—");
    minBtn->setFixedSize(30,30);
    minBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    QPushButton *maxBtn = new QPushButton("□");
    maxBtn->setFixedSize(30,30);
    maxBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(maxBtn, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
            setGeometry(m_normalGeometry);
        } else {
            m_normalGeometry = geometry();
            showMaximized();
        }
    });
    QPushButton *closeBtn = new QPushButton("×");
    closeBtn->setFixedSize(30,30);
    closeBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    QLabel *titleLabel = new QLabel("Monastery");
    QFont titleFont("Noto Serif", 10, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #D4AF37;");   // gold contrast like Aureus

    QWidget *leftSpacer = new QWidget();
    leftSpacer->setFixedWidth(90);

    titleLayout->addWidget(leftSpacer);
    titleLayout->addStretch();
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(minBtn);
    titleLayout->addWidget(maxBtn);
    titleLayout->addWidget(closeBtn);
    mainLayout->addWidget(m_titleBar);

    // menuBar - dark leather with readable menu items
    m_menuBar = new QMenuBar;
    m_menuBar->setStyleSheet("QMenuBar { background-color: #3C2F2F; color: #D4AF37; }"
                             "QMenuBar::item { background-color: transparent; color: #D4AF37; }"
                             "QMenuBar::item:selected { background-color: #5C4A3F; color: #F5E8C7; }"
                             "QMenu { background-color: #3C2F2F; color: #D4AF37; border: 1px solid #5C4A3F; }"
                             "QMenu::item { background-color: transparent; color: #D4AF37; }"
                             "QMenu::item:selected { background-color: #5C4A3F; color: #F5E8C7; }");
    QMenu *fileMenu = m_menuBar->addMenu("&File");
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addAction(m_printAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);
    foreach (QAction *action, fileMenu->actions()) {
        action->setIconVisibleInMenu(false);
    }
    QMenu *editMenu = m_menuBar->addMenu("&Edit");
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editMenu->addSeparator();
    editMenu->addAction(m_cutAction);
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(m_pageBreakAction);
    mainLayout->addWidget(m_menuBar);

    // toolBar - leather theme with readable elements
    m_toolBar = new QToolBar;
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setIconSize(QSize(16, 16));
    m_toolBar->setStyleSheet("QToolBar {"
                             "  background-color: #6F5A4A;"
                             "  border-left: 8px solid #3C2F2F;"
                             "  border-right: 8px solid #3C2F2F;"
                             "  border-top: 0;"
                             "  border-bottom: 0;"
                             "  padding: 4px 0;"
                             "}"
                             "QToolButton { background-color: transparent; border: none; padding: 2px; }"
                             "QToolButton:hover { background-color: #8B7355; border-radius: 2px; }"
                             "QToolButton:pressed { background-color: #5C4A3F; }"
                             "QComboBox { background-color: #5C4A3F; color: #F5E8C7; border: 1px solid #8B7355; border-radius: 2px; padding: 2px; min-width: 60px; }"
                             "QComboBox:hover { background-color: #8B7355; }"
                             "QComboBox::drop-down { border: none; background-color: #5C4A3F; }"
                             "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 4px solid #F5E8C7; margin-right: 4px; }"
                             "QComboBox QAbstractItemView { background-color: #3C2F2F; color: #F5E8C7; border: 1px solid #5C4A3F; selection-background-color: #8B7355; }");

    // Set icons (keep all the existing icon lines exactly as they are)
    m_boldAction->setIcon(QIcon(":/icons/bold.png"));
    m_italicAction->setIcon(QIcon(":/icons/italic.png"));
    m_underlineAction->setIcon(QIcon(":/icons/underline.png"));
    m_alignLeftAction->setIcon(QIcon(":/icons/alignleft.png"));
    m_alignCenterAction->setIcon(QIcon(":/icons/aligncenter.png"));
    m_alignRightAction->setIcon(QIcon(":/icons/alignright.png"));
    m_justifyAction->setIcon(QIcon(":/icons/justify.png"));
    m_bulletAction->setIcon(QIcon(":/icons/bullet.png"));
    m_numberAction->setIcon(QIcon(":/icons/numbered.png"));
    m_toolBar->addAction(m_newAction);
    m_toolBar->addAction(m_openAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addSeparator();
    QFontComboBox *fontCombo = new QFontComboBox();
    fontCombo->setCurrentFont(QFont("Noto Serif"));
    connect(fontCombo, QOverload<const QString &>::of(&QFontComboBox::currentTextChanged), this, &MonasteryFrame::onFontChanged);
    m_toolBar->addWidget(fontCombo);
    QComboBox *sizeCombo = new QComboBox();
    sizeCombo->addItems({"8", "10", "12", "14", "16", "18", "20", "24", "28", "32"});
    sizeCombo->setCurrentText("12");
    connect(sizeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this, &MonasteryFrame::onSizeChanged);
    m_toolBar->addWidget(sizeCombo);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_boldAction);
    m_toolBar->addAction(m_italicAction);
    m_toolBar->addAction(m_underlineAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_alignLeftAction);
    m_toolBar->addAction(m_alignCenterAction);
    m_toolBar->addAction(m_alignRightAction);
    m_toolBar->addAction(m_justifyAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_bulletAction);
    m_toolBar->addAction(m_numberAction);
    mainLayout->addWidget(m_toolBar);

    // editor
    m_editor = new MonasteryEditor(this);
    mainLayout->addWidget(m_editor, 1);

    // statusBar - dark leather + permanent word count
    m_statusBar = new QStatusBar;
    m_statusBar->setSizeGripEnabled(true);
    m_statusBar->setStyleSheet("background-color: #3C2F2F; color: #D4AF37;");
    m_statusBar->setFont(QFont("Noto Serif", 8));
    m_wordCountLabel = new QLabel("Words: 0");
    m_wordCountLabel->setAlignment(Qt::AlignRight);
    m_statusBar->addPermanentWidget(m_wordCountLabel);
    mainLayout->addWidget(m_statusBar);

    setLayout(mainLayout);

    // Global stylesheet for dark leather theme (QMessageBox + QFileDialog)
    qApp->setStyleSheet("QMessageBox { background-color: #3C2F2F; color: #D4AF37; }"
                        "QMessageBox QLabel { color: #D4AF37; font-weight: bold; }"
                        "QMessageBox QPushButton { background-color: #6F5A4A; color: #D4AF37; border: 1px solid #3C2F2F; padding: 5px; }"
                        "QMessageBox QPushButton:hover { background-color: #8B6F5A; }"
                        "QFileDialog { background-color: #3C2F2F; color: #D4AF37; }"
                        "QFileDialog QLabel, QFileDialog QLineEdit, QFileDialog QTreeView, QFileDialog QListView, QFileDialog QComboBox, QFileDialog QHeaderView::section { color: #D4AF37; background-color: #3C2F2F; }"
                        "QFileDialog QPushButton { background-color: #6F5A4A; color: #D4AF37; border: 1px solid #3C2F2F; padding: 4px 8px; }"
                        "QFileDialog QPushButton:hover { background-color: #8B6F5A; }"
                        "QMenu { background-color: #6F5A4A; color: #D4AF37; border: 1px solid #3C2F2F; }"
                        "QMenu::item:selected { background-color: #8B6F5A; color: #D4AF37; }");

    setWindowIcon(QIcon(":/icons/monastery.png"));

    setMinimumSize(680, 460);  // lets us shrink freely while keeping UI usable
    m_normalGeometry = geometry();

    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MonasteryFrame::onAutoSave);
    m_autoSaveTimer->start(30000);  // 30 seconds

    connect(m_editor->textEdit(), &QTextEdit::textChanged, this, &MonasteryFrame::updateWordCount);
    connect(m_undoAction, &QAction::triggered, m_editor->textEdit(), &QTextEdit::undo);
    connect(m_redoAction, &QAction::triggered, m_editor->textEdit(), &QTextEdit::redo);
    connect(m_cutAction, &QAction::triggered, m_editor->textEdit(), &QTextEdit::cut);
    connect(m_copyAction, &QAction::triggered, m_editor->textEdit(), &QTextEdit::copy);
    connect(m_pasteAction, &QAction::triggered, m_editor->textEdit(), &QTextEdit::paste);

    // Install event filter on child widgets for cursor updates
    m_titleBar->installEventFilter(this);
    m_menuBar->installEventFilter(this);
    m_toolBar->installEventFilter(this);
    m_editor->installEventFilter(this);
    m_statusBar->installEventFilter(this);

    m_statusBar->showMessage("Ready");
    updateWordCount();
}

MonasteryFrame::~MonasteryFrame() {
    // Qt handles cleanup
}

QString MonasteryFrame::getRealAppDir() {
    QByteArray appImage = qgetenv("APPIMAGE");
    if (!appImage.isEmpty()) {
        return QFileInfo(QString::fromUtf8(appImage)).absolutePath();
    } else {
        return QApplication::applicationDirPath();
    }
}

void MonasteryFrame::createDocsFolder() {
    m_docsDir = getRealAppDir() + "/Docs";
    QDir().mkpath(m_docsDir);
}

void MonasteryFrame::createActions() {
    m_newAction = new QAction("&New", this);
    m_newAction->setIcon(QIcon(":/icons/new.png"));
    m_newAction->setShortcut(QKeySequence::New);
    connect(m_newAction, &QAction::triggered, this, &MonasteryFrame::onNew);

    m_openAction = new QAction("&Open", this);
    m_openAction->setIcon(QIcon(":/icons/open.png"));
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MonasteryFrame::onOpen);

    m_saveAction = new QAction("&Save", this);
    m_saveAction->setIcon(QIcon(":/icons/save.png"));
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &MonasteryFrame::onSave);

    m_saveAsAction = new QAction("Save &As...", this);
    m_saveAsAction->setIcon(QIcon::fromTheme("document-save-as"));
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAction, &QAction::triggered, this, &MonasteryFrame::onSaveAs);

    m_printAction = new QAction("&Print", this);
    m_printAction->setShortcut(QKeySequence::Print);
    connect(m_printAction, &QAction::triggered, this, &MonasteryFrame::onPrint);

    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &MonasteryFrame::onExit);

    m_boldAction = new QAction(this);
    m_boldAction->setCheckable(true);
    m_boldAction->setToolTip("Bold");
    m_boldAction->setShortcut(QKeySequence("Ctrl+B"));
    connect(m_boldAction, &QAction::triggered, this, &MonasteryFrame::onBold);

    m_italicAction = new QAction(this);
    m_italicAction->setCheckable(true);
    m_italicAction->setToolTip("Italic");
    m_italicAction->setShortcut(QKeySequence("Ctrl+I"));
    connect(m_italicAction, &QAction::triggered, this, &MonasteryFrame::onItalic);

    m_underlineAction = new QAction(this);
    m_underlineAction->setCheckable(true);
    m_underlineAction->setToolTip("Underline");
    m_underlineAction->setShortcut(QKeySequence("Ctrl+U"));
    connect(m_underlineAction, &QAction::triggered, this, &MonasteryFrame::onUnderline);

    QActionGroup *alignGroup = new QActionGroup(this);

    m_alignLeftAction = new QAction(this);
    m_alignLeftAction->setCheckable(true);
    m_alignLeftAction->setToolTip("Align Left");
    connect(m_alignLeftAction, &QAction::triggered, this, &MonasteryFrame::onAlignLeft);
    alignGroup->addAction(m_alignLeftAction);

    m_alignCenterAction = new QAction(this);
    m_alignCenterAction->setCheckable(true);
    m_alignCenterAction->setToolTip("Align Center");
    connect(m_alignCenterAction, &QAction::triggered, this, &MonasteryFrame::onAlignCenter);
    alignGroup->addAction(m_alignCenterAction);

    m_alignRightAction = new QAction(this);
    m_alignRightAction->setCheckable(true);
    m_alignRightAction->setToolTip("Align Right");
    connect(m_alignRightAction, &QAction::triggered, this, &MonasteryFrame::onAlignRight);
    alignGroup->addAction(m_alignRightAction);

    m_justifyAction = new QAction(this);
    m_justifyAction->setCheckable(true);
    m_justifyAction->setToolTip("Justify");
    connect(m_justifyAction, &QAction::triggered, this, &MonasteryFrame::onJustify);
    alignGroup->addAction(m_justifyAction);

    m_bulletAction = new QAction(this);
    m_bulletAction->setToolTip("Bulleted List");
    connect(m_bulletAction, &QAction::triggered, this, &MonasteryFrame::onBulletList);

    m_numberAction = new QAction(this);
    m_numberAction->setToolTip("Numbered List");
    connect(m_numberAction, &QAction::triggered, this, &MonasteryFrame::onNumberedList);

    m_undoAction = new QAction("Undo", this);
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = new QAction("Redo", this);
    m_redoAction->setShortcut(QKeySequence::Redo);

    m_cutAction = new QAction("Cu&t", this);
    m_cutAction->setShortcut(QKeySequence::Cut);

    m_copyAction = new QAction("&Copy", this);
    m_copyAction->setShortcut(QKeySequence::Copy);

    m_pasteAction = new QAction("&Paste", this);
    m_pasteAction->setShortcut(QKeySequence::Paste);

    m_pageBreakAction = new QAction("Insert Page &Break", this);
    connect(m_pageBreakAction, &QAction::triggered, this, &MonasteryFrame::onInsertPageBreak);
}

void MonasteryFrame::createMenus() {
    // Menus are created in constructor
}

void MonasteryFrame::createToolBar() {
    // Toolbar is created in constructor
}

void MonasteryFrame::createStatusBar() {
    // Status bar is created in constructor
}

void MonasteryFrame::onNew() {
    if (m_editor->textEdit()->document()->isModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Unsaved Changes",
            "Document has unsaved changes. Save before creating new?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel) return;
        if (reply == QMessageBox::Yes) onSave();
    }
    m_editor->textEdit()->clear();
    m_editor->textEdit()->document()->setModified(false);
    m_currentFilePath.clear();
    m_statusBar->showMessage("New document created");
}

void MonasteryFrame::onOpen() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open HTML", m_docsDir, "HTML files (*.html)", nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString html = QString::fromUtf8(file.readAll());
        m_editor->textEdit()->setHtml(html);
        m_editor->textEdit()->document()->setModified(false);
        m_currentFilePath = fileName;
        m_statusBar->showMessage("File opened: " + fileName);
    }
}

void MonasteryFrame::onSave() {
    if (m_currentFilePath.isEmpty() || m_currentFilePath.contains("Monastery_AutoSave.html")) {
        QString fileName = QFileDialog::getSaveFileName(this, "Save HTML", m_docsDir, "HTML files (*.html)", nullptr, QFileDialog::DontUseNativeDialog);
        if (fileName.isEmpty()) return;
        if (!fileName.endsWith(".html")) fileName += ".html";
        m_currentFilePath = fileName;
    }
    QFile file(m_currentFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_editor->textEdit()->toHtml();
        m_editor->textEdit()->document()->setModified(false);
        m_statusBar->showMessage("Saved to " + m_currentFilePath);
    }
}

void MonasteryFrame::onExit() {
    QApplication::quit();
}

void MonasteryFrame::onAutoSave() {
    QString fileName = m_docsDir + "/Monastery_AutoSave.html";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_editor->textEdit()->toHtml();
        m_statusBar->showMessage("Saved to " + fileName);
    }
}

void MonasteryFrame::onPrint() {
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_editor->textEdit()->print(&printer);
    }
}

void MonasteryFrame::onInsertPageBreak() {
    QTextCursor cursor = m_editor->textEdit()->textCursor();
    cursor.insertHtml("<hr style=\"border: 1px dashed #666;\">");
    m_editor->textEdit()->setTextCursor(cursor);
}

void MonasteryFrame::onBold() {
    QTextCharFormat fmt;
    fmt.setFontWeight(m_boldAction->isChecked() ? QFont::Bold : QFont::Normal);
    m_editor->textEdit()->mergeCurrentCharFormat(fmt);
}

void MonasteryFrame::onItalic() {
    QTextCharFormat fmt;
    fmt.setFontItalic(m_italicAction->isChecked());
    m_editor->textEdit()->mergeCurrentCharFormat(fmt);
}

void MonasteryFrame::onUnderline() {
    QTextCharFormat fmt;
    fmt.setFontUnderline(m_underlineAction->isChecked());
    m_editor->textEdit()->mergeCurrentCharFormat(fmt);
}

void MonasteryFrame::onAlignLeft() {
    m_editor->textEdit()->setAlignment(Qt::AlignLeft);
}

void MonasteryFrame::onAlignCenter() {
    m_editor->textEdit()->setAlignment(Qt::AlignCenter);
}

void MonasteryFrame::onAlignRight() {
    m_editor->textEdit()->setAlignment(Qt::AlignRight);
}

void MonasteryFrame::onJustify() {
    m_editor->textEdit()->setAlignment(Qt::AlignJustify);
}

void MonasteryFrame::onBulletList() {
    m_editor->textEdit()->textCursor().insertList(QTextListFormat::ListDisc);
}

void MonasteryFrame::onNumberedList() {
    m_editor->textEdit()->textCursor().insertList(QTextListFormat::ListDecimal);
}

void MonasteryFrame::onFontChanged(const QString &font) {
    QFont currentFont = m_editor->textEdit()->currentFont();
    currentFont.setFamily(font);
    m_editor->textEdit()->setCurrentFont(currentFont);
}

void MonasteryFrame::onSizeChanged(const QString &size) {
    bool ok;
    int pointSize = size.toInt(&ok);
    if (ok) {
        QTextCharFormat format = m_editor->textEdit()->currentCharFormat();
        format.setFontPointSize(pointSize);
        m_editor->textEdit()->mergeCurrentCharFormat(format);
    }
}

void MonasteryFrame::updateWordCount() {
    QString text = m_editor->textEdit()->toPlainText();
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    m_wordCountLabel->setText(QString("Words: %1").arg(words.count()));
}

void MonasteryFrame::closeEvent(QCloseEvent *event) {
    if (m_editor->textEdit()->document()->isModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Unsaved Changes",
            "Document has unsaved changes. Save before exiting?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (reply == QMessageBox::Yes) onSave();
    }
    event->accept();
}

void MonasteryFrame::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Check for resize areas first (8-pixel border)
        const int border = 8;
        QRect rect = this->rect();
        QPoint pos = event->pos();

        if (pos.x() <= border && pos.y() <= border) {
            m_resizeDirection = TopLeft;
        } else if (pos.x() >= rect.width() - border && pos.y() <= border) {
            m_resizeDirection = TopRight;
        } else if (pos.x() <= border && pos.y() >= rect.height() - border) {
            m_resizeDirection = BottomLeft;
        } else if (pos.x() >= rect.width() - border && pos.y() >= rect.height() - border) {
            m_resizeDirection = BottomRight;
        } else if (pos.x() <= border) {
            m_resizeDirection = Left;
        } else if (pos.x() >= rect.width() - border) {
            m_resizeDirection = Right;
        } else if (pos.y() <= border) {
            m_resizeDirection = Top;
        } else if (pos.y() >= rect.height() - border) {
            m_resizeDirection = Bottom;
        } else {
            m_resizeDirection = None;
        }

        if (m_resizeDirection != None) {
            m_resizing = true;
            m_resizeStartPos = this->pos();  // Window position when resize started
            m_resizeStartSize = size();  // Window size when resize started
            m_resizeStartMousePos = event->globalPosition().toPoint();  // Mouse position when resize started
            event->accept();
        } else if (m_titleBar->geometry().contains(event->pos())) {
            // Only start dragging if not in resize area and in title bar
            m_dragging = true;
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        } else {
            QWidget::mousePressEvent(event);
        }
    } else {
        QWidget::mousePressEvent(event);
    }
}

void MonasteryFrame::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    } else if (m_resizing) {
        QPoint delta = event->globalPosition().toPoint() - m_resizeStartMousePos;
        QPoint newPos = m_resizeStartPos;
        QSize newSize = m_resizeStartSize;

        switch (m_resizeDirection) {
            case Left:
                newPos.setX(m_resizeStartPos.x() + delta.x());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() - delta.x()));
                break;
            case Right:
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() + delta.x()));
                break;
            case Top:
                newPos.setY(m_resizeStartPos.y() + delta.y());
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() - delta.y()));
                break;
            case Bottom:
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() + delta.y()));
                break;
            case TopLeft:
                newPos.setX(m_resizeStartPos.x() + delta.x());
                newPos.setY(m_resizeStartPos.y() + delta.y());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() - delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() - delta.y()));
                break;
            case TopRight:
                newPos.setY(m_resizeStartPos.y() + delta.y());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() + delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() - delta.y()));
                break;
            case BottomLeft:
                newPos.setX(m_resizeStartPos.x() + delta.x());
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() - delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() + delta.y()));
                break;
            case BottomRight:
                newSize.setWidth(std::max(minimumWidth(), m_resizeStartSize.width() + delta.x()));
                newSize.setHeight(std::max(minimumHeight(), m_resizeStartSize.height() + delta.y()));
                break;
            default:
                break;
        }

        setGeometry(QRect(newPos, newSize));
        event->accept();
    } else {
        // Update cursor based on position
        const int border = 8;
        QRect rect = this->rect();
        QPoint pos = event->pos();

        if (pos.x() <= border && pos.y() <= border) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (pos.x() >= rect.width() - border && pos.y() <= border) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (pos.x() <= border && pos.y() >= rect.height() - border) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (pos.x() >= rect.width() - border && pos.y() >= rect.height() - border) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (pos.x() <= border || pos.x() >= rect.width() - border) {
            setCursor(Qt::SizeHorCursor);
        } else if (pos.y() <= border || pos.y() >= rect.height() - border) {
            setCursor(Qt::SizeVerCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }

        QWidget::mouseMoveEvent(event);
    }
}

void MonasteryFrame::mouseReleaseEvent(QMouseEvent *event) {
    m_dragging = false;
    m_resizing = false;
    m_resizeDirection = None;
    QWidget::mouseReleaseEvent(event);
}

void MonasteryFrame::mouseDoubleClickEvent(QMouseEvent *event) {
    if (m_titleBar->geometry().contains(event->pos())) {
        if (isMaximized()) {
            showNormal();
            setGeometry(m_normalGeometry);
        } else {
            m_normalGeometry = geometry();
            showMaximized();
        }
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void MonasteryFrame::resizeEvent(QResizeEvent *event) {
    m_titleBar->setFixedWidth(width());
    QWidget::resizeEvent(event);
}

bool MonasteryFrame::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        // Update cursor based on global position relative to main window
        QPoint globalPos = mouseEvent->globalPosition().toPoint();
        QPoint localPos = mapFromGlobal(globalPos);

        const int border = 8;
        QRect rect = this->rect();

        if (localPos.x() >= 0 && localPos.x() < rect.width() &&
            localPos.y() >= 0 && localPos.y() < rect.height()) {
            if (localPos.x() <= border && localPos.y() <= border) {
                setCursor(Qt::SizeFDiagCursor);
            } else if (localPos.x() >= rect.width() - border && localPos.y() <= border) {
                setCursor(Qt::SizeBDiagCursor);
            } else if (localPos.x() <= border && localPos.y() >= rect.height() - border) {
                setCursor(Qt::SizeBDiagCursor);
            } else if (localPos.x() >= rect.width() - border && localPos.y() >= rect.height() - border) {
                setCursor(Qt::SizeFDiagCursor);
            } else if (localPos.x() <= border || localPos.x() >= rect.width() - border) {
                setCursor(Qt::SizeHorCursor);
            } else if (localPos.y() <= border || localPos.y() >= rect.height() - border) {
                setCursor(Qt::SizeVerCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MonasteryFrame::onSaveAs() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save HTML", m_docsDir, "HTML files (*.html)", nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".html")) fileName += ".html";
    m_currentFilePath = fileName;
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_editor->textEdit()->toHtml();
        m_editor->textEdit()->document()->setModified(false);
        m_statusBar->showMessage("Saved to " + fileName);
    }
}

QIcon MonasteryFrame::createToolbarIcon(const QString &symbol) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setPen(QColor(40, 40, 40));
    p.setFont(QFont("Noto Sans", 11, QFont::Bold));
    p.drawText(pix.rect(), Qt::AlignCenter, symbol);
    return QIcon(pix);
}