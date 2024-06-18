#include "AppInfo.h"
#include <QGuiApplication>

AppInfo::AppInfo(QObject *parent)
    : QObject{parent}
{
    setversion(APPLICATION_VERSION);
    setappname(APPNAME);
}
