#ifndef MONASTERYEDITOR_H
#define MONASTERYEDITOR_H

#include <QWidget>
#include <QSyntaxHighlighter>
#include <QString>
#include <hunspell/hunspell.h>

class QLabel;
class QTextEdit;
class QTextDocument;

typedef Hunhandle Hunspell;

class HunspellHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    HunspellHighlighter(QTextDocument *parent, const QString &userDicPath);
    ~HunspellHighlighter();

    void addWord(const QString &word);
    void reloadDictionary();   // new: forces full reload

protected:
    void highlightBlock(const QString &text) override;

private:
    Hunspell *m_hunspell = nullptr;
    QString m_userDicPath;
};

class MonasteryEditor : public QWidget {
    Q_OBJECT

public:
    MonasteryEditor(QWidget *parent = nullptr);
    QTextEdit* textEdit() { return m_textEdit; }
    void toggleRuler(bool show);
    void refreshHighlighter();   // new method to force reload

private slots:
    void showContextMenu(const QPoint &pos);

private:
    QLabel *m_ruler;
    QTextEdit *m_textEdit;
    HunspellHighlighter *m_highlighter = nullptr;
    QString m_userDicPath;
};

#endif // MONASTERYEDITOR_H