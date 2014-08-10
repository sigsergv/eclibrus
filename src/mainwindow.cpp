/*
 * mainwindow.cpp
 *
 * Created on: Dec 27, 2011
 * Author: Sergey Stolyarov
 */

#include <QtWidgets>
#include <QtDebug>

#include "mainwindow.h"
#include "settings.h"
#include "browsertab.h"
#include "aboutdialog.h"
#include "devicesconfigdialog.h"
#include "preferencesdialog.h"
#include "managebooksondevicedialog.h"

//#include "ui_mainwindow.h"

MainWindow * MainWindow::instance = 0;

struct MainWindow::Private
{
    QTabWidget * tabs;
};

MainWindow::MainWindow() :
    QMainWindow()
{
    p = new Private();

    p->tabs = new QTabWidget(this);
    p->tabs->setTabsClosable(true);
    p->tabs->setFocusPolicy(Qt::StrongFocus);
    p->tabs->setContentsMargins(0, 0, 0, 0);
    p->tabs->setDocumentMode(true);
    setCentralWidget(p->tabs);

    setWindowIcon(QIcon(":/eclibrus-32.png"));
    
    QAction *newBrowserTabAction = new QAction(tr("&New browser tab"), this);
    newBrowserTabAction->setShortcut(Qt::Key_T + Qt::CTRL);
    QAction * prefsAction = new QAction(tr("&Preferences"), this);
    prefsAction->setMenuRole(QAction::PreferencesRole);
    prefsAction->setShortcut(Qt::Key_P + Qt::CTRL);
    QAction * quitAction = new QAction(tr("&Quit"), this);
    quitAction->setMenuRole(QAction::QuitRole);
    QAction * devicesConfigAction = new QAction(tr("&Manage removable devices"), this);
    devicesConfigAction->setShortcut(Qt::Key_D + Qt::CTRL);
    QAction * manageBooksOnDeviceAction = new QAction(tr("Manage &books on device"), this);
    manageBooksOnDeviceAction->setShortcut(Qt::Key_B + Qt::CTRL);
    QAction * librarySummaryInfoAction = new QAction(tr("Library &summary"), this);
    QAction * genresSummaryInfoAction = new QAction(tr("&Genres summary"), this);
    QAction * aboutAction = new QAction(tr("&About EcLibRus"), this);
    aboutAction->setMenuRole(QAction::AboutRole);

    // connect actions
    connect(p->tabs, SIGNAL(tabCloseRequested(int)),
        this, SLOT(closeTab(int)));
    connect(p->tabs, SIGNAL(currentChanged(int)),
        this, SLOT(tabChanged(int)));
    connect(newBrowserTabAction, SIGNAL(triggered()),
        this, SLOT(newBrowserTab()));
    connect(aboutAction, SIGNAL(triggered()),
        this, SLOT(showAboutDialog()));
    connect(devicesConfigAction, SIGNAL(triggered()),
        this, SLOT(showDevicesConfigDialog()));
    connect(prefsAction, SIGNAL(triggered()),
            this, SLOT(showPreferencesDialog()));
    connect(quitAction, SIGNAL(triggered()),
        this, SLOT(close()));
    connect(manageBooksOnDeviceAction, SIGNAL(triggered()),
        this, SLOT(manageBooksOnDevice()));
    connect(librarySummaryInfoAction, SIGNAL(triggered()),
        this, SLOT(showLibrarySummary()));
    connect(genresSummaryInfoAction, SIGNAL(triggered()),
        this, SLOT(showGenresSummary()));

    QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newBrowserTabAction);
    fileMenu->addAction(prefsAction);
    fileMenu->addAction(quitAction);
    QMenu * devicesMenu = menuBar()->addMenu(tr("&Devices"));
    devicesMenu->addAction(devicesConfigAction);
    devicesMenu->addAction(manageBooksOnDeviceAction);
    QMenu * infoMenu = menuBar()->addMenu(tr("&Information"));
    infoMenu->addAction(librarySummaryInfoAction);
    infoMenu->addAction(genresSummaryInfoAction);
    QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);

    QSettings * settings = Eclibrus::Config::settings();

    // restore window settings
    settings->beginGroup("MainWindow");
    restoreGeometry(settings->value("geometry").toByteArray());
    restoreState(settings->value("state").toByteArray());
    settings->endGroup();

}

MainWindow * MainWindow::inst()
{
    if (instance == 0) {
        instance = new MainWindow;
    }
    return instance;
}

void MainWindow::rememberGeometryAndState()
{
    QSettings * settings = Eclibrus::Config::settings();
    settings->beginGroup("MainWindow");
    settings->setValue("geometry", saveGeometry());
    settings->setValue("state", saveState());
    settings->endGroup();
}

void MainWindow::moveEvent(QMoveEvent * event)
{
    rememberGeometryAndState();
    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent * event)
{
    rememberGeometryAndState();
    event->accept();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings * settings = Eclibrus::Config::settings();

    // write layout settings and sync
    rememberGeometryAndState();
    settings->sync();
    event->accept();
}

void MainWindow::newBrowserTab(const QUrl & url)
{
    BrowserTab * tab = new BrowserTab(this);
    if (url.isEmpty()) {
#ifdef QT_NO_DEBUG
        tab->setUrl(QUrl("qrc:/index.ru.html"));
#else
        tab->setUrl(QUrl("eclib:author?id=10536"));
        //tab->setUrl(QUrl("eclib:book?id=114532"));
#endif
    } else {
        tab->setUrl(url);
    }
    p->tabs->addTab(tab, tr("Library browser"));
    // move focus to new tab search box
    p->tabs->setCurrentWidget(tab);
}

void MainWindow::closeTab(int index)
{
    qDebug() << "Close tab" << index;
    QWidget * tab = p->tabs->widget(index);
    if (!tab) {
        return;
    }
    delete tab;
    p->tabs->removeTab(index);
}

void MainWindow::tabChanged(int index)
{
    QWidget * tab = p->tabs->widget(index);
    if (tab == 0) {
        return;
    }

    tab->setFocus();
}

void MainWindow::show()
{
    // create new browser tab
    newBrowserTab();
    QMainWindow::show();
}

void MainWindow::showAboutDialog()
{
    AboutDialog dlg;
    dlg.exec();
}

void MainWindow::showDevicesConfigDialog()
{
    DevicesConfigDialog dlg(this);
    dlg.exec();
}

void MainWindow::showPreferencesDialog()
{
    PreferencesDialog dlg(this);
    dlg.exec();
}

void MainWindow::manageBooksOnDevice()
{
    ManageBooksOnDeviceDialog dlg;
    dlg.exec();
}

void MainWindow::showLibrarySummary()
{
    // create new tab and load special page there
    newBrowserTab(QUrl("eclib:info/library-summary"));
}

void MainWindow::showGenresSummary()
{
    newBrowserTab(QUrl("eclib:info/genres-summary"));
}
