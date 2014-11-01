#include"igit_glviewer.h"
#include<QKeyEvent>
#include<qdebug.h>
#include<manipulatedFrame.h>
#include <qframe.h>

#include<GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#define BUFFER_OFFSET(offset) ((GLubyte*) NULL + offset)
#define NumberOf(array)        (sizeof(array)/sizeof(array[0]))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLViewer::GLViewer(QWidget *parent, const QGLWidget* constWidget, Qt::WindowFlags) :QGLViewer(parent, constWidget)
{
	g_display_world_axis_ = true;
	g_dispalay_XZ_grid_ = false;
	g_display_sparse_points_ = false;
	g_display_dense_points_ = false;

	g_display_vertices_ = false;
	g_display_wire_frame_ = false;
	g_display_flat_ = false;
	g_display_flat_line_ = false;
	g_display_texture_ = false;
	g_display_cameras_ = false;
	g_set_key_frame_ = false;
	g_play_path_ = false;

	g_select_points_mode_ = false;
	g_diplay_select_window_ = false;

	g_texture_id_ = -1;

	//------------ set the interpolation  ------------------------------//
	// myFrame is the Frame that will be interpolated.
	qglviewer::Frame* myFrame = new qglviewer::Frame();

	// Set myFrame as the KeyFrameInterpolator interpolated Frame.
	g_kfi_.setFrame(myFrame);
	g_kfi_.setLoopInterpolation();

	connect(&g_kfi_, SIGNAL(interpolated()), SLOT(updateGL()));
	connect(&g_kfi_, SIGNAL(interpolated()), SLOT(setCamera()));

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLViewer::~GLViewer()
{}
/////////////////////////////////////////////////init/////////////////////////////////////////////////////////////////////////
void GLViewer::init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DITHER);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// BLEND
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Texture Mapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#if 0
	// vertices
	GLfloat * vertices = new GLfloat [3 * g_vertices_->size()];
	for (int i = 0; i < g_vertices_->size(); i++)
	{
		vertices[i * 3 + 0] = (*g_vertices_)[i].x;
		vertices[i * 3 + 1] = (*g_vertices_)[i].y;
		vertices[i * 3 + 2] = (*g_vertices_)[i].z;
	}

	// texture coordinates
	GLfloat *texcoords = new GLfloat[2 * g_texture_coords_->size()];
	for (int i = 0; i < g_texture_coords_->size(); i++)
	{
		texcoords[i * 2 + 0] = (*g_texture_coords_)[i].x();
		texcoords[i * 2 + 1] = (*g_texture_coords_)[i].y();
	}

	//facet indices
	GLubyte * faceIndices = new GLubyte[3 * g_facets_->size()];
	for (int i = 0; i < g_facets_->size(); i++)
	{
		faceIndices[i * 3 + 0] = (*g_facets_)[i][0];
		faceIndices[i * 3 + 1] = (*g_facets_)[i][1];
		faceIndices[i * 3 + 2] = (*g_facets_)[i][2];
	}

	// edge indices
	GLubyte * edgeIndices = new GLubyte[2 * g_edges_->size()];
	for (int i = 0; i < g_edges_->size(); i++)
	{
		edgeIndices[i * 2 + 0] = (*g_edges_)[i].first;
		edgeIndices[i * 2 + 1] = (*g_edges_)[i].second;
	}

	enum{ Vertices, Element_facets, Element_edges, TexCoord, NumAttris };
	GLuint arrays[1];
	GLuint buffers[NumAttris];

	glGenVertexArrays(1, arrays);// 1.0创建定点数组对象// 将顶点数组放到缓冲区中，提高渲染的速度

	
	glBindVertexArray(arrays[0]); // 2.0 初始化新对象
	glGenBuffers(NumAttris, buffers);// 3.0 创建缓冲区对象

	//vertices
	glBindBuffer(GL_ARRAY_BUFFER, buffers[Vertices]); // 4.0 激活缓冲区对象
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);// 5.0 用数据分配和初始化缓冲区对象
	glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
	glEnableClientState(GL_VERTEX_ARRAY);

	// texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TexCoord]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glTexCoordPointer(2, GL_FLOAT, 0, BUFFER_OFFSET(0));
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//face elements
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Element_facets]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faceIndices), faceIndices, GL_STATIC_DRAW);

	// edge elements
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Element_edges]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edgeIndices), edgeIndices, GL_STATIC_DRAW);


	// drawing 
	glBindVertexArray(arrays[0]);
	glPushMatrix();
	glDrawElements(GL_TRIANGLES, NumberOf(faceIndices), GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
	glPopMatrix();
	
	glDrawElements(GL_LINES, NumberOf(edgeIndices), GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));

