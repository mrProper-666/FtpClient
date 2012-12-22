#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QFtp>
#include <QtDebug>
#include <QTreeWidget>
#include <QProgressDialog>
#include <QFile>
#include <QMessageBox>
#include <QMenu>
#include <QSettings>
#include <QKeyEvent>

#include "localmodel.h"

enum LOG_STATUS{
    error,
    success,
    neutral
};

typedef struct {
    QString host;
    QString user;
    QString psswd;
    int port;
} sessions;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_localTree_clicked(const QModelIndex &index);
    void on_connectButton_clicked();
    void stateChangedSlot(int);
    void contentList(QUrlInfo info);
    void processItem(QTreeWidgetItem *item, int /*column*/);
    void on_cdUp_clicked();
    void on_serverWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void cancelDownload();
    void updateDataTransferProgress(qint64 readBytes, qint64 totalBytes);
    void openMenusItem();

    void on_localList_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    localModel *CLModel;
    QFtp ftp;
    QHash<QString, bool> isDirectory;
    QString currentPath;
    QProgressDialog *progressDialog;
    QFile *file;
    QMenu *menu;
    QHash<QString,sessions> sessionsList;

    void addLogString(QString string, LOG_STATUS status = neutral);
    void populateMenusBtn();
    void addMenusItem(QString host, QString user, QString psswd, int port);

protected:
    void keyPressEvent(QKeyEvent * ev);

signals:
};

#endif // MAINWINDOW_H
