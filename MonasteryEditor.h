#ifndef MONASTERYEDITOR_H
#define MONASTERYEDITOR_H

#include <QWidget>

class QLabel;
class QTextEdit;

class MonasteryEditor : public QWidget {
    Q_OBJECT

public:
    MonasteryEditor(QWidget *parent = nullptr);
    QTextEdit* textEdit() { return m_textEdit; }
    void toggleRuler(bool show);

private:
    QLabel *m_ruler;
    QTextEdit *m_textEdit;
};

#endif // MONASTERYEDITOR_H