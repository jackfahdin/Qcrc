#include "mainwindow.h"

#include <InfoVersion.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QString("%1-v%2 (Jackfahdin)")
                         .arg(APPNAME, APPLICATION_VERSION));
    w.show();
    return a.exec();
}
