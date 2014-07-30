/*
 * settings.h
 *
 * Created on: Dec 27, 2011
 * Author: Sergei Stolyarov
 */
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QString>
#include <QChar>
#include <QList>

class QSettings;
class QWidget;

namespace Eclibrus
{
namespace Config
{
    QSettings * settings();
    QString version();
    QString uiLang();
    QString uiLangsPath();
    QString profilePath();
    QString librusecLibraryPath();
    void setLibrusecLibraryPath(const QString & path);
    QString fb2ReaderProgram();
    void setFb2ReaderProgram(const QString & path);
    bool showDownloadIcon();
    void setShowDownloadIcon(bool show);
    QString coversCachePath();
    QString exportedFb2CachePath();
    QString pagesCachePath();
    QString filenameInProfile(const QString & filename);
    QString extUrlNamespace();

    /**
     * Directory on device where books are stored
     */
    QString deviceLibraryPath();
    QString saveBookLastPath();
    void setSaveBookLastPath(const QString & path);
    int maxCoverPreviewWidth();
    int itemsPerPage();
}
}

#endif /* SETTINGS_H_ */

