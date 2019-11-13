#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#include "opencv2/opencv.hpp"
#include "slic.h"
#include <QThread>
#include <QMutex>

namespace Ui {
class MainWindow;
}
//https://zhuanlan.zhihu.com/p/53270619
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initUI();
    void initData();
    void initMenu();
    void BilateralFilter();
    void toGray();

private:
    Ui::MainWindow *ui;
    QImage oriImage;
    QImage destImage;
    QImage grayImage;
    QVector<QVector<uchar>> grayValues;
    QString imageName;

    QMenu fileMenu;
    QMenu filterMenu;
    QAction *openAction, * saveAction, *bilateralAction,*slicAction;
    bool isCalculate;

    SLIC *s;
    QThread *slicThread;            //到底实用一个线程去做，还是两个线程
    QThread *bilateralThread;
signals:
    void updateFile(bool);
    void startSlic(cv::Mat, int);
    void startBilateral();

public slots:
    void openFile();
    void saveFile();
    void bilateralSlot();
    void slicSlot();
    void updateImage(bool flag);
    void handleResults(QImage im);
};

#endif // MAINWINDOW_H
