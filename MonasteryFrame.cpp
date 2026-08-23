#include "MonasteryFrame.h"
#include "MonasteryEditor.h"
#include "Theme.h"
#include <QApplication>
#include <algorithm>
#include <memory>
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
#include <QPrinter>
#include <QPrintDialog>
#include <QPageLayout>
#include <QPageSize>
#include <QWebEnginePage>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QPainter>
#include <QEventLoop>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QStringConverter>
#include <QSettings>
#include <QSignalBlocker>
#include <QColor>
#include <QFont>

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

MonasteryFrame::MonasteryFrame(QWidget *parent) : QWidget(parent), m_currentTheme(themeForId(ThemeId::Leather)) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    setStyleSheet("QWidget { background-color: #3C2F2F; }");  // Match title bar color for borders
    setFont(QFont("Noto Serif", 12));
    setGeometry(80, 40, 1100, 850);

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

    m_minBtn = new QPushButton("—");
    m_minBtn->setFixedSize(30,30);
    m_minBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(m_minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    m_maxBtn = new QPushButton("□");
    m_maxBtn->setFixedSize(30,30);
    m_maxBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(m_maxBtn, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
            setGeometry(m_normalGeometry);
        } else {
            m_normalGeometry = geometry();
            showMaximized();
        }
    });
    m_closeBtn = new QPushButton("×");
    m_closeBtn->setFixedSize(30,30);
    m_closeBtn->setStyleSheet("border: none; background: transparent; color: white;");
    connect(m_closeBtn, &QPushButton::clicked, this, &QWidget::close);

    m_titleLabel = new QLabel("Monastery — Untitled");
    QFont titleFont("Noto Serif", 10, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #D4AF37;");   // gold contrast like Aureus

    QWidget *leftSpacer = new QWidget();
    leftSpacer->setFixedWidth(90);

    titleLayout->addWidget(leftSpacer);
    titleLayout->addStretch();
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_minBtn);
    titleLayout->addWidget(m_maxBtn);
    titleLayout->addWidget(m_closeBtn);
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
    editMenu->addAction(m_findAction);
    editMenu->addSeparator();
    editMenu->addAction(m_pageBreakAction);
    editMenu->addSeparator();
    editMenu->addAction(m_narrowMarginsAction);
    QMenu *viewMenu = m_menuBar->addMenu("&View");
    viewMenu->addAction(m_focusModeAction);

    QMenu *themeMenu = m_menuBar->addMenu("&Theme");
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->setExclusive(true);
    for (const Theme &th : allThemes()) {
        QAction *action = themeMenu->addAction(th.name);
        action->setCheckable(true);
        action->setData(th.id);
        m_themeGroup->addAction(action);
        const ThemeId id = th.themeId;
        connect(action, &QAction::triggered, this, [this, id](bool checked) {
            if (checked)
                applyTheme(id);
        });
    }

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
    connect(m_editor, &MonasteryEditor::wordCountChanged, this, [this](int count) {
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
    });
    connect(m_editor, &MonasteryEditor::dirtyChanged, this, [this](bool) {
        updateTitleBar();
    });
    connect(m_editor, &MonasteryEditor::ready, this, [this](bool ok) {
        if (ok) {
            applyTheme(m_currentTheme.themeId);
            QTimer::singleShot(150, this, &MonasteryFrame::maybeRestoreAutosave);
        }
    });
    connect(m_editor->webView()->page(), &QWebEnginePage::pdfPrintingFinished,
            this, [this](const QString &path, bool success) {
        if (!success)
            QMessageBox::warning(this, "PDF Export Failed",
                                 "Could not write the PDF:\n" + path);
        else
            m_statusBar->showMessage("Exported PDF: " + path);
    });

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
                        "QMenu::item:selected { background-color: #8B6F5A; color: #D4AF37; }"
                        "QDialog { background-color: #3C2F2F; color: #D4AF37; }"
                        "QDialog QLabel { color: #D4AF37; }"
                        "QLineEdit { background-color: #5C4A3F; color: #F5E8C7; border: 1px solid #8B7355; padding: 4px; }"
                        "QDialog QPushButton { background-color: #6F5A4A; color: #D4AF37; border: 1px solid #3C2F2F; padding: 5px; }");

    setWindowIcon(QIcon(":/icons/monastery.png"));

    setMinimumSize(680, 460);  // lets us shrink freely while keeping UI usable
    m_normalGeometry = geometry();

    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MonasteryFrame::onAutoSave);
    m_autoSaveTimer->start(30000);  // 30 seconds

    // Route everything through the web editor
    connect(m_undoAction, &QAction::triggered, this, [this]() { m_editor->execCommand("undo"); });
    connect(m_redoAction, &QAction::triggered, this, [this]() { m_editor->execCommand("redo"); });
    connect(m_cutAction,   &QAction::triggered, this, [this]() { m_editor->execCommand("cut"); });
    connect(m_copyAction,  &QAction::triggered, this, [this]() { m_editor->execCommand("copy"); });
    connect(m_pasteAction, &QAction::triggered, this, [this]() { m_editor->execCommand("paste"); });

    // Label refresh is callback-only; the editor polls JS without QEventLoop.
    m_wordCountPollTimer = new QTimer(this);
    m_wordCountPollTimer->setInterval(900);
    connect(m_wordCountPollTimer, &QTimer::timeout, this, &MonasteryFrame::updateWordCount);
    m_wordCountPollTimer->start();

    // Install event filter on child widgets for cursor updates
    m_titleBar->installEventFilter(this);
    m_menuBar->installEventFilter(this);
    m_toolBar->installEventFilter(this);
    m_statusBar->installEventFilter(this);

    m_statusBar->showMessage("Ready");
    updateWordCount();

    QSettings settings(QStringLiteral("Monastery"), QStringLiteral("Monastery"));
    applyTheme(themeIdFromString(settings.value(QStringLiteral("theme"), QStringLiteral("leather")).toString()));
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

    m_narrowMarginsAction = new QAction("Narrow Margins", this);
    m_narrowMarginsAction->setCheckable(true);
    m_narrowMarginsAction->setChecked(false);
    connect(m_narrowMarginsAction, &QAction::triggered, this, &MonasteryFrame::onToggleNarrowMargins);

    m_findAction = new QAction("&Find...", this);
    m_findAction->setShortcut(QKeySequence::Find);
    m_findAction->setShortcutContext(Qt::ApplicationShortcut);
    connect(m_findAction, &QAction::triggered, this, &MonasteryFrame::onFind);
    addAction(m_findAction);

    m_focusModeAction = new QAction("&Focus Mode", this);
    m_focusModeAction->setCheckable(true);
    m_focusModeAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_focusModeAction->setShortcutContext(Qt::ApplicationShortcut);
    connect(m_focusModeAction, &QAction::triggered, this, &MonasteryFrame::onToggleFocusMode);
    addAction(m_focusModeAction);
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
    if (!confirmProceedIfDirty())
        return;
    m_editor->setHtml("<p></p>");
    m_currentFilePath.clear();
    m_editor->markClean();
    updateTitleBar();
    m_statusBar->showMessage("New document created");
    updateWordCount();
}

