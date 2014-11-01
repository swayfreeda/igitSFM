#include "data_type.h"


////////////////////////////////////ANGle-AXIS METHOD///////////////////////////////////////////////
float angleAxisFromDirections(qglviewer::Vec & src, qglviewer::Vec& dest, qglviewer::Vec & axis)
{
	// source direction
	cv::Mat direc1(3, 1, CV_32FC1);
	direc1.at<float>(0) = src.x; direc1.at<float>(1) = src.y; direc1.at<float>(2) = src.z;

	// destinate direction
	cv::Mat direc2(3, 1, CV_32FC1);
	direc2.at<float>(0) = dest.x; direc2.at<float>(1) = dest.y; direc2.at<float>(2) = dest.z;

	// axis
	cv::Mat Axis = direc1.cross(direc2);
	double innerPro = direc1.dot(direc2);
	double angle = acos(innerPro / (norm(direc1)*norm(direc2)));

	Axis = Axis / cv::norm(Axis);

	axis.x = Axis.at<float>(0);
	axis.y = Axis.at<float>(1);
	axis.z = Axis.at<float>(2);

	direc1.release();
	direc2.release();
	Axis.release();

	return angle;


}





///////////////////////////////////////constructor///////////////////////////////////////////////
Camera::Camera(const Camera & cam)
{
	this->focal_= cam.focal_;
	this->k0 = cam.k0;
	this->k1 = cam.k1;

    this->color_ = cam.color_;
	this->img_dir_ = cam.img_dir_;

	cam.rotation_.copyTo(this->rotation_);
	cam.trans_.copyTo(this->trans_);
	cam.project_.copyTo(this->project_);

	cam.pos_.copyTo(this->pos_);
	cam.dir_.copyTo(this->dir_);
	
	cam.xaxis_.copyTo(this->xaxis_);
	cam.yaxis_.copyTo(this->yaxis_);
	cam.zaxis_.copyTo(this->zaxis_);

}
//////////////////////////////////////project////////////////////////////////////////////////////
QPoint  Camera::project(const Point &coord3D)
{
	cv::Mat p3D(4, 1, CV_32FC1);
	cv::Mat p2D(3, 1, CV_32FC1);

	// cout<<"3D: "<<"x: "<<pt.x<<", y: "<<pt.y<<", z: "<<pt.z<<endl;
	p3D.at<float>(0) = coord3D.x;
	p3D.at<float>(1) = coord3D.y;
	p3D.at<float>(2) = coord3D.z;
	p3D.at<float>(3) = 1.0;

	p2D = project_ * p3D;

	QPoint coord2D;
	coord2D.setX((int)p2D.at<float>(0) / p2D.at<float>(2));
	coord2D.setY((int)p2D.at<float>(1) / p2D.at<float>(2));

	p3D.release();
	p2D.release();
	return coord2D;
}
///////////////////////////////////////draw//////////////////////////////////////////////////////
void Camera::draw()
{

	qglviewer::Vec axis;
	qglviewer::Vec src(0, 0, -1);
	qglviewer::Vec dirst(this->dir_.at<float>(0), this->dir_.at<float>(1), this->dir_.at<float>(2));

	// compute the angle
	float angle = angleAxisFromDirections(src, dirst, axis);

	float x = this->pos_.at<float>(0);
	float y = this->pos_.at<float>(1);
	float z = this->pos_.at<float>(2);

	glTranslatef(this->pos_.at<float>(0), this->pos_.at<float>(1), this->pos_.at<float>(2));
	glRotatef(angle * 180 / 3.1415, axis.x, axis.y, axis.z);
	glColor3f((GLfloat)(this->color_.red()) / 255.0,
		(GLfloat)(this->color_.green()) / 255.0,
		(GLfloat)(this->color_.blue()) / 255.0);

	glBegin(GL_QUADS);

	glVertex3f(-0.4, 0.3, -this->focal_ / (5000.0));
	glVertex3f(0.4, 0.3, -this->focal_ / (5000.0));
	glVertex3f(0.4, -0.3, -this->focal_ / (5000.0));
	glVertex3f(-0.4, -0.3, -this->focal_ / (5000.0));

	glEnd();

	glBegin(GL_LINES);
	glVertex3f(-0.4, 0.3, -this->focal_ / (5000.0));
	glVertex3f(0, 0, 0);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.4, 0.3, -this->focal_ / (5000.0));
	glVertex3f(0, 0, 0);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.4, -0.3, -this->focal_ / (5000.0));
	glVertex3f(0, 0, 0);
	glEnd();


	glBegin(GL_LINES);
	glVertex3f(-0.4, -0.3, -this->focal_ / (5000.0));
	glVertex3f(0, 0, 0);
	glEnd();
}
/////////////////////////////////////compute Position and Dir ///////////////////////////////////////////////////////////
void Camera::computePosAndDir()
{
	// compute the position
	cv::Mat pos = -rotation_.t()* trans_;
	for (int i = 0; i<3; i++)
	{
		pos_.at<float>(i) = pos.at<float>(i);
	}
	pos.release();
	// compute the direction
	cv::Mat dir_oir(3, 1, CV_32FC1);
	dir_oir.setTo(0);
	dir_oir.at<float>(2) = -1;

	cv::Mat dir = rotation_.t()* dir_oir;
	for (int i = 0; i<3; i++)
	{
		dir_.at<float>(i) = dir.at<float>(i);
	}

	pos.release();
	dir_oir.release();
	dir.release();

}
/////////////////////////////////////overload =//////////////////////////////////////////////////////////////////////////
Camera & Camera::operator =(const Camera & cam)
{
	this->focal_ = cam.focal_;
	this->k0 = cam.k0;
	this->k1 = cam.k1;

	this->color_ = cam.color_;
	this->img_dir_ = cam.img_dir_;

	cam.rotation_.copyTo(this->rotation_);
	cam.trans_.copyTo(this->trans_);
	cam.project_.copyTo(this->project_);

	cam.pos_.copyTo(this->pos_);
	cam.dir_.copyTo(this->dir_);

	cam.xaxis_.copyTo(this->xaxis_);
	cam.yaxis_.copyTo(this->yaxis_);
	cam.zaxis_.copyTo(this->zaxis_);

	return *this;
}

