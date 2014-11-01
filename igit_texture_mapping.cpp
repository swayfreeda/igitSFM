#include"igit_texture_mapping.h"
#include"igit_functions.h"
#include "igit_SFM.h"


#include<qdir.h>
#include<qtextstream.h>
#include<opencv2\opencv.hpp>
#include<opencv2\core\core.hpp>
#include<qdebug.h>
#include<QProgressDialog>
#include"qprocess.h"

#include"qapplication.h"
#include <ctime>

//******************************************NON CLASS MEMBERS****************************************************************//
///////////////////////////////////////////readProjFromFile///////////////////////////////////////////////////////////////////
void readProjFromFile(cv::Mat& proj, QString txt_name)
{
	//  cout<<txt_name.toStdString()<<endl;
	QFile file(txt_name);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	}
	else{

		QTextStream in(&file);
		int line_num = 0;
		while (!in.atEnd())
		{
			QString line = in.readLine();
			// cout<<line.toStdString()<<endl;
			if (line_num > 0)
			{
				QStringList fields = line.split(" ");
				for (int i = 0; i < 4; i++)
				{
					proj.at<float>(line_num - 1, i) = fields.takeFirst().toFloat();
				}
			}
			line_num++;
		}
	}
}
///////////////////////////////////////////decomposedProjMatrix///////////////////////////////////////////////////////////////
Camera decomposeProjMatrix(const cv::Mat & proj)
{
	Camera cam;

	// projection matrix
	proj.copyTo(cam.project_);

	// direction
	cam.dir_.at<float>(0) = cam.project_.at<float>(2, 0);
	cam.dir_.at<float>(1) = cam.project_.at<float>(2, 1);
	cam.dir_.at<float>(2) = cam.project_.at<float>(2, 2);
	cam.dir_ = cam.dir_ / cv::norm(cam.dir_);

	// get position
	cv::Mat KR(3, 3, CV_32FC1);
	cv::Mat KT(3, 1, CV_32FC1);

	// get position
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			KR.at<float>(i, j) = cam.project_.at<float>(i, j);
		}
	}
	for (int i = 0; i < 3; i++)
		KT.at<float>(i, 0) = cam.project_.at<float>(i, 3);

	cam.pos_ = -KR.inv()* KT;

	// compute the focal
	cv::Mat R0(3, 1, CV_32FC1);
	cv::Mat R1(3, 1, CV_32FC1);
	cv::Mat R2(3, 1, CV_32FC1);
	for (int i = 0; i < 3; i++)
	{
		R0.at<float>(i) = KR.at<float>(0, i);
		R1.at<float>(i) = KR.at<float>(1, i);
		R2.at<float>(i) = KR.at<float>(2, i);
	}
	cam.focal_ = 0.5*abs(norm(R0.cross(R2))) + 0.5*abs(norm(R1.cross(R2)));

	// axises of the camera
	cam.zaxis_ = cam.dir_;
	cam.yaxis_ = cam.zaxis_.cross(R0);
	cam.yaxis_ = cam.yaxis_ / norm(cam.yaxis_);

	cam.xaxis_ = cam.yaxis_.cross(cam.zaxis_);
	cam.xaxis_ = cam.xaxis_ / norm(cam.xaxis_);

	R0.release();
	R1.release();
	R2.release();
	KR.release();
	KT.release();
	return cam;
}
///////////////////////////////////////////loadVisibilityFromPatchFile////////////////////////////////////////////////////////
QVector<QVector<int> > loadVisibilityFromPatchFile(QString patch_file_name)
{
	QVector<QVector<int> > vis;
	QFile file(patch_file_name);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	}
	else{

		QTextStream in(&file);
		int line_num = 0;
		int counter = 0;

		while (!in.atEnd())
		{
			QString line = in.readLine();
			QStringList fields = line.split(" ");
			if (line_num == 1)
			{
				int points_num = fields.takeFirst().toInt();
				//vis.resize(points_num);
			}
			if (line_num > 1)
			{
				if (line.startsWith("PATCH"))
				{
					counter = 0;
				}
				if (counter == 5){
					QVector<int> tmp;
					while (fields.size() != 0)
					{
						tmp.append(fields.takeFirst().toInt());
					}
					tmp.pop_back();
					vis.append(tmp);
				}
				counter++;
			}
			line_num++;
		}
		return vis;
	}
}