void MonasteryFrame::onOpen() {
    if (!confirmProceedIfDirty())
        return;

    QString fileName = QFileDialog::getOpenFileName(this, "Open HTML", m_docsDir, "HTML files (*.html)", nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString html = QString::fromUtf8(file.readAll());
        m_editor->setHtml(html);
        m_currentFilePath = fileName;
        m_editor->markClean();
        updateTitleBar();
        m_statusBar->showMessage("File opened: " + fileName);
        updateWordCount();
    } else {
        QMessageBox::warning(this, "Open Failed",
                             "Could not read:\n" + fileName + "\n" + file.errorString());
    }
}

void MonasteryFrame::onSave() {
    saveNow();
}

void MonasteryFrame::onExit() {
    close();
}

void MonasteryFrame::onAutoSave() {
    m_editor->fetchHtml([this](const QString &html) {
        if (wouldClobberManuscript(html)) {
            m_statusBar->showMessage("Autosave skipped — empty or incomplete editor content");
            return;
        }
        const QString fileName = hasNamedDocument()
            ? autosaveSidecarPath()
            : (m_docsDir + "/Monastery_AutoSave.html");
        if (writeHtmlFile(fileName, html))
            m_statusBar->showMessage("Auto-saved to " + fileName);
    });
}

void MonasteryFrame::onPrint() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export PDF", m_docsDir, "PDF files (*.pdf)", nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf")) fileName += ".pdf";

    QPageLayout layout(QPageSize(QPageSize::Letter),
                       QPageLayout::Portrait,
                       QMarginsF(0.75, 0.75, 0.75, 0.75),
                       QPageLayout::Inch);
    m_editor->webView()->page()->printToPdf(fileName, layout);
    m_statusBar->showMessage("Exporting to PDF: " + fileName);
}

