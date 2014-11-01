#ifndef IGIT_GLVIEWER_H
#define IGIT_GLVIEWER_H
#include"data_type.h"

#include "qglviewer.h"

#include <QObject>
#include<qdir.h>

typedef PointXYZRGBNormal Point;

class GLViewer : public QGLViewer
{
	Q_OBJECT

public:
	GLViewer(QWidget *parent = 0, const QGLWidget* constWidget = 0, Qt::WindowFlags f = 0);
	~GLViewer();

	virtual void init();
	virtual void viewAll();
	virtual void draw();

	virtual void mousePressEvent(QMouseEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual QString helpString();

	// load texture images
	void makeTextures();

	//----------------------------shared pointers ---------------------------------------//
	// sparse points
	void setSparsePointsPtr(QVector<Point>* ptr){
		g_sparse_pts_ = ptr;
	}
	// dense points
	void setDensePointsPtr(QVector<Point>* ptr){
		g_dense_pts_ = ptr;
	}
	// mesh vertices
	void setMeshVeticesPtr(QVector<Point>* ptr){
		g_vertices_ = ptr;
	}
	// mesh facets
	void setMeshFacetPtr(QVector<QVector<int> > * ptr){
		g_facets_ = ptr;
	}
	// mesh edges
	void setMeshEdgesPtr(QVector<QPair<int, int> >* ptr){
		g_edges_ = ptr;
	}
	// points indices
	void setPointsIdsPtr(QSet<int> *ptr)
	{
		g_points_ids_ = ptr;
	}
	// set texture coordinates ptr
	void setTextureCoordatesPtr(QVector<QPointF>* ptr){ g_texture_coords_ = ptr; }
	// set cameras
	void setCamerasPtr(QMap< QString, Camera> *ptr){ g_cameras_ = ptr; }
	// plane3D
	void setPlane3DPtr(Plane3D * ptr){ g_plane_ = ptr; }

	//---------------------------draw fucntions-----------------------------------------//
	// draw selected points
	void drawSelectedPoints();
	// draw select window
	void drawSelectWindow();
	// draw cameras
	void drawCameras();

	//----------------------------select points------------------------------------------//
	void selectPoints();
	void deleteSelectedPoints();

	//----------------------------debug-------------------------------------------------//
	void drawBasePlane();

protected:

	void drawWorldAxis(double width, double length);
	void drawXZGrid();
	void drawSparsePoints();
	void drawDensePoints();

	/// mesh related
	void drawVertices();
	void drawWireFrame();
	void drawFlat();
	void drawFlatLine();
	void drawTexture();

	// bounding box
	void computeSceneBoundingBox();

protected slots:

	inline void toggle_display_sparse(bool flag)
	{
		viewAll();
		showEntireScene();

		g_display_sparse_points_ = flag;

		if (flag == true)
		{
			g_display_dense_points_ = false;
			g_display_vertices_ = false;
			g_display_wire_frame_ = false;
			g_display_flat_ = false;
			g_display_flat_line_ = false;
			g_display_texture_ = false;

			QString outputText = QString("%1 points").arg(g_sparse_pts_->size());
			emit statusBar(outputText);
		}
		updateGL();
	}
	inline void toggle_display_dense(bool flag)
	{
		viewAll();
		showEntireScene();

		g_display_dense_points_ = flag;
		if (flag == true)
		{
			g_display_sparse_points_ = false;
			g_display_vertices_ = false;
			g_display_wire_frame_ = false;
			g_display_flat_ = false;
			g_display_flat_line_ = false;
			g_display_texture_ = false;

			QString outputText = QString("%1 points").arg(g_dense_pts_->size());
			emit statusBar(outputText);
		}
		updateGL();
	}
	inline void toggle_display_vertices(bool flag)
	{
		viewAll();
		showEntireScene();

		g_display_vertices_ = flag;

		QString outputText = QString("%1 vertices").arg(g_vertices_->size());
		emit statusBar(outputText);

		if (flag == true)
		{
			g_display_sparse_points_ = false;
			g_display_dense_points_ = false;
			g_display_wire_frame_ = false;
			g_display_flat_ = false;
			g_display_flat_line_ = false;
			g_display_texture_ = false;
		}
		updateGL();
	}
	inline void toggle_display_wire_frame(bool flag){

		viewAll();
		showEntireScene();
		g_display_wire_frame_ = flag;

		QString outputText = QString("%1 vertices %1 facets").arg(g_vertices_->size()).arg(g_facets_->size());
		emit statusBar(outputText);

		if (flag == true)
		{
			g_display_sparse_points_ = false;
			g_display_dense_points_ = false;
			g_display_vertices_ = false;
			g_display_flat_ = false;
			g_display_flat_line_ = false;
			g_display_texture_ = false;
		}
		updateGL();
	}
	inline void toggle_display_flat(bool flag)
	{
		viewAll();
		showEntireScene();
		g_display_flat_ = flag;

		QString outputText = QString("%1 vertices %1 facets").arg(g_vertices_->size()).arg(g_facets_->size());
		emit statusBar(outputText);

		if (flag == true)
		{
			g_display_sparse_points_ = false;
			g_display_dense_points_ = false;
			g_display_vertices_ = false;
			g_display_wire_frame_ = false;
			g_display_flat_line_ = false;
			g_display_texture_ = false;
		}
		updateGL();
	}
	inline void toggle_display_flat_line(bool flag)
	{
		viewAll();
		showEntireScene();
		g_display_flat_line_ = flag;

		QString outputText = QString("%1 vertices %1 facets").arg(g_vertices_->size()).arg(g_facets_->size());
		emit statusBar(outputText);

		if (flag == true)
		{
			g_display_sparse_points_ = false;
			g_display_dense_points_ = false;
			g_display_vertices_ = false;
			g_display_wire_frame_ = false;
			g_display_flat_ = false;
			g_display_texture_ = false;
		}
		updateGL();
	}
	inline void toggle_display_texture(bool flag)
	{
		viewAll();
		showEntireScene();
		g_display_texture_ = flag;

		QString outputText = QString("%1 vertices %1 facets").arg(g_vertices_->size()).arg(g_facets_->size());
		emit statusBar(outputText);

		if (flag == true)
		{
			if (g_texture_id_ == -1)
			{
				makeTextures();
			}
			g_display_sparse_points_ = false;
			g_display_dense_points_ = false;
			g_display_vertices_ = false;
			g_display_wire_frame_ = false;
			g_display_flat_ = false;
			g_display_flat_line_ = false;
		}
			updateGL();
		}
	inline void toggle_display_cameras(bool flag)
		{
			g_display_cameras_ = flag;
			if (flag == true)
			{
				viewAll();
			}
			updateGL();
		}
	inline void toggle_select_points(bool flag)
		{
			g_select_points_mode_ = flag;
			updateGL();
		}

	// catch several posion and orientarions of cameras to interplatet them into a path
	void toggle_setKeyFrame(bool flag)
		{
			g_set_key_frame_ = flag;

			if (flag == true)
			{
				g_kfi_.deletePath();
				g_counter_ = 0;
				statusBar("  Get key frame to interpolator: adjust the camera and press CTRL + S to save it");
			}
			updateGL();
		}
	// auto play the camera along the path interplated
	void toggle_play_path(bool flag)
		{
			g_play_path_ = flag;

			if (flag == true)
			{
				if (g_counter_ == 0)
				{
					statusBar("Add Key Frame first!");
				}
				else{
					g_kfi_.startInterpolation();
					statusBar(" Play Path");
				}
			}
			if (flag == false)
			{
				g_kfi_.stopInterpolation();
			}
			updateGL();
		}
	// set the position and orientation of camera, needed by play path
	void setCamera()
		{
			camera()->setPosition(g_kfi_.frame()->position());
			camera()->setOrientation(g_kfi_.frame()->orientation());
			updateGL();
		}
	// set texture image dir
	void setTextureImageDir(QString dir){ g_texture_name_dir_ = dir;  }

	//--------------------------------------debug--------------------------------------------------//
	void toggle_debug_base_plane(bool flag){
	
		g_dsplay_base_plane_ = flag;
		updateGL();
	}
	
signals:
		// transmmit message to mainwindow
		void statusBar(QString info);
		void textEdit(QString info);

private:
		bool g_display_world_axis_;
		bool g_dispalay_XZ_grid_;
		bool g_display_sparse_points_;
		bool g_display_dense_points_;

		bool g_display_vertices_;
		bool g_display_wire_frame_;
		bool g_display_flat_;
		bool g_display_flat_line_;
		bool g_display_texture_;
		bool g_display_cameras_;

		bool g_set_key_frame_;
		bool g_play_path_;

		bool g_select_points_mode_;
		bool g_diplay_select_window_;

		//--------------------------------debug--------------------------------//
		bool g_dsplay_base_plane_;

		QVector<Point>* g_sparse_pts_;
		QVector<Point>* g_dense_pts_;

		QVector<Point>* g_vertices_;
		QVector<QVector<int> >*g_facets_;
		QVector<QPair<int, int> >*g_edges_;
		QVector<QPointF> *g_texture_coords_;

		float g_min_x_;
		float g_min_y_;
		float g_min_z_;
		float g_max_x_;
		float g_max_y_;
		float g_max_z_;

		// used for select points
		QRect g_select_window_;

		// current selected points
		QSet<int> g_current_selected_points_;
		QSet<int> *g_points_ids_;


		qglviewer::KeyFrameInterpolator g_kfi_;
		int g_counter_;

		// final texture coordidates corresponding to each facet
		QVector<QVector<QPoint> > *g_final_texture_coords_;

		// all the cameras
		QMap<QString, Camera> *g_cameras_;

		Plane3D * g_plane_;

		QString g_texture_name_dir_;

		GLuint g_texture_id_;
	};

#endif