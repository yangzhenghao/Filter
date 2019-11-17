#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <QDebug>
#include <cmath>
#include <vector>
#include <QMetaType>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    openAction=saveAction=bilateralAction=slicAction=NULL;
    isCalculate=false;

    initUI();
    initMenu();
    initData();

    connect(this, SIGNAL(updateFile(bool)),this, SLOT(updateImage(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
    if(openAction)
        delete openAction;
    if(saveAction)
        delete saveAction;
    if(bilateralAction)
        delete bilateralAction;
    if(slicAction)
        delete slicAction;
    if(slicThread)
    {
        slicThread->quit();
        slicThread->wait();
    }
    if(bilateralThread)
    {
        bilateralThread->quit();
        bilateralThread->wait();
    }
}

void MainWindow::initUI()
{
    this->setWindowTitle("Filter");
    ui->label->setStyleSheet("border:2px solid black;");
    ui->destLabel->setStyleSheet("border:2px solid black;");
    ui->label->setText("Source Image");
    ui->destLabel->setText("dest Image");
}

void MainWindow::initData()
{
    s=NULL;
    b=NULL;
    slicThread=NULL;
    bilateralThread=NULL;
    isCalculate =false;
    isOperate=false;
}
void MainWindow::initMenu()
{
    fileMenu.setTitle(QStringLiteral("文件"));
    fileMenu.clear();
    openAction=new QAction(QStringLiteral("打开图片"),this);
    saveAction=new QAction(QStringLiteral("保存"),this);
    fileMenu.addAction(openAction);
    fileMenu.addAction(saveAction);
    ui->menuBar->addMenu(&fileMenu);

    connect(openAction,SIGNAL(triggered()),this, SLOT(openFile()));
    connect(saveAction,SIGNAL(triggered()),this, SLOT(saveFile()));

    filterMenu.setTitle(QStringLiteral("滤镜"));
    filterMenu.clear();
    bilateralAction=new QAction(QStringLiteral("人像美肤"),this);
    slicAction=new QAction(QStringLiteral("晶格化"),this);
    filterMenu.addAction(bilateralAction);
    filterMenu.addAction(slicAction);
    ui->menuBar->addMenu(&filterMenu);

    connect(bilateralAction,SIGNAL(triggered()),this, SLOT(bilateralSlot()));
    connect(slicAction,SIGNAL(triggered()),this, SLOT(slicSlot()));
}
void MainWindow::bilateralSlot()
{
    //operateLock.lock();
    if(isOperate)
    {
        QMessageBox::critical(NULL, QStringLiteral("警告"),QStringLiteral("请等待上一次操作结果") );
        //operateLock.unlock();
        return;
    }
    ui->informLabel->setText(QStringLiteral(""));
    isOperate=true;
    //operateLock.unlock();
    isCalculate=false;
    qDebug()<<"main ui:"<<QThread::currentThreadId();
    if(b)
        delete b;
    if(!bilateralThread)
        //delete bilateralThread;
        //return;
    bilateralThread = new QThread();
    b=new Bilateral();
    b->moveToThread(bilateralThread);

    connect(bilateralThread, &QThread::finished, bilateralThread, &QObject::deleteLater);
    connect(bilateralThread, &QThread::finished, b, &QObject::deleteLater);
    connect(this, SIGNAL(startBilateral(QImage)), b, SLOT(run(QImage)));
    connect(b,SIGNAL(resultReady(QImage)), this, SLOT(handleResults(QImage)));
    bilateralThread->start();
    ui->destLabel->setText(QStringLiteral("正在人脸美肤...请稍等"));
    emit startBilateral(oriImage);

}

void MainWindow::slicSlot()
{
    //operateLock.lock();
    if(isOperate)
    {
        QMessageBox::critical(NULL, QStringLiteral("警告"),QStringLiteral("请等待上一次操作结果") );
        //operateLock.unlock();
        return;
    }
    ui->informLabel->setText(QStringLiteral(""));
    isOperate=true;
    //operateLock.unlock();
    isCalculate=false;
    qDebug()<<"main ui:"<<QThread::currentThreadId();
    isCalculate=false;
    if(s)
        delete s;
    if(!slicThread)
        //delete slicThread;
        //return;
    slicThread = new QThread();
    s=new SLIC();
    s->moveToThread(slicThread);
    cv::Mat img=QImage2cvMat(oriImage);

    qRegisterMetaType< cv::Mat >("cv::Mat");
    connect(slicThread, &QThread::finished, slicThread, &QObject::deleteLater);
    connect(slicThread, &QThread::finished, s, &QObject::deleteLater);
    connect(this, SIGNAL(startSlic(cv::Mat,int)), s, SLOT(run(cv::Mat,int)));
    connect(s,SIGNAL(resultReady(QImage)), this, SLOT(handleResults(QImage)));
    slicThread->start();
    ui->destLabel->setText(QStringLiteral("正在晶格化...请稍等"));
    emit startSlic(img, 360);
}

void MainWindow::handleResults(QImage im)
{
    destImage=im;
    emit updateFile(false);
//    operateLock.lock();
    isOperate=false;
//    operateLock.unlock();
}



void MainWindow::updateImage(bool flag)
{

    if(flag)
    {
        QImage image(imageName);
        oriImage=image;
        QPixmap pix(imageName);
        QPixmap dest;
        if(pix.size().height()<=ui->label->size().height()&&
                pix.size().width()<=ui->label->size().width())
            dest=pix;
        else
            dest=pix.scaled(ui->label->size(),Qt::KeepAspectRatio);

        ui->label->setPixmap(dest);
        ui->label->show();
        ui->informLabel->setText("");
        isCalculate=false;
    }
    else
    {
        isCalculate=true;
        QPixmap pix=QPixmap::fromImage(destImage);
        QPixmap dest;
        if(pix.size().height()<=ui->label->size().height()&&
                pix.size().width()<=ui->label->size().width())
            dest=pix;
        else
            dest=pix.scaled(ui->destLabel->size(),Qt::KeepAspectRatio);
        ui->informLabel->setText(QStringLiteral("结果图片已被缩放,请导出..."));
        ui->destLabel->setPixmap(dest);
        ui->destLabel->show();
    }
}



void MainWindow::openFile()
{
    QStringList    fileNameList;
    QString fileName0;
    QFileDialog* fd = new QFileDialog(this);//创建对话框
    fd->resize(240,320);    //设置显示的大小
    //fd->setFilter(QString("Images (*.png *.jpg)")); //设置文件过滤器
    fd->setViewMode(QFileDialog::List);  //设置浏览模式，有 列表（list） 模式和 详细信息（detail）两种方式
    if ( fd->exec() == QDialog::Accepted )   //如果成功的执行
    {
        fileNameList = fd->selectedFiles();      //返回文件列表的名称
        fileName0 = fileNameList[0];            //取第一个文件名
    }
    else
        fd->close();
    if(""!=fileName0)
    {
        if(fileName0.endsWith(".png")||fileName0.endsWith(".jpg")||fileName0.endsWith(".JPG")||fileName0.endsWith(".PNG"))
        {
            imageName=fileName0;
            emit updateFile(true);
        }
        else
        {
            QMessageBox::critical(NULL, QStringLiteral("警告"),QStringLiteral("请选取png或jpg单个文件") );
            return;
        }
    }

}

void MainWindow::saveFile()
{
    if(!isCalculate)
    {
        QMessageBox::critical(NULL, QStringLiteral("警告"),QStringLiteral("请点击计算按钮，再导出结果") );
        return;
    }
    QString filename1;
    for(int j=0;j<imageName.length();j++)
    {
        if(imageName[j]=='.')
            filename1+="_new.";
        else
            filename1+=imageName[j];
    }
    destImage.save(filename1);
    QMessageBox::information(NULL, QStringLiteral("提醒"),QStringLiteral("导出成功, 已存至相同目录") );
    ui->informLabel->setText(QStringLiteral("已导出"));
}
