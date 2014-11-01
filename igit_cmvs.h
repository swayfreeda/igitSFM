#ifndef IGIT_CMVS_H
#define IGIT_CMVS_H

#include "data_type.h"

#include <QObject>
#include <QTextEdit>

typedef PointXYZRGBNormal Point;

class CMVS : public QObject
{
   Q_OBJECT

public:
	CMVS(){}
    
	bool prepare_for_PMVS();
	void genOption();
	void cmvs();
	void pmvs2();

	void collect_dense_points();

	// load point from a PLY File
	bool loadPointsFromPLYFile(QString file_name);

	void setDensePointsPtr(QVector<Point>* ptr){
		c_dense_pts_ = ptr;
	}
	void setImgDirs(QVector<QString>& dirs){
		c_img_dirs_ = dirs;
	}
	void setImgFolder(QString &folder){
		c_img_folder_ = folder;
	}
	void setPointsIdsPtr(QSet<int> *ptr)
	{
		c_points_ids_ = ptr;
	}

public slots:
	void run();

signals:
	// transmmit message to mainwindow
	void statusBar(QString info);
	void textEdit(QString info);

	// enable mainwindow display
	void enableActionDense();
private:
	
	//QTextEdit * s_output_text_;
	QVector<QString> c_img_dirs_;
	QString c_img_folder_;

	QVector<Point> *c_dense_pts_;
	QSet<int> * c_points_ids_;
};

#endif