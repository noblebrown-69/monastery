#include "MonasteryEditor.h"
#include "MonasteryFrame.h"
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPalette>
#include <QFontDatabase>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextDocument>
#include <QFile>
#include <QMenu>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDebug>
#include <hunspell/hunspell.h>

HunspellHighlighter::HunspellHighlighter(QTextDocument *parent, const QString &userDicPath)
    : QSyntaxHighlighter(parent), m_userDicPath(userDicPath)
{
    QString affPath = "/usr/share/hunspell/en_US.aff";
    QString dicPath = "/usr/share/hunspell/en_US.dic";
    if (!QFile::exists(affPath)) {
        affPath = "/usr/share/hunspell/en_GB.aff";
        dicPath = "/usr/share/hunspell/en_GB.dic";
    }

    m_hunspell = Hunspell_create(affPath.toUtf8().constData(), dicPath.toUtf8().constData());

    // Load user dictionary by reading every line and adding each word
    QFile userFile(m_userDicPath);
    if (userFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&userFile);
        while (!in.atEnd()) {
            QString word = in.readLine().trimmed();
            if (!word.isEmpty()) {
                Hunspell_add(m_hunspell, word.toUtf8().constData());
            }
        }
        qDebug() << "✅ Loaded user dictionary from" << m_userDicPath;
    }
}

HunspellHighlighter::~HunspellHighlighter() {
    if (m_hunspell) Hunspell_destroy(m_hunspell);
}

void HunspellHighlighter::addWord(const QString &word) {
    if (m_hunspell) {
        Hunspell_add(m_hunspell, word.toUtf8().constData());
        QFile f(m_userDicPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&f);
            out << word << "\n";
            qDebug() << "✅ Added to user dictionary:" << word;
        }
    }
}

bool HunspellHighlighter::isMisspelled(const QString &word) {
    if (m_hunspell && !word.isEmpty()) {
        return !Hunspell_spell(m_hunspell, word.toUtf8().constData());
    }
    return false;
}

void HunspellHighlighter::highlightBlock(const QString &text) {
    QTextCharFormat errorFormat;
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    QRegularExpression wordRegex("\\b\\w+\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured();
        if (!Hunspell_spell(m_hunspell, word.toUtf8().constData())) {
            setFormat(match.capturedStart(), match.capturedLength(), errorFormat);
        }
    }
}

MonasteryEditor::MonasteryEditor(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(0);

    m_textEdit = new QTextEdit(this);

    // Editor appearance - exact match to Aureus "Leather Zibaldone" theme
    m_textEdit->setStyleSheet("QTextEdit {"
                              "  background-color: #F5E8C7;"
                              "  color: #000000;"
                              "  selection-background-color: #D4AF37;"
                              "  selection-color: #000000;"
                              "}");

    QString fontFamily = "Georgia";
    if (!QFontDatabase::families().contains(fontFamily)) {
        fontFamily = "Noto Serif";
        if (!QFontDatabase::families().contains(fontFamily)) {
            fontFamily = "EB Garamond";
        }
    }
    m_textEdit->setFont(QFont(fontFamily, 12));

    m_userDicPath = MonasteryFrame::getRealAppDir() + "/user.dic";
    m_highlighter = new HunspellHighlighter(m_textEdit->document(), m_userDicPath);

    m_textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_textEdit, &QTextEdit::customContextMenuRequested, this, &MonasteryEditor::showContextMenu);

    layout->addWidget(m_textEdit);
    setLayout(layout);
}

void MonasteryEditor::refreshHighlighter() {
    if (m_highlighter) delete m_highlighter;
    m_highlighter = new HunspellHighlighter(m_textEdit->document(), m_userDicPath);
    m_highlighter->rehighlight();
}

void MonasteryEditor::showContextMenu(const QPoint &pos) {
    QMenu *menu = m_textEdit->createStandardContextMenu(pos);

    // Check if we're on a misspelled word
    QTextCursor cursor = m_textEdit->cursorForPosition(pos);
    cursor.select(QTextCursor::WordUnderCursor);
    QString word = cursor.selectedText().trimmed();

    bool isMisspelled = m_highlighter && m_highlighter->isMisspelled(word);

    if (isMisspelled) {
        menu->addSeparator();
        QAction *addToDict = menu->addAction("Add to Dictionary");
        connect(addToDict, &QAction::triggered, this, [this, word]() {
            if (!word.isEmpty() && m_highlighter) {
                qDebug() << "Right-click add requested for word:" << word;
                m_highlighter->addWord(word);
                refreshHighlighter();
                qDebug() << "✅ Refreshed highlighter - underline should clear";
            }
        });
    }

    menu->exec(m_textEdit->mapToGlobal(pos));
    delete menu;
}

