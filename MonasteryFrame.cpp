#include "MonasteryFrame.h"
#include "MonasteryEditor.h"
#include "Theme.h"
#include "DocumentIo.h"
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
#include <QPrinterInfo>
#include <QPageLayout>
#include <QPageSize>
#include <QMarginsF>
#include <QProcess>
#include <QTemporaryFile>
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
#include <QCoreApplication>
#include <QList>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QDialogButtonBox>
#include <cstdio>
#include <QElapsedTimer>

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


static const QString kDocumentFilter =
    QStringLiteral("Documents (*.html *.md *.markdown *.txt *.docx);;HTML (*.html);;Markdown (*.md *.markdown *.txt);;Word (*.docx)");

static QString ensureDocumentSuffix(QString fileName)
{
    const QString lower = fileName.toLower();
    if (lower.endsWith(QLatin1String(".html"))
        || lower.endsWith(QLatin1String(".md"))
        || lower.endsWith(QLatin1String(".markdown"))
        || lower.endsWith(QLatin1String(".txt"))
        || lower.endsWith(QLatin1String(".docx")))
        return fileName;
    return fileName + QStringLiteral(".html");
}

class MarkdownHighlighter : public QSyntaxHighlighter {
public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr)
        : QSyntaxHighlighter(parent)
    {
        m_heading.setForeground(QColor(QStringLiteral("#4E9A06")));
        m_heading.setFontWeight(QFont::Bold);
        m_bold.setForeground(QColor(QStringLiteral("#CC0000")));
        m_bold.setFontWeight(QFont::Bold);
        m_code.setForeground(QColor(QStringLiteral("#3465A4")));
    }

protected:
    void highlightBlock(const QString &text) override
    {
        static const QRegularExpression headingRe(QStringLiteral("^#{1,6}\\s+.*"));
        static const QRegularExpression boldRe(QStringLiteral("(\\*\\*[^*]+\\*\\*|__[^_]+__)"));
        static const QRegularExpression codeRe(QStringLiteral("`[^`]+`"));
        if (headingRe.match(text).hasMatch())
            setFormat(0, text.size(), m_heading);
        for (auto it = boldRe.globalMatch(text); it.hasNext(); ) {
            const auto mm = it.next();
            setFormat(mm.capturedStart(), mm.capturedLength(), m_bold);
        }
        for (auto it = codeRe.globalMatch(text); it.hasNext(); ) {
            const auto mm = it.next();
            setFormat(mm.capturedStart(), mm.capturedLength(), m_code);
        }
    }

private:
    QTextCharFormat m_heading;
    QTextCharFormat m_bold;
    QTextCharFormat m_code;
};

static bool listenModeEnabled()
{
    const QStringList args = QCoreApplication::arguments();
    if (args.contains(QStringLiteral("--listen"))
        || args.contains(QStringLiteral("--restore-no"))
        || args.contains(QStringLiteral("--restore-yes"))
        || args.contains(QStringLiteral("--save-as"))
        || args.contains(QStringLiteral("--print-info"))
        || args.contains(QStringLiteral("--print-to-file"))
        || args.contains(QStringLiteral("--print-lp")))
        return true;
    const QString env = QString::fromLocal8Bit(qgetenv("MONASTERY_RESTORE")).toLower();
    if (env == QLatin1String("no") || env == QLatin1String("yes")
        || env == QLatin1String("discard") || env == QLatin1String("restore"))
        return true;
    return qEnvironmentVariableIntValue("MONASTERY_LISTEN") != 0;
}

static void listenLog(const char *key, const QString &value)
{
    std::fprintf(stdout, "%s: %s\n", key, qPrintable(value));
    std::fflush(stdout);
}

static QString argValueAfter(const QString &flag)
{
    const QStringList args = QCoreApplication::arguments();
    const int i = args.indexOf(flag);
    if (i >= 0 && i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-')))
        return args.at(i + 1);
    return QString();
}

static QString listenPrintToFilePath()
{
    return argValueAfter(QStringLiteral("--print-to-file"));
}

static bool listenPrintLpRequested()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--print-lp"));
}

static bool listenPrintLpExecute()
{
    return qEnvironmentVariableIntValue("MONASTERY_PRINT_LP") != 0
        || qEnvironmentVariableIntValue("SHOIN_PRINT_LP") != 0;
}

static QPageLayout defaultPrintPageLayout()
{
    return QPageLayout(QPageSize(QPageSize::Letter),
                       QPageLayout::Portrait,
                       QMarginsF(0.75, 0.75, 0.75, 0.75),
                       QPageLayout::Inch);
}

static QPageLayout pageLayoutFromPrinter(const QPrinter &printer)
{
    const QPageLayout layout = printer.pageLayout();
    return layout.isValid() ? layout : defaultPrintPageLayout();
}

static QStringList cupsLpArgv(const QString &printerName, int copies, const QString &pdfPath)
{
    QStringList args;
    if (!printerName.isEmpty())
        args << QStringLiteral("-d") << printerName;
    if (copies > 1)
        args << QStringLiteral("-n") << QString::number(copies);
    args << pdfPath;
    return args;
}

