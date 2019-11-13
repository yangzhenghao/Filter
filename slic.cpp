#include "slic.h"
#include <cmath>
#include <QDebug>

SLIC::SLIC(QObject *parent) : QObject(parent)
{

    nc=40;
    ITERATION=10;
    clusterNum=100;
}

void SLIC::run(cv::Mat img,int num)
{
    qDebug()<<"SLIC:"<<QThread::currentThreadId();
    setClusterNumAndImage(img, num);
    initCluster();
    generateSuperPixel();
    checkConnectivity();

}

void SLIC::calculateStride()
{
    int width = image.cols;
    int height = image.rows;

    stride = int(std::sqrt(float(width*height)/clusterNum)+0.5);
}


void SLIC::initCluster()
{
    kseedsa.clear();
    kseedsb.clear();
    kseedsl.clear();
    kseedsx.clear();
    kseedsy.clear();

    int width=image.cols;
    int height = image.rows;

    calculateStride();
    BGR2LAB();
    const int MINSPSIZE=3;
    stride=std::max(stride, MINSPSIZE);

    assert (stride<=std::min(width, height));

    seedNumX=(width/stride);
    seedNumY=(height/stride);
    seedNum=seedNumX*seedNumY;

    kseedsa.resize(seedNum);
    kseedsb.resize(seedNum);
    kseedsl.resize(seedNum);
    kseedsx.resize(seedNum);
    kseedsy.resize(seedNum);

    float step_x=float(width)/seedNumX;
    float step_y = float(height)/seedNumY;

    assert (step_x>=MINSPSIZE&&step_y>=MINSPSIZE);

    int n=0;
    for(int y=0;y<seedNumY;y++)
    {
        for(int x=0;x<seedNumX;x++)
        {
            kseedsx[n]=step_x*(x+0.5)+0.5;
            kseedsy[n]=step_y*(y+0.5)+0.5;

            int sx=int(kseedsx[n]);
            int sy=int(kseedsy[n]);


            kseedsl[n]=labImage.at<cv::Vec3b>(sy,sx)[0];
            kseedsa[n]=labImage.at<cv::Vec3b>(sy,sx)[1];
            kseedsb[n]=labImage.at<cv::Vec3b>(sy,sx)[2];
            n++;
        }
    }

}


double SLIC::computeDis(int ci, int x,int y)
{
    double disl=kseedsl[ci]-labImage.at<cv::Vec3b>(y,x)[0];
    double disa=kseedsa[ci]-labImage.at<cv::Vec3b>(y,x)[1];
    double disb=kseedsb[ci]-labImage.at<cv::Vec3b>(y,x)[2];

    double disx=kseedsx[ci]-x;
    double disy=kseedsy[ci]-y;

    double dc=disl*disl+disa*disa+disb*disb;
    double ds=disx*disx+disy*disy;

    return sqrt(dc/nc/nc+ds/stride/stride);
}


void SLIC::generateSuperPixel()
{
    distances=QVector<QVector<double> >(image.rows,QVector<double>(image.cols, DBL_MAX));
    lables=QVector<QVector<double> >(image.rows,QVector<double>(image.cols, -1));
    int width =image.cols;
    int height = image.rows;
    for(int it=0;it<ITERATION;it++)
    {
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
            {
                 distances[i][j]=DBL_MAX;
                 lables[i][j]=-1;
            }
        }
        
        for(int k=0;k<kseedsl.size();k++)
        {
            for(int i=kseedsy[k]-stride;i<kseedsy[k]+stride;i++)
            {
                for(int j=kseedsx[k]-stride;j<kseedsx[k]+stride;j++)
                {
                    if(i>=0&&i<height&&j>=0&&j<width)
                    {
                        double dis=computeDis(k,j,i);
                        if(dis<distances[i][j])
                        {
                            distances[i][j]=dis;
                            lables[i][j]=k;
                        }
                    }
                }
            }
        }
        QVector<int> kseedcount;
        for(int i=0;i<kseedsa.size();i++)
        {
            kseedsa[i]=kseedsb[i]=kseedsl[i]=kseedsx[i]=kseedsy[i]=0;
            kseedcount.push_back(0);
        }

        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
            {
                int c_id=lables[i][j];
                if(c_id!=-1)
                {
                    kseedsl[c_id]+=labImage.at<cv::Vec3b>(i,j)[0];
                    kseedsa[c_id]+=labImage.at<cv::Vec3b>(i,j)[1];
                    kseedsb[c_id]+=labImage.at<cv::Vec3b>(i,j)[2];
                    kseedsx[c_id]+=j;
                    kseedsy[c_id]+=i;
                    kseedcount[c_id]+=1;
                }
            }
        }
        for (int i=0;i<kseedsa.size();i++)
        {
            kseedsl[i]/=kseedcount[i];
            kseedsa[i]/=kseedcount[i];
            kseedsb[i]/=kseedcount[i];
            kseedsx[i]/=kseedcount[i];
            kseedsy[i]/=kseedcount[i];
        }

    }
}

