#ifndef IGIT_POISSON_H
#define IGIT_POISSON_H

#include"data_type.h"

#include <QObject>
#include <QTextEdit>

typedef PointXYZRGBNormal Point;

class Poisson : public QObject
{
	Q_OBJECT

public:
	Poisson(){}
	void setMeshVeticesPtr(QVector<Point>* ptr){ p_vertices_ = ptr; }
	void setMeshFacetPtr(QVector<QVector<int> > * ptr){ p_facets_ = ptr; }
	void setMeshEdgesPtr(QVector<QPair<int, int> >* ptr){ p_edges_ = ptr; }
	void setDensePointsPtr(QVector<Point>* ptr){
		p_dense_pts_ = ptr;
	}
	void setPointsIdsPtr(QSet<int> *ptr){
		p_points_ids_ = ptr;
	}

	// save points to PLY for Poisson Reconstruction
	bool savePointsToPLYFile(QString file_name);

	// trim facets
	void trimmingFacets();

	// some vertices are eliminated and then the mesh should be updated from the facets infomation
	void updateMesh(QVector<QVector<Point> > & facets);

protected:
	bool loadMeshFromPLY(QString file_name);

public slots:
	void run();

signals:
    // transmmit message to mainwindow
	void statusBar(QString info);
	void textEdit(QString info);

	// enable mainwindow display
	void enableActionMesh();

private:

	// dense points
	QVector<Point>* p_dense_pts_;

    // points indices
	QSet<int> * p_points_ids_;

	// vertices of the mesh
	QVector<Point> *p_vertices_;

	// facets of the edges
	QVector<QVector<int> > *p_facets_;

	// edges of the mesh
	QVector<QPair<int, int> > *p_edges_;
};


#endif