static QString formatCommandLine(const QString &program, const QStringList &args)
{
    QStringList parts;
    parts << program;
    parts += args;
    return parts.join(QLatin1Char(' '));
}

static void listenLogPrinters()
{
    const QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
    const QString def = QPrinterInfo::defaultPrinterName();
    bool brother = false;
    for (const QPrinterInfo &info : printers) {
        if (info.printerName().contains(QStringLiteral("Brother"), Qt::CaseInsensitive)) {
            brother = true;
            break;
        }
    }
    listenLog("printers", QString::number(printers.size()));
    listenLog("default_printer", def.isEmpty() ? QStringLiteral("(none)") : def);
    listenLog("has_brother", brother ? QStringLiteral("yes") : QStringLiteral("no"));
    listenLog("print_dialog", printers.isEmpty() ? QStringLiteral("qt") : QStringLiteral("cups"));
}


static void showFamilyInCombo(QFontComboBox *combo, const QString &family)
{
    if (!combo || family.isEmpty())
        return;
    QSignalBlocker block(combo);
    combo->setCurrentFont(QFont(family));
    if (QString::compare(combo->currentText(), family, Qt::CaseInsensitive) == 0)
        return;
    if (!combo->isEditable())
        combo->setEditable(true);
    combo->setEditText(family);
    if (QString::compare(combo->currentText(), family, Qt::CaseInsensitive) != 0)
        combo->setCurrentText(family);
}

static void showSizeInCombo(QComboBox *combo, int pt)
{
    if (!combo || pt <= 0)
        return;
    QSignalBlocker block(combo);
    const QString s = QString::number(pt);
    if (combo->findText(s) < 0) {
        int i = 0;
        for (; i < combo->count(); ++i) {
            if (combo->itemText(i).toInt() > pt)
                break;
        }
        combo->insertItem(i, s);
    }
    combo->setCurrentText(s);
}


static bool isDocxPath(const QString &path)
{
    return QFileInfo(path).suffix().toLower() == QLatin1String("docx");
}

