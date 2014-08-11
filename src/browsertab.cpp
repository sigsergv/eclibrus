/*  
 * browsertab.cpp
 *  
 * Created on: Dec 28, 2011
 * Author: Sergey Stolyarov
 */ 

#include <QtWidgets>
#include <QtWebKit>
#include <QWebFrame>
#include <QtDebug>

#include "browsertab.h"
#include "eclibwebview.h"
#include "db.h"
#include "librarynam.h"
#include "settings.h"
#include "devices.h"
#include "mainwindow.h"
#include "eclibrenderers.h"
#include "exportbooksprogress.h"
#include "fb2/fb2.h"

struct BrowserTab::Private
{
    QToolButton * histBackButton;
    QToolButton * histForwardButton;
    QLineEdit * searchTextEntry;
    QToolButton * startSearchButton;
    EclibWebView * view;
    Eclibrus::DeviceInfo deviceToExport();
};

/*
 * select device to export books, if there is just one connected
 * and registered device then return it, otherwise display device selector
 * window.
 */
Eclibrus::DeviceInfo BrowserTab::Private::deviceToExport()
{
    Eclibrus::DeviceInfo di;

    QList<Eclibrus::DeviceInfo> devices = Eclibrus::connectedRegisteredDevices();
    if (devices.size() == 0) {
        // no connected devices
        QMessageBox::warning(MainWindow::inst(), tr("Warning"), 
            tr("Please connect external device, it's required for this action."));
        return di;
    } else if (devices.size() > 1) {
        // there are more than one connected registered device,
        // so we must to decide where do we want export books.
        // TODO: display device selector
        QMessageBox::warning(MainWindow::inst(), tr("Warning"), 
            tr("More than one registered device connected, that's not yet supported, sorry."));
        return di;
    } 
    
    return devices[0];
}

BrowserTab::BrowserTab(QWidget * parent) :
    QWidget(parent)

{
    /*
     * Browser tab contains top toolbar with search and navigation widgets, 
     * and also search results widget (QtWebkit)
     */
    p = new Private();

    p->histBackButton = new QToolButton(this);
    p->histForwardButton = new QToolButton(this);
    p->startSearchButton = new QToolButton(this);
    p->searchTextEntry = new QLineEdit(this);
#if QT_VERSION >= 0x040700
    p->searchTextEntry->setPlaceholderText(tr("Type any text here and press [Enter] to start search"));
#endif
    p->view = new EclibWebView(this);
    p->histBackButton->setDefaultAction(p->view->pageAction(QWebPage::Back));
    p->histForwardButton->setDefaultAction(p->view->pageAction(QWebPage::Forward));
    p->startSearchButton->setText(tr("Search"));

    p->startSearchButton->setFocusProxy(p->searchTextEntry);
    p->startSearchButton->setFocusPolicy(Qt::ClickFocus);
    p->histBackButton->setFocusProxy(p->searchTextEntry);
    p->histBackButton->setFocusPolicy(Qt::ClickFocus);
    p->histForwardButton->setFocusProxy(p->searchTextEntry);
    p->histForwardButton->setFocusPolicy(Qt::ClickFocus);

    p->view->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
    QNetworkAccessManager * oldManager = p->view->page()->networkAccessManager();
    Eclibrus::LibraryNAM * newManager = new Eclibrus::LibraryNAM(oldManager, this);
    p->view->page()->setNetworkAccessManager(newManager);

    QLayout * topLayout = new QHBoxLayout;
    topLayout->addWidget(p->histBackButton);
    topLayout->addWidget(p->histForwardButton);
    topLayout->addWidget(p->searchTextEntry);
    topLayout->addWidget(p->startSearchButton);
    topLayout->setAlignment(p->histBackButton, Qt::AlignLeft);
    
    QLayout * layout = new QVBoxLayout(this);
    layout->addItem(topLayout);
    layout->setAlignment(topLayout, Qt::AlignTop);
    layout->addWidget(p->view);

    // connect signals
    connect(p->view, SIGNAL(linkClicked(const QUrl &)),
        this, SLOT(linkClicked(const QUrl &)));
    connect(p->view, SIGNAL(linkMiddleClicked(const QUrl &)),
        this, SLOT(linkMiddleClicked(const QUrl &)));
    connect(p->searchTextEntry, SIGNAL(returnPressed()),
        this, SLOT(startSearch()));
    connect(p->view, SIGNAL(bookDownloadRequested(int)),
        this, SLOT(downloadBook(int)));
    connect(p->startSearchButton, SIGNAL(clicked()),
        this, SLOT(startSearch()));
    
    setFocusProxy(p->searchTextEntry);
}

