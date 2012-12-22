#include "localmodel.h"

#define cDir QDir::homePath()

localModel::localModel(QObject *parent) :
    QObject(parent)
{
    QString sPath = "C:/";
    treeModel = new QFileSystemModel(this);
    treeModel->setRootPath(sPath);
    treeModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    listModel = new QFileSystemModel(this);
    listModel->setRootPath(sPath);
    listModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
}


QFileSystemModel * localModel::getTreeModel(){
    return treeModel;
}

QFileSystemModel * localModel::getListModel(){
    return listModel;
}

