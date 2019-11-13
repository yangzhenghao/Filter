#ifndef BILATERAL_H
#define BILATERAL_H

#include <QObject>
#include "opencv2/opencv.hpp"
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QPair>
#include <QThread>
#include "function.h"
#include <QPainter>
#include <QPen>

class Bilateral : public QObject
{
    Q_OBJECT
public:
    explicit Bilateral(QObject *parent = nullptr);
    void setImage(QImage im){oriImage=im;}
private:
    QImage oriImage;
    QImage destImage;
    QImage grayImage;
    QVector<QVector<uchar>> grayValues;
signals:
    void resultReady(QImage im);
public slots:
    void run(QImage);
    void toGray();
};

#endif // BILATERAL_H