//******************************************* CLASS MEMBERS*****************************************************************//
////////////////////////////////////////////iniitialization///////////////////////////////////////////////////////////////////
void TextureMapping::initialization()
{

	//-----------------------------load texture images ---------------------------------//
	// texture images are loaded from pmvs/visualize
	// we load all the images right now which used a lot of memory, which should be changed
	emit textEdit("[Loading Texture Images...");
	emit statusBar("Loading Texture Images");

	//***********************************************************************************//
	// 考虑到内存需要做一些修改。 主要是两个部分用到了图像：1. MRF 进行梯度运算 2. 创建纹理图像
	// 进行拼图。 前者需要所有的图像，但是实际上只需要灰度图像即可。后者需要彩色图像，但是只需要
	//只需要加载实际用到的视角即可。这样可以起到节省内存的作用
	//************************************************************************************//
	t_gray_images_.clear();
	loadGrayImages();
	emit textEdit(" Done!]");


	//----------------------------load projection matrix--------------------------------//
	// we loaded projection matrix from pmvs/txt/ and each txt corresponding to a projection 
	// matrix
	emit textEdit("[ Loading Projection Matrix...");
	emit statusBar("Loading Projection Matrix");

	t_cameras_->clear();
	if (loadCameras())emit enableActionCameras();
	emit textEdit(" Done!]");

	//----------------------------load visibility information --------------------------//
	// visibility information are loaded from *.patch file, this file containes the coordinates,
	// the normal, the photometric consistency score of each patch, following the visible views
	// The visible views are the visibility information we needed
	emit textEdit("    [ Loading Visibility Infomation...");
	emit statusBar("Loading Visibility Infomation");

	for (int i = 0; i < t_dense_pts_->size(); i++)(*t_dense_pts_)[i].vis.clear();
	loadVisibility();
	emit textEdit("    Done!]");

	//------------------------------intial data -------------------------------------------//
	// visibility data in t_dense_vis_ is one to one correponding to t_points_ids not t_dense_pts_
	// for some points in t_dense_pts_ may be deleted as noise
	// Note that the data in t_points_ids are not from little to large, it' order is interupted!!!
#if 1
	t_dense_vis_.clear();
	foreach(int id, *t_points_ids_)
	{
		t_dense_vis_.append((*t_dense_pts_)[id].vis);
	}
#endif

#if 0
	t_dense_vis_.clear();
	foreach(Point pt, *t_dense_pts_)
	{
		t_dense_vis_.append(pt.vis);
	}
#endif
	//--------------------------------visibility of vertices -----------------------------------------------------//
	// Note that the visibility loaded from file are visibility of dense points and the vertices are interploated by
	// poisson suface reconstruction, and hence the visibilty of the vertices should be computed. Once the visibilities 
	// of the facets are computed the visibilities of a facet can be obtainded by calculate the intersection set of 
	// the visibilities fof the vertex. And besides, the normal of a vertex can by caluculate as the mean of it's nieghbours
	emit textEdit("    [Compute the Visibility of V ertices...");
	emit statusBar("Compute the Visibility of Vertices");
	t_vertices_vis_.clear();
	computeVertexNormalAndVisibility();
	emit textEdit("    Done!]");

	int valid_num0 = 0;
	for (int i = 0; i < t_vertices_vis_.size(); i++)
	{
		if (t_vertices_vis_[i].size() > 0) valid_num0++;
	}
	float aspect = (float)valid_num0 / (float)t_vertices_vis_.size();
	QString txt0 = QString("%1 Valid Vectices %2 Vertices Radio: %3").arg(valid_num0).arg(t_vertices_vis_.size()).arg(aspect);
	textEdit(txt0);

	//---------------------------compute the visibility of facet from file-----------------------//
	// the visibility information of the facet needed to be computed. First the visibility of each
	// vertices are computed, and then the visibility of the facet are computed as the intersection
	// of the visible views of all the vertices
	emit textEdit("    [Compute the Visibility of Facets...");
	emit statusBar("Compute the Visibility of Facets");

	t_initial_facet_vis_.clear();
	t_initial_facet_vis_.resize(t_facets_->size());
	computeFacetsVisibilityFromFile();
	emit textEdit("    Done!]");

	int valid_num11 = 0;
	int total_num11 = t_initial_facet_vis_.size();
	for (int i = 0; i < t_initial_facet_vis_.size(); i++)
	{
		if (t_initial_facet_vis_[i].size() >= 2) valid_num11++;
	}
	float radio11 = (float)valid_num11 / (float)total_num11;
	QString txt11 = QString("Valid facets num: %1 Total num : %2 Radio : %3").arg(valid_num11).arg(total_num11).arg(radio11);
	textEdit(txt11);

	//---------------------------compute the texture coordinates ----------------------//
	// each vertices of the facet are projected into their corresponding view of images, and
	// the texture coordinates are retained for filter the visibility and for texture mapping
	// and in the final the texture coordinates should be normalized to [0, 1]
	emit textEdit("    [Compute the Texture Coordinates...");
	emit statusBar("Compute the Texture Coordinates");

	t_initial_facet_coordinates_.clear();
	t_initial_facet_coordinates_.resize(t_facets_->size());
	computeTextureCoordinates();
	emit textEdit("    Done!]");


#if 1
	//---------------------------filter the visibility----------------------------------------//
	// the visibility of each facet is filterd, to improve the quality of the texture image, 
	// each points only the view through which the 3D point projected into the nearly center
	// of the image are maitained, others are filtered out.
	emit textEdit("    [Filter the Visibility...");
	emit statusBar("Filter the Visibility");

	filterVisibility();
	emit textEdit("    Done!]");

	int valid_num1 = 0;
	int total_num1 = t_initial_facet_vis_.size();
	for (int i = 0; i < t_initial_facet_vis_.size(); i++)
	{
		if (t_initial_facet_vis_[i].size() >= 2) valid_num1++;
	}
	float radio = (float)valid_num1 / (float)total_num1;
	QString txt1 = QString("Valid facets num: %1 Total num : %2 Radio : %3").arg(valid_num1).arg(total_num1).arg(radio);
	textEdit(txt1);

#endif

	//---------------------------computeFacetsVisibilityViaProjection-------------------------------------------//
	/// A part of facets' visibiulity can not be found from the visibility loaded from file, as a supplement, we
	// calculate the visibilities through projection. We project the facet to all the views and if the projectd coordinates
	// are among a reasonable region the visibility and the coordinates are maintained.
	emit textEdit("    [compute Facets Visibility Via Projection...");
	emit statusBar("compute Facets Visibility ViaProjection");
	computeFacetsVisibilityViaProjection();
	emit textEdit("    Done!]");


#if 0
	//----------------------------computeFacetsVisibilityAndCoordinate-------------------//
	// compute the texture coordinates and visibility of facets
	// we project the vertices of each facet into all the views and if the projections of all the
	// vertices are close to the center of an image, we consider the view is visible for the facet
	// We can do this because there are very little occlusions from air to ground view.
	emit textEdit("[Compute Facets Visibility And Texture Coordinates...");
	emit statusBar("Compute Facets Visibility And Texture Coordinates");

	computeFacetsVisibilityAndCoordinates();
	emit textEdit("Done!]");
#endif
	//--------------------------trim the facets------------------------------------------------//
	// after poisson surface reconstruction there are many facets that are generated without points
	// nearby, and hence there will be no texture mapping form them. We elaminate the facets that
	// hase no visible view

	emit textEdit("[Trim the Surface...");
	emit statusBar("Trim the Surface");

	facetTrimmer();
	emit textEdit("Done!]");

	//*********************************************************************************************//
	//* 还可以根据NCC 或者 SIFT描述子进行进一步的删除，另外本身自带的visibility在这里没有用到    *//
	//*                                                                                          *//
	//***********************************************************************************************//
}
///////////////////////////////////////////loadTextureImages/////////////////////////////////////////////////////////////////
bool TextureMapping::loadGrayImages()