/////////////////////////////////////fitting plane///////////////////////////////////////////////////////////////////////
void Plane3D::fittingPlane(QVector<Point> & points)
{
	int  pts_num = points.size();
	cv::Mat features(pts_num, 3, CV_32FC1);

	for (int i = 0; i< points.size(); i++)
	{
		features.at<float>(i, 0) = points[i].x;
		features.at<float>(i, 1) = points[i].y;
		features.at<float>(i, 2) = points[i].z;
	}

	// center of the plane
	cv::Mat mean(3, 1, CV_32FC1);
	mean.setTo(0);
	for (int i = 0; i< pts_num; i++)
	{
		mean.at<float>(0) += features.at<float>(i, 0) / pts_num;
		mean.at<float>(1) += features.at<float>(i, 1) / pts_num;
		mean.at<float>(2) += features.at<float>(i, 2) / pts_num;
	}
	for (int i = 0; i< 3; i++) p_center_[i] = mean.at<float>(i);

	for (int i = 0; i<pts_num; i++)
	{
		features.at<float>(i, 0) -= mean.at<float>(0);
		features.at<float>(i, 1) -= mean.at<float>(1);
		features.at<float>(i, 2) -= mean.at<float>(2);
	}

	// compute parameters
	cv::Mat covar_matrix = (features.t()*features) / pts_num;

	cv::SVD svd;
	cv::Mat U, S, V;
	svd.compute(covar_matrix, S, U, V);


	 p_normal_[0] = U.at<float>(0,2);
	 p_normal_[1] = U.at<float>(1,2);
	 p_normal_[2] = U.at<float>(2,2);


	qglviewer::Vec frameZ = p_normal_;
	p_frame_y_ = qglviewer::Vec(U.at<float>(0, 0), U.at<float>(1, 0), U.at<float>(2, 0));
	p_frame_x_ = cross(p_frame_y_, frameZ);

	float d = p_normal_ *p_center_;
	p_d_ = -d;

	features.release();
	mean.release();
	covar_matrix.release();
	U.release();
	V.release();
	S.release();
}
////////////////////////////////////cvt3Dto2D////////////////////////////////////////////////////////////////////////////
QPointF Plane3D::cvt3Dto2D(const Point &pt3D)
{
	// translate 3D to 2D
	QPointF pt2D;
	qglviewer::Vec p(pt3D.x, pt3D.y, pt3D.z);
	pt2D.setX( (p - p_center_)* p_frame_x_ );
	pt2D.setY( (p - p_center_)* p_frame_y_ );

	return pt2D;
}
////////////////////////////////////cvt2Dto3D///////////////////////////////////////////////////////////////////////////
Point Plane3D::cvt2Dto3D(const QPointF &pt2D)
{
	qglviewer::Vec pt = pt2D.x()*  p_frame_x_ + pt2D.y() * p_frame_y_ + p_center_;
	Point pt3D(pt.x, pt.y, pt.z);

	return pt3D;
}