#endif


}
////////////////////////////////////////////////viewAll///////////////////////////////////////////////////////////////////////////
void GLViewer::viewAll()
{
	computeSceneBoundingBox();
	setSceneBoundingBox(qglviewer::Vec(g_min_x_, g_min_y_, g_min_z_), qglviewer::Vec(g_max_x_, g_max_y_, g_max_z_));
}
/////////////////////////////////////////////////draw///////////////////////////////////////////////////////////////////
void GLViewer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);

	// draw world axis
	if (g_display_world_axis_ == true){
		drawWorldAxis(1.0, 1.0);
	}
	// draw XZ grid
	if (g_dispalay_XZ_grid_ == true){
		drawXZGrid();
	}

	// draw selected points
	if (g_select_points_mode_ == true)
	{
		drawSelectedPoints();
	}

	// display select window
	if (g_diplay_select_window_ == true)
	{
		drawSelectWindow();
	}

	// draw sparse points generated from Structure from Motion
	if (g_display_sparse_points_ == true){
		drawSparsePoints();
	}

	// draw dense points generated from CMVS && PMVS
	if (g_display_dense_points_ == true){
		drawDensePoints();
	}
	//---------------------------------------draw cameras----------------------------------------------------//
	if (g_display_cameras_ == true)
	{
		drawCameras();
	}

	//---------------------------------------mesh related---------------------------------------------------//
	// display mesh vertices
	if (g_display_vertices_ == true){
		drawVertices();
	}

	// display mesh in wire frame mode 
	if (g_display_wire_frame_ == true){
		drawWireFrame();
	}

	// display mesh in Flat Mode
	if (g_display_flat_ == true){
		drawFlat();
	}

	// diaplay mesh in Flat Line Mode
	if (g_display_flat_line_ == true){
		drawFlatLine();
	}

	// draw mesh with texture
	if (g_display_texture_ == true){
		drawTexture();
	}

	//----------------------------------------debug--------------------------------------------------------//
	if (g_dsplay_base_plane_ == true)
	{
		drawBasePlane();
	}
	glFlush();
}
/////////////////////////////////////////////////mousePressEvent//////////////////////////////////////////////////////////////
void GLViewer::mousePressEvent(QMouseEvent * e)
{
	if ((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::ControlModifier) && (g_select_points_mode_ == true))
	{
		g_select_window_ = QRect(e->pos(), e->pos());

		qDebug() << e->pos().x() << ", " << e->pos().y() << endl;

		//QString txt = QString("x: %1 y: %2").arg( e->pos().x() ).arg( e->pos().y() );
		//statusBar(txt);
		g_diplay_select_window_ = true;
		updateGL();
	}
	else{
		QGLViewer::mousePressEvent(e);
	}
}
////////////////////////////////////////////////mouseReleaseEvent//////////////////////////////////////////////////////////////////
void GLViewer::mouseReleaseEvent(QMouseEvent *e)
{
	// ************************ be carefull ****************************//
	if (g_select_points_mode_ == true)
	{
		//tring txt = QString("selected %1 points").arg(g_current_selected_points_.size());
		//atusBar(txt);

		g_diplay_select_window_ = false;
		updateGL();
	}
	else{
		QGLViewer::mouseReleaseEvent(e);
	}
}
/////////////////////////////////////////////////mouswMoveEvent/////////////////////////////////////////////////////////////////////
void GLViewer::mouseMoveEvent(QMouseEvent *e)
{
	if ((e->modifiers() == Qt::ControlModifier) && (g_select_points_mode_ == true))
	{
		g_select_window_.setBottomRight(e->pos());

		qDebug() << e->pos().x() << ", " << e->pos().y() << endl;
		//QString txt = QString("x: %1 y: %2").arg( e->pos().x() ).arg( e->pos().y() );
		//statusBar(txt);

		// select points
		selectPoints();

		updateGL();
	}
	else{

		QGLViewer::mouseMoveEvent(e);
	}

}
/////////////////////////////////////////////////wheelEvent/////////////////////////////////////////////////////////////////////
void GLViewer::wheelEvent(QWheelEvent *e)
{
	QGLViewer::wheelEvent(e);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLViewer::keyPressEvent(QKeyEvent *e)
{
	if ((e->key() == Qt::Key_1) && (e->modifiers() == Qt::CTRL) && g_set_key_frame_ == true)
	{
		qDebug() << "Position: " << camera()->position().x << " " << camera()->position().y << " " << camera()->position().z << endl;
		qDebug() << "Angle: " << camera()->orientation().angle() << " " << endl;
		qDebug() << "axis: " << camera()->orientation().axis().x << " " << camera()->orientation().axis().y << " " << camera()->orientation().axis().z << endl;

		qglviewer::ManipulatedFrame* frame = new qglviewer::ManipulatedFrame();
		frame->setPosition(camera()->position());
		frame->setOrientation(camera()->orientation());
		g_kfi_.addKeyFrame(frame);
		g_counter_++;

		QString text = QString("%1 key frames in total").arg(g_counter_);
		statusBar(text);
		updateGL();
	}
	if ((e->key() == Qt::Key_Plus) && (g_play_path_ == true))
	{
		g_kfi_.setInterpolationSpeed(g_kfi_.interpolationSpeed() + 0.25);
		updateGL();
	}
	if ((e->key() == Qt::Key_Minus) && (g_play_path_ == true))
	{
		g_kfi_.setInterpolationSpeed(g_kfi_.interpolationSpeed() - 0.25);
		updateGL();
	}
	// select points
	if ((e->key() == Qt::Key_Delete) && (g_select_points_mode_ == true))
	{
		deleteSelectedPoints();
		updateGL();
	}
	else{
		QGLViewer::keyPressEvent(e);
	}
}
////////////////////////////////////////////////makeTextures///////////////////////////////////////////////////////////////
void GLViewer::makeTextures()
{
	QImage tex, buf;
	if (!buf.load(g_texture_name_dir_))
	{
		statusBar("Warining: No Texture Image Existed!");
		QImage dummy(1024, 1024, QImage::Format_RGB32);
		dummy.fill(Qt::green);
		buf = dummy;
	}
	tex = convertToGLFormat(buf);
	glGenTextures(1, &g_texture_id_);

	glBindTexture(GL_TEXTURE_2D, g_texture_id_);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
/////////////////////////////////////////////////helpString/////////////////////////////////////////////////////////////////
QString GLViewer::helpString()
{
	QString text("<h2> MeshLive 1.0 [2006.10.18.1]<p></h2>");
	text += "An easy and extensible mesh interaction C++ program for real-time applications..<p> ";
	text += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Designed by hywu, jpan, xlvector. Since 2006.<p> ";
	text += "Based on:<p> ";
	text += "QT (http://www.trolltech.com/)<p> ";
	text += "libQGLViewer (http://artis.imag.fr/~Gilles.Debunne/QGLViewer/)<p> ";
	text += "CGAL (http://www.cgal.org/, http://www-sop.inria.fr/geometrica/team/Pierre.Alliez/)<p> ";
	text += "OpenMesh (http://www.openmesh.org/)<p> ";
	text += "Boost (http://www.boost.org/)<p> ";
	text += "OpenCV (http://sourceforge.net/projects/opencvlibrary/)<p> ";
	text += "Python (http://www.python.org/)<p> ";
	text += "etc.<p> ";

	return text;

}
/////////////////////////////////////////////////drawWorldAxis////////////////////////////////////////////////////////////////////
void GLViewer::drawWorldAxis(double width, double length)
{
	double axisLength = length;

	//drawAxis(length);
#if 1
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(width);

	//axis X
	glBegin(GL_LINES);
	{
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(axisLength, 0.0, 0.0);
	}
	glEnd();

	//axis Y
	glBegin(GL_LINES);
	{
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, axisLength, 0.0);
	}
	glEnd();

	// axis Z
	glBegin(GL_LINES);
	{
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, axisLength);
	}
	glEnd();

	glLineWidth(1.0);;
	glDisable(GL_LINE_SMOOTH);

	glColor3f(1.0, 0.0, 0.0);
	renderText(axisLength, 0.0, 0.0, "X", QFont("helvetica", 12, QFont::Bold, TRUE));

	glColor3f(0.0, 1.0, 0.0);
	renderText(0.0, axisLength, 0.0, "Y", QFont("helvetica", 12, QFont::Bold, TRUE));

	glColor3f(0.0, 0.0, 1.0);
	renderText(0.0, 0.0, axisLength, "Z", QFont("helvetica", 12, QFont::Bold, TRUE));

#endif
}
/////////////////////////////////////////////////drawXZGrid////////////////////////////////////////////////////////////////////
void GLViewer::drawXZGrid()
{
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0);

	glColor3f(1.0, 0.0, 1.0);

	int n = 51;
	for (int i = -n; i <= n; i++)
	{
		glBegin(GL_LINES);

		glVertex3f(-n*0.5, 0.0, i* 0.5);
		glVertex3f(n*0.5, 0.0, i* 0.5);

		glEnd();
	}
	for (int i = -n; i <= n; i++)
	{
		glBegin(GL_LINES);

		glVertex3f(i* 0.5, 0.0, -n*0.5);
		glVertex3f(i* 0.5, 0.0, n*0.5);

		glEnd();

	}
}
/////////////////////////////////////////////////drawSparsePoints////////////////////////////////////////////////////////////////////
void GLViewer::drawSparsePoints()
{
	glDisable(GL_LIGHTING);
	glPushMatrix();

	glBegin(GL_POINTS);
	foreach(Point pt, *g_sparse_pts_)
	{
		glColor3f((GLfloat)pt.r / 255.0, (GLfloat)pt.g / 255.0, (GLfloat)pt.b / 255.0);
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();

	glPopMatrix();
}
/////////////////////////////////////////////////drawWorldAxis////////////////////////////////////////////////////////////////////
void GLViewer::drawDensePoints()
{
	glPushMatrix();

	glBegin(GL_POINTS);
	foreach(int id, *g_points_ids_)
	{
		Point pt = g_dense_pts_->at(id);
		glColor3f((GLfloat)pt.r / 255.0, (GLfloat)pt.g / 255.0, (GLfloat)pt.b / 255.0);
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();

	glPopMatrix();
}
/////////////////////////////////////////////////drawSelectedPoints//////////////////////////////////////////////////////////////
void GLViewer::drawSelectedPoints()
{
	// Draws current selected points.
	glPushMatrix();
	glColor3f(0.9f, 0.3f, 0.3f);
	glBegin(GL_POINTS);
	foreach(int it, g_current_selected_points_)
	{
		Point pt = g_dense_pts_->at(it);
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();
	glPopMatrix();
}
/////////////////////////////////////////////////drawSelectedPoints//////////////////////////////////////////////////////////////
void GLViewer::drawSelectWindow()
{
	startScreenCoordinatesSystem();
	glEnable(GL_BLEND);

	//the select window
	glLineWidth(2.0);
	glColor4f(0.4f, 0.9f, 0.1f, 0.5f);
	glBegin(GL_LINE_LOOP);
	glVertex2i(g_select_window_.left(), g_select_window_.top());
	glVertex2i(g_select_window_.right(), g_select_window_.top());
	glVertex2i(g_select_window_.right(), g_select_window_.bottom());
	glVertex2i(g_select_window_.left(), g_select_window_.bottom());
	glEnd();

	glDisable(GL_BLEND);
	stopScreenCoordinatesSystem();
}
/////////////////////////////////////////////////drawCameras/////////////////////////////////////////////////////////////////////
void GLViewer::drawCameras()
{
	foreach(QString key, g_cameras_->keys())
	{
		foreach(Camera cam, g_cameras_->values(key))
		{
			glPushMatrix();
			cam.draw();
			glPopMatrix();
		}
	}
}
/////////////////////////////////////////////////select points///////////////////////////////////////////////////////////////////
void GLViewer::selectPoints()
{
	g_current_selected_points_.clear();

	foreach(int id, *g_points_ids_)
	{
		Point pt = g_dense_pts_->at(id);
		qglviewer::Vec p3D(pt.x, pt.y, pt.z);
		qglviewer::Vec p2D = camera()->projectedCoordinatesOf(p3D, NULL);

		if (g_select_window_.contains(QPoint((int)p2D.x, (int)p2D.y)))
		{
			g_current_selected_points_.insert(id);
		}
	}
}
////////////////////////////////////////////////deleteSelectedPoints//////////////////////////////////////////////////////////////
void GLViewer::deleteSelectedPoints()
{
	*g_points_ids_ = g_points_ids_->subtract(g_current_selected_points_);
	g_current_selected_points_.clear();

	updateGL();
}
/////////////////////////////////////////////////drawVertices ///////////////////////////////////////////////////////////////////
void GLViewer::drawVertices()
{
	//--------------------------------- draw vertices -----------------------------------------//
	glPushMatrix();
	glPointSize(2.0);
	glColor3f(0.5f, 0.5f, 0.5f);

	glBegin(GL_POINTS);
	foreach(Point pt, *g_vertices_)
	{
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();

	glPointSize(1.0);
	glPopMatrix();
}
/////////////////////////////////////////////////drawWireFrame ///////////////////////////////////////////////////////////////////
void GLViewer::drawWireFrame()
{
	//--------------------------------- draw lines -----------------------------------------//
	glPushMatrix();
	glLineWidth(1.0);
	glColor3f(0.5f, 0.5f, 0.5f);

	for (QVector<QPair<int, int> > ::const_iterator iter = g_edges_->constBegin();
		iter != g_edges_->constEnd(); iter++)
	{

		int id0 = iter->first;
		int id1 = iter->second;

		glBegin(GL_LINES);
		glVertex3f((*g_vertices_)[id0].x, (*g_vertices_)[id0].y, (*g_vertices_)[id0].z);
		glVertex3f((*g_vertices_)[id1].x, (*g_vertices_)[id1].y, (*g_vertices_)[id1].z);
		glEnd();
	}

	glLineWidth(1.0);
	glPopMatrix();

}
/////////////////////////////////////////////////drawFlat//////////////////////////////////////////////////////////////////////
void GLViewer::drawFlat()
{
	//------------------------------- draw facets --------------------------------------//
	glPushMatrix();
	glColor4f(0.5f, 1.0f, 0.5f, 1.0f);

	foreach(QVector<int> facet, *g_facets_)
	{
		glBegin(GL_TRIANGLES);
		foreach(int id, facet)
		{
			glVertex3f((*g_vertices_)[id].x, (*g_vertices_)[id].y, (*g_vertices_)[id].z);
		}
		glEnd();
	}

	glPopMatrix();
}
/////////////////////////////////////////////////drawFlatLine ///////////////////////////////////////////////////////////////////
void GLViewer::drawFlatLine()
{
	//------------------------------- draw facets --------------------------------------//
	glPushMatrix();
	glColor4f(0.5f, 1.0f, 0.5f, 1.0f);

	foreach(QVector<int> facet, *g_facets_)
	{
		glBegin(GL_TRIANGLES);
		foreach(int id, facet)
		{
			glVertex3f((*g_vertices_)[id].x, (*g_vertices_)[id].y, (*g_vertices_)[id].z);
		}
		glEnd();
	}
	glPopMatrix();

	//--------------------------------- draw lines -----------------------------------------//
	glPushMatrix();
	glLineWidth(1.0);
	glColor3f(1.0f, 1.0f, 1.0f);

	for (QVector<QPair<int, int> > ::const_iterator iter = g_edges_->constBegin();
		iter != g_edges_->constEnd(); iter++)
	{
		int id0 = iter->first;
		int id1 = iter->second;

		glBegin(GL_LINES);
		glVertex3f((*g_vertices_)[id0].x, (*g_vertices_)[id0].y, (*g_vertices_)[id0].z);
		glVertex3f((*g_vertices_)[id1].x, (*g_vertices_)[id1].y, (*g_vertices_)[id1].z);
		glEnd();
	}

	glLineWidth(1.0);
	glPopMatrix();
}
/////////////////////////////////////////////////drawTexture ///////////////////////////////////////////////////////////////////
void GLViewer::drawTexture()
{
	//glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0); // very important otthewise the color will be affected by drawAxies(double, double);

	//***** draw texture with textures with all triangulations  *****//
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_texture_id_);
	foreach(QVector<int> facet, *g_facets_)
	{
		glBegin(GL_TRIANGLES);
		foreach(int id, facet)
		{
			//glTexCoord2f((*g_texture_coords_)[id].y(), 1-(*g_texture_coords_)[id].x());
			glTexCoord2f((*g_texture_coords_)[id].x(), 1-(*g_texture_coords_)[id].y());
			glVertex3f((*g_vertices_)[id].x, (*g_vertices_)[id].y, (*g_vertices_)[id].z);
		}
		glEnd();
	}
	//glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
}
////////////////////////////////////////////////computeSceneBoundingBox////////////////////////////////////////////////////////////
void GLViewer::computeSceneBoundingBox()
{
	g_min_x_ = g_min_y_ = g_min_z_ = 1000000;
	g_max_x_ = g_max_y_ = g_max_z_ = -1000000;

	foreach(Point pt, *g_sparse_pts_)
	{
		if (g_min_x_ > pt.x) g_min_x_ = pt.x;
		if (g_min_y_ > pt.y) g_min_y_ = pt.y;
		if (g_min_z_ > pt.z) g_min_z_ = pt.z;

		if (g_max_x_ < pt.x) g_max_x_ = pt.x;
		if (g_max_y_ < pt.y) g_max_y_ = pt.y;
		if (g_max_z_ < pt.z) g_max_z_ = pt.z;

	}

	foreach(Point pt, *g_dense_pts_)
	{
		if (g_min_x_ > pt.x) g_min_x_ = pt.x;
		if (g_min_y_ > pt.y) g_min_y_ = pt.y;
		if (g_min_z_ > pt.z) g_min_z_ = pt.z;

		if (g_max_x_ < pt.x) g_max_x_ = pt.x;
		if (g_max_y_ < pt.y) g_max_y_ = pt.y;
		if (g_max_z_ < pt.z) g_max_z_ = pt.z;

	}

	foreach(Point pt, *g_vertices_)
	{
		if (g_min_x_ > pt.x) g_min_x_ = pt.x;
		if (g_min_y_ > pt.y) g_min_y_ = pt.y;
		if (g_min_z_ > pt.z) g_min_z_ = pt.z;

		if (g_max_x_ < pt.x) g_max_x_ = pt.x;
		if (g_max_y_ < pt.y) g_max_y_ = pt.y;
		if (g_max_z_ < pt.z) g_max_z_ = pt.z;

	}

	foreach(QString key, g_cameras_->keys())
	{
		foreach(Camera value, g_cameras_->values(key))
		{
			if (g_min_x_ > value.pos_.at<float>(0)) g_min_x_ = value.pos_.at<float>(0);
			if (g_min_y_ > value.pos_.at<float>(1)) g_min_y_ = value.pos_.at<float>(1);
			if (g_min_z_ > value.pos_.at<float>(2)) g_min_z_ = value.pos_.at<float>(2);

			if (g_max_x_ < value.pos_.at<float>(0)) g_max_x_ = value.pos_.at<float>(0);
			if (g_max_y_ < value.pos_.at<float>(1)) g_max_y_ = value.pos_.at<float>(1);
			if (g_max_z_ < value.pos_.at<float>(2)) g_max_z_ = value.pos_.at<float>(2);
		}
	}
}

//----------------------------debug-------------------------------------------------//
void GLViewer::drawBasePlane()
{
	glEnable(GL_LIGHTING);
	glPushMatrix();

	// boundary
	glColor4f(0.0, 1.0, 0.0, 0.5);
	glLineWidth(4);
	glBegin(GL_LINE_LOOP);
	glVertex3f(g_plane_->p_top_left_.x, g_plane_->p_top_left_.y, g_plane_->p_top_left_.z);
	glVertex3f(g_plane_->p_top_right_.x, g_plane_->p_top_right_.y, g_plane_->p_top_right_.z);
	glVertex3f(g_plane_->p_bottom_right_.x, g_plane_->p_bottom_right_.y, g_plane_->p_bottom_right_.z);
	glVertex3f(g_plane_->p_bottom_left_.x, g_plane_->p_bottom_left_.y, g_plane_->p_bottom_left_.z);
	glEnd();

	glColor3f(1, 0, 0);
	glBegin(GL_POINTS);

	foreach(Point pt, g_plane_->p_points_on_plane_)
	{
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();

	glLineWidth(1.0);
	glColor3f(1.0f, 1.0f, 1.0f);

	// edges
	for (QVector<QPair<int, int> > ::const_iterator iter = g_edges_->constBegin();
		iter != g_edges_->constEnd(); iter++)
	{
		int id0 = iter->first;
		int id1 = iter->second;

		glBegin(GL_LINES);
		glVertex3f(g_plane_->p_points_on_plane_[id0].x,
			       g_plane_->p_points_on_plane_[id0].y, 
				   g_plane_->p_points_on_plane_[id0].z);
		glVertex3f(g_plane_->p_points_on_plane_[id1].x,
			       g_plane_->p_points_on_plane_[id1].y, 
				   g_plane_->p_points_on_plane_[id1].z);
		glEnd();
	}
	glPointSize(5);
	glColor3f(0.0, 1.0, 0.0);

	// center
	glBegin(GL_POINTS);
	glVertex3f(g_plane_->p_center_.x, g_plane_->p_center_.y, g_plane_->p_center_.z);
	glEnd();

	glPointSize(1.0);

	glPopMatrix();


	//-------------------------------------------transformed quad--------------------------------------------------//
	glColor4f(0.0, 1.0, 1.0, 0.5);
	glLineWidth(4);
	glBegin(GL_LINE_LOOP);
	glVertex3f(g_plane_->p_top_left_trans_.x, g_plane_->p_top_left_trans_.y, g_plane_->p_top_left_trans_.z);
	glVertex3f(g_plane_->p_top_right_trans_.x, g_plane_->p_top_right_trans_.y, g_plane_->p_top_right_trans_.z);
	glVertex3f(g_plane_->p_bottom_right_trans_.x, g_plane_->p_bottom_right_trans_.y, g_plane_->p_bottom_right_trans_.z);
	glVertex3f(g_plane_->p_bottom_left_trans_.x, g_plane_->p_bottom_left_trans_.y, g_plane_->p_bottom_left_trans_.z);
	glEnd();

#if 1	
	glColor3f(1, 1, 0);
	glBegin(GL_POINTS);
	foreach(Point pt, g_plane_->p_transformed_pts_)
	{
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();
#endif
#if 0
	//----------------------------------------image position-------------------------------------------------------//
	glColor3f(1, 1, 0);
	glBegin(GL_POINTS);
	foreach(Point pt, g_plane_->p_image_pixels_)
	{
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();
#endif	
	//----------------------------------------transforme image position--------------------------------------------//
#if 0
	glColor3f(1, 0, 1);// purple
	glBegin(GL_POINTS);
	foreach(Point pt, g_plane_->p_image_pixels_trans_)
	{
		glVertex3f(pt.x, pt.y, pt.z);
	}
	glEnd();

	glLineWidth(1);
#endif
	glDisable(GL_LIGHTING);
}