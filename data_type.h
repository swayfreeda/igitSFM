#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include"stdlib.h"
#include"algorithm"
#include"cmath"
#include<vector>
#include"cmath"
#include"opencv2/opencv.hpp"

#include <iostream>
#include <string>
#include<QSet>
#include<qglviewer.h>

using namespace std;

////////////////////////////////////////////////PointXY///////////////////////////////////////////////////////////////////
class PointXY
{
public:

    PointXY(){x = 0; y=0;}
    PointXY(const PointXY&pt)
    {
       this->x = pt.x;
       this->y = pt.y;
    }
    PointXY(float x, float y)
    {
      this->x = x;
      this->y = y;
    }

    PointXY & operator- (const PointXY & pt)
    {
        this->x -= pt.x;
        this->y -= pt.y;

        return *this;
    }

    PointXY &operator = (const PointXY & pt)
    {
      this->x = pt.x;
      this->y = pt.y;
    }

    float x;
    float y;
};
////////////////////////////////////////////////PointXYZ///////////////////////////////////////////////////////////////////
class PointXYZ
{
public:

    PointXYZ(){}
    PointXYZ(const PointXYZ&pt)
    {
      this->x = pt.x;
      this->y = pt.y;
      this->z = pt.z;
    }
    PointXYZ(float x, float y, float z)
    {
       this->x = x;
       this->y = y;
       this->z = z;
    }

    PointXYZ & operator =(const PointXYZ & pt)
    {
        this->x = pt.x;
        this->y = pt.y;
        this->z = pt.z;

        return *this;
    }

    bool operator ==(const PointXYZ &pt)
    {
        // return(x == pt.x && y==pt.y &&z == pt.z);
        return(abs(x -pt.x)<0.01 && abs(y-pt.y)<0.01 &&abs(z - pt.z)<0.01);
    }

public:
    QSet<int> vis;

    float x;
    float y;
    float z;
    bool isProcessed;
    bool inconsist_;

};
////////////////////////////////////////////////PointXYZRGB////////////////////////////////////////////////////////////////
class PointXYZRGB
{
public:

    PointXYZRGB(){}
    PointXYZRGB(const PointXYZRGB&pt)
    {
       this->x = pt.x;
       this->y = pt.y;
       this->z = pt.z;

       this->r = pt.r;
       this->g = pt.g;
       this->b = pt.b;
       
       this->isProcessed = pt.isProcessed;
       this->id = pt.id;

       this-> vis = pt.vis;
       this-> inconsist_ = pt.inconsist_;

    }
    PointXYZRGB(float x, float y, float z, uchar r, uchar g, uchar b)
   {
        this->x = x;
        this->y = y;
        this->z = z;
     
        this->r = (float)r;
        this->g = (float)g;
        this->b = (float)b;
   }

    bool isProcessed;

    int id;

    float x;
    float y;
    float z;

    float r;
    float g;
    float b;

    QSet<int> vis;
    bool inconsist_;
};
////////////////////////////////////////////////PointXYZRGBNormal//////////////////////////////////////////////////////////
class PointXYZRGBNormal
{
public:

    PointXYZRGBNormal(){

        this->x = 0;
        this->y = 0;
        this->z = 0;

        this->r = 0;
        this->g = 0;
        this->b = 0;

        this->normal_x = 0;
        this->normal_y = 0;
        this->normal_z = 0;
		
		vis.clear();
    }
    PointXYZRGBNormal(float x, float y, float z){

        this->x = x;
        this->y = y;
        this->z = z;

        this->r = 0;
        this->g = 0;
        this->b = 0;

        this->normal_x = 0;
        this->normal_y = 0;
        this->normal_z = 0;
    }
	PointXYZRGBNormal(const PointXYZRGBNormal&pt)
	{
		this->x = pt.x;
		this->y = pt.y;
		this->z = pt.z;

		this->r = pt.r;
		this->g = pt.g;
		this->b = pt.b;

		this->normal_x = pt.normal_x;
		this->normal_y = pt.normal_y;
		this->normal_z = pt.normal_z;

		this->id = id;	
		foreach(int id, pt.vis)
		{
			this->vis.append(id);
		}
	}
	PointXYZRGBNormal & operator =(const PointXYZRGBNormal & pt)
    {
        this->x = pt.x;
        this->y = pt.y;
        this->z = pt.z;

        this->r = pt.r;
        this->g = pt.g;
        this->b = pt.b;

        this->normal_x = pt.normal_x;
        this->normal_y = pt.normal_y;
        this->normal_z = pt.normal_z;

        this->id = id;
		foreach(int id, pt.vis)
		{
			this->vis.append(id);
		}

        return *this;
    }
	bool operator < (const PointXYZRGBNormal& p) const
	{
		bool flag0 = false;
		bool flag1 = false;
		bool flag2 = false;

		if (this->x != p.x)
		{
			flag0 = this->x< p.x ? true : false;
		}
		if ((this->x == p.x) && (this->y != p.y))
		{
			flag1 = this->y< p.y ? true : false;
		}
		if ((this->x == p.x) && (this->y == p.y)
			&& (this->z != p.z))
		{
			flag2 = this->z < p.z ? true : false;
		}

		return flag0 || flag1 || flag2;
	}

public:
    int id;

