#ifndef APPINFO_H
#define APPINFO_H

#include <QObject>
#include <stdafx.h>
#include <singleton.h>
#include <InfoVersion.h>

class AppInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY_CREATE(QString, version)
    Q_PROPERTY_CREATE(QString, appname)
private:
    explicit AppInfo(QObject *parent = nullptr);
public:
    Q_SINGLETON_CREATE(AppInfo)
    // Q_HIDE_CONSTRUCTOR(AppInfo)
};

#endif // APPINFO_H
