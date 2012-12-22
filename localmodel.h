#ifndef LOCALMODEL_H
#define LOCALMODEL_H

#include <QObject>
#include <QDir>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QDateTime>
#include <QString>
#include <QIcon>
#include <QtDebug>

class localModel : public QObject
{
    Q_OBJECT
public:
    explicit localModel(QObject *parent = 0);
    QFileSystemModel *listModel;
    QFileSystemModel *treeModel;
    QDir currentDir;
    QFileSystemModel * getTreeModel();
    QFileSystemModel * getListModel();
    
signals:
    
public slots:

private slots:
    
};

#endif // LOCALMODEL_H
