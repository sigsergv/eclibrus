/*
 * settings.cpp
 *
 * Created on: Dec 27, 2011
 * Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtDebug>
#include <QWebSecurityOrigin>

#include "settings.h"

static QSettings * _settings = 0;
static QString uiLangsPath;

namespace Eclibrus
{
namespace Config
{
    
    QSettings * settings()
    {
        if (0 == ::_settings) {
            // init settings on first call
            QCoreApplication::setApplicationName("Eclibrus");
            QCoreApplication::setApplicationVersion(ECLIBRUS_VERSION);
            QCoreApplication::setOrganizationName("regolit.com");
            QCoreApplication::setOrganizationDomain("eclibrus.regolit.com");
            QWebSecurityOrigin::addLocalScheme("eclib");

            ::_settings = new QSettings(profilePath()+"/eclibrus.ini", QSettings::IniFormat);

            // TODO: really clean local cover images cache?
            QString covers = coversCachePath();
            if (!covers.isEmpty()) {
                QDir d(covers);
                if (d.exists()) {
                    foreach (const QString & fn, d.entryList(QDir::Files)) {
                        d.remove(fn);
                    }
                }
            }
        }
        
        return ::_settings; 
    }

    QString uiLang()
    {
        QString lang = "en";
        QString langEnv = qgetenv("LANG");

        if (langEnv.contains("ru_RU")) {
            lang = "ru";
        }

        return lang;
    }

    QString uiLangsPath()
    {
        // first check for local paths
        QString localPath = QCoreApplication::applicationDirPath() + QDir::separator() + "translations";
        QDir d(localPath);
        QStringList files = d.entryList(QDir::Files);
        foreach (QString f, files) {
            if (f.endsWith(".qm")) {
                ::uiLangsPath = localPath;
                break;
            }
        }
        
        // find directory with translations
        if (::uiLangsPath.isEmpty()) {
#ifdef Q_OS_UNIX
            // check standard dirs
            QStringList checkPaths;
            checkPaths << "/usr/share/eclibrus/translations/";

            foreach (QString path, checkPaths) {
                QDir d(path);
                bool found = false;
                if (d.exists()) {
                    // check for *.qm files there
                    QStringList files = d.entryList(QDir::Files);
                    foreach (QString f, files) {
                        if (f.endsWith(".qm")) {
                            ::uiLangsPath = path;
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        break;
                    }
                }
            }
#endif
        }
        return ::uiLangsPath;
    }

    QString profilePath()
    {
        QDir dir;

        QString path = QDir::homePath() + "/.eclibrus";
        if (!dir.exists(path) && !dir.mkpath(path)) {
            // TODO: do something if it's not possible to create new directory
            return QString();
        }

        return path;
    }

    QString librusecLibraryPath()
    {
        QSettings * settings = ::Eclibrus::Config::settings();
        QString path = settings->value("Library/librusec_archives_path", "").toString();
        QDir dir(path);

        if (!dir.isAbsolute() || !dir.exists()) {
            path = "";
        } else {
            path = dir.canonicalPath();
        }

        return path;
    }

    void setLibrusecLibraryPath(const QString & path)
    {
        
        QSettings * settings = ::Eclibrus::Config::settings();
        settings->setValue("Library/librusec_archives_path", path);
    }

    bool showDownloadIcon()
    {
        QSettings * settings = ::Eclibrus::Config::settings();
        bool show = settings->value("MainWindow/show_download_icon").toBool();

        return show;
    }

    void setShowDownloadIcon(bool show)
    {
        QSettings * settings = ::Eclibrus::Config::settings();
        settings->setValue("MainWindow/show_download_icon", show);
    }

    QString fb2ReaderProgram()
    {
        QSettings * settings = ::Eclibrus::Config::settings();
        QString path = settings->value("Library/fb2_reader_program", "").toString();
        return path;
    }

    void setFb2ReaderProgram(const QString & path)
    {
        QSettings * settings = ::Eclibrus::Config::settings();
        settings->setValue("Library/fb2_reader_program", path);
    }

    QString _cachePath(const QString & dirname)
    {
        QDir profileDir(profilePath());

        QString path = filenameInProfile(dirname);
        QFileInfo fi(path);

        if (fi.isFile()) {
            // try to remove, we don't need a filename with the same name here
            if (!profileDir.remove(dirname)) {
                return QString();
            }
        }

        if (!fi.exists()) {
            if (!profileDir.mkdir(dirname)) {
                return QString();
            }
        }

        return path;
    }

    QString coversCachePath() 
    {
        return _cachePath("covers-cache");
    }

    QString exportedFb2CachePath()
    {
        return _cachePath("fb2zip-cache");
    }

    QString pagesCachePath() 
    {
        return _cachePath("pages-cache");
    }

    QString filenameInProfile(const QString & filename)
    {
        QString profile = profilePath();
        QString fullFilename = profile + "/" + filename;
        QDir dir(fullFilename);

        return dir.canonicalPath();
    }

    QString extUrlNamespace()
    {
        return "eclib.ns.regolit.com";
    }

    QString deviceLibraryPath()
    {
        return "Books";
    }

    QString saveBookLastPath()
    {
        QSettings * settings = ::Eclibrus::Config::settings();
        QString path = settings->value("MainWindow/saveBookLastPath", "").toString();
        QFileInfo fi(path);

        if (path.isEmpty() || !fi.isDir() || !fi.exists()) {
            // use home directory
            return QDir::homePath();
        }
        return path;
    }

    void setSaveBookLastPath(const QString & path)
    {
        QSettings * settings = ::Eclibrus::Config::settings();
        QFileInfo fi(path);
        if (!path.isEmpty() && fi.isDir() && fi.exists()) {
            settings->setValue("MainWindow/saveBookLastPath", path);
        }
    }

    int maxCoverPreviewWidth()
    {
        return 200;
    }

    int itemsPerPage()
    {
        return 100;
    }
}
}

