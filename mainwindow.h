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
    void initMenu();
    void BilateralFilter();
    void toGray();

private:
    Ui::MainWindow *ui;
    QImage oriImage;
    QImage destImage;
    QImage grayImage;
    QVector<QVector<uchar>> grayValues;

    QMenu fileMenu;
    QMenu filterMenu;

    QString imageName;

    QAction *openAction, * saveAction, *bilateralAction,*slicAction;

    bool isCalculate;
signals:
    void updateFile(bool);

public slots:
    void openFile();
    void saveFile();
    void bilateralSlot();
    void slicSlot();
    void updateImage(bool flag);
};

#endif // MAINWINDOW_H