static QString listenSaveAsPath()
{
    const QStringList args = QCoreApplication::arguments();
    const int i = args.indexOf(QStringLiteral("--save-as"));
    if (i >= 0 && i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-')))
        return args.at(i + 1);
    const QString env = QString::fromLocal8Bit(qgetenv("MONASTERY_SAVE_AS"));
    if (!env.isEmpty())
        return env;
    return QString::fromLocal8Bit(qgetenv("LISTEN_SAVE_AS"));
}

static void listenLogDocxPeek(const QString &path)
{
    bool hasCt = false;
    bool hasDoc = false;
    bool leftover = false;
    QString err;
    DocumentIo::docxPeek(path, &hasCt, &hasDoc, &leftover, &err);
    listenLog("has_content_types", hasCt ? QStringLiteral("yes") : QStringLiteral("no"));
    listenLog("has_document_xml", hasDoc ? QStringLiteral("yes") : QStringLiteral("no"));
    listenLog("leftover_html", leftover ? QStringLiteral("yes") : QStringLiteral("no"));
}

// Proof-only: MONASTERY_RESTORE=no or --restore-no. Not the GUI default.
static int scriptedRestoreChoice()
{
    const QString env = QString::fromLocal8Bit(qgetenv("MONASTERY_RESTORE")).toLower();
    if (env == QLatin1String("no") || env == QLatin1String("discard"))
        return 0;
    if (env == QLatin1String("yes") || env == QLatin1String("restore"))
        return 1;
    const QStringList args = QCoreApplication::arguments();
    if (args.contains(QStringLiteral("--restore-no")))
        return 0;
    if (args.contains(QStringLiteral("--restore-yes")))
        return 1;
    return -1;
}

static bool isSourceDocumentPath(const QString &path)
{
    const QString ext = QFileInfo(path).suffix().toLower();
    return ext == QLatin1String("md")
        || ext == QLatin1String("markdown")
        || ext == QLatin1String("txt");
}

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
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        if (m_restoreDialogUp)
            return;
        close();
    });

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
    editMenu->addAction(m_checklistAction);
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
    m_checklistAction->setIcon(QIcon(":/icons/checklist.png"));
    m_toolBar->addAction(m_newAction);
    m_toolBar->addAction(m_openAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addSeparator();
    m_fontCombo = new QFontComboBox();
    m_fontCombo->setEditable(true);
    m_fontCombo->setCurrentFont(QFont("Noto Serif"));
    connect(m_fontCombo, &QComboBox::textActivated, this, &MonasteryFrame::onFontChanged);
    m_toolBar->addWidget(m_fontCombo);
    m_sizeCombo = new QComboBox();
    m_sizeCombo->addItems({"8", "10", "12", "14", "16", "18", "20", "24", "28", "32"});
    m_sizeCombo->setCurrentText("12");
    connect(m_sizeCombo, &QComboBox::textActivated, this, &MonasteryFrame::onSizeChanged);
    m_toolBar->addWidget(m_sizeCombo);
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
    m_toolBar->addAction(m_checklistAction);
    mainLayout->addWidget(m_toolBar);

    // editor: HTML parchment + markdown source (QStackedWidget; WebEngine stays)
    m_editorStack = new QStackedWidget(this);
    m_editor = new MonasteryEditor(m_editorStack);
    m_mdEdit = new QPlainTextEdit(m_editorStack);
    {
        QFont mono;
        mono.setFamilies({QStringLiteral("DejaVu Sans Mono"), QStringLiteral("Liberation Mono"),
                          QStringLiteral("Courier New"), QStringLiteral("monospace")});
        mono.setPointSize(12);
        m_mdEdit->setFont(mono);
    }
    m_mdEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_mdEdit->setTabStopDistance(32);
    m_mdEdit->setStyleSheet(QStringLiteral(
        "QPlainTextEdit { background-color: #1e1a17; color: #F5E8C7; border: none;"
        " selection-background-color: #5C4A3F; }"));
    new MarkdownHighlighter(m_mdEdit->document());
    connect(m_mdEdit, &QPlainTextEdit::modificationChanged, this, [this](bool) {
        updateTitleBar();
    });
    connect(m_mdEdit, &QPlainTextEdit::textChanged, this, [this]() {
        if (isMarkdownMode())
            updateWordCount();
    });
    m_editorStack->addWidget(m_editor);
    m_editorStack->addWidget(m_mdEdit);
    m_editorStack->setCurrentWidget(m_editor);
    mainLayout->addWidget(m_editorStack, 1);
    connect(m_editor, &MonasteryEditor::wordCountChanged, this, [this](int count) {
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
    });
    connect(m_editor, &MonasteryEditor::selectionFontChanged, this, &MonasteryFrame::onSelectionFontChanged);
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
            this, &MonasteryFrame::onPdfPrintingFinished);

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
    connect(m_undoAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->undo(); return; }
        m_editor->execCommand("undo");
    });
    connect(m_redoAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->redo(); return; }
        m_editor->execCommand("redo");
    });
    connect(m_cutAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->cut(); return; }
        m_editor->execCommand("cut");
    });
    connect(m_copyAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->copy(); return; }
        m_editor->execCommand("copy");
    });
    connect(m_pasteAction, &QAction::triggered, this, [this]() {
        if (isMarkdownMode()) { m_mdEdit->paste(); return; }
        m_editor->execCommand("paste");
    });

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

    m_checklistAction = new QAction(this);
    m_checklistAction->setToolTip("Checklist");
    connect(m_checklistAction, &QAction::triggered, this, &MonasteryFrame::onChecklist);

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
    setMarkdownMode(false);
    m_editor->setHtml("<p></p>");
    m_currentFilePath.clear();
    m_editor->markClean();
    if (m_mdEdit) {
        QSignalBlocker block(m_mdEdit);
        m_mdEdit->clear();
        m_mdEdit->document()->setModified(false);
    }
    updateTitleBar();
    m_statusBar->showMessage("New document created");
    updateWordCount();
}

void MonasteryFrame::onOpen() {
    if (!confirmProceedIfDirty())
        return;

    QString fileName = QFileDialog::getOpenFileName(this, "Open", m_docsDir, kDocumentFilter, nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;
    openPath(fileName);
}

void MonasteryFrame::onSave() {
    saveNow();
}

void MonasteryFrame::onExit() {
    if (m_restoreDialogUp)
        return;
    close();
}

void MonasteryFrame::onAutoSave() {
    if (isMarkdownMode()) {
        if (!m_mdEdit || !m_mdEdit->document()->isModified())
            return;
        const QString dest = hasNamedDocument()
            ? (QFileInfo(m_currentFilePath).absolutePath() + "/"
               + QFileInfo(m_currentFilePath).completeBaseName() + "_autosave.md")
            : (m_docsDir + "/Monastery_AutoSave.md");
        QFile f(dest);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            f.write(m_mdEdit->toPlainText().toUtf8());
            m_statusBar->showMessage("Auto-saved to " + dest);
        }
        return;
    }
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
    QPrinter printer(QPrinter::HighResolution);
    const QString defName = QPrinterInfo::defaultPrinterName();
    if (!defName.isEmpty())
        printer.setPrinterName(defName);
    printer.setPageLayout(defaultPrintPageLayout());

    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (isMarkdownMode()) {
        if (m_mdEdit)
            m_mdEdit->print(&printer);
        m_statusBar->showMessage("Printed");
        return;
    }

    const QPageLayout layout = pageLayoutFromPrinter(printer);
    const bool toFile = printer.outputFormat() == QPrinter::PdfFormat
                        || !printer.outputFileName().isEmpty();
    if (toFile) {
        QString fileName = printer.outputFileName();
        if (fileName.isEmpty())
            return;
        if (!fileName.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive))
            fileName += QStringLiteral(".pdf");
        m_editor->webView()->page()->printToPdf(fileName, layout);
        m_statusBar->showMessage("Printing to file: " + fileName);
        return;
    }

    if (m_printTemp) {
        m_printTemp->deleteLater();
        m_printTemp = nullptr;
    }
    auto *tmp = new QTemporaryFile(QDir::tempPath() + QStringLiteral("/monastery-print-XXXXXX.pdf"), this);
    tmp->setAutoRemove(true);
    if (!tmp->open()) {
        delete tmp;
        QMessageBox::warning(this, "Print Failed", "Could not create a temporary PDF.");
        return;
    }
    const QString tmpPath = tmp->fileName();
    tmp->close();
    m_printTemp = tmp;
    m_pendingLpPdf = tmpPath;
    m_pendingLpPrinter = printer.printerName();
    m_pendingLpCopies = qMax(1, printer.copyCount());
    m_editor->webView()->page()->printToPdf(tmpPath, layout);
    m_statusBar->showMessage("Printing...");
}

