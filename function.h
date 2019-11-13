#ifndef FUNCTION_H
#define FUNCTION_H
#include "opencv2/opencv.hpp"
#include <QImage>
#include <QPixmap>
#include <vector>
#include <QDebug>

cv::Mat QImage2cvMat(const QImage &image);

QImage cvMat2QImage(const cv::Mat &img);
//输入应当是灰度图
std::vector<cv::Rect> faceDetector(cv::Mat &pic);

#endif // FUNCTION_H