BrowserTab::~BrowserTab()
{
    delete p;
    p = 0;
}

void BrowserTab::exportBookToDevice(const Eclibrus::DeviceInfo & di, int bookId)
{
    QDir target_dir(di.mountPoint);
    QWidget * mbb = MainWindow::inst();
    QDate d = QDate::currentDate();
    QString subdir = d.toString("yyyy-MM-dd");
    
    QString device_lib_dir = Eclibrus::prepareLibraryDir(di, subdir);
    if (device_lib_dir.isEmpty()) {
        QMessageBox::warning(mbb, tr("Warning"),
            tr("Cannot export book to device's library directory."));
        return;
    }

    QList<int> books;
    books << bookId;

    ExportBooksProgress dlg(books, di, device_lib_dir, mbb);
    dlg.exec();
}

void BrowserTab::exportAllBooksFromPageToDevice(const Eclibrus::DeviceInfo & di)
{
    QDir target_dir(di.mountPoint);
    QWidget * mbb = MainWindow::inst();
    QDate d = QDate::currentDate();
    QString subdir = d.toString("yyyy-MM-dd");
    
    QString device_lib_dir = Eclibrus::prepareLibraryDir(di, subdir);
    if (device_lib_dir.isEmpty()) {
        QMessageBox::warning(mbb, tr("Warning"),
            tr("Cannot export book to device's library directory."));
        return;
    }

    QString html = p->view->page()->mainFrame()->toHtml();
    QString ns = Eclibrus::Config::extUrlNamespace();
    ns = ns.replace(".", "\\.");
    QRegExp re(QString("<a href=\"http://%1/save-to-device\\?id=([0-9]+)\">")
        .arg(ns));
    
    int pos = 0;
    QList<int> books;
    while ((pos=re.indexIn(html, pos)) != -1) {
        books << re.cap(1).toInt();
        pos += re.matchedLength();
    }
    ExportBooksProgress dlg(books, di, device_lib_dir, mbb);
    dlg.exec();
}

void BrowserTab::setUrl(const QUrl & url)
{
    p->view->setUrl(url);
}

