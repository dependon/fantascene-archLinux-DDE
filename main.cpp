#include <QObject>

#include "application.h"
#include "wallpaper.h"
#include "dbuswallpaperservice.h"
#include "settings.h"
#include <malloc.h>
#include "settingwindow.h"
#include <DMainWindow>
#include <QProcess>
#include <QThread>
#include <QDesktopWidget>
#include <QDebug>
#include <QCoreApplication>
#include <DDesktopEntry>
#include <QFile>
#include <DTitlebar>
#include <QStandardPaths>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
void cpToTmp()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/deepin-dreamscene/";
    QDir dir(path);
    if (!dir.exists()) {
        QDir dir1;
        dir1.mkpath(path);
    }
}
int main(int argc, char *argv[])
{
    cpToTmp();
    QString path = "/opt/durapps/fantascene-dynamic-wallpaper/";

    mallopt(M_ARENA_MAX, 1);

    Application a(argc, argv);
    a.loadTranslator();
    a.setApplicationVersion(DApplication::buildVersion("1.0.0"));
//    a.setTheme("light");
    setlocale(LC_NUMERIC, "C");

    if (a.setSingleInstance("fantascene-dynamic-wallpaper")) {
        bool isShowMainWindow = true;
#ifdef QT_NO_DEBUG
        isShowMainWindow = false;
        if (QFileInfo(path + "dde-desktop").isFile() && !QFileInfo(path + "dde-desktop").isExecutable()) {
            int iRet = QProcess::execute("pkexec chmod 777 " + path + "dde-desktop " + path + "config.ini");
            if (iRet != 0) {
                return 0;
            }
            isShowMainWindow = true;
        } else {
            qDebug() << "可以启动: " << path + "dde-desktop";
        }

        dApp->m_startDesktop  = QThread::create([ = ]() {
            //打印当前路径
            qDebug() << "打印当前路径1";
            qDebug() << QCoreApplication::applicationDirPath();
            qDebug() << "打印当前路径2";

            QProcess::execute("killall dde-desktop");
            qDebug() << "关闭原生dde-desktop";
            qDebug() << "loading new dde-desktop";
            dApp->m_startDesktopProcess = new QProcess(dApp);
            if (QFileInfo(path + "dde-desktop").isFile()) {

                dApp->m_startDesktopProcess->start("bash /opt/durapps/fantascene-dynamic-wallpaper/startdesktop.sh");
                dApp->m_processId = dApp->m_startDesktopProcess->processId();
                qDebug() << "processId" << dApp->m_processId;
                dApp->m_startDesktopProcess->waitForFinished();
            }
            qDebug() << "启动失败: " << path + "dde-desktop";
        });
        dApp->m_startDesktop->start();
#endif

        DMainWindow *mainwindw = new DMainWindow();
        settingWindow *window = new settingWindow(mainwindw, mainwindw);
        mainwindw->setCentralWidget(window);
        int index = 0;
        for (const QString &arg : qApp->arguments()) {
            if (arg == "min") {
                index++;
            }
        }
        if (index == 0 && isShowMainWindow) {
            mainwindw->show();
        }
        mainwindw->setFixedSize(QSize(640, 500));
        mainwindw->setWindowTitle("动态壁纸");
        mainwindw->setWindowIcon(QIcon(":/install/wallpaper.png"));
        mainwindw->titlebar()->setIcon(QIcon(":/install/wallpaper.png"));
        mainwindw->titlebar()->setWindowTitle("动态壁纸");

        mainwindw->move(qApp->desktop()->screen()->rect().center() - mainwindw->rect().center());

        Wallpaper *w = new Wallpaper(window->getCurrentPath(), window->getCurrentNumber());
        dApp->setDesktopTransparent();
//        qDebug() << qApp->desktop()->screenNumber();
//        qDebug() << qApp->desktop()->primaryScreen();
//        qDebug() << qApp->desktop()->screenCount();

        DBusWallpaperService *dbusInter = new DBusWallpaperService(w);
//        Q_UNUSED(dbusInter);

        QDBusConnection::sessionBus().registerService("com.deepin.dde.fantascene");
        QDBusConnection::sessionBus().registerObject("/com/deepin/dde/fantascene", "com.deepin.dde.fantascene", w);

        QString envName("DDE_SESSION_PROCESS_COOKIE_ID");

        QByteArray cookie = qgetenv(envName.toUtf8().data());
        qunsetenv(envName.toUtf8().data());

        if (!cookie.isEmpty()) {
            QDBusInterface iface("com.deepin.SessionManager",
                                 "/com/deepin/SessionManager",
                                 "com.deepin.SessionManager",
                                 QDBusConnection::sessionBus());
            iface.asyncCall("Register", QString(cookie));
        }


    } else {
        QDBusInterface iface("com.deepin.dde.fantascene",
                             "/com/deepin/dde/fantascene",
                             "com.deepin.dde.fantascene",
                             QDBusConnection::sessionBus());
        iface.asyncCall("activeWindow");
        return 0;
    }

    return a.exec();
}


