#include"igit_functions.h"

///////////////////////////////////////////K Nearest Neighbours search////////////////////////////////////////////////////////
QVector< QVector<int> > kNearesNeighbours(int Knn, QVector<Point> &querys, QVector<Point> &points)
{
	QVector<QVector<int> > neighbours;
	neighbours.resize(querys.size());

	cv::Mat mSamples(points.size(), 3, CV_32FC1);
	cv::Mat mQuery(querys.size(), 3, CV_32FC1);

	// set points
	for (int i = 0; i < points.size(); i++)
	{
		mSamples.at<float>(i, 0) = points[i].x;
		mSamples.at<float>(i, 1) = points[i].y;
		mSamples.at<float>(i, 2) = points[i].z;
	}

	// set querys
	for (int i = 0; i < querys.size(); i++)
	{
		mQuery.at<float>(i, 0) = querys[i].x;
		mQuery.at<float>(i, 1) = querys[i].y;
		mQuery.at<float>(i, 2) = querys[i].z;
	}

	cv::flann::KDTreeIndexParams params(4);
	cv::flann::Index neighbours_search(mSamples, params);

	cv::Mat mIndices;
	cv::Mat mDists;
	neighbours_search.knnSearch(mQuery, mIndices, mDists, Knn, cv::flann::SearchParams(128));


	// indices of nearest neighbours
	for (int i = 0; i < mIndices.rows; i++)
	{
		for (int j = 0; j < mIndices.cols; j++)
		{
			neighbours[i].append(mIndices.at<int>(i, j));
		}
	}

	mDists.release();
	mIndices.release();
	mQuery.release();
	mSamples.release();

	return neighbours;
}
///////////////////////////////////////////K Nearset Neighbours search on2D /////////////////////////////////////////////////
QVector < QVector<int> >kNearestNeighbours2D(int Knn, QVector<QPointF> &querys, QVector<QPointF> &points)
{
	QVector<QVector<int> > neighbours;
	neighbours.resize(querys.size());

	cv::Mat mSamples(points.size(), 2, CV_32FC1);
	cv::Mat mQuery(querys.size(), 2, CV_32FC1);

	// set points
	for (int i = 0; i < points.size(); i++)
	{
		mSamples.at<float>(i, 0) = points[i].x();
		mSamples.at<float>(i, 1) = points[i].y();
	}

	// set querys
	for (int i = 0; i < querys.size(); i++)
	{
		mQuery.at<float>(i, 0) = querys[i].x();
		mQuery.at<float>(i, 1) = querys[i].y();
	}

	cv::flann::KDTreeIndexParams params(4);
	cv::flann::Index neighbours_search(mSamples, params);

	cv::Mat mIndices;
	cv::Mat mDists;
	neighbours_search.knnSearch(mQuery, mIndices, mDists, Knn, cv::flann::SearchParams(128));


	// indices of nearest neighbours
	for (int i = 0; i < mIndices.rows; i++)
	{
		for (int j = 0; j < mIndices.cols; j++)
		{
			neighbours[i].append(mIndices.at<int>(i, j));
		}
	}

	mDists.release();
	mIndices.release();
	mQuery.release();
	mSamples.release();

	return neighbours;
}
//////////////////////////////////////////projectionFrom3DTo2D///////////////////////////////////////////////////////////////
void  projectionFrom3DTo2D(Point pt, cv::Mat &proj, QPoint &p)
{
	cv::Mat p3D(4, 1, CV_32FC1);
	cv::Mat p2D(3, 1, CV_32FC1);

	// cout<<"3D: "<<"x: "<<pt.x<<", y: "<<pt.y<<", z: "<<pt.z<<endl;
	p3D.at<float>(0) = pt.x;
	p3D.at<float>(1) = pt.y;
	p3D.at<float>(2) = pt.z;
	p3D.at<float>(3) = 1.0;

	p2D = proj * p3D;

	p.setX((int)p2D.at<float>(0) / p2D.at<float>(2));
	p.setY((int)p2D.at<float>(1) / p2D.at<float>(2));

	p3D.release();
	p2D.release();
}
//////////////////////////////////////////convertMatToQImage/////////////////////////////////////////////////////////////////
void convertMatToQImage(cv::Mat_<cv::Vec3b> &img_cv, QImage & img_qt)
{
	// convert QImage to 32 bit
	img_qt.convertToFormat(QImage::Format_RGB32);

	img_qt.scaled(img_cv.rows, img_cv.cols);

	int lineNum = 0;

	int height = img_cv.rows;

	int width = img_cv.cols;

	uchar * imgBits = img_qt.bits();

	for (int i = 0; i < height; i++)
	{
		lineNum = i* width * 4;
		for (int j = 0; j < width; j++)
		{
			imgBits[lineNum + j * 4 + 2] = img_cv(i, j)[2];
			imgBits[lineNum + j * 4 + 1] = img_cv(i, j)[1];
			imgBits[lineNum + j * 4 + 0] = img_cv(i, j)[0];
		}
	}
}
//////////////////////////////////////////convertQImageToMat/////////////////////////////////////////////////////////////////
void convertQImageToMat(QImage & img_qt, cv::Mat_<cv::Vec3b> &img_cv)
{
	img_cv.create(img_qt.height(), img_qt.width());

	img_qt.convertToFormat(QImage::Format_RGB32);

	int lineNum = 0;

	int height = img_qt.height();

	int width = img_qt.width();

	uchar *imgBits = img_qt.bits();

	for (int i = 0; i < height; i++)
	{
		lineNum = i* width * 4;
		for (int j = 0; j < width; j++)
		{
			img_cv(i, j)[2] = imgBits[lineNum + j * 4 + 2];
			img_cv(i, j)[1] = imgBits[lineNum + j * 4 + 1];
			img_cv(i, j)[0] = imgBits[lineNum + j * 4 + 0];
		}
	}
}
//////////////////////////////////////////areaCoordinates////////////////////////////////////////////////////////////////////
Point areaCoordinates(const QPointF &pt, const QPointF &v0, const QPointF &v1, const QPointF &v2)
{
	float L0, L1, L2;

	qglviewer::Vec vp0(pt.x() - v0.x(), pt.y() - v0.y(), 0.0);
	qglviewer::Vec vp1(pt.x() - v1.x(), pt.y() - v1.y(), 0.0);
	qglviewer::Vec vp2(pt.x() - v2.x(), pt.y() - v2.y(), 0.0);

	qglviewer::Vec vp12 = cross(vp1, vp2);
	L0 = qAbs(vp12.norm());

	qglviewer::Vec vp02 = cross(vp0, vp2);
	L1 = qAbs(vp02.norm());

	qglviewer::Vec vp01 = cross(vp0, vp1);
	L2 = qAbs(vp01.norm());

	float L = L0 + L1 + L2;
#if 0
	qglviewer::Vec v01(v1.x() - v0.x(), v1.y() - v0.y(), 1.0);
	qglviewer::Vec v02(v2.x() - v0.x(), v2.y() - v0.y(), 1.0);
	qglviewer::Vec v12 = cross(v01, vp02);
	float L = qAbs(vp02.norm());

	float L_tmp = L0 + L1 + L2;
#endif
	Point p(L0 / L, L1 / L, L2 / L);

	return p;
}