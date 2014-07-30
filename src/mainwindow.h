/*
 * mainwindow.h
 *
 * Created on: Dec 27, 2011
 * Author: Sergei Stolyarov
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include <QUrl>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    static MainWindow * inst();

private:
    MainWindow();
    static MainWindow * instance;
    struct Private;
    Private * p;

protected:
    void rememberGeometryAndState();
    void moveEvent(QMoveEvent * event);
    void resizeEvent(QResizeEvent * event);
    void closeEvent(QCloseEvent *event);

public slots:
    void show();
    void newBrowserTab(const QUrl & url = QUrl());

protected slots:
    void closeTab(int index);
    void tabChanged(int index);
    void showAboutDialog();
    void showDevicesConfigDialog();
    void showPreferencesDialog();
    void manageBooksOnDevice();
    void showLibrarySummary();
    void showGenresSummary();
};

#endif /* MAINWINDOW_H_ */
