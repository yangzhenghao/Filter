#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <QDebug>
#include <cmath>
#include <vector>
#include <function.h>
#include <QPainter>
#include <QPen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    openAction=saveAction=bilateralAction=slicAction=NULL;
    isCalculate=false;

    initUI();
    initMenu();

    this->setWindowTitle("Filter");
    ui->label->setStyleSheet("border:2px solid black;");
    ui->destLabel->setStyleSheet("border:2px solid black;");
    ui->label->setText("Source Image");
    ui->destLabel->setText("dest Image");

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
}

void MainWindow::initUI()
{

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
    destImage=oriImage;
    int r=25;
    double sigma_d=25*2.0, sigma_r=25/2.0;
    double coeffD=-0.5/sigma_d/sigma_d;
    double coeffR=-0.5/sigma_r/sigma_r;
    toGray();

    cv::Mat grayMat=QImage2cvMat(grayImage);
    std::vector<cv::Rect> faces=faceDetector(grayMat);

    for(auto face:faces)
    {
        int height=face.y+face.height;
        int width = face.x+face.width;
        for(int i=face.y;i<height;i++)
        {
            for(int j=face.x;j<width;j++)
            {
                double weightSum=0, weight_tmp=0;
                double pixelSumR=0, pixelSumG=0, pixelSumB=0;
                for(int m=-r;m<=r;m++)
                {
                    for(int n=-r;n<=r;n++)
                    {
                        int x_tmp=j+n;
                        int y_tmp=i+m;

                        x_tmp = x_tmp < 0 ? 0 : x_tmp;
                        x_tmp = x_tmp > width - 1 ? width - 1 : x_tmp;   // 边界处理，replicate
                        y_tmp = y_tmp < 0 ? 0 : y_tmp;
                        y_tmp = y_tmp > height - 1 ? height - 1 : y_tmp;

                        weight_tmp = coeffD*(m*m+n*n)+coeffR*(grayValues[i][j]-grayValues[y_tmp][x_tmp])*(grayValues[i][j]-grayValues[y_tmp][x_tmp]);
                        weight_tmp = std::exp(weight_tmp);

                        //int pixcel_dif = (int)abs(img_tmp[y_tmp * m_nChannels * nx + m_nChannels * x_tmp + k] - img_tmp[i * m_nChannels * nx + m_nChannels * j + k]);
                        //double weight_tmp = d_metrix[m + r][n + r] * r_metrix[pixcel_dif];  // 复合权重

                        //pixcel_sum += img_tmp[y_tmp * m_nChannels * nx + m_nChannels * x_tmp + k] * weight_tmp;
                        //pixelSum+=weight_tmp*grayValues[y_tmp][x_tmp];
                        pixelSumB += weight_tmp*qBlue(oriImage.pixel(x_tmp, y_tmp));
                        pixelSumR += weight_tmp*qRed(oriImage.pixel(x_tmp, y_tmp));
                        pixelSumG += weight_tmp*qGreen(oriImage.pixel(x_tmp, y_tmp));
                        weightSum += weight_tmp;
                    }
                }
                pixelSumB /= weightSum;
                pixelSumG /= weightSum;
                pixelSumR /= weightSum;
                pixelSumB=pixelSumB>255?255:pixelSumB;
                pixelSumG=pixelSumG>255?255:pixelSumG;
                pixelSumR=pixelSumR>255?255:pixelSumR;
                destImage.setPixel(j,i,qRgb(pixelSumR,pixelSumG,pixelSumB));
                QPainter painter(&destImage);
                painter.setPen(QPen(Qt::blue,4));
                painter.drawRect(face.x,face.y,face.width,face.height);
            }
        }
    }
     emit updateFile(false);
}

void MainWindow::slicSlot()
{
    SLIC s;
    cv::Mat img=QImage2cvMat(oriImage);
    s.setClusterNumAndImage(img,360);
    s.run();
    destImage=cvMat2QImage(s.getResult());
    emit updateFile(false);
}

//将原图转化为灰度图
void MainWindow::toGray()
{
    grayValues.clear();
    //record.clear();
    int height = oriImage.height();
    int width = oriImage.width();
//    QPair<int,int> ijIndex;
    QImage ret(width, height, QImage::Format_Indexed8);
    ret.setColorCount(256);
    for(int i = 0; i < 256; i++)
    {
        ret.setColor(i, qRgb(i, i, i));
    }
    switch(oriImage.format())
    {
        case QImage::Format_Indexed8:
        for(int i = 0; i < height; i ++)
        {
            const uchar *pSrc = (uchar *)oriImage.constScanLine(i);
            uchar *pDest = (uchar *)ret.scanLine(i);
            memcpy(pDest, pSrc, width);
            QVector<uchar> grayi;
            grayi.reserve(width);
            for( int j = 0; j < width; j ++)
            {
                grayi.push_back(pDest[j]);
            }
            grayValues.push_back(grayi);

        }
        break;
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        for(int i = 0; i < height; i ++)
        {

            const QRgb *pSrc = (QRgb *)oriImage.constScanLine(i);
            QVector<uchar> grayi;
            grayi.reserve(width);
            uchar *pDest = (uchar *)ret.scanLine(i);
            for( int j = 0; j < width; j ++)
            {
                 pDest[j] = qGray(pSrc[j]);
                 grayi.push_back(pDest[j]);
            }
            grayValues.push_back(grayi);
        }
        break;
    }
   grayImage=ret;
   ui->destLabel->setText(QStringLiteral("灰度图转换完毕，正在裁剪，请稍等..."));
   update();
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
}