void MonasteryFrame::onPdfPrintingFinished(const QString &path, bool success)
{
    if (listenModeEnabled())
        listenLog("pdf_print", success ? QStringLiteral("success") : QStringLiteral("fail"));

    const bool cupsJob = !m_pendingLpPdf.isEmpty() && path == m_pendingLpPdf;
    if (cupsJob) {
        const QString pdf = m_pendingLpPdf;
        const QString printerName = m_pendingLpPrinter;
        const int copies = m_pendingLpCopies;
        m_pendingLpPdf.clear();
        m_pendingLpPrinter.clear();
        m_pendingLpCopies = 1;

        auto cleanupTemp = [this]() {
            if (m_printTemp) {
                m_printTemp->deleteLater();
                m_printTemp = nullptr;
            }
        };

        if (!success) {
            cleanupTemp();
            if (!listenModeEnabled())
                QMessageBox::warning(this, "Print Failed",
                                     "Could not render the page for printing.");
            if (m_listenPrintPending)
                requestListenQuit();
            return;
        }

        const QStringList args = cupsLpArgv(printerName, copies, pdf);
        if (listenModeEnabled())
            listenLog("lp_argv", formatCommandLine(QStringLiteral("lp"), args));

        const bool allowLp = !listenModeEnabled() || listenPrintLpExecute();
        if (!allowLp) {
            listenLog("lp_dry_run", QStringLiteral("yes"));
            cleanupTemp();
            if (m_listenPrintPending)
                requestListenQuit();
            return;
        }

        QProcess *lp = new QProcess(this);
        connect(lp, &QProcess::finished, this,
                [this, lp](int code, QProcess::ExitStatus st) {
            const QByteArray err = lp->readAllStandardError();
            lp->deleteLater();
            if (m_printTemp) {
                m_printTemp->deleteLater();
                m_printTemp = nullptr;
            }
            if (code != 0 || st != QProcess::NormalExit) {
                if (!listenModeEnabled()) {
                    const QString msg = err.isEmpty()
                        ? QStringLiteral("lp failed")
                        : QString::fromLocal8Bit(err);
                    QMessageBox::warning(this, "Print Failed", msg);
                } else {
                    listenLog("lp_error", err.isEmpty() ? QStringLiteral("lp failed")
                                                        : QString::fromLocal8Bit(err));
                }
            } else {
                m_statusBar->showMessage("Sent to printer");
                if (listenModeEnabled())
                    listenLog("lp_sent", QStringLiteral("yes"));
            }
            if (m_listenPrintPending)
                requestListenQuit();
        });
        lp->start(QStringLiteral("lp"), args);
        if (!lp->waitForStarted(3000)) {
            if (!listenModeEnabled())
                QMessageBox::warning(this, "Print Failed", "Could not start lp.");
            else
                listenLog("lp_error", QStringLiteral("could not start lp"));
            lp->deleteLater();
            if (m_printTemp) {
                m_printTemp->deleteLater();
                m_printTemp = nullptr;
            }
            if (m_listenPrintPending)
                requestListenQuit();
        }
        return;
    }

    if (!success) {
        if (!listenModeEnabled())
            QMessageBox::warning(this, "Print Failed",
                                 "Could not write the print file:\n" + path);
    } else {
        m_statusBar->showMessage("Printed to: " + path);
    }

    if (m_listenPrintPending)
        requestListenQuit();
}