{
	cv::Mat img;

	QDir dir;
	if (dir.exists(tr("pmvs/visualize")))
	{
		QString path = QDir::currentPath() + tr("/pmvs/visualize");
		dir.setPath(path);
		dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		QStringList filter;
		filter << "*.jpg";

		// ply files
		QFileInfoList fileList = dir.entryInfoList(filter);
		int nFiles = fileList.size();

		for (int i = 0; i < nFiles; i++)
		{
			// text file
			QString img_file_name = fileList.at(i).filePath();
			textEdit(img_file_name);

			QStringList fields = img_file_name.split("/");
			QString name(fields.takeLast());
			//QImage img;
			//img.load(img_file_name);
			img = cv::imread(img_file_name.toStdString().c_str(), 0);

			if (t_height_ == 0 || t_width_ == 0)
			{
				t_height_ = img.rows;
				t_width_ = img.cols;
			}
			t_gray_images_.insert(name, img);
		}
	}
	else{
		statusBar("Error to read projection matrixs");
	}
	img.release();
	return true;

}
///////////////////////////////////////////loadTextureImages/////////////////////////////////////////////////////////////////
bool TextureMapping::loadTextureImages()
{
	QImage img;
	t_texture_images_.clear();
	QDir dir;
	if (!dir.exists(tr("pmvs/visualize"))) return false;

	for (QMap<uint, uint> ::iterator iter = t_label_mapping_.begin(); iter != t_label_mapping_.end(); iter++)
	{
		uint vis_id = iter.key();
		QString name;
		name.sprintf("%08d.jpg", (int)vis_id);

		QString img_dir = "pmvs/visualize/" + name;

		textEdit(img_dir);
		img.load(img_dir);

		t_texture_images_.insert(name, img);
	}
	return true;
}
//////////////////////////////////////////////loadProjMatrix/////////////////////////////////////////////////////////////////
bool TextureMapping::loadCameras()
{
	QDir dir;
	if (dir.exists(tr("pmvs/txt")))
	{
		QString path = QDir::currentPath() + tr("/pmvs/txt");
		dir.setPath(path);
		dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		QStringList filter;
		filter << "*.txt";

		// ply files
		QFileInfoList fileList = dir.entryInfoList(filter);
		int nFiles = fileList.size();

		for (int i = 0; i < nFiles; i++)
		{
			// text file
			QString txt_file_name = fileList.at(i).filePath();
			textEdit(txt_file_name);

			QStringList fields = txt_file_name.split("/");
			QString name(fields.takeLast());
			name.replace(".txt", ".jpg");

			if (!txt_file_name.isEmpty())
			{
				cv::Mat proj(3, 4, CV_32FC1);
				readProjFromFile(proj, txt_file_name);
				Camera cam = decomposeProjMatrix(proj);

				cam.color_ = QColor((int)rand() & 255, (int)rand() & 255, (int)rand() & 255);
				cam.img_dir_ = name.toStdString();

				t_cameras_->insert(name, cam);
			}
		}
	}
	else{
		statusBar("Error to read projection matrixs");
	}
	return true;
}
///////////////////////////////////////////loadVisibility////////////////////////////////////////////////////////////////////
bool TextureMapping::loadVisibility()
{
	QVector<QVector<int> > all_vis;

	QDir dir;
	if (!dir.exists(tr("pmvs/models"))) return false;
	QString path = QDir::currentPath() + tr("/pmvs/models");
	dir.setPath(path);
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QStringList filter;
	filter << "*.patch";

	// ply files
	QFileInfoList fileList = dir.entryInfoList(filter);
	int nFiles = fileList.size();

	// create a progress dialog
	//QProgressDialog progress;
	//progress.setLabelText(tr("Loading Visibility Points..."));
	//progress.setWindowModality(Qt::WindowModal);
	//progress.setRange(0, nFiles);

	int nSteps = 0;
	for (int i = 0; i < nFiles; i++)
	{
		//progress.setValue(nSteps);
		//qApp->processEvents();

		// the last file containes all the points
		QString patch_file_name = fileList.at(i).filePath();

		textEdit("Loading Visibility From " + patch_file_name);
		//load points
		QVector<QVector<int> > sub_vis = loadVisibilityFromPatchFile(patch_file_name);
		foreach(QVector<int> v, sub_vis)
		{
			all_vis.append(v);
		}
		sub_vis.clear();
		//if (progress.wasCanceled())
		//{
		//	return false;
		//}

		nSteps++;

	}
	if (all_vis.size() != t_dense_pts_->size())
	{
		statusBar("Warning： Visibility Does not Match Dense Points! ");
	}
	else{
		// we do this because some points may be elaminated as noise and the visibility must correspod to each 
		// point

		for (int i = 0; i < all_vis.size(); i++)
		{
			(*t_dense_pts_)[i].vis.swap(all_vis[i]);
		}
	}
	all_vis.clear();
	return true;
}
///////////////////////////////////////////comput Vertex visibility /////////////////////////////////////////////////////////
void TextureMapping::computeVertexNormalAndVisibility()
{
	//----------------------------------------compute K nearest neighbours-------------------------------------------------//
	// dense points are generated from PMVS and contain visibility information, while vertices of mesh are not for they are obtained
	// by interplotation. We first compute the neighbours of each vertex and then get the visibility from the neighbours.
	emit textEdit("  [Compute K Nearest Neighbours...");
	emit statusBar("Compute K Neares Neighbours ");
	int Knn = 5;
#if 1
	QVector<Point> dense_points;
	foreach(int id, *t_points_ids_)
	{
		dense_points.append((*t_dense_pts_)[id]);
	}
#endif
	QVector<QVector<int> > neighbours = kNearesNeighbours(Knn, *t_vertices_, dense_points);
	emit textEdit("  Done!]");

	//----------------------------------------compute the normal of each vertex------------------------------------------//
	// Dense points generated from cmvs owns normal information but the vertices  do not. We recovered the normal of each 
	// vertex by calculating the mean of the neighbours's normals.
	emit textEdit("  [Compute Normals of Vertices...");
	emit statusBar("Compute Normals of Vertices ");
	int counter = 0;
	foreach(QVector<int> nhbr, neighbours)
	{
		qglviewer::Vec normal(0, 0, 0);
		foreach(int pt_id, nhbr)
		{
			normal.x += (*t_dense_pts_)[pt_id].normal_x;
			normal.y += (*t_dense_pts_)[pt_id].normal_y;
			normal.z += (*t_dense_pts_)[pt_id].normal_z;
		}
		normal = normal / Knn;
		normal.normalize();

		(*t_vertices_)[counter].normal_x = normal.x;
		(*t_vertices_)[counter].normal_y = normal.y;
		(*t_vertices_)[counter].normal_z = normal.z;

		counter++;
	}
	emit textEdit("  Done!]");

	//----------------------------------------computhe the visibility of each vertex-------------------------------------//
	// we have no visibility information about the vertices, since the vertices do not reconstructed by PMVS
	// but are generated by interplotation. We get the visibility of information a vertex by computing the histgram for 
	// visibility of K neighbours, and the vis whose merge more than 0.75 * Knn are maintained
	emit textEdit("  [Compute Visibility of Vertices...");
	emit statusBar("Compute Visibility of Vertices ");
	foreach(QVector<int>nhbr, neighbours)
	{
		// the key of map is the view index
		// the value of the map is the number it merges in neighbors
		QMap<int, int> map_vis;
		foreach(int pt_id, nhbr)// for each neighboring point
		{
			foreach(int vis_id, t_dense_vis_[pt_id])// the visible views of each neighboring point
			{
				if (map_vis.contains(vis_id)) map_vis[vis_id]++;
				else{
					map_vis[vis_id] =1;
				}
			}
		}
		// check the number each view merges, only whose value above 0.75 of its neighbors are maintained
		QVector<int> single_pt_vis;
		foreach(int key, map_vis.keys())
		{
			foreach(int value, map_vis.values(key))
			{
				if (value >= 2)
				{
					single_pt_vis.append(key);
				}
			}
		}
		t_vertices_vis_.append(single_pt_vis);
	}
	emit textEdit("  Done!]");
}
///////////////////////////////////////////computeVerticesVisibilityFromFile/////////////////////////////////////////////////
bool TextureMapping::computeFacetsVisibilityFromFile()
{
	//---------------------------------------compute the visibility of each facet-----------------------------------------//
	// since we have obtained the view of each vertices, and each facet is composed by several vertices
	// hence the view of each facet is computed by intersection of view set of each vertices
	int facet_id = 0;
	foreach(QVector<int> facet, *t_facets_)
	{
		QSet<int> intersection_vis;
		for (int i = 0; i < t_cameras_->size(); i++)
		{
			intersection_vis.insert(i);
		}

		// compute the intersection of the visibility of each vertices
		foreach(int id, facet)
		{
			QSet<int> tmp;
			foreach(int vis, t_vertices_vis_[id])
			{
				tmp.insert(vis);
			}
			intersection_vis = intersection_vis.intersect(tmp);
		}

		foreach(int id, intersection_vis)
		{
			t_initial_facet_vis_[facet_id].append(id);
		}
		facet_id++;
	}
	return true;
}
///////////////////////////////////////////computeFacetsVisibilityViaProjection//////////////////////////////////////////////
void TextureMapping::computeFacetsVisibilityViaProjection()
{
	for (int i = 0; i < t_initial_facet_vis_.size(); i++)
	{
		// ignore the facet that has visible views
		if (t_initial_facet_vis_[i].size() >= 2)continue;

		t_initial_facet_vis_[i].clear();
		t_initial_facet_coordinates_[i].clear();

		for (int j = 0; j < t_cameras_->size(); j++)// each vis
		{
			//**** project matrix****//
			int vis_id = j;
			QString name;
			name.sprintf("%08d.jpg", vis_id);
			Camera cam = (*t_cameras_)[name];

			//**** check whether this facet is visible in this view ****//
			bool valid = true;
			QVector<QPoint> coordinates;
			for (int k = 0; k < (*t_facets_)[i].size(); k++)// each vertex
			{
				int pt_id = (*t_facets_)[i][k];
				//*************** check the angle between vertex normal and the line************************//
				//*                                                                                        *//
				//*                                                                                        *//
				//**************************************end*************************************************//
				Point pt3D = (*t_vertices_)[pt_id];
				QPoint pt2D;
				projectionFrom3DTo2D(pt3D, cam.project_, pt2D);
				coordinates.append(pt2D);
				if (!isValidVis(pt2D, 0))
				{
					valid = false;
					break;
				}
			}
			//**** if the facet is view is visible ****//
			if (valid == true)
			{
				t_initial_facet_vis_[i].append(vis_id);
				t_initial_facet_coordinates_[i].append(coordinates);
			}
		}
	}

	int valid_num1 = 0;
	int total_num1 = t_initial_facet_vis_.size();
	for (int i = 0; i < t_initial_facet_vis_.size(); i++)
	{
		if (t_initial_facet_vis_[i].size() >= 2) valid_num1++;
	}
	float radio = (float)valid_num1 / (float)total_num1;
	QString txt1 = QString("Valid facets num: %1 Total num : %2 Radio : %3").arg(valid_num1).arg(total_num1).arg(radio);
	textEdit(txt1);
}
////////////////////////////////////////////computeTextureCoordinates///////////////////////////////////////////////////////////
void TextureMapping::computeTextureCoordinates()
{
	for (int i = 0; i < t_initial_facet_vis_.size(); i++)
	{
		if (t_initial_facet_vis_[i].size() == 0) continue;

		// texture coordinates of a facet in all views
		QVector< QVector<QPoint> > coordinates_of_all_views;
		for (int j = 0; j < t_initial_facet_vis_[i].size(); j++)
		{
			int vis_id = t_initial_facet_vis_[i][j];
			QString name;
			name.sprintf("%08d.jpg", vis_id);
			Camera cam = (*t_cameras_)[name];

			// coordinages
			QVector<QPoint> coordinates_of_each_view;
			foreach(int pt_id, (*t_facets_)[i])
			{
				Point pt3D = (*t_vertices_)[pt_id];
				QPoint pt2D;
				projectionFrom3DTo2D(pt3D, cam.project_, pt2D);

				coordinates_of_each_view.append(pt2D);
			}
			coordinates_of_all_views.append(coordinates_of_each_view);
		}
		t_initial_facet_coordinates_[i].swap(coordinates_of_all_views);
	}
}
//////////////////////////////////////////// filterVisibility //////////////////////////////////////////////////////////////////
void TextureMapping::filterVisibility()
{
	for (int i = 0; i < t_initial_facet_vis_.size(); i++)
	{
		if (t_initial_facet_vis_[i].size() == 0) continue;

		for (int j = 0; j < t_initial_facet_vis_[i].size(); j++)
		{
			// each visible view of facet
			int vis_id = t_initial_facet_vis_[i][j];
			// projected vertices of each facet
			QVector<QPoint> projected_vertices = t_initial_facet_coordinates_[i][j];

			//-------------------whether this view should be aborted or not ------------------------------//
			// if a facet is visible in a view it must satify that all the the projections of all
			// the verticea are near the center of the image
			bool aborted = false;
			for (int k = 0; k < projected_vertices.size(); k++)
			{
				QPoint pt2D = projected_vertices[k];
				if (!isValidVis(pt2D, 0.1))// this threshold is important
				{
					aborted = true;
					break;
				}
			}
			if (aborted == true)
			{
				t_initial_facet_vis_[i].clear();
				t_initial_facet_coordinates_[i].clear();
			}
		}
	}
}
//////////////////////////////////////////// isValidVis ///////////////////////////////////////////////////////////////////////
bool TextureMapping::isValidVis(QPoint pt, float thresh)
{
	int w0 = int(thresh * (float)t_width_ + 0.5);
	int w1 = int((1 - thresh)* (float)t_width_ + 0.5);
	int h0 = int(thresh * (float)t_height_ + 0.5);
	int h1 = int((1 - thresh) * (float)t_height_ + 0.5);

	if (pt.x() > w0 && pt.x() < w1 && pt.y() > h0&& pt.y() < h1)
	{
		return true;
	}
	else{
		return false;
	}
}
///////////////////////////////////////////facet trimmer //////////////////////////////////////////////////////////////////////
void TextureMapping::facetTrimmer()
{
	//---------------------eliminate the facets with visible views  less than 2--------------------------------//
	QVector<QVector<int> > facets_new;
	QVector<QVector<QVector<QPoint> > > texture_coordinates_new;
	QVector<QVector<int > > facet_vis_new;

	for (int i = 0; i < t_initial_facet_vis_.size(); i++)
	{
		if (t_initial_facet_vis_[i].size() > 1)
		{
			facets_new.append((*t_facets_)[i]);
			texture_coordinates_new.append(t_initial_facet_coordinates_[i]);
			facet_vis_new.append(t_initial_facet_vis_[i]);
		}
	}
	t_facets_->swap(facets_new);
	t_initial_facet_coordinates_.swap(texture_coordinates_new);
	t_initial_facet_vis_.swap(facet_vis_new);


	//------------------update vertice, facets, edges, and texture coordinates---------------------------------//
	// collect all the facets
	QVector<QVector<Point> > all_facets;
	all_facets.resize(t_facets_->size());
	int counter = 0;
	foreach(QVector<int> facet, *t_facets_)
	{
		foreach(int id, facet)
		{
			all_facets[counter].append((*t_vertices_)[id]);
		}
		counter++;
	}

	// updating
	updateMesh(all_facets);

	facets_new.clear();
	texture_coordinates_new.clear();
	facet_vis_new.clear();
	all_facets.clear();
}
///////////////////////////////////////////updateMeshWithTextureCoords///////////////////////////////////////////////
void TextureMapping::updateMesh(QVector<QVector<Point> > & facets)
{
	//-----------------------------------update vertices, edges and facets------------------------------------//
	// create  a table  for inquerying the new index of vertex
	// Note: we can do this because the structure of Point is special defined(overlod of operator < in "data_type.h").
	// and may not work for other type of structure.
	map<Point, int> table;
	foreach(QVector<Point> facet, facets)
	{
		foreach(Point pt, facet)
		{
			table.insert(make_pair(pt, 0));
		}
	}

	//attach index to each point
	int index = 0;
	for (map<Point, int> ::iterator iter = table.begin(); iter != table.end(); iter++)
	{
		iter->second = index;
		index++;
	}

	QVector<Point> new_vertices;
	QVector<QVector<int> > new_facets;
	QVector<QPair<int, int> > new_edges;

	// get new vertices
	for (map<Point, int> ::iterator iter = table.begin(); iter != table.end(); iter++)
	{
		new_vertices.append(iter->first);
	}

	// get new facets
	foreach(QVector<Point> facet, facets)
	{
		QVector<int> facetID;
		foreach(Point pt, facet)
		{
			facetID.append(table[pt]);
		}
		new_facets.append(facetID);
	}

	// get new edges
	QSet<QPair<int, int> > e;
	foreach(QVector<int> facet, new_facets)
	{
		int pt_num = facet.size();
		for (int i = 0; i < pt_num; i++)
		{
			int id0 = facet[i];
			int id1 = facet[(i + 1) % pt_num];

			if (id0 < id1) e.insert(qMakePair(id0, id1));
			if (id1 < id0) e.insert(qMakePair(id1, id0));
		}
	}
	QSet<QPair<int, int> > ::const_iterator iter = e.constBegin();
	while (iter != e.constEnd())
	{
		new_edges << (*iter);
		iter++;
	}


	t_vertices_->swap(new_vertices);
	t_edges_->swap(new_edges);
	t_facets_->swap(new_facets);

	table.clear();
	new_vertices.clear();
	new_facets.clear();
	new_edges.clear();
	e.clear();

}
///////////////////////////////////////////computeFacetsVisibilityAndCoordinates/////////////////////////////////////
void TextureMapping::computeFacetsVisibilityAndCoordinates()
{
	cv::Mat proj;
	t_initial_facet_vis_.clear();
	t_initial_facet_coordinates_.clear();

	t_initial_facet_vis_.resize(t_facets_->size());
	t_initial_facet_coordinates_.resize(t_facets_->size());

	for (int i = 0; i < t_facets_->size(); i++)// each facet
	{
		for (int j = 0; j < t_cameras_->size(); j++)// each vis
		{
			//**** project matrix****//
			int vis_id = j;
			QString name;
			name.sprintf("%08d.jpg", vis_id);
			Camera cam = (*t_cameras_)[name];

			//**** check whether this facet is visible in this view ****//
			bool valid = true;
			QVector<QPoint> coordinates;
			for (int k = 0; k < (*t_facets_)[i].size(); k++)// each vertex
			{
				int pt_id = (*t_facets_)[i][k];
				//*************** check the angle between vertex normal and the line************************//
				//*                                                                                        *//
				//*                                                                                        *//
				//**************************************end*************************************************//
				Point pt3D = (*t_vertices_)[pt_id];
				QPoint pt2D;
				projectionFrom3DTo2D(pt3D, cam.project_, pt2D);
				coordinates.append(pt2D);
				if (!isValidVis(pt2D, 0))
				{
					valid = false;
					break;
				}
			}
			//**** if the facet is view is visible ****//
			if (valid == true)
			{
				t_initial_facet_vis_[i].append(vis_id);
				t_initial_facet_coordinates_[i].append(coordinates);
			}
		}
	}
	proj.release();
}
//////////////////////////////////////////facetRelation////////////////////////////////////////////////////////////////////////
TextureMapping::FacetRelationShip TextureMapping::facetRelation(int i, int j)
{
	QVector<int> facet0 = (*t_facets_)[i];
	QVector<int> facet1 = (*t_facets_)[j];

	int counter = 0;
	foreach(int id0, facet0)
	{
		if (facet1.contains(id0))
		{
			counter++;
		}
	}
	if (counter == 1) return TextureMapping::COMMON_VERTEX;
	else if (counter == 2)  return TextureMapping::COMMON_EDGE;
	else {
		return TextureMapping::NONE;
	}
}
///////////////////////////////////////////kNearestNeighboursFacets////////////////////////////////////////////////////////////
QVector<QVector<int> > TextureMapping::kNearestNeighboursFacets(TextureMapping::FacetRelationShip type)
{
	//-------------------------------------  get the center of the facets  ---------------------------------------//
	QVector<Point> centers;
	foreach(QVector<int> facet, *t_facets_)
	{
		Point c(0, 0, 0);
		int N = facet.size();
		foreach(int id, facet)
		{
			c.x += 1.0 / (float)N * (*t_vertices_)[id].x;
			c.y += 1.0 / (float)N * (*t_vertices_)[id].y;
			c.z += 1.0 / (float)N * (*t_vertices_)[id].z;
		}
		centers.append(c);
	}

	//-------------------------------------- find the candidate neighbours of each facet -----------------------//
	QVector< QVector<int> >neighbours_tmp = kNearesNeighbours(10, centers, centers);


	//---------------------------------------find the final neighbours of each facet----------------------------//
	QVector< QVector<int> > neighbours;
	neighbours.resize(t_facets_->size());

	for (int i = 0; i < neighbours_tmp.size(); i++)
	{
		int id0 = i;
		for (int j = 0; j < neighbours_tmp[i].size(); j++)
		{
			int id1 = neighbours_tmp[i][j];
			if (id0 == id1) continue;

			// facets share a vertex or a facets are considered as neighbours
			if (facetRelation(id0, id1) == type)
			{
				neighbours[i].append(id1);
			}
		}
	}
	return neighbours;
}
////////////////////////////////////////////MRF_Optimization()//////////////////////////////////////////////////////////////////
void TextureMapping::MRF_Optimization()
{
#if DEBUG_
	//********************************debug--intial view labels*******************************//
	QFile file("DEBUG_intial_facet_vis.txt");
	if (!file.open(QIODevice::WriteOnly))
	{
	}
	QTextStream out(&file);
	out << t_initial_facet_vis_.size() << endl;
	foreach(QVector<int> vis, t_initial_facet_vis_)
	{
		foreach(int vis_id, vis)
		{
			out << vis_id << " ";
		}
		out << endl;
	}
#endif
	//*********************************optimize with MRF **************************************//

	//calculate neighbours of facets 
	textEdit("[Computing neighbours....");
	QVector<QVector<int> > neighbours = kNearestNeighboursFacets(TextureMapping::COMMON_EDGE);
	textEdit("Done!]\n");

	int site_num = t_facets_->size();
	int label_num = t_cameras_->size();

	IGIT_MRF mrf(site_num, label_num);
	mrf.setCandidateLabels(t_initial_facet_vis_);
	mrf.setCandidateTextureCoordinates(t_initial_facet_coordinates_);
	mrf.setFacetsPtr(t_facets_);
	mrf.setImagePtr(&t_gray_images_);
	mrf.setLambda(50000);
	mrf.setMaxIterNum(5);
	mrf.setNeighbours(neighbours);
	mrf.setVerticesPtr(t_vertices_);

	textEdit("[Computing Data Term Array...");
	mrf.computeDataTermArray();
	textEdit("Done!]\n");

	textEdit("[MRF Optimization...");
	mrf.optimization();
	textEdit("Done!]\n");

	// get final results
	t_final_facets_vis_.clear();
	t_final_facets_coordinates_.clear();
	t_final_facets_vis_ = mrf.getFinalLabels();


	for (int i = 0; i < t_final_facets_vis_.size(); i++)
	{
		int vis_id = t_final_facets_vis_[i];

		int ind = 0;
		for (int j = 0; j < t_initial_facet_vis_[i].size(); j++)
		{
			if (t_initial_facet_vis_[i][j] == vis_id)
			{
				ind = j;
				break;
			}
		}
		// ind  应该包含在t_initial_facet_vis_[i] 之中，如何保证？ MRF的lambda区很大的值
		t_final_facets_coordinates_.append(t_initial_facet_coordinates_[i][ind]);
	}
	neighbours.clear();
	//mrf.release();

#if 0
	//**************************NO MRF Optimization******************************//
	t_final_facets_vis_.clear();
	t_final_facets_coordinates_.clear();
	for (int i = 0; i < t_initial_facet_coordinates_.size(); i++)
	{

		QVector<QPair<float, int> > tables;
		for (int j = 0; j < t_initial_facet_coordinates_[i].size(); j++)
		{
			// triangulation in default
			QPoint p0 = t_initial_facet_coordinates_[i][j][0];
			QPoint p1 = t_initial_facet_coordinates_[i][j][1];
			QPoint p2 = t_initial_facet_coordinates_[i][j][2];

			// area of the triangulation
			qglviewer::Vec v0(p1.x() - p0.x(), p1.y() - p0.y(), 1);
			qglviewer::Vec v1(p2.x() - p0.x(), p2.y() - p0.y(), 1);
			qglviewer::Vec vv = cross(v0, v1);
			float area = 0.5 * qAbs(vv.norm());
			tables.append(qMakePair(area, j));
		}

		if (tables.size() == 0)
		{
			t_final_facets_vis_.append(-1);
			QVector<QPoint>p;
			p.resize(3);
			t_final_facets_coordinates_.append(p);
		}
		qSort(tables.begin(), tables.end());
		int id = tables[tables.size() - 1].second;

		t_final_facets_vis_.append(t_initial_facet_vis_[i][id]);
		t_final_facets_coordinates_.append(t_initial_facet_coordinates_[i][id]);
	}

#endif

}
////////////////////////////////////////////collectUsefulvis///////////////////////////////////////////////////////////////////
void TextureMapping::collectUsefulVis()
{
	t_label_mapping_.clear();
	foreach(uint vis_id, t_final_facets_vis_)
	{
		if (t_label_mapping_.contains(vis_id))continue;

		t_label_mapping_[vis_id] = 0;
	}
	int counter = 0;
	for (QMap<uint, uint> ::iterator iter = t_label_mapping_.begin(); iter != t_label_mapping_.end(); iter++)
	{
		iter.value() = counter;
		counter++;
	}
}
////////////////////////////////////////////getTransfromMat/////////////////////////////////////////////////////////////////////
cv::Mat TextureMapping::getTransfromMat()
{
	// fitting a plane from all the vertices
	t_projected_coordinates_.clear();
	t_plane_->fittingPlane(*t_vertices_);

	//---------------------------------------------Projected the points onto the plane----------------------------------------------//
	// project all the vertices onto the plane
	for (int i = 0; i < t_vertices_->size(); i++)
	{
		QPointF pt = t_plane_->cvt3Dto2D((*t_vertices_)[i]);
		t_projected_coordinates_.append(pt);
	}

	//----------------------------------------- a rectangle bounding box of this plane--------------------------------------------------//
	float min_x = 20000;
	float min_y = 20000;
	float max_x = -20000;
	float max_y = -20000;
	foreach(QPointF pt, t_projected_coordinates_)
	{
		if (min_x > pt.x()) min_x = pt.x();
		if (min_y > pt.y()) min_y = pt.y();
		if (max_x < pt.x()) max_x = pt.x();
		if (max_y < pt.y()) max_y = pt.y();
	}

	t_plane_->p_top_left_ = t_plane_->cvt2Dto3D(QPointF(min_x, min_y));
	t_plane_->p_top_right_ = t_plane_->cvt2Dto3D(QPointF(max_x, min_y));
	t_plane_->p_bottom_right_ = t_plane_->cvt2Dto3D(QPointF(max_x, max_y));
	t_plane_->p_bottom_left_ = t_plane_->cvt2Dto3D(QPointF(min_x, max_y));


	// transfom this plane to a image, size(1024, 1024), we get a affine matrix, indeed, this matrix is composed 
	// of rotation, translation, and unform scaling
	vector<cv::Point2f>src;
	cv::Point2f src_top_left_(min_x, min_y); src.push_back(src_top_left_);
	cv::Point2f src_top_right_(max_x, min_y); src.push_back(src_top_right_);
	cv::Point2f src_bottom_right_(max_x, max_y); src.push_back(src_bottom_right_);
	cv::Point2f src_bottom_left_(min_x, max_y); src.push_back(src_bottom_left_);
	vector<cv::Point2f> dst;


	cv::Point2f dst_top_left_(0.0, 0.0); dst.push_back(dst_top_left_);
	cv::Point2f dst_top_right_(1.0, 0.0); dst.push_back(dst_top_right_);
	cv::Point2f dst_bottom_right_(1.0, 1.0); dst.push_back(dst_bottom_right_);
	cv::Point2f dst_bottom_left_(0.0, 1.0); dst.push_back(dst_bottom_left_);
	
	cv::Mat transform = cv::findHomography(src, dst, 0, 0.001);

	src.clear();
	dst.clear();
	return transform;
}
///////////////////////////////////////////inWhichFacet////////////////////////////////////////////////////////////////////
int  TextureMapping::inWhichFacet(const QPointF &pt, const QVector<int>&facets, const QVector<QPolygonF> &polygons)
{
	for (int i = 0; i < facets.size(); i++)
	{
		int facet_id = facets[i];

		if (polygons[facet_id].containsPoint(pt, Qt::WindingFill))
		{
			return facet_id;
		}
		// for the case points are located on the edge
		QPointF _pt(pt.x() + 0.001, pt.y() + 0.001);
		if (polygons[facet_id].containsPoint(_pt, Qt::WindingFill))
		{
			return facet_id;
		}
	}

	return -1;
}
///////////////////////////////////////////createTextureImage///////////////////////////////////////////////////////////////////
void TextureMapping::createTextureImage(int imgSize)
{
	//-------------------------------------compute the transform Matrix ---------------------------------------/
	// all the vertices is 3D space are p[rojected onto the base plane first, and then a bounding box is found.
	// A tranform matrix is calculated to transform the bounding box into a standard rectangle with corners being
	// (0,0),(1,0), (1,1), (0,1). Indeed this is a affine transform composed with rotation, translation and unuiform
	// scaling. 2D coordinates of the vertices projected on the base plane are computed the same.
	cv::Mat transform = getTransfromMat();

	//contemprary variables used for calculation 
	cv::Mat coord_t(3, 1, CV_64FC1);// texture coordinates
	cv::Mat coord_b(3, 1, CV_64FC1);// coordinates on the base plane

	//-------------------------------------compute the texure coordinates ---------------------------------------/
	// compute the texture coordinate of each vertices
	textEdit("Beginning Creating Image...");
	t_texture_coords_->clear();
	foreach(QPointF pt_src, t_projected_coordinates_)
	{
		//2D coordinates of  projected points
		coord_b.at<double>(0) = pt_src.x();
		coord_b.at<double>(1) = pt_src.y();
		coord_b.at<double>(2) = 1.0;

		// transfrom the coordinates on the base plane to the image
		// and the results are the texture coordinates
		coord_t = transform * coord_b;

		QPointF pt_dst(coord_t.at<double>(0), coord_t.at<double>(1));

		//qDebug() << pt_dst.x() << " " << pt_dst.y() << endl;
		t_texture_coords_->append(pt_dst);

#if DEBUG_
		t_plane_->p_transformed_pts_.append(t_plane_->cvt2Dto3D(pt_dst));
#endif
	}

	//-------------------------------------back projection--------------- -------------------------------------/
	// for each position (u, v) in image, a inverse transfrom is conducted to get it's corresponding coordinate
	// in the base plane
	cv::Mat transform_inv = transform.inv();

	int texture_size = imgSize;
	int validH = texture_size;
	int validW = texture_size;

#if DEBUG_
	QVector<QColor> color_table;
	color_table.resize(t_cameras_->size());
	for (int i = 0; i < color_table.size(); i++)
	{
		color_table[i].setRed(rand() & 255);
		color_table[i].setGreen(rand() & 255);
		color_table[i].setBlue(rand() & 255);

	}
	for (int i = 0; i < transform_inv.rows; i++)
	{
		for (int j = 0; j < transform_inv.cols; j++)
		{
			qDebug() << transform_inv.at<double>(i, j) << " ";
		}
		qDebug() << endl;
	}
	foreach(QPointF pt2D, t_projected_coordinates_)
	{
		Point pt3D = t_plane_->cvt2Dto3D(pt2D);
		t_plane_->p_points_on_plane_.append(pt3D);
	}

	t_plane_->p_top_left_trans_ = t_plane_->cvt2Dto3D(QPointF(0, 0));
	t_plane_->p_top_right_trans_ = t_plane_->cvt2Dto3D(QPointF(1, 0));
	t_plane_->p_bottom_right_trans_ = t_plane_->cvt2Dto3D(QPointF(1, 1));
	t_plane_->p_bottom_left_trans_ = t_plane_->cvt2Dto3D(QPointF(0, 1));
#endif

	QVector<QPointF>b_coords;
	b_coords.resize(validH * validW);
	for (int i = 0; i < validH; i++)
	{
		for (int j = 0; j < validW; j++)
		{
			coord_t.at<double>(0) = (float)j / (float)texture_size;
			coord_t.at<double>(1) = (float)i / (float)texture_size;
			coord_t.at<double>(2) = 1.0;
			coord_b = transform_inv * coord_t;

			b_coords[i *validW + j].setX(coord_b.at<double>(0));
			b_coords[i *validW + j].setY(coord_b.at<double>(1));
#if DEBUG_
			QPointF img_pos(coord_t.at<double>(0), coord_t.at<double>(1));
			QPointF img_pos_trans(coord_b.at<double>(0), coord_b.at<double>(1));
			t_plane_->p_image_pixels_.append(t_plane_->cvt2Dto3D(img_pos));
			t_plane_->p_image_pixels_trans_.append(t_plane_->cvt2Dto3D(img_pos_trans));
#endif
		}
		}

	//------------------------------------K nearest neighbours ----------------------------------------//
	// k nerest neighbours of the back projected points are computed for aidding locating the 
	// facet the point falls in
	int Knn = 15;
	QVector<QVector<int> > neighbours = kNearestNeighbours2D(Knn, b_coords, t_projected_coordinates_);

	//------------------------------------Create a Vertex to Facet table  ----------------------------------------//
	//for each vertex we find all the facets that contain the vertex
	QVector<QVector<int> > vertex2facets;
	vertex2facets.resize(t_vertices_->size());
	for (int i = 0; i < t_facets_->size(); i++)
	{
		for (int j = 0; j < (*t_facets_)[i].size(); j++)
		{
			int pt_id = (*t_facets_)[i][j];
			vertex2facets[pt_id].append(i);
		}
	}

	//------------------------------------Finding candidate facets--------------------------------------------//
	// firs severak neighbouring points are found, and then all the facets that contain the neigbouring points
	// are treated as candidates that may contain the point.
	QVector<QVector<int> > facets_to_check;
	facets_to_check.resize(b_coords.size());
	for (int i = 0; i < neighbours.size(); i++)
	{
		QSet<int> facets;
		for (int j = 0; j < neighbours[i].size(); j++)
		{
			int id = neighbours[i][j];
			for (int k = 0; k < vertex2facets[id].size(); k++)
			{
				facets.insert(vertex2facets[id][k]);
			}
		}
		foreach(int id, facets)
		{
			facets_to_check[i].append(id);
		}
	}
	//------------------------------------reformat the facet to polygon--------------------------------------------//
	// each facet is stored as a polygon so that we can use the function 'containsPoint' to check
	// whether a point is in interior of the facet
	QVector<QPolygonF > all_facets;
	all_facets.resize(t_facets_->size());
	for (int i = 0; i < t_facets_->size(); i++)
	{
		for (int j = 0; j < (*t_facets_)[i].size(); j++)
		{
			int pt_id = (*t_facets_)[i][j];
			all_facets[i].append(t_projected_coordinates_[pt_id]);
		}
	}


	//-----------------------------------------create texture images ---------------------------------------------//
	// the texture images are created pixel by pixel. Fist a position (u,v) in the texure image is transformed back to 
	// the base plane to find which facet it locates in and the area coordinates are computed at the same time. From the 
	// facet we can find the visible view and from the corrsponding projected coordinates of the facet combined with 
	// the area coordinate, the position of (u, v) on the refrence image can be located. and then the pixel value is
	// assigned to (u, v).

	t_texture_image_ = QImage(texture_size, texture_size, QImage::Format_RGB32);

#if DEBUG_
	QImage mask_image(texture_size, texture_size, QImage::Format_RGB32);
#endif
	for (int i = 0; i < t_texture_image_.height(); i++)
	{
		for (int j = 0; j < t_texture_image_.width(); j++)
		{
			t_texture_image_.setPixel(j, i, qRgb(255, 255, 255));
#if DEBUG_
			mask_image.setPixel(j, i, qRgb(0, 0, 0));
#endif
		}
	}
	//collect the visible views actually used
	collectUsefulVis();

	//load parts of images
	loadTextureImages();

	// initialization for t_labels // 255 means no image pixel
	for (int i = 0; i < texture_size; i++)
	{
		QVector<uint> tmp(texture_size, 255);
		t_labels_.append(tmp);
	}

	// create texture image
	QImage ref_img;
	QString name;
	int id0, id1, id2; // vertex indices of a facet
	QPointF v0, v1, v2; // coordiante of a facet
	int ww, hh;
	for (int i = 0; i < validH; i++)
	{
		for (int j = 0; j < validW; j++)
		{
			QPointF pt = b_coords[i *validW + j];
			QVector<int> neigh_id = neighbours[i *validW + j];
			QVector<int> candidate_facets = facets_to_check[i *validW + j];

			int facet_id = inWhichFacet(pt, candidate_facets, all_facets);
			int vis_id = t_final_facets_vis_[facet_id];

			if (facet_id == -1 || vis_id == -1)
			{
				t_texture_image_.setPixel(j, i, qRgb(0, 0, 0));
#if DEBUG_
				//mask_image.setPixel(j, i, qRgb(0,0,0));
#endif
				continue;
			}
			//------------------------------------area coordinates-----------------------------------------------------//
			// check the back projected point loacted in which facet, and compute the area coordinate of the point,
			// the area coordinate is compute as the ratio of the triangle to the whole facet
			id0 = (*t_facets_)[facet_id][0];
			id1 = (*t_facets_)[facet_id][1];
			id2 = (*t_facets_)[facet_id][2];
			v0 = t_projected_coordinates_[id0];
			v1 = t_projected_coordinates_[id1];
			v2 = t_projected_coordinates_[id2];

			Point coord_area = areaCoordinates(pt, v0, v1, v2);

			//--------------------------------------compute the ref point on image---------------------------------------//
			// the cooridnate on the visible image can be iterploated by the area coordinates
			//t_labels_.setPixel(j, i, qRgb(vis_id, vis_id, vis_id));
			t_labels_[i][j] = vis_id;

#if DEBUG_
			mask_image.setPixel(j, i, qRgb(color_table[vis_id].red(), color_table[vis_id].green(), color_table[vis_id].blue()));
#endif
			QVector<QPoint> coords_ref = t_final_facets_coordinates_[facet_id];
			ww = (int)(coord_area.x * coords_ref[0].x() + coord_area.y * coords_ref[1].x() + coord_area.z * coords_ref[2].x() + 0.5);
			hh = (int)(coord_area.x * coords_ref[0].y() + coord_area.y * coords_ref[1].y() + coord_area.z * coords_ref[2].y() + 0.5);
			coords_ref.clear();

			name.sprintf("%08d.jpg", vis_id);
			ref_img = t_texture_images_[name];
			t_texture_image_.setPixel(j, i, ref_img.pixel(QPoint(ww, hh)));
		}
	}

	t_projected_coordinates_.clear();
	b_coords.clear();
	neighbours.clear();
	vertex2facets.clear();

	coord_t.release();
	coord_b.release();
	transform.release();
	transform_inv.release();

	t_texture_image_.save("texture_image.png");
#if DEBUG_
	mask_image.save("DEBUG_mask.png");
#endif
	}
