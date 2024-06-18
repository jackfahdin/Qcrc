#include "Log.h"
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QGuiApplication>
#include <iostream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QThread>
#include <QSettings>
#include <QRegularExpression>
#include <InfoVersion.h>

#ifdef WIN32

#include <process.h>

#else
#include <unistd.h>
#endif

#ifndef QT_ENDL
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#    define QT_ENDL Qt::endl
#  else
#    define QT_ENDL endl
#  endif
#endif

static QString g_app = {};
static QString g_file_path = {};
static bool g_logError = false;

static std::unique_ptr<QFile> g_logFile = nullptr;
static std::unique_ptr<QTextStream> g_logStream = nullptr;

static int g_logLevel = 4;

std::map<QtMsgType, int> logLevelMap = {
        {QtFatalMsg,    0},
        {QtCriticalMsg, 1},
        {QtWarningMsg,  2},
        {QtInfoMsg,     3},
        {QtDebugMsg,    4}
};

QString Log::prettyProductInfoWrapper() {
    auto productName = QSysInfo::prettyProductName();
#if defined(Q_OS_WIN)
    QSettings regKey{QString::fromUtf8(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion)"), QSettings::NativeFormat};
    if (regKey.contains(QString::fromUtf8("CurrentBuildNumber"))) {
        auto buildNumber = regKey.value(QString::fromUtf8("CurrentBuildNumber")).toInt();
        if (buildNumber > 0) {
            if (buildNumber < 9200) {
                productName = QString::fromUtf8("Windows 7 build %1").arg(buildNumber);
            } else if (buildNumber < 10240) {
                productName = QString::fromUtf8("Windows 8 build %1").arg(buildNumber);
            } else if (buildNumber < 22000) {
                productName = QString::fromUtf8("Windows 10 build %1").arg(buildNumber);
            } else {
                productName = QString::fromUtf8("Windows 11 build %1").arg(buildNumber);
            }
        }
    }
#endif
    return productName;
}

static inline void messageHandler(const QtMsgType type, const QMessageLogContext &context, const QString &message) {
    if (message == "Could not get the INetworkConnection instance for the adapter GUID.") {
        return;
    }
    if (logLevelMap[type] > g_logLevel) {
        return;
    }
    if (!message.isEmpty()) {
        QString levelName;
        switch (type) {
            case QtDebugMsg:
                levelName = QStringLiteral("Debug");
                break;
            case QtInfoMsg:
                levelName = QStringLiteral("Info");
                break;
            case QtWarningMsg:
                levelName = QStringLiteral("Warning");
                break;
            case QtCriticalMsg:
                levelName = QStringLiteral("Critical");
                break;
            case QtFatalMsg:
                levelName = QStringLiteral("Fatal");
                break;
        }
        QString fileAndLineLogStr;
        if (context.file) {
            std::string strFileTmp = context.file;
            const char *ptr = strrchr(strFileTmp.c_str(), '/');
            if (nullptr != ptr) {
                char fn[512] = {0};
                sprintf_s(fn, sizeof(fn), "%s", ptr + 1);
                strFileTmp = fn;
            }
            const char *ptrTmp = strrchr(strFileTmp.c_str(), '\\');
            if (nullptr != ptrTmp) {
                char fn[512] = {0};
                sprintf_s(fn, sizeof(fn), "%s", ptrTmp + 1);
                strFileTmp = fn;
            }
            fileAndLineLogStr = QString::fromStdString("[%1:%2]").arg(QString::fromStdString(strFileTmp), QString::number(context.line));
        }
        const QString finalMessage = QString::fromStdString("%1[%2]%3[%4]:%5").arg(
                QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz"),
                levelName,
                fileAndLineLogStr,
                QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId())),
                message);
        if ((type == QtInfoMsg) || (type == QtDebugMsg)) {
            std::cout << qPrintable(finalMessage) << std::endl;
        } else {
            std::cerr << qPrintable(finalMessage) << std::endl;
        }
        if (g_logError) {
            return;
        }
        if (!g_logFile) {
            g_logFile = std::make_unique<QFile>(g_file_path);
            if (!g_logFile->open(QFile::WriteOnly | QFile::Text | QFile::Append)) {
                std::cerr << "Can't open file to write: " << qPrintable(g_logFile->errorString()) << std::endl;
                g_logFile.reset();
                g_logError = true;
                return;
            }
        }
        if (!g_logStream) {
            g_logStream = std::make_unique<QTextStream>();
            g_logStream->setDevice(g_logFile.get());
        }
        (*g_logStream) << finalMessage << QT_ENDL;
        g_logStream->flush();
    }
}

void Log::setup(char *argv[], const QString &app, int level) {
    Q_ASSERT(!app.isEmpty());
    if (app.isEmpty()) {
        return;
    }
    g_logLevel = level;
    static bool once = false;
    if (once) {
        return;
    }
    QString applicationPath = QString::fromStdString(argv[0]);
    once = true;
    g_app = app;
    const QString logFileName = QString("%1_%2.log").arg(g_app, QDateTime::currentDateTime().toString("yyyyMMdd"));
#if 0
    const QString logDirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/log";
#else
    // 获取应用程序目录路径
    QFileInfo appFileInfo(applicationPath);
    const QString appDirPath = appFileInfo.absolutePath();
    // 设置日志目录为应用程序目录下的log子目录
    const QString logDirPath = appDirPath + "/log";
#endif
    const QDir logDir(logDirPath);
    if (!logDir.exists()) {
        logDir.mkpath(logDirPath);
    }
    g_file_path = logDir.filePath(logFileName);
    qInstallMessageHandler(messageHandler);
    qInfo() << "===================================================";
    qInfo() << "[AppName]" << g_app;
    qInfo() << "[AppVersion]" << APPLICATION_VERSION;
    qInfo() << "[AppPath]" << applicationPath;
    qInfo() << "[QtVersion]" << QT_VERSION_STR;
#ifdef WIN32
    qInfo() << "[ProcessId]" << QString::number(_getpid());
#else
    qInfo()<<"[ProcessId]"<<QString::number(getpid());
#endif
    qInfo() << "[GitHashCode]" << COMMIT_HASH;
    qInfo() << "[DeviceInfo]";
    qInfo() << "[DeviceId]" << QSysInfo::machineUniqueId();
    qInfo() << "[Manufacturer]" << prettyProductInfoWrapper();
    qInfo() << "[CPU_ABI]" << QSysInfo::currentCpuArchitecture();
    qInfo() << "[LOG_LEVEL]" << g_logLevel;
    qInfo() << "[LOG_PATH]" << g_file_path;
    qInfo() << "===================================================";
}