void MonasteryFrame::onInsertPageBreak() {
    m_editor->execCommand("insertHTML",
        "<div class=\"page-break\" style=\"page-break-after: always; border: none; border-top: 1px dashed #8B7355; margin: 30px 0;\"></div>");
}

void MonasteryFrame::onBold() { m_editor->execCommand("bold"); }

void MonasteryFrame::onItalic()        { m_editor->execCommand("italic"); }
void MonasteryFrame::onUnderline()     { m_editor->execCommand("underline"); }

void MonasteryFrame::onAlignLeft()   { m_editor->execCommand("justifyLeft"); }
void MonasteryFrame::onAlignCenter() { m_editor->execCommand("justifyCenter"); }
void MonasteryFrame::onAlignRight()  { m_editor->execCommand("justifyRight"); }
void MonasteryFrame::onJustify()     { m_editor->execCommand("justifyFull"); }

void MonasteryFrame::onBulletList()    { m_editor->execCommand("insertUnorderedList"); }
void MonasteryFrame::onNumberedList()  { m_editor->execCommand("insertOrderedList"); }

void MonasteryFrame::onFontChanged(const QString &font) {
    m_editor->execCommand("fontName", font);
}

void MonasteryFrame::onSizeChanged(const QString &size) {
    bool ok = false;
    const int pt = size.toInt(&ok);
    if (!ok || pt <= 0)
        return;
    m_editor->applyFontSize(pt);
}

void MonasteryFrame::updateWordCount() {
    m_editor->requestWordCount([this](int count) {
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
    });
}

