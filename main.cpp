#include <QtGui/QApplication>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("Ftp Client");
    a.setApplicationVersion("v0.1");
    a.setOrganizationName("David&&mrProper");

    QTextCodec *codec = QTextCodec::codecForName("CP-1251");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);

    MainWindow w;
    w.show();
    
    return a.exec();
}
