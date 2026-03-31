#include <QApplication>
#include <QDir>
#include "MonasteryFrame.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Monastery");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Monastery");

    MonasteryFrame frame;
    frame.show();

    return app.exec();
}