void MonasteryFrame::closeEvent(QCloseEvent *event) {
    if (!confirmProceedIfDirty()) {
        event->ignore();
        return;
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
    if (saveNow())
        updateTitleBar();
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

void MonasteryFrame::onToggleNarrowMargins() {
    m_narrowMargins = m_narrowMarginsAction->isChecked();

    // Change padding on the .page element live in the web view
    QString padding;
    if (m_currentTheme.pageAsObject)
        padding = m_narrowMargins ? "0.5in 0.5in" : "1in 0.85in";
    else
        padding = m_narrowMargins ? "0.5in 0.5in" : "0.75in";

    QString js = QString("var p = document.querySelector('.page'); if (p) p.style.padding = '%1';").arg(padding);
    m_editor->webView()->page()->runJavaScript(js);

    m_statusBar->showMessage(m_narrowMargins ? "Narrow margins enabled" : "Standard margins");
}

bool MonasteryFrame::hasNamedDocument() const {
    return !m_currentFilePath.isEmpty() && !m_currentFilePath.contains("Monastery_AutoSave.html");
}

QString MonasteryFrame::documentDisplayName() const {
    if (!hasNamedDocument())
        return QStringLiteral("Untitled");
    return QFileInfo(m_currentFilePath).fileName();
}

QString MonasteryFrame::autosaveSidecarPath() const {
    const QFileInfo info(m_currentFilePath);
    return info.absolutePath() + "/" + info.completeBaseName() + "_autosave.html";
}

void MonasteryFrame::updateTitleBar() {
    const bool dirty = m_editor && m_editor->isDirty();
    QString title = QStringLiteral("Monastery — ") + documentDisplayName();
    if (dirty)
        title += QStringLiteral(" *");
    if (m_titleLabel)
        m_titleLabel->setText(title);
    setWindowTitle(title);
}

bool MonasteryFrame::htmlLooksEmpty(const QString &html) const {
    QString t = html;
    t.replace(QRegularExpression("<[^>]+>"), QStringLiteral(" "));
    t.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    t.replace(QStringLiteral("&#160;"), QStringLiteral(" "));
    return t.trimmed().isEmpty();
}

bool MonasteryFrame::wouldClobberManuscript(const QString &incoming) const {
    if (htmlLooksEmpty(incoming))
        return true;
    const QString lastGood = m_editor ? m_editor->lastGoodHtml() : QString();
    if (lastGood.size() > 200 && incoming.trimmed().size() * 10 < lastGood.size())
        return true;
    return false;
}

bool MonasteryFrame::writeHtmlFile(const QString &path, const QString &html) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not write:\n" + path + "\n" + file.errorString());
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << html;
    out.flush();
    file.close();
    if (file.error() != QFile::NoError) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not finish writing:\n" + path + "\n" + file.errorString());
        return false;
    }
    return true;
}

bool MonasteryFrame::persistDocument(const QString &path, const QString &html, bool markCleanAfter) {
    if (wouldClobberManuscript(html)) {
        m_statusBar->showMessage("Save skipped — empty or incomplete editor content. Last good copy kept.");
        return false;
    }
    if (!writeHtmlFile(path, html))
        return false;
    m_statusBar->showMessage("Saved to " + path);
    if (markCleanAfter) {
        m_editor->markClean();
        updateTitleBar();
    }
    return true;
}

