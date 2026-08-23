#ifndef MONASTERYEDITOR_H
#define MONASTERYEDITOR_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QTimer>
#include <functional>
#include "Theme.h"

class MonasteryEditor : public QWidget {
    Q_OBJECT
public:
    explicit MonasteryEditor(QWidget *parent = nullptr);
    void execCommand(const QString &cmd, const QString &value = QString());
    QString getHtml();
    void setHtml(const QString &html);
    int getWordCount();
    void refreshHighlighter();

    QWebEngineView* webView() const { return m_webView; }

    void fetchHtml(const std::function<void(const QString &)> &callback);
    void requestWordCount(const std::function<void(int)> &callback);
    bool queryDirtyNow();
    void markClean();
    void markDirty();
    bool isDirty() const { return m_dirty; }
    QString lastGoodHtml() const { return m_cachedHtml; }
    void applyFontSize(int pointSize);
    void applyTheme(const Theme &theme);

signals:
    void wordCountChanged(int count);
    void dirtyChanged(bool dirty);
    void ready(bool ok);

private:
    void considerHtmlCache(const QString &html);
    void pollEditorState();

    QWebEngineView *m_webView;
    QTimer *m_pollTimer;
    int m_cachedWordCount = 0;
    QString m_cachedHtml;
    bool m_isLoaded = false;
    bool m_dirty = false;
};

#endif // MONASTERYEDITOR_H
