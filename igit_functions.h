#ifndef IGIT_FUNCTIONS_H
#define IGIT_FUNCTIONS_H

#include"data_type.h"

#include<qvector.h>
#include<opencv2\opencv.hpp>
#include<opencv2\core\core.hpp>
#include<qimage.h>


typedef PointXYZRGBNormal Point;
///////////////////////////////////////////K Nearest Neighbours for 3D points////////////////////////////////////////
QVector< QVector<int> > kNearesNeighbours(int Knn, QVector<Point> &querys, QVector<Point> &points);

///////////////////////////////////////////K Nearest Neighbours for 2D points  //////////////////////////////////////
QVector < QVector<int> >kNearestNeighbours2D(int Knn, QVector<QPointF> &querys, QVector<QPointF> &points);

//////////////////////////////////////////projectionFrom3DTo2D///////////////////////////////////////////////////////////////
void  projectionFrom3DTo2D(Point pt, cv::Mat &proj, QPoint &p);

//////////////////////////////////////////convertMatToQImage/////////////////////////////////////////////////////////////////
void convertMatToQImage(cv::Mat_<cv::Vec3b> &img_cv, QImage & img_qt);

//////////////////////////////////////////convertQImageToMat/////////////////////////////////////////////////////////////////
void convertQImageToMat(QImage & img_qt, cv::Mat_<cv::Vec3b> &img_cv);

//////////////////////////////////////////areaCoordinates////////////////////////////////////////////////////////////////////
Point areaCoordinates(const QPointF &pt, const QPointF &v0, const QPointF &v1, const QPointF &v2);

#endif