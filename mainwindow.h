#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "data_type.h"
#include <mainwindow.h>
#include "igit_sfm.h"
#include "igit_cmvs.h"
#include "igit_poisson.h"

#include "igit_glviewer.h"
#include "igit_texture_mapping.h"

#include <QMainWindow>
#include <QThread>
#include<QMap>

typedef PointXYZRGBNormal Point;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();


public slots:
	bool loadImages();
	void allinOne();
	void paraSet();
	bool save2PLY();
	bool loadPointsFromPLY();
	bool loadMesh();
	bool saveMesh();

	// m_sfm_ info to show on statusBar
	void sfmMessageToStatusBar(QString str);
	// m_sfm_info to show on lineEdit
	void sfmMessageToTextEdit(QString str);
	// make display sparse points enabale
	void enableActionSparsePoints();


	// m_cmvs_ info to show on statusBar
	void cmvsMessageToStatusBar(QString str);
	// m_cvms_ info to show on lineEdit
	void cmvsMessageToTextEdit(QString str);
	// make display dense points enabale
	void enableActionDensePoints();

	// givlewer info to show on statusBar
	void viewerMessageToStatusBar(QString str);
	// m_cvms_ info to show on lineEdit
	void  viewerMessageToTextEdit(QString str);


	// m_poisson info to show on statusBar
	void poissonMessageToStatusBar(QString str);
	// m_poisson info to show on lineEdit
	void poissonMessageToTextEdit(QString str);
	void enableActionMesh();


	// m_texture_mapping_ info to show on statusBar
	void textureMappingToStatusBar(QString str);
	// m_texture_mapping info to show on lineEdit
	void textureMappingToTextEdit(QString str);
	void enableActionTexture();
	void enableActionCameras();


signals:
	void displaySparsePoints();
	void displayDensePoints(bool);
	void displayTexture(bool );
	void textureImageDir(QString);

private:

private:
	Ui::MainWindow *ui;

	SFM* m_sfm_;
	CMVS* m_cmvs_;
	Poisson * m_poisson_;
	TextureMapping * m_texture_mapping_;
	
	// folders of images
	QVector<QString> m_img_dirs_;
	QString m_img_folder_;

	// sparse points generated from structure from modtion
	QVector<Point> m_sparse_pts_;
	// dense points generated from pmvs 
	QVector<Point> m_dense_pts_;
	
	// indices of dense points // used for selecting and deleting points // QSet has a substract function
	QSet<int> m_points_ids_;

	// vertices of mesh generated from poisson surface reconstruction
	QVector<Point> m_vertices_;
	// facets of mesh generated from poisson surface reconstruction
	QVector<QVector<int> >m_facets_;
	// edges of mesh generated from poisson surface reconstruction
	QVector<QPair<int, int> >m_edges_;
	//texture coordinate of each vertices
	QVector<QPointF> m_texture_coords_;


	// all images
	//QMap<QString, QImage>  m_images_;

	// all the cameras
	QMap<QString, Camera> m_cameras_;

	QThread m_thread_;
    
	Plane3D m_plane_;
};

#endif // MAINWINDOW_H
