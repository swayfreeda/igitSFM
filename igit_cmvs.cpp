#include"igit_cmvs.h"

#include <QTextStream>
#include <QDir>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

////////////////////////////////////////prepare_for_PMVS/////////////////////////////////////////////////
bool CMVS::prepare_for_PMVS()
{
	emit textEdit("[ prepare for PMVS... ");
	// write projection matrixs
	QDir dir;
	if(!dir.exists("bundle"))
	{
		emit statusBar("Run SFM Again!");
		// QMessageBox::warning(this, tr("warning"), tr("bundle results do not existed!"));
		return false;
	}

	// run Bundle2PMVS.exe to generage camera projection matrix
	QProcess cmd;
	cmd.start("Bundle2PMVS.exe list.txt bundle/bundle.out");
	while(cmd.waitForFinished())
	{
	}

	// distort images
	cmd.start("RadialUndistort list.txt bundle/bundle.out pmvs");
	while(cmd.waitForFinished())
	{
	}

	// copy and rename images
	// make folders
	dir.mkdir("pmvs/txt");
	dir.mkdir("pmvs/visualize");
	dir.mkdir("pmvs/models");

	int nFiles = c_img_dirs_.size();
	for(int i=0; i< nFiles; i++)
	{
		// camera projection files
		QString src_file_name, dst_file_name;
		src_file_name.sprintf("pmvs/%08d.txt", i);
		dst_file_name.sprintf("pmvs/txt/%08d.txt",i);
		QFile::copy(src_file_name, dst_file_name);
		QFile::remove(src_file_name);

		QString outputText = tr("moving ") + src_file_name + tr(" to ") + dst_file_name;
		emit textEdit(outputText);

		// image files
		QStringList fields = c_img_dirs_.at(i).split("/");
		QString img_name = fields.takeLast();
		int id = img_name.indexOf(".");
		src_file_name = tr("pmvs/")+ img_name.insert(id, ".rd");
		dst_file_name.sprintf("pmvs/visualize/%08d.jpg",i);
		QFile::copy(src_file_name, dst_file_name);
		QFile::remove(src_file_name);

		outputText = tr("moving ") + src_file_name + tr(" to ") + dst_file_name;
		emit textEdit(outputText);

	}
	emit textEdit("Done!]\n");


}
////////////////////////////////////////cmvs//////////////////////////////////////////////////////////////
void CMVS::cmvs()
{
	emit textEdit("[Run Cmvs...");
	QProcess cmd;
	cmd.start("cmvs.exe pmvs/ 40 4");
	while(cmd.waitForFinished())
	{
	}
	emit textEdit("Done!]\n");
}
////////////////////////////////////////genOption/////////////////////////////////////////////////////////
void CMVS::genOption()
{
	emit textEdit("[Generate Options...");
	QProcess cmd;
	cmd.start("genOption.exe pmvs/");
	while(cmd.waitForFinished())
	{
	}
	emit textEdit("Done]\n");  

}
////////////////////////////////////////pmvs2/////////////////////////////////////////////////////////////
void CMVS::pmvs2()
{
	emit textEdit("[Run pmvs...");

	QString path = QDir::currentPath() + tr("/pmvs");
	QDir dir;
	dir.setPath(path);
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QStringList filter;
	filter<<"option-*";

	QFileInfoList fileList = dir.entryInfoList(filter);

	int nOpts = fileList.size();

	for(int i=0; i< nOpts; i++)
	{
		// option files
		QString optFile = fileList.at(i).filePath();
		QStringList fields= optFile.split("/");

		// commad line arguments
		QString cmdArgu = tr("pmvs2.exe pmvs/ ") + fields.takeLast();
		emit textEdit(cmdArgu);

		string cmdArgu_str = cmdArgu.toStdString();
		system(cmdArgu_str.c_str());

#if 0  // 有问题，没有运行结果，不知道原因
		QProcess cmd;
		cmd.start(cmdArgu);
		while(cmd.waitForFinished())
	    {
	    }
#endif

	}
	emit textEdit("pmvs Done!]\n");

}
////////////////////////////////////////run/////////////////////////////////////////////////////////////
void CMVS::run()
{
	emit statusBar(tr("Prepare for PMVS"));
	prepare_for_PMVS();
	emit(tr("Prepare for PMVS Done!") );

	emit statusBar(tr("Run Cmvs") );
	cmvs();
	emit statusBar(tr("Run Cmvs Done!") ); 

	emit statusBar(tr("Generate Option") );
	genOption();
	emit statusBar(tr("Generate Option") );

	emit statusBar(tr("Run PMVS") );
	pmvs2();
	emit statusBar(tr("Run PMVS Done!") );

	emit statusBar(tr("Collect Dense Points") );
	collect_dense_points();
	emit statusBar(tr("Collect Dense Points Done!") );

	emit enableActionDense();
}
////////////////////////////////////////collect_dense_points//////////////////////////////////////////////
void CMVS::collect_dense_points()
{
	c_dense_pts_->clear();

	QDir dir;
	if(dir.exists(tr("pmvs/models")))
	{
		QString path = QDir::currentPath() + tr("/pmvs/models");
		dir.setPath(path);
		dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		QStringList filter;
		filter<<"*.ply";

		// ply files
		QFileInfoList fileList = dir.entryInfoList(filter);
		int nFiles = fileList.size();	

		for(int i=0; i< nFiles; i++)
		{
			// the last file containes all the points
			QString ply_file_name = fileList.at(i).filePath();
			//load points
			loadPointsFromPLYFile(ply_file_name);
		}

		for (int i = 0; i < c_dense_pts_->size(); i++)
		{
			c_points_ids_->insert(i);
		}
	}
}
////////////////////////////////////////loadPointsFromPLYFile//////////////////////////////////////////////
bool CMVS::loadPointsFromPLYFile(QString file_name)
{
	QFile file(file_name);
	if(!file.open(QIODevice::ReadOnly))
	{
		emit textEdit(tr("Fail to Load Sparse Points!"));
		return false;
	}

	QTextStream in(&file);
	int line_Num = 0;
	while(!in.atEnd())
	{
		QString line = in.readLine();
		QStringList fields = line.split(" ");

		// points begin
		if(line_Num>12)
		{
			Point pt;

			pt.x = fields.takeFirst().toFloat();
			pt.y = fields.takeFirst().toFloat();
			pt.z = fields.takeFirst().toFloat();

			pt.normal_x = fields.takeFirst().toFloat();
			pt.normal_y = fields.takeFirst().toFloat();
			pt.normal_z = fields.takeFirst().toFloat();

			pt.r = fields.takeFirst().toInt();
			pt.g = fields.takeFirst().toInt();
			pt.b = fields.takeFirst().toInt();

			c_dense_pts_->append(pt);
		}

		line_Num++;
	}

	return true;
}
