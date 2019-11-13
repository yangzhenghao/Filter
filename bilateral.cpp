#include "bilateral.h"
#include <QDebug>

Bilateral::Bilateral(QObject *parent) : QObject(parent)
{

}

void Bilateral::run(QImage im)
{
    qDebug()<<"Bilaterak thread id:"<<QThread::currentThreadId();
    setImage(im);
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

    emit resultReady(destImage);
}

void Bilateral::toGray()
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
}