bool MonasteryFrame::maybeStartListenPrint()
{
    if (listenPrintLpRequested()) {
        const QString def = QPrinterInfo::defaultPrinterName();
        const QStringList args = cupsLpArgv(def, 1, QStringLiteral("/tmp/monastery-print.pdf"));
        listenLog("lp_argv", formatCommandLine(QStringLiteral("lp"), args));
        listenLog("lp_dry_run", QStringLiteral("yes"));
    }

    const QString dest = listenPrintToFilePath();
    if (dest.isEmpty())
        return false;

    if (isMarkdownMode()) {
        if (!m_mdEdit)
            return false;
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(dest);
        printer.setPageLayout(defaultPrintPageLayout());
        m_mdEdit->print(&printer);
        const bool ok = QFileInfo(dest).exists() && QFileInfo(dest).size() > 0;
        listenLog("pdf_print", ok ? QStringLiteral("success") : QStringLiteral("fail"));
        return false;
    }
    if (!m_editor || !m_editor->webView() || !m_editor->webView()->page())
        return false;
    m_listenPrintPending = true;
    m_editor->webView()->page()->printToPdf(dest, defaultPrintPageLayout());
    m_statusBar->showMessage("Printing to file: " + dest);
    return true;
}


void MonasteryFrame::onInsertPageBreak() {
    if (isMarkdownMode())
        return;
    m_editor->execCommand("insertHTML",
        "<div class=\"page-break\" style=\"page-break-after: always; border: none; border-top: 1px dashed #8B7355; margin: 30px 0;\"></div>");
}

void MonasteryFrame::onBold() { if (!isMarkdownMode()) m_editor->execCommand("bold"); }

void MonasteryFrame::onItalic()        { if (!isMarkdownMode()) m_editor->execCommand("italic"); }
void MonasteryFrame::onUnderline()     { if (!isMarkdownMode()) m_editor->execCommand("underline"); }

void MonasteryFrame::onAlignLeft()   { if (!isMarkdownMode()) m_editor->execCommand("justifyLeft"); }
void MonasteryFrame::onAlignCenter() { if (!isMarkdownMode()) m_editor->execCommand("justifyCenter"); }
void MonasteryFrame::onAlignRight()  { if (!isMarkdownMode()) m_editor->execCommand("justifyRight"); }
void MonasteryFrame::onJustify()     { if (!isMarkdownMode()) m_editor->execCommand("justifyFull"); }

void MonasteryFrame::onBulletList()    { if (!isMarkdownMode()) m_editor->execCommand("insertUnorderedList"); }
void MonasteryFrame::onNumberedList()  { if (!isMarkdownMode()) m_editor->execCommand("insertOrderedList"); }
void MonasteryFrame::onChecklist() { if (!isMarkdownMode()) m_editor->insertChecklist(); }

void MonasteryFrame::onFontChanged(const QString &font) {
    if (isMarkdownMode())
        return;
    m_editor->applyFontFamily(font);
}


void MonasteryFrame::onSelectionFontChanged(const QString &family, int pt)
{
    if (isMarkdownMode())
        return;
    if ((m_fontCombo && m_fontCombo->hasFocus()) || (m_sizeCombo && m_sizeCombo->hasFocus()))
        return;
    showFamilyInCombo(m_fontCombo, family);
    showSizeInCombo(m_sizeCombo, pt);
}

