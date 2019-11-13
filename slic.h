#ifndef SLIC_H
#define SLIC_H

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

class SLIC : public QObject
{
    Q_OBJECT
public:
    explicit SLIC(QObject *parent = nullptr);
    void calculateStride();
    void initCluster();
    void BGR2LAB();
    double computeDis(int ci, int x,int y);
    void generateSuperPixel();
    void checkConnectivity();



    cv::Mat getResult(){return newImage;}
    
private:
    //QImage image;
    cv::Mat image;       //存储顺序是BGR
    cv::Mat labImage;
    cv::Mat newImage;    //晶格化的新图像
    QString imagePath;   //怎么把数据从main_ui传送过来
    

    int clusterNum;
    int stride;
    int seedNumX;
    int seedNumY;
    int seedNum;
    int nc;

    double spatialFactor;
    QVector<double> kseedsx,kseedsy,kseedsl,kseedsa,kseedsb;
    QVector<QVector<double> > distances,lables;
    
    int ITERATION;
    

signals:
    void resultReady(QImage im);
public slots:
    void run(cv::Mat img,int num);
    void setClusterNumAndImage(cv::Mat img,int num){image=img;clusterNum=num;}
};

#endif // SLIC_H
