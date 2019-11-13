#ifndef FUNCTION_H
#define FUNCTION_H
#include "opencv2/opencv.hpp"
#include <QImage>
#include <QPixmap>
#include <vector>
cv::Mat QImage2cvMat(const QImage &image)
{
    cv::Mat mat;
    qDebug() << image.format();
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_RGB2BGR);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

QImage cvMat2QImage(const cv::Mat &img)
{
    QImage imgIn= QImage((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
    return imgIn;
}

//输入应当是灰度图
std::vector<cv::Rect> faceDetector(cv::Mat &pic)
{
    assert(pic.channels()==1);
    cv::Mat src;
    src=pic;
    cv::equalizeHist(src,src);

    cv::CascadeClassifier faces_cascade;
    faces_cascade.load("haarcascade_frontalface_alt2.xml");
    //faces_cascade.load("haarcascade_frontalface_alt_tree.xml");
    std::vector<cv::Rect> faces;
    faces_cascade.detectMultiScale(src,faces,1.1, 3, 0|CV_HAAR_SCALE_IMAGE);

    return faces;

}

#endif // FUNCTION_H