    float x;
    float y;
    float z;

    float r;
    float g;
    float b;

    float normal_x;
    float normal_y;
    float normal_z;

    QVector<int> vis;
};

typedef PointXYZRGBNormal Point;
////////////////////////////////////////////////Camera//////////////////////////////////////////////////////////////////////
class Camera{

public:
	// constructor
	Camera()
	{
		rotation_.create(3, 3, CV_32FC1);
		trans_.create(3, 1, CV_32FC1);
		project_.create(3, 4, CV_32FC1);

		pos_.create(3, 1, CV_32FC1);
		dir_.create(3, 1, CV_32FC1);

		xaxis_.create(3, 1, CV_32FC1);
		yaxis_.create(3, 1, CV_32FC1);
		zaxis_.create(3, 1, CV_32FC1);

		rotation_.setTo(0);
		trans_.setTo(0);
		project_.setTo(0);

		pos_.setTo(0);
		dir_.setTo(0);

		xaxis_.setTo(0);
		yaxis_.setTo(0);
		zaxis_.setTo(0);

		focal_ = 2000.0;  // Ä¬ÈÏÖµ
	}

	~Camera()
	{
		rotation_.release();
		trans_.release();
		project_.release();
		pos_.release();
		dir_.release();

	}

	// overload operater =[]
	Camera & operator =(const Camera & cam);
	
	//constructor
	Camera(const Camera & cam);

	// computet the direction and the position of the camera
	void computePosAndDir();

	// draw camera
	void draw();

	//write camera information
	//void write(cv::FileStorage& fs) const;

	// read camera information
	//void read(const cv::FileNode& node);

	// projcet a point from 3D to 2D
	//qglviewer::Vec  project(const qglviewer::Vec & coord);

	// project a point from 3D to 2D
	QPoint  project(const Point &coord);


public:

	float focal_;
	cv::Mat rotation_;
	cv::Mat trans_;
	cv::Mat project_;

	float k0;
	float k1;

	cv::Mat pos_;
	cv::Mat dir_;
	string img_dir_;

	QColor color_;

	// axis
	cv::Mat xaxis_;
	cv::Mat yaxis_;
	cv::Mat zaxis_;
};

///////////////////////////////////////////////Plane3D/////////////////////////////////////////////////////////////////////
class Plane3D
{
public:

	Plane3D(){}

	// fit a plane to the points stores in P
	void fittingPlane(QVector<Point> &points);

	// project a 3D point onto the plane
	QPointF cvt3Dto2D(const Point &pt3D);

	// convert 2D to 3D
	Point cvt2Dto3D(const QPointF &pt2D);

	

public:
	
	QVector<Point> p_points_on_plane_;
	QVector<Point> p_transformed_pts_;
	
	QVector<Point> p_image_pixels_;
	QVector<Point> p_image_pixels_trans_;

	Point p_top_left_;
	Point p_top_right_;
	Point p_bottom_right_;
	Point p_bottom_left_;

	Point p_top_left_trans_;
	Point p_top_right_trans_;
	Point p_bottom_right_trans_;
	Point p_bottom_left_trans_;

	// coordinat system
	qglviewer::Vec p_normal_;
	float p_d_;
	qglviewer::Vec p_frame_x_;
	qglviewer::Vec p_frame_y_;
	qglviewer::Vec p_center_;

};
#endif // DATATYPE_H