void SLIC::checkConnectivity()
{
    int label = 0, adjlabel = 0;
    const int lims = (image.cols * image.rows) / (kseedsy.size());

    const int dx4[4] = {-1,  0,  1,  0};
    const int dy4[4] = { 0, -1,  0,  1};

   /* Initialize the new cluster matrix. */
   QVector<QVector<int>> new_clusters;
   for (int i = 0; i < image.cols; i++) {
       QVector<int> nc;
       for (int j = 0; j < image.rows; j++) {
           nc.push_back(-1);
       }
       new_clusters.push_back(nc);
   }

   for (int i = 0; i < image.cols; i++)
   {
       for (int j = 0; j < image.rows; j++)
       {
           if (new_clusters[i][j] == -1)
           {
               QVector<CvPoint> elements;
               elements.push_back(cvPoint(i, j));

               /* Find an adjacent label, for possible use later. */
               for (int k = 0; k < 4; k++)
               {
                   int x = elements[0].x + dx4[k], y = elements[0].y + dy4[k];

                   if (x >= 0 && x < image.cols && y >= 0 && y < image.rows)
                   {
                       if (new_clusters[x][y] >= 0) {
                           adjlabel = new_clusters[x][y];
                       }
                   }
               }

               int count = 1;
               for (int c = 0; c < count; c++) {
                   for (int k = 0; k < 4; k++) {
                       int x = elements[c].x + dx4[k], y = elements[c].y + dy4[k];

                       if (x >= 0 && x < image.cols && y >= 0 && y < image.rows) {
                           if (new_clusters[x][y] == -1 && lables[i][j] == lables[x][y]) {
                               elements.push_back(cvPoint(x, y));
                               new_clusters[x][y] = label;
                               count += 1;
                           }
                       }
                   }
               }

               /* Use the earlier found adjacent label if a segment size is
                  smaller than a limit. */
               if (count <= lims >> 2) {
                   for (int c = 0; c < count; c++) {
                       new_clusters[elements[c].x][elements[c].y] = adjlabel;
                   }
                   label -= 1;
               }
               label += 1;
           }
       }
   }
    cv::Mat hsvImage;
    cv::cvtColor(image,hsvImage, CV_BGR2HSV);
   //对分割区域内的RGB求和
   QVector<QVector<double>> RGBSum(label+1, QVector<double>(6,0));
   for (int i = 0; i < image.cols; i++)
   {
       for (int j = 0; j < image.rows; j++)
       {
           //BGR
            RGBSum[new_clusters[i][j]][0]+=labImage.at<cv::Vec3b>(i,j)[0];
            RGBSum[new_clusters[i][j]][1]+=labImage.at<cv::Vec3b>(i,j)[1];
            RGBSum[new_clusters[i][j]][2]+=labImage.at<cv::Vec3b>(i,j)[2];
            RGBSum[new_clusters[i][j]][3]+=1;
            RGBSum[new_clusters[i][j]][4]+=j;   //保存行
            RGBSum[new_clusters[i][j]][5]+=i;   //保存列
       }
   }
   for(int i=0;i<RGBSum.size();i++)
   {
       if(RGBSum[i][3]!=0)
       {
           RGBSum[i][0]/=RGBSum[i][3];
           RGBSum[i][1]/=RGBSum[i][3];
           RGBSum[i][2]/=RGBSum[i][3];
           RGBSum[i][4]/=RGBSum[i][3];
           RGBSum[i][5]/=RGBSum[i][3];
       }
   }
   newImage=cv::Mat(image.rows,image.cols,labImage.type());
   cv::Mat newImage1;
   for (int i = 0; i < image.cols; i++)
   {
       for (int j = 0; j < image.rows; j++)
       {
           newImage.at<cv::Vec3b>(i,j)[0]=int(RGBSum[new_clusters[i][j]][0]+0.5);
           newImage.at<cv::Vec3b>(i,j)[1]=int(RGBSum[new_clusters[i][j]][1]+0.5);
           newImage.at<cv::Vec3b>(i,j)[2]=int(RGBSum[new_clusters[i][j]][2]+0.5);
//             int x=RGBSum[new_clusters[i][j]][5];
//             int y=RGBSum[new_clusters[i][j]][4];
//             newImage.at<cv::Vec3b>(i,j)[0]=image.at<cv::Vec3b>(x,y)[2];
//             newImage.at<cv::Vec3b>(i,j)[1]=image.at<cv::Vec3b>(x,y)[1];
//             newImage.at<cv::Vec3b>(i,j)[2]=image.at<cv::Vec3b>(x,y)[0];
       }
   }
   cv::cvtColor(newImage, newImage1,CV_Lab2RGB);
   newImage=newImage1;
   emit resultReady(cvMat2QImage(newImage));
   cv::imwrite("temp1.jpg", newImage);
}

void SLIC::BGR2LAB()
{
    cv::cvtColor(image, labImage,CV_BGR2Lab);
}