void MonasteryFrame::dumpListenSelectionFont()
{
    if (!listenModeEnabled() || !m_editor || isMarkdownMode())
        return;

    auto family = std::make_shared<QString>();
    auto pt = std::make_shared<int>(0);
    auto caretFam = std::make_shared<QString>();
    auto caretPt = std::make_shared<int>(0);
    auto found = std::make_shared<bool>(false);
    auto done = std::make_shared<bool>(false);

    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < 4000) {
        *done = false;
        m_editor->requestHeadingFont([=](const QString &f, int p, const QString &cf, int cp, bool hit) {
            *family = f;
            *pt = p;
            *caretFam = cf;
            *caretPt = cp;
            *found = hit;
            *done = true;
        });
        QElapsedTimer wait;
        wait.start();
        while (!*done && wait.elapsed() < 400) {
            QEventLoop loop;
            QTimer::singleShot(40, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (*found && (!family->isEmpty() || !caretFam->isEmpty()))
            break;
        QEventLoop pause;
        QTimer::singleShot(80, &pause, &QEventLoop::quit);
        pause.exec();
    }

    const QString heading = family->isEmpty() ? *caretFam : *family;
    const int headingPt = *pt > 0 ? *pt : *caretPt;
    const QString caret = caretFam->isEmpty() ? *family : *caretFam;
    const int cpt = *caretPt > 0 ? *caretPt : *pt;
    if (!heading.isEmpty())
        listenLog("heading_font", heading);
    if (headingPt > 0)
        listenLog("heading_size", QString::number(headingPt));
    if (!caret.isEmpty())
        listenLog("caret_font", caret);
    if (cpt > 0)
        listenLog("caret_size", QString::number(cpt));
    onSelectionFontChanged(caret.isEmpty() ? heading : caret, cpt > 0 ? cpt : headingPt);
    if (m_fontCombo)
        listenLog("toolbar_font", m_fontCombo->currentText());
    if (m_sizeCombo)
        listenLog("toolbar_size", m_sizeCombo->currentText());
}


void MonasteryFrame::onSizeChanged(const QString &size) {
    if (isMarkdownMode())
        return;
    bool ok = false;
    const int pt = size.toInt(&ok);
    if (!ok || pt <= 0)
        return;
    m_editor->applyFontSize(pt);
}

void MonasteryFrame::updateWordCount() {
    if (isMarkdownMode()) {
        if (!m_wordCountLabel || !m_mdEdit)
            return;
        const QString t = m_mdEdit->toPlainText().trimmed();
        int count = 0;
        if (!t.isEmpty())
            count = t.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size();
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
        return;
    }
    m_editor->requestWordCount([this](int count) {
        m_wordCountLabel->setText(QString("Words: %1").arg(count));
    });
}

void MonasteryFrame::closeEvent(QCloseEvent *event) {
    if (m_restoreDialogUp) {
        event->ignore();
        return;
    }
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
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", m_docsDir, kDocumentFilter, nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;
    fileName = ensureDocumentSuffix(fileName);
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
    const bool dirty = documentIsDirty();
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
    QString err;
    if (!DocumentIo::writeFromHtml(path, html, &err)) {
        if (listenModeEnabled())
            listenLog("save_error", err.isEmpty() ? QStringLiteral("write failed") : err);
        else
            QMessageBox::warning(this, "Save Failed",
                                 "Could not write:\n" + path + "\n" + err);
        return false;
    }
    return true;
}

bool MonasteryFrame::persistDocument(const QString &path, const QString &html, bool markCleanAfter) {
    QString body = html;
    if (htmlLooksEmpty(body) && m_editor && !isMarkdownMode() && !htmlLooksEmpty(m_editor->lastGoodHtml()))
        body = m_editor->lastGoodHtml();
    if (wouldClobberManuscript(body)) {
        if (listenModeEnabled())
            listenLog("save_error", QStringLiteral("wouldClobber empty or incomplete html"));
        m_statusBar->showMessage("Save skipped — empty or incomplete editor content. Last good copy kept.");
        return false;
    }
    if (!writeHtmlFile(path, body))
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
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", m_docsDir, kDocumentFilter, nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return false;
    fileName = ensureDocumentSuffix(fileName);
    m_currentFilePath = fileName;
    updateTitleBar();
    return true;
}

QString MonasteryFrame::waitForEditorHtml()
{
    QString captured;
    if (!m_editor)
        return captured;

    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < 8000) {
        auto done = std::make_shared<bool>(false);
        m_editor->fetchHtml([&](const QString &html) {
            captured = html;
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
            QTimer::singleShot(400, &loop, &QEventLoop::quit);
            loop.exec();
        }
        if (!htmlLooksEmpty(captured))
            return captured;
        QEventLoop pause;
        QTimer::singleShot(80, &pause, &QEventLoop::quit);
        pause.exec();
    }
    if (htmlLooksEmpty(captured))
        captured = m_editor->lastGoodHtml();
    return captured;
}

bool MonasteryFrame::saveNow() {
    if (!ensureSavePath())
        return false;

    const bool destSource = isSourceDocumentPath(m_currentFilePath);
    if (isMarkdownMode() && destSource)
        return saveMarkdownNow();

    QString html;
    if (isMarkdownMode())
        html = DocumentIo::markdownToHtml(m_mdEdit ? m_mdEdit->toPlainText() : QString());
    else
        html = waitForEditorHtml();
    return persistDocument(m_currentFilePath, html, true);
}

bool MonasteryFrame::confirmProceedIfDirty() {
    const bool dirty = documentIsDirty();
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
    QString html;
    const bool readable = file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (readable)
        html = QString::fromUtf8(file.readAll());
    file.close();

    auto finishListen = [this](const QString &opened, const QString &recovery) {
        if (listenModeEnabled()) {
            emitListenHealth(opened, recovery);
            if (!maybeStartListenPrint())
                requestListenQuit();
        }
    };

    if (!readable || htmlLooksEmpty(html)) {
        QString opened;
        if (!m_pendingOpenPath.isEmpty()) {
            opened = m_pendingOpenPath;
            const QString pending = m_pendingOpenPath;
            m_pendingOpenPath.clear();
            openPath(pending);
            if (!m_currentFilePath.isEmpty())
                opened = m_currentFilePath;
        }
        finishListen(opened, QString());
        return;
    }

    int choice = scriptedRestoreChoice();
    if (choice < 0) {
        m_restoreDialogUp = true;
        const bool prevQuit = qApp->quitOnLastWindowClosed();
        qApp->setQuitOnLastWindowClosed(false);
        show();
        raise();
        activateWindow();
        choice = askRestoreAutosave();
        qApp->setQuitOnLastWindowClosed(prevQuit);
        m_restoreDialogUp = false;
        show();
        raise();
        activateWindow();
    }

    if (choice == 1) {
        setMarkdownMode(false);
        m_editor->setHtml(html);
        m_currentFilePath.clear();
        m_editor->markDirty();
        updateTitleBar();
        m_statusBar->showMessage("Restored autosave");
        updateWordCount();
        m_pendingOpenPath.clear();
        finishListen(QString(), QStringLiteral("restored"));
        return;
    }

    QFile::remove(path);
    m_statusBar->showMessage("Autosave discarded");
    QString opened;
    if (!m_pendingOpenPath.isEmpty()) {
        opened = m_pendingOpenPath;
        const QString pending = m_pendingOpenPath;
        m_pendingOpenPath.clear();
        openPath(pending);
        if (!m_currentFilePath.isEmpty())
            opened = m_currentFilePath;
    }
    finishListen(opened, QStringLiteral("discarded"));
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
    if (isMarkdownMode() && m_mdEdit) {
        QTextDocument::FindFlags flags;
        if (backward)
            flags |= QTextDocument::FindBackward;
        if (!m_mdEdit->find(needle, flags)) {
            QTextCursor c = m_mdEdit->textCursor();
            c.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
            m_mdEdit->setTextCursor(c);
            m_mdEdit->find(needle, flags);
        }
        return;
    }
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
    m_checklistAction->setIcon(tinted(QStringLiteral(":/icons/checklist.png")));
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

bool MonasteryFrame::isMarkdownSourcePath(const QString &path) const
{
    return isSourceDocumentPath(path);
}

bool MonasteryFrame::isMarkdownMode() const
{
    return m_markdownMode;
}

void MonasteryFrame::setFormatActionsEnabled(bool on)
{
    const QList<QAction *> acts = {
        m_boldAction, m_italicAction, m_underlineAction,
        m_alignLeftAction, m_alignCenterAction, m_alignRightAction, m_justifyAction,
        m_bulletAction, m_numberAction, m_checklistAction, m_pageBreakAction
    };
    for (QAction *a : acts) {
        if (a)
            a->setEnabled(on);
    }
    if (m_fontCombo)
        m_fontCombo->setEnabled(on);
    if (m_sizeCombo)
        m_sizeCombo->setEnabled(on);
    if (!on && m_mdEdit) {
        const QFont f = m_mdEdit->font();
        showFamilyInCombo(m_fontCombo, f.family());
        showSizeInCombo(m_sizeCombo, f.pointSize());
    }
}

void MonasteryFrame::setMarkdownMode(bool on)
{
    m_markdownMode = on;
    if (m_editorStack && m_mdEdit && m_editor)
        m_editorStack->setCurrentWidget(on ? static_cast<QWidget *>(m_mdEdit)
                                          : static_cast<QWidget *>(m_editor));
    setFormatActionsEnabled(!on);
}

bool MonasteryFrame::documentIsDirty() const
{
    if (isMarkdownMode())
        return m_mdEdit && m_mdEdit->document()->isModified();
    if (!m_editor)
        return false;
    return m_editor->isDirty() || m_editor->queryDirtyNow();
}

bool MonasteryFrame::openPath(const QString &path)
{
    if (path.isEmpty())
        return false;
    const QString abs = QFileInfo(path).absoluteFilePath();
    if (!m_didOfferRestore) {
        m_pendingOpenPath = abs;
        return true;
    }
    if (isSourceDocumentPath(abs))
        return loadMarkdownDocument(abs);
    return loadHtmlDocument(abs);
}

bool MonasteryFrame::loadHtmlDocument(const QString &path)
{
    QString err;
    const QString html = DocumentIo::htmlFromFile(path, &err);
    if (!err.isEmpty()) {
        if (listenModeEnabled())
            listenLog("open_error", err);
        else
            QMessageBox::warning(this, "Open Failed",
                                 "Could not read:\n" + path + "\n" + err);
        return false;
    }
    setMarkdownMode(false);
    m_editor->setHtml(html);
    m_currentFilePath = path;
    m_editor->markClean();
    updateTitleBar();
    m_statusBar->showMessage("File opened: " + path);
    updateWordCount();
    return true;
}

bool MonasteryFrame::loadMarkdownDocument(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Failed",
                             "Could not read:\n" + path);
        return false;
    }
    const QString text = QString::fromUtf8(file.readAll());
    file.close();
    setMarkdownMode(true);
    {
        QSignalBlocker block(m_mdEdit);
        m_mdEdit->setPlainText(text);
    }
    m_mdEdit->document()->setModified(false);
    m_currentFilePath = path;
    updateTitleBar();
    m_statusBar->showMessage("File opened: " + path);
    updateWordCount();
    return true;
}

