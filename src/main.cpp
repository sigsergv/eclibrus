/*
 * main.cpp
 *
 * Created on: Dec 27, 2011
 * Author: Sergei Stolyarov
 */

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

#include <QtWidgets>
#include <QtDebug>

#include "settings.h"
#include "mainwindow.h"
#include "db.h"
#include "devices.h"
#include "mountnotifier.h"


int main(int argv, char *args[])
{
    // init settings
    Eclibrus::Config::settings();

    //QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QApplication app(argv, args);

    // load localization
    QTranslator translator;
    translator.load("eclibrus_" + Eclibrus::Config::uiLang(), Eclibrus::Config::uiLangsPath());
    app.installTranslator(&translator);

    // init mount notifier
    MountNotifier mn;

    app.setQuitOnLastWindowClosed(true);
    QSqlError se = Eclibrus::Db::init();
    if (se.type() != QSqlError::NoError) {
        qCritical() << "database connection error:" << se.databaseText() << "\n";
        qCritical() << se.driverText();
        exit(1);
    }

    MainWindow * win = MainWindow::inst();
    QApplication::setActiveWindow(win);
    win->show();

    return app.exec();    
}
