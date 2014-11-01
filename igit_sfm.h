#ifndef  IGIT_SFM_H
#define  IGIT_SFM_H

#include "data_type.h"

#include <QObject>
#include <QTextEdit>

typedef PointXYZRGBNormal Point;

class SFM : public QObject
{
   Q_OBJECT

public:
	SFM(){}
    
	void extract_focal();
	void extract_feature();
	void feature_matching();
	void bundler();

	void collect_sparse_points();
	bool loadPointsFromPLYFile(QString file_name);

	void setSparsePointsPtr(QVector<Point>* ptr){
		s_sparse_pts_ = ptr;
	}

	void setImgDirs(QVector<QString>& dirs){
		s_img_dirs_ = dirs;
	}
	void setImgFolder(QString &folder){
		s_img_folder_ = folder;
	}

public slots:
	void run();

signals:
	// transmmit message to mainwindow
	void statusBar(QString info);
	void textEdit(QString info);

	// enable mainwindow display
	void enableActionSparse();
private:
	
	//QTextEdit * s_output_text_;
	QVector<QString> s_img_dirs_;
	QString s_img_folder_;

	QVector<Point> *s_sparse_pts_;
};


#endif