bool MonasteryFrame::saveMarkdownNow()
{
    if (!ensureSavePath())
        return false;
    if (!isSourceDocumentPath(m_currentFilePath)) {
        const QString html = DocumentIo::markdownToHtml(m_mdEdit ? m_mdEdit->toPlainText() : QString());
        return persistDocument(m_currentFilePath, html, true);
    }
    if (!m_mdEdit) {
        m_statusBar->showMessage("No markdown buffer to save");
        return false;
    }
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not write:\n" + m_currentFilePath);
        return false;
    }
    const QByteArray bytes = m_mdEdit->toPlainText().toUtf8();
    if (file.write(bytes) != bytes.size()) {
        QMessageBox::warning(this, "Save Failed",
                             "Short write:\n" + m_currentFilePath);
        return false;
    }
    m_mdEdit->document()->setModified(false);
    updateTitleBar();
    m_statusBar->showMessage("Saved to " + m_currentFilePath);
    return true;
}

int MonasteryFrame::askRestoreAutosave()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Restore Autosave"));
    dlg.setModal(true);
    dlg.setWindowModality(Qt::WindowModal);
    dlg.setAttribute(Qt::WA_QuitOnClose, false);
    dlg.setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    QLabel *msg = new QLabel(QStringLiteral("An autosaved document was found. Restore it?"));
    msg->setWordWrap(true);
    lay->addWidget(msg);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
    lay->addWidget(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    show();
    raise();
    const int rc = dlg.exec();
    show();
    raise();
    activateWindow();
    return rc == QDialog::Accepted ? 1 : 0;
}