void BrowserTab::linkClicked(const QUrl & url)
{
    if (url.host() != Eclibrus::Config::extUrlNamespace()) {
        // handle only trusted, known URLs
        // We are treating some external URLs as internal commands
        return;
    }
    QUrlQuery urlQuery(url);
    QString method = url.path();
    method.remove(0,1);
    qDebug() << "method:" << method;

    if (method == "read-book-now") {
        // we want to READ book, so find it, extract from the library, save to temporary location
        // and pass full path to some external application
        QString reader_executable = Eclibrus::Config::fb2ReaderProgram();
        // first expand "reader_executable" to complete path using env var PATH
#ifdef Q_OS_UNIX
        QString path_env_var = qgetenv("PATH");
        QDir dir;
        QString filename;
        QFileInfo fi;
        QString resolved_executable;

        foreach (const QString & d, path_env_var.split(":")) {
            dir.setPath(d);
            filename = dir.absoluteFilePath(reader_executable);
            fi.setFile(filename);
            if (fi.isExecutable()) {
                resolved_executable = fi.canonicalFilePath();
                break;
            }
        }
        
        if (!resolved_executable.isEmpty()) {
            // executable is found so we can export book now
            int id = urlQuery.queryItemValue("id").toInt();
            QString cache_path = Eclibrus::Config::exportedFb2CachePath();
            QPair<QString, QString> pair = Eclibrus::Db::archivedBookFile(id);
            QString archive = pair.first;
            QString book_file = pair.second;
            QString library_path = Eclibrus::Config::librusecLibraryPath() + QDir::separator();
            QString archive_path = library_path + archive;
            QString output_path = cache_path + QDir::separator() + book_file + ".zip";
            fi.setFile(output_path);
            if (!fi.exists()) {
                FB2::exportBookToArchive(archive_path, book_file, output_path);
            }
            fi.refresh();
            if (fi.exists()) {
                // launch executable
                QStringList args;
                args << output_path;
                QProcess::startDetached(resolved_executable, args);
            }
        }

#endif
    } else if (method == "save-to-device") {
        Eclibrus::DeviceInfo di = p->deviceToExport();
        if (!di.isEmpty()) {
            int id = urlQuery.queryItemValue("id").toInt();
            exportBookToDevice(di, id);
        }
    } else if (method == "export-all-page-books") {
        // get page HTML, extract all book ids, pass
        Eclibrus::DeviceInfo di = p->deviceToExport();
        if (!di.isEmpty()) {
            exportAllBooksFromPageToDevice(di);
        }
    } else if (method == "download-book") {
        int id = urlQuery.queryItemValue("id").toInt();
        downloadBook(id);
    }
}

void BrowserTab::linkMiddleClicked(const QUrl & url)
{
    MainWindow::inst()->newBrowserTab(url);
}

void BrowserTab::downloadBook(int bookId)
{
    // display Save dialog
    QString last_path = Eclibrus::Config::saveBookLastPath();
    QFileDialog save_dialog;
    QPair<QString, QString> pair = Eclibrus::Db::archivedBookFile(bookId);
    QString basename = Eclibrus::Plain::bookFileName(bookId);
#ifdef Q_OS_MAC
    QString filename = basename;
#else
    QString filename = basename + ".fb2.zip";
#endif
    qDebug() << "filename" << filename;
    QStringList name_filters;

    if (pair.first.isEmpty()) {
        QMessageBox::warning(MainWindow::inst(), tr("Warning"), tr("Cannot find archive in the library database."));
        return;
    }

    name_filters << tr("FictionBook2 files (*.fb2.zip) (*.fb2.zip)");
    save_dialog.setNameFilters(name_filters);
    save_dialog.setDirectory(last_path);
    save_dialog.setAcceptMode(QFileDialog::AcceptSave);
    save_dialog.setConfirmOverwrite(true);
    //save_dialog.setFileMode(QFileDialog::ExistingFile);
    save_dialog.selectFile(filename);
    int res = save_dialog.exec();

    if (QDialog::Accepted == res) {
        Eclibrus::Config::setSaveBookLastPath(save_dialog.directory().canonicalPath());
        QStringList selected = save_dialog.selectedFiles();
        if (selected.size() > 0) {
            QString archive = pair.first;
            QString book_file = pair.second;
            QString library_path = Eclibrus::Config::librusecLibraryPath() + QDir::separator();
            QString archive_path = library_path + archive;
            QString output_path = selected[0];

            FB2::exportBookToArchive(archive_path, book_file, output_path);
        }
    }
}

void BrowserTab::startSearch()
{
    QString text = p->searchTextEntry->text();
    if (0 == text.size()) {
        return;
    }
    QUrl url;
    QUrlQuery urlQuery;
    if (text.startsWith("eclib:")) {
        // consider as URL
        url.setUrl(text);
    }

    if (!url.isValid() || url.scheme() != "eclib") {
        url.setUrl("eclib:search/plain");
        urlQuery.addQueryItem("q", text);
        url.setQuery(urlQuery);
    }
    p->view->load(url);
}