//////////////////////////////////////////prepareForEditing////////////////////////////////////////////////////////////////////
void TextureMapping::prepareForEditing()
{
	QVector<QImage> images;
	QVector<QString> img_names;
	QVector<QPoint> postions;
	QImage labels(t_texture_image_.width(), t_texture_image_.height(), t_texture_image_.format());
	for (int i = 0; i < labels.height(); i++)
	{
		for (int j = 0; j < labels.width(); j++)
		{
			labels.setPixel(j, i, qRgb(255, 255, 255));
		}
	}
	//-----------------------------------create images, names and points-------------------------------------------------------//
	int vis_num = t_label_mapping_.size();
	assert(vis_num < 255);

	foreach(uint key, t_label_mapping_.keys())// key is the old label
	{
		foreach(uint value, t_label_mapping_.values(key))// value is the new index
		{

			int min_x = 100000;
			int max_x = -100000;
			int min_y = 1000000;
			int max_y = -1000000;

			int vis_id = key;
			for (int i = 0; i < t_labels_.size(); i++)
			{
				for (int j = 0; j < t_labels_[i].size(); j++)
				{
					if (t_labels_[i][j] != vis_id) continue;

					if (min_x > j)min_x = j;
					if (max_x < j)max_x = j;
					if (min_y > i)min_y = i;
					if (max_y < i)max_y = i;

					int g = value / 255;
					int r = value % 255;				
				    labels.setPixel(j, i, qRgb(r, g, 0));					
				}
			}

			// src image positions
			postions.append(QPoint(min_x, min_y));

			QRect rect(QPoint(min_x, min_y), QPoint(max_x, max_y));
			QImage roi = t_texture_image_.copy(rect);
#if 0
			for (int i = 0; i < roi.height(); i++)
			{
				for (int j = 0; j < roi.width(); j++)
				{
					if (t_labels_[i + min_y][j + min_x] == vis_id) continue;
					roi.setPixel(j, i, qRgb(255, 255, 255));
				}
			}
#endif
			images.append(roi);
			// image names
			QString name = QString("%1.png").arg(value);
			img_names.append(name);
		}
	}
	//--------------------------------------------------out files------------------------------------------------------------//
	QDir dir;
	if (dir.exists("poissonEditing"))
	{
		dir.rmdir("poissonEditing");
	}
	dir.mkdir("poissonEditing");

	// save image names to images.txt
	QFile file0("poissonEditing/imagenames.txt");
	file0.open(QIODevice::WriteOnly);
	QTextStream out0(&file0);
	foreach(QString name, img_names)
	{
		QString _name = "poissonEditing/" + name;
		out0 << _name << endl;
	}

	// save src image positions to srcpos.txt
	QFile file1("poissonEditing/srcpos.txt");
	file1.open(QIODevice::WriteOnly);
	QTextStream out1(&file1);
	foreach(QPoint pt, postions)
	{
		out1 << pt.x() << " " << pt.y() << endl;
	}

	for (int i = 0; i < images.size(); i++)
	{
		QString _name = "poissonEditing/" + img_names[i];
		images[i].save(_name);
	}

	// savel label image
	labels.save("poissonEditing/labels.png");

	images.clear();
	img_names.clear();
	postions.clear();
}
//////////////////////////////////////////poissongImageEditing/////////////////////////////////////////////////////////////////
void TextureMapping::poissonImageEditing()
{
	QDir dir;

	if (dir.exists("poissonEditing"))
	{

		//----------------------------------------------gradient composite----------------------------------------------------//
		QString gradient_coposite = "GradientComposite.exe --in poissonEditing/imagenames.txt --outX poissonEditing/dx.half --outY poissonEditing/dy.half --labels poissonEditing/labels.png --hFill --positions poissonEditing/srcpos.txt";
		textEdit(gradient_coposite);
		QProcess cmd0;
		cmd0.start(gradient_coposite);

		float mean_r = 0;
		float mean_g = 0;
		float mean_b = 0;
		while (cmd0.waitForFinished())
		{
			//perform jhead.exe to obtain the whole exif information
			QString result(cmd0.readAllStandardOutput());
			QStringList exif_info = result.split("\n");

			// Camera make
			foreach(QString tmp, exif_info)
			{
				if (tmp.startsWith("Average:"))
				{
					QStringList fields = tmp.split(" ");
					fields.takeFirst();
					mean_r = fields.takeFirst().toFloat();
					mean_g = fields.takeFirst().toFloat();
					mean_b = fields.takeFirst().toFloat();
				}
			}
		}
		QString out = QString("Mean: %1 %2 %3").arg(mean_r).arg(mean_g).arg(mean_b);
		textEdit(out);

		//---------------------------------------------gradientToImage----------------------------------------------------------//
		int imgH = t_texture_image_.height();
		int imgW = t_texture_image_.width();
		QString gradient2image = QString("GradientsToImage.exe --inX poissonEditing/dx.half --inY poissonEditing/dy.half --out texture_image.png --average %1 %2 %3 --width %4 --height %5").arg(mean_r).arg(mean_g).arg(mean_b).arg(imgH).arg(imgW);
		textEdit(gradient2image);
		QProcess cmd1;
		cmd1.start(gradient2image);
		while (cmd1.waitForFinished())
		{
		}

#if DEBUG_
		t_texture_image_.save("texture_image_tmp.png");
#endif
		t_texture_image_.load("texture_image.png");

#ifndef DEBUG_
		dir.remove("poissonEditing");
#endif

	}

}
//////////////////////////////////////////imageEditing/////////////////////////////////////////////////////////////////////////
void TextureMapping::imageEditing()
{
	//-------------------------------------------prepare for image editing --------------------------------------------------//
	// Some inputs are need by the poisson image editing.
	// 1. Src Images. For regions come from the same image(i.e. have the same view label), a bounding box is extracted and then 
	// a corresponding image is created from the original texture image.
	// 2. Image Names. All the names of the src images should be saved  in a txt.
	// 3. Poistions. The position of the bounding box needed in this process. The top left point of each bounding box are saved
	// 4. Label Image. The label image has the same size as the original texture image, and the corresponding pixel value represent
	// the view label, note that, the pixel value should be match the image in order, and the surplus values will be considered as
	// pixels without visible image(background).
	// All these files will be create in the prepare
	emit textEdit("[Preparing for Image Editing...");

	prepareForEditing();

	t_labels_.clear();
	t_label_mapping_.clear();
	emit textEdit("Done!]");

	emit textEdit("Image Editing...");

	poissonImageEditing();

	emit textEdit("Dond!]");
}
///////////////////////////////////////////run/////////////////////////////////////////////////////////////////////////////////
void TextureMapping::run()
{
	const double begin = (double)clock() / CLOCKS_PER_SEC;
	//----------------------------------------initialization------------------------------------------------------------//
	// through initialization we obtain the intial visible information of each facet and the corresponding texture coordinates
	// t_intial_facets_vis_ stores the initial visible information
	// t_intial_texture_coordinates_ stores the intial texture coordinates

	emit textEdit("[Initialization...");
	emit statusBar("Initialization");

	initialization();
	t_dense_vis_.clear();
	t_vertices_vis_.clear();
	//----------------------------------------MRF Based opeimization-----------------------------------------------------//
	// a MRF framework is applied to optimized the visibility
	// input are t_intial_facets_vis_ and t_intial_texture_coordinates. Note that at intial stage, each facet has more than
	// one visible view and after the optimization each facet owns only one unique visible view.
	// At this stage, we only consider the area of the projected facet, and we select the view with the largest area

	emit textEdit("[MRF Optimiztion...");
	emit statusBar("MRF Optimiztion");
	const double begin_MRF = (double)clock() / CLOCKS_PER_SEC;
	MRF_Optimization();

	t_initial_facet_coordinates_.clear();
	t_initial_facet_vis_.clear();
	t_gray_images_.clear();

	const double end_MRF = (double)clock() / CLOCKS_PER_SEC;
	emit textEdit(tr("Done!]"));
	QString txt_MRF = QString("MRF Optimization Time is %1 \n").arg(end_MRF - begin_MRF);
	emit textEdit(txt_MRF);
	//-----------------------------------------Create Texture Images -----------------------------------------------------//
	// first ground was computed by PCA and all the vertices are projceted into this plane. So the 3D problem is converted 
	// into 2.5D problem. Next the bounding box of the  projected mesh  is computed and is transformed into a texture image
	// with fix size (1024 * 1024) in our framework. Finally the created image is inteploated pixel by pixel through the 
	// projected mesh and from all the images.
#if 1
	emit textEdit("[Creating Texture Image...");
	emit statusBar("Creating Texture Image");
	const double begin_create_image = (double)clock() / CLOCKS_PER_SEC;

	createTextureImage(1024*6);

	t_texture_images_.clear();
	t_final_facets_vis_.clear();
	t_final_facets_coordinates_.clear();
	t_projected_coordinates_.clear();

	const double end_create_image = (double)clock() / CLOCKS_PER_SEC;
	emit textEdit(tr("Done!]"));
	QString txt_create_image = QString("Create Texture Image Time is %1 \n").arg(end_create_image - begin_create_image);
	emit textEdit(txt_create_image);

	//---------------------------------------Poisson Image Edit----------------------------------------------------------//
	// The texture image are obtained by compositing subimages from several images, and unavoidablly there will be seems 
	// in positions where pixels are on the edge of different images. Poisson image editing is applied here to eliminate the
	// seams in the texture image.
	emit textEdit("[Poisson Image Editing...");
	emit statusBar("Poisson Image Editing");
	const double begin_poisson = (double)clock() / CLOCKS_PER_SEC;

	imageEditing();

	const double end_poisson = (double)clock() / CLOCKS_PER_SEC;
	emit textEdit(tr("Done!]"));
	QString txt_poisson = QString("Poisson Image Editing Time is %1 \n").arg(end_poisson - begin_poisson);
	emit textEdit(txt_poisson);

	emit textureImageDir("texture_image.png");
	emit enableTexture();

	const double end = (double)clock() / CLOCKS_PER_SEC;
	QString txt = QString("Total Time is %1 \n").arg(end - begin);
	textEdit(txt);
#endif
}