bool MonasteryFrame::ensureSavePath() {
    if (hasNamedDocument())
        return true;
    QString fileName = QFileDialog::getSaveFileName(this, "Save HTML", m_docsDir, "HTML files (*.html)", nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return false;
    if (!fileName.endsWith(".html"))
        fileName += ".html";
    m_currentFilePath = fileName;
    updateTitleBar();
    return true;
}

bool MonasteryFrame::saveNow() {
    if (!ensureSavePath())
        return false;

    auto done = std::make_shared<bool>(false);
    auto ok = std::make_shared<bool>(false);
    m_editor->fetchHtml([this, done, ok](const QString &html) {
        *ok = persistDocument(m_currentFilePath, html, true);
        *done = true;
    });
    if (!*done) {
        QEventLoop loop;
        QTimer pump;
        pump.setInterval(15);
        QObject::connect(&pump, &QTimer::timeout, [&]() {
            if (*done)
                loop.quit();
        });
        pump.start();
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();
    }
    if (!*done)
        *ok = persistDocument(m_currentFilePath, m_editor->lastGoodHtml(), true);
    return *ok;
}

bool MonasteryFrame::confirmProceedIfDirty() {
    const bool dirty = m_editor->isDirty() || m_editor->queryDirtyNow();
    if (!dirty)
        return true;

    const QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Unsaved Changes",
        "Document has unsaved changes. Save before continuing?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return false;
    if (reply == QMessageBox::Yes)
        return saveNow();
    return true;
}

void MonasteryFrame::maybeRestoreAutosave() {
    if (m_didOfferRestore)
        return;
    m_didOfferRestore = true;

    const QString path = m_docsDir + "/Monastery_AutoSave.html";
    QFile file(path);
    if (!file.exists())
        return;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    const QString html = QString::fromUtf8(file.readAll());
    if (htmlLooksEmpty(html))
        return;

    const auto reply = QMessageBox::question(
        this, "Restore Autosave",
        "An autosaved document was found. Restore it?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;

    m_editor->setHtml(html);
    m_currentFilePath.clear();
    m_editor->markDirty();
    updateTitleBar();
    m_statusBar->showMessage("Restored autosave");
    updateWordCount();
}

void MonasteryFrame::setupFindDialog() {
    if (m_findDialog)
        return;

    m_findDialog = new QDialog(this);
    m_findDialog->setWindowTitle("Find");
    m_findDialog->setModal(false);
    QVBoxLayout *layout = new QVBoxLayout(m_findDialog);
    QLabel *label = new QLabel("Find:");
    m_findEdit = new QLineEdit(m_findDialog);
    layout->addWidget(label);
    layout->addWidget(m_findEdit);

    QHBoxLayout *buttons = new QHBoxLayout;
    QPushButton *nextBtn = new QPushButton("Next");
    QPushButton *prevBtn = new QPushButton("Previous");
    QPushButton *closeBtn = new QPushButton("Close");
    buttons->addWidget(prevBtn);
    buttons->addWidget(nextBtn);
    buttons->addWidget(closeBtn);
    layout->addLayout(buttons);

    connect(nextBtn, &QPushButton::clicked, this, &MonasteryFrame::onFindNext);
    connect(prevBtn, &QPushButton::clicked, this, &MonasteryFrame::onFindPrevious);
    connect(closeBtn, &QPushButton::clicked, m_findDialog, &QDialog::hide);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &MonasteryFrame::onFindNext);
}

void MonasteryFrame::runFind(bool backward) {
    if (!m_findEdit)
        return;
    const QString needle = m_findEdit->text();
    if (needle.isEmpty())
        return;
    QWebEnginePage::FindFlags flags{};
    if (backward)
        flags |= QWebEnginePage::FindBackward;
    m_editor->webView()->page()->findText(needle, flags);
}

void MonasteryFrame::onFind() {
    setupFindDialog();
    m_findDialog->show();
    m_findDialog->raise();
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void MonasteryFrame::onFindNext() {
    runFind(false);
}

void MonasteryFrame::onFindPrevious() {
    runFind(true);
}

void MonasteryFrame::onToggleFocusMode() {
    m_focusMode = m_focusModeAction->isChecked();
    if (m_menuBar)
        m_menuBar->setVisible(!m_focusMode);
    if (m_toolBar)
        m_toolBar->setVisible(!m_focusMode);
    if (m_statusBar)
        m_statusBar->setVisible(!m_focusMode);
}


void MonasteryFrame::applyUiFont(const Theme &theme)
{
    QFont ui;
    if (theme.themeId == ThemeId::WordPerfect)
        ui.setFamilies({"IBM Plex Mono", "Fixed", "Courier New", "DejaVu Sans Mono", "sans-serif"});
    else if (theme.themeId == ThemeId::Leather)
        ui.setFamilies({"Noto Serif", "Georgia", "serif"});
    else
        ui.setFamilies({"Courier New", "Liberation Mono", "DejaVu Sans Mono", "monospace"});
    ui.setPointSize(10);
    setFont(ui);
    if (m_titleLabel) {
        QFont title = ui;
        title.setPointSize(10);
        title.setBold(true);
        m_titleLabel->setFont(title);
    }
    if (m_statusBar) {
        QFont status = ui;
        status.setPointSize(8);
        m_statusBar->setFont(status);
    }
    if (m_wordCountLabel)
        m_wordCountLabel->setFont(m_statusBar ? m_statusBar->font() : ui);
}

void MonasteryFrame::colorizeToolbarIcons(const Theme &theme)
{
    const QColor tint(theme.accent);
    auto tinted = [tint](const QString &path) {
        QPixmap src(path);
        if (src.isNull())
            return QIcon();
        QPixmap dest(src.size());
        dest.fill(Qt::transparent);
        QPainter p(&dest);
        p.drawPixmap(0, 0, src);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(dest.rect(), tint);
        p.end();
        return QIcon(dest);
    };

    m_newAction->setIcon(tinted(QStringLiteral(":/icons/new.png")));
    m_openAction->setIcon(tinted(QStringLiteral(":/icons/open.png")));
    m_saveAction->setIcon(tinted(QStringLiteral(":/icons/save.png")));
    m_boldAction->setIcon(tinted(QStringLiteral(":/icons/bold.png")));
    m_italicAction->setIcon(tinted(QStringLiteral(":/icons/italic.png")));
    m_underlineAction->setIcon(tinted(QStringLiteral(":/icons/underline.png")));
    m_alignLeftAction->setIcon(tinted(QStringLiteral(":/icons/alignleft.png")));
    m_alignCenterAction->setIcon(tinted(QStringLiteral(":/icons/aligncenter.png")));
    m_alignRightAction->setIcon(tinted(QStringLiteral(":/icons/alignright.png")));
    m_justifyAction->setIcon(tinted(QStringLiteral(":/icons/justify.png")));
    m_bulletAction->setIcon(tinted(QStringLiteral(":/icons/bullet.png")));
    m_numberAction->setIcon(tinted(QStringLiteral(":/icons/numbered.png")));
}

void MonasteryFrame::applyTheme(ThemeId id)
{
    const Theme t = themeForId(id);
    m_currentTheme = t;
    applyUiFont(t);

    const QString chromeFg = t.pageAsObject ? t.pageBg : t.textOnChrome;
    const QString btnCss = QStringLiteral("border: none; background: transparent; color: %1;").arg(t.textOnChrome);

    setStyleSheet(QStringLiteral("QWidget { background-color: %1; }").arg(t.chromeBg));

    if (m_titleBar)
        m_titleBar->setStyleSheet(QStringLiteral("background-color: %1;").arg(t.chromeBg));
    if (m_titleLabel)
        m_titleLabel->setStyleSheet(QStringLiteral("color: %1;").arg(t.textOnChrome));
    if (m_minBtn)
        m_minBtn->setStyleSheet(btnCss);
    if (m_maxBtn)
        m_maxBtn->setStyleSheet(btnCss);
    if (m_closeBtn)
        m_closeBtn->setStyleSheet(btnCss);

    if (m_menuBar) {
        m_menuBar->setStyleSheet(QStringLiteral(
            "QMenuBar { background-color: %1; color: %2; }"
            "QMenuBar::item { background-color: transparent; color: %2; padding: 4px 8px; }"
            "QMenuBar::item:selected { background-color: %3; color: %4; }"
            "QMenu { background-color: %5; color: %6; border: 1px solid %7; }"
            "QMenu::item { background-color: transparent; color: %6; }"
            "QMenu::item:selected { background-color: %3; color: %4; }"
            "QMenu::separator { height: 1px; background: %7; }")
            .arg(t.menuBarBg, t.menuBarText, t.menuSelectedBg, t.menuSelectedFg,
                 t.themeId == ThemeId::WordPerfect ? t.menuBarBg : t.chromeBg,
                 t.themeId == ThemeId::WordPerfect ? t.menuBarText : t.textOnChrome,
                 t.chromeLo));
    }

    if (m_toolBar) {
        m_toolBar->setStyleSheet(QStringLiteral(
            "QToolBar {"
            "  background-color: %1;"
            "  border-left: 8px solid %2;"
            "  border-right: 8px solid %2;"
            "  border-top: 0;"
            "  border-bottom: 0;"
            "  padding: 4px 0;"
            "}"
            "QToolButton { background-color: transparent; border: none; padding: 2px; }"
            "QToolButton:hover { background-color: %3; border-radius: 2px; }"
            "QToolButton:pressed { background-color: %4; }"
            "QComboBox { background-color: %4; color: %5; border: 1px solid %3; border-radius: 2px; padding: 2px; min-width: 60px; }"
            "QComboBox:hover { background-color: %3; }"
            "QComboBox::drop-down { border: none; background-color: %4; }"
            "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 4px solid %5; margin-right: 4px; }"
            "QComboBox QAbstractItemView { background-color: %2; color: %5; border: 1px solid %4; selection-background-color: %3; selection-color: %5; }")
            .arg(t.chromeMid, t.chromeBg, t.chromeHi, t.chromeLo, chromeFg));
    }

    if (m_statusBar)
        m_statusBar->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(t.chromeBg, t.textOnChrome));
    if (m_wordCountLabel)
        m_wordCountLabel->setStyleSheet(QStringLiteral("color: %1;").arg(t.textOnChrome));

    const QString menuBg = (t.themeId == ThemeId::WordPerfect) ? t.menuBarBg : t.chromeMid;
    const QString menuFg = (t.themeId == ThemeId::WordPerfect) ? t.menuBarText : t.textOnChrome;
    QString dialogCss = QStringLiteral(
        "QMessageBox { background-color: __BG__; color: __FG__; }"
        "QMessageBox QLabel { color: __FG__; font-weight: bold; }"
        "QMessageBox QPushButton { background-color: __MID__; color: __FG__; border: 1px solid __BG__; padding: 5px; }"
        "QMessageBox QPushButton:hover { background-color: __HI__; }"
        "QFileDialog { background-color: __BG__; color: __FG__; }"
        "QFileDialog QLabel, QFileDialog QLineEdit, QFileDialog QTreeView, QFileDialog QListView, QFileDialog QComboBox, QFileDialog QHeaderView::section { color: __FG__; background-color: __BG__; }"
        "QFileDialog QPushButton { background-color: __MID__; color: __FG__; border: 1px solid __BG__; padding: 4px 8px; }"
        "QFileDialog QPushButton:hover { background-color: __HI__; }"
        "QMenu { background-color: __MENUBG__; color: __MENUFG__; border: 1px solid __BG__; }"
        "QMenu::item:selected { background-color: __SELBG__; color: __SELFG__; }"
        "QDialog { background-color: __BG__; color: __FG__; }"
        "QDialog QLabel { color: __FG__; }"
        "QLineEdit { background-color: __LO__; color: __CHROMEFG__; border: 1px solid __HI__; padding: 4px; }"
        "QDialog QPushButton { background-color: __MID__; color: __FG__; border: 1px solid __BG__; padding: 5px; }");
    dialogCss.replace(QStringLiteral("__BG__"), t.chromeBg);
    dialogCss.replace(QStringLiteral("__FG__"), t.textOnChrome);
    dialogCss.replace(QStringLiteral("__MID__"), t.chromeMid);
    dialogCss.replace(QStringLiteral("__HI__"), t.chromeHi);
    dialogCss.replace(QStringLiteral("__LO__"), t.chromeLo);
    dialogCss.replace(QStringLiteral("__MENUBG__"), menuBg);
    dialogCss.replace(QStringLiteral("__MENUFG__"), menuFg);
    dialogCss.replace(QStringLiteral("__SELBG__"), t.menuSelectedBg);
    dialogCss.replace(QStringLiteral("__SELFG__"), t.menuSelectedFg);
    dialogCss.replace(QStringLiteral("__CHROMEFG__"), chromeFg);
    qApp->setStyleSheet(dialogCss);

    colorizeToolbarIcons(t);

    if (m_themeGroup) {
        const QSignalBlocker blocker(m_themeGroup);
        for (QAction *action : m_themeGroup->actions())
            action->setChecked(action->data().toString() == t.id);
    }

    if (m_editor)
        m_editor->applyTheme(t);

    if (m_editor && m_narrowMargins) {
        const QString padding = QStringLiteral("0.5in 0.5in");
        m_editor->webView()->page()->runJavaScript(
            QStringLiteral("var p = document.querySelector('.page'); if (p) p.style.padding = '%1';").arg(padding));
    }

    QSettings settings(QStringLiteral("Monastery"), QStringLiteral("Monastery"));
    settings.setValue(QStringLiteral("theme"), t.id);
}
