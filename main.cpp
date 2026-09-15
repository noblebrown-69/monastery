#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <cstdio>
#include "DocumentIo.h"
#include "MonasteryFrame.h"

static int runHeadlessConvert(const QString &inPath, const QString &outPath)
{
    QString err;
    const QFileInfo inInfo(inPath);
    const QString html = DocumentIo::htmlFromFile(inPath, &err);
    bool ok = err.isEmpty();
    if (ok)
        ok = DocumentIo::writeFromHtml(outPath, html, &err);
    const QFileInfo outInfo(outPath);
    std::fprintf(stdout, "app: %s\n", qPrintable(QCoreApplication::applicationName()));
    std::fprintf(stdout, "in: %s\n", qPrintable(inInfo.absoluteFilePath()));
    std::fprintf(stdout, "out: %s\n", qPrintable(QFileInfo(outPath).absoluteFilePath()));
    std::fprintf(stdout, "in_size: %lld\n", static_cast<long long>(inInfo.size()));
    std::fprintf(stdout, "out_size: %lld\n", static_cast<long long>(outInfo.size()));
    std::fprintf(stdout, "format: %s\n", qPrintable(DocumentIo::detectFormatName(inPath)));
    std::fprintf(stdout, "table_count: %lld\n", static_cast<long long>(html.toLower().count(QStringLiteral("<table"))));
    std::fprintf(stdout, "has_font_span: %s\n",
                 (html.contains(QLatin1String("font-family"), Qt::CaseInsensitive)
                  || html.contains(QLatin1String("font-size"), Qt::CaseInsensitive)) ? "yes" : "no");
    std::fprintf(stdout, "health: %s\n", (ok && outInfo.exists() && outInfo.size() > 0) ? "ok" : "fail");
    if (!ok && !err.isEmpty())
        std::fprintf(stderr, "error: %s\n", qPrintable(err));
    return (ok && outInfo.exists() && outInfo.size() > 0) ? 0 : 1;
}

// Skip the binary name, --convert and its two operands, and proof-only flags.
static QString firstLeftoverExistingFile(const QStringList &args)
{
    for (int i = 1; i < args.size(); ++i) {
        const QString a = args.at(i);
        if (a == QLatin1String("--convert")) {
            i += 2;
            continue;
        }
        if (a == QLatin1String("--save-as")
            || a == QLatin1String("--print-to-file")) {
            i += 1;
            continue;
        }
        if (a == QLatin1String("--restore-no")
            || a == QLatin1String("--restore-yes")
            || a == QLatin1String("--listen")
            || a == QLatin1String("--print-info")
            || a == QLatin1String("--print-lp"))
            continue;
        if (a.startsWith(QLatin1Char('-')))
            continue;
        const QFileInfo fi(a);
        if (fi.exists() && fi.isFile())
            return fi.absoluteFilePath();
    }
    return QString();
}

static void ensureSpellcheckDictionariesPath(int argc, char *argv[])
{
    if (!qEnvironmentVariableIsEmpty("QTWEBENGINE_DICTIONARIES_PATH"))
        return;

    const QByteArray appdir = qgetenv("APPDIR");
    if (!appdir.isEmpty()) {
        const QString p = QString::fromLocal8Bit(appdir)
            + QStringLiteral("/usr/share/qtwebengine_dictionaries");
        if (QDir(p).exists()) {
            qputenv("QTWEBENGINE_DICTIONARIES_PATH", QFile::encodeName(p));
            return;
        }
    }

    const QString homeLocal = QDir::homePath()
        + QStringLiteral("/.local/share/qtwebengine_dictionaries");
    if (QFileInfo::exists(homeLocal + QStringLiteral("/en-US.bdic"))) {
        qputenv("QTWEBENGINE_DICTIONARIES_PATH", QFile::encodeName(homeLocal));
        return;
    }

    if (argc > 0) {
        const QString beside = QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath()
            + QStringLiteral("/qtwebengine_dictionaries");
        if (QFileInfo::exists(beside + QStringLiteral("/en-US.bdic"))) {
            qputenv("QTWEBENGINE_DICTIONARIES_PATH", QFile::encodeName(beside));
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    ensureSpellcheckDictionariesPath(argc, argv);
    QApplication app(argc, argv);
    app.setApplicationName("Monastery");
    app.setApplicationVersion("2.0.1");
    app.setOrganizationName("Monastery");

    const QStringList args = app.arguments();
    const int conv = args.indexOf(QStringLiteral("--convert"));
    if (conv >= 0) {
        if (conv + 2 >= args.size()) {
            std::fprintf(stderr, "usage: %s --convert <in> <out>\n", qPrintable(app.applicationName()));
            return 1;
        }
        return runHeadlessConvert(args.at(conv + 1), args.at(conv + 2));
    }

    MonasteryFrame frame;
    const QString leftover = firstLeftoverExistingFile(args);
    if (!leftover.isEmpty())
        frame.openPath(leftover);
    frame.show();

    const bool listen = args.contains(QStringLiteral("--listen"))
        || args.contains(QStringLiteral("--restore-no"))
        || args.contains(QStringLiteral("--restore-yes"))
        || args.contains(QStringLiteral("--save-as"))
        || args.contains(QStringLiteral("--print-info"))
        || args.contains(QStringLiteral("--print-to-file"))
        || args.contains(QStringLiteral("--print-lp"))
        || qEnvironmentVariableIntValue("MONASTERY_LISTEN") != 0
        || !qEnvironmentVariable("MONASTERY_RESTORE").isEmpty()
        || !qEnvironmentVariable("MONASTERY_SAVE_AS").isEmpty();
    if (listen) {
        QTimer::singleShot(20000, &app, []() {
            std::fprintf(stdout, "health: timeout\n");
            std::fflush(stdout);
            QCoreApplication::quit();
        });
    }

    return app.exec();
}
