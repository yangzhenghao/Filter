#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDir temp("../resource/hh.png");
    qDebug()<<QApplication::applicationFilePath();
    qDebug()<<temp.absolutePath();
    MainWindow w;
    w.show();

    return a.exec();
}