void MonasteryFrame::emitListenHealth(const QString &openedPath, const QString &recovery)
{
    if (!listenModeEnabled())
        return;
    if (!recovery.isEmpty())
        listenLog("recovery", recovery);
    if (!openedPath.isEmpty())
        listenLog("opened", openedPath);

    const bool docx = isDocxPath(openedPath) || isDocxPath(m_currentFilePath);
    bool healthOk = true;
    if (isMarkdownMode() && m_mdEdit) {
        const QString buf = m_mdEdit->toPlainText();
        listenLog("mode", QStringLiteral("markdown-source"));
        listenLog("has_hash", buf.contains(QLatin1Char('#')) ? QStringLiteral("yes") : QStringLiteral("no"));
        listenLog("has_pipe", buf.contains(QLatin1Char('|')) ? QStringLiteral("yes") : QStringLiteral("no"));
        listenLog("html_preview", QStringLiteral("no"));
    } else if (!openedPath.isEmpty() || docx) {
        listenLog("mode", docx ? QStringLiteral("html-docx") : QStringLiteral("html"));
        listenLog("html_preview", QStringLiteral("yes"));
        if (docx) {
            QString herr;
            const bool ooxml = DocumentIo::docxLooksHealthy(openedPath.isEmpty() ? m_currentFilePath : openedPath, &herr);
            listenLog("ooxml", ooxml ? QStringLiteral("yes") : QStringLiteral("no"));
            if (!ooxml)
                healthOk = false;
        }
    }

    const QString dest = listenSaveAsPath();
    if (!dest.isEmpty()) {
        QString html;
        if (isMarkdownMode())
            html = DocumentIo::markdownToHtml(m_mdEdit ? m_mdEdit->toPlainText() : QString());
        else
            html = waitForEditorHtml();
        {
            const QString probe = html.isEmpty() && m_editor ? m_editor->lastGoodHtml() : html;
            listenLog("table_count", QString::number(probe.toLower().count(QStringLiteral("<table"))));
            const bool fontSpan = probe.contains(QLatin1String("font-family"), Qt::CaseInsensitive)
                || probe.contains(QLatin1String("font-size"), Qt::CaseInsensitive);
            listenLog("has_font_span", fontSpan ? QStringLiteral("yes") : QStringLiteral("no"));
        }
        if (htmlLooksEmpty(html)) {
            listenLog("save_error", QStringLiteral("empty editor html"));
            healthOk = false;
        } else {
            QString err;
            if (!DocumentIo::writeFromHtml(dest, html, &err)) {
                listenLog("save_error", err.isEmpty() ? QStringLiteral("writeFromHtml failed") : err);
                healthOk = false;
            } else {
                listenLog("saved", QFileInfo(dest).absoluteFilePath());
            }
        }
        if (isDocxPath(dest)) {
            listenLogDocxPeek(dest);
            QString herr;
            if (!DocumentIo::docxLooksHealthy(dest, &herr))
                healthOk = false;
        }
    } else {
        const QString probe = m_editor ? m_editor->lastGoodHtml() : QString();
        listenLog("table_count", QString::number(probe.toLower().count(QStringLiteral("<table"))));
        const bool fontSpan = probe.contains(QLatin1String("font-family"), Qt::CaseInsensitive)
            || probe.contains(QLatin1String("font-size"), Qt::CaseInsensitive);
        listenLog("has_font_span", fontSpan ? QStringLiteral("yes") : QStringLiteral("no"));
        if (docx)
            listenLogDocxPeek(openedPath);
    }

    dumpListenSelectionFont();

    listenLogPrinters();

    listenLog("still_running", QStringLiteral("yes"));
    listenLog("health", healthOk ? QStringLiteral("ok") : QStringLiteral("fail"));
}

void MonasteryFrame::requestListenQuit()
{
    if (!listenModeEnabled() || m_listenQuitArmed)
        return;
    m_listenQuitArmed = true;
    QTimer::singleShot(800, qApp, []() {
        QCoreApplication::quit();
    });
}
