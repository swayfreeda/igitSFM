#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "igit_sfm.h"
#include "igit_cmvs.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QProgressDialog>
#include <qactiongroup.h>

//***************************************************NO CLASS MEMBER**********************************************//

////////////////////////////////////////////////////load dense points///////////////////////////////////////////////
QVector<Point>  loadPointsFromPLYFile_debug(QString file_name)
{
	QVector<Point> points;

	QFile file(file_name);
	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream in(&file);
		int line_Num = 0;
		while (!in.atEnd())
		{
			QString line = in.readLine();
			QStringList fields = line.split(" ");

			// points begin
			if (line_Num > 12)
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

				points.append(pt);
			}

			line_Num++;
		}
	}
	return points;
}
/////////////////////////////////////////////////load visiility///////////////////////////////////////////////////
QVector<QVector<int> > loadVisibilityFromPatchFile_denug(QString patch_file_name)
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



//***************************************************CLASS MEMBER************************************************//
MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// tools
	QIcon icon0;
	icon0.addFile(QStringLiteral("../icons/fileOpen.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->actionLoad_Images->setIcon(icon0);
	ui->mainToolBar->addAction(ui->actionLoad_Images);

	QIcon icon1;
	icon1.addFile(QStringLiteral("../icons/fileSave.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->actionSave_Points_As_PLY->setIcon(icon1);
	ui->mainToolBar->addAction(ui->actionSave_Points_As_PLY);

	QIcon icon2;
	icon2.addFile(QStringLiteral("../icons/sfm.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->actionSFM->setIcon(icon2);
	ui->mainToolBar->addAction(ui->actionSFM);

	QIcon icon3;
	icon3.addFile(QStringLiteral("../icons/cmvs.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->actionCMVS->setIcon(icon3);
	ui->mainToolBar->addAction(ui->actionCMVS);

	QIcon icon4;
	icon4.addFile(QStringLiteral("../icons/dense.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->actionDense_Points->setIcon(icon4);
	ui->mainToolBar->addAction(ui->actionDense_Points);


	// display action group 
	QActionGroup *displayActions = new QActionGroup(this);
	displayActions->addAction(ui->actionSparse_Points);
	displayActions->addAction(ui->actionDense_Points);
	displayActions->addAction(ui->actionVetices);
	displayActions->addAction(ui->actionWire_Frame);
	displayActions->addAction(ui->actionFlat);
	displayActions->addAction(ui->actionFlatLine);
	displayActions->addAction(ui->actionTexure);
	ui->actionWire_Frame->setChecked(true);

	//----------------------------------------structure from motion ---------------------------------------------------------//
	m_sfm_ = new SFM();
	m_sfm_->setSparsePointsPtr(&m_sparse_pts_);

	//----------------------------------------CMVS && PMVS-------------------------------------------------------------------//
	m_cmvs_ = new CMVS();
	m_cmvs_->setDensePointsPtr(&m_dense_pts_);
	m_cmvs_->setPointsIdsPtr(&m_points_ids_);

	//------------------------------------------POISSON----------------------------------------------------------------------//
	m_poisson_ = new Poisson();
	m_poisson_->setMeshVeticesPtr(&m_vertices_);
	m_poisson_->setMeshEdgesPtr(&m_edges_);
	m_poisson_->setMeshFacetPtr(&m_facets_);
	m_poisson_->setDensePointsPtr(&m_dense_pts_);
	m_poisson_->setPointsIdsPtr(&m_points_ids_);


	//------------------------------------------TEXTURE MAPPING-------------------------------------------------------------//
	m_texture_mapping_ = new TextureMapping();
	m_texture_mapping_->setPointsIdsPtr(&m_points_ids_);
	m_texture_mapping_->setDensePointsPtr(&m_dense_pts_);
	m_texture_mapping_->setMeshVeticesPtr(&m_vertices_);
	m_texture_mapping_->setMeshFacetPtr(&m_facets_);
	m_texture_mapping_->setMeshEdgesPtr(&m_edges_);
	//m_texture_mapping_->setImagesPtr(&m_images_);
	m_texture_mapping_->setCamerasPtr(&m_cameras_);
	m_texture_mapping_->setTextureCoordatesPtr(&m_texture_coords_);
	m_texture_mapping_->setPlane3DPtr(&m_plane_);

	//-----------------------------------------GLVIEWER---------------------------------------------------------------------//
	ui->viewer->setSparsePointsPtr(&m_sparse_pts_);
	ui->viewer->setDensePointsPtr(&m_dense_pts_);
	ui->viewer->setMeshVeticesPtr(&m_vertices_);
	ui->viewer->setMeshEdgesPtr(&m_edges_);
	ui->viewer->setMeshFacetPtr(&m_facets_);
	ui->viewer->setPointsIdsPtr(&m_points_ids_);
	ui->viewer->setCamerasPtr(&m_cameras_);
	ui->viewer->setPlane3DPtr(&m_plane_);
	ui->viewer->setTextureCoordatesPtr(&m_texture_coords_);

	//----------------------------------------SLOTS AND SIGNALS------------------------------------------------------------//
	//load images
	connect(ui->actionLoad_Images, SIGNAL(triggered()), this, SLOT(loadImages()));
	// save points
	connect(ui->actionSave_Points_As_PLY, SIGNAL(triggered()), this, SLOT(save2PLY()));
	// load points from PLY
	connect(ui->actionLoad_Points, SIGNAL(triggered()), this, SLOT(loadPointsFromPLY()));
	//save mesh
	connect(ui->actionSave_Mesh, SIGNAL(triggered()), this, SLOT(saveMesh()));
	//load mesh
	connect(ui->actionLoad_Mesh, SIGNAL(triggered()), SLOT(loadMesh()));

	// run sfm
	connect(ui->actionSFM, SIGNAL(triggered()), m_sfm_, SLOT(run()), Qt::QueuedConnection);
	m_sfm_->moveToThread(&m_thread_);
	m_thread_.start();

	// run cmvs
	connect(ui->actionCMVS, SIGNAL(triggered()), m_cmvs_, SLOT(run()));
	m_cmvs_->moveToThread(&m_thread_);
	m_thread_.start();

	// run poisson
	connect(ui->actionPoisson_Surface, SIGNAL(triggered()), m_poisson_, SLOT(run()), Qt::QueuedConnection);
	m_poisson_->moveToThread(&m_thread_);
	m_thread_.start();

	// all in one
	connect(ui->actionAll_in_One, SIGNAL(triggered()), this, SLOT(allinOne()));

	// para set
	connect(ui->actionParams_Setting, SIGNAL(triggered()), this, SLOT(allinOne()));

	// display functions
	connect(ui->actionSparse_Points, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_sparse(bool)));
	connect(ui->actionDense_Points, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_dense(bool)));
	connect(ui->actionVetices, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_vertices(bool)));
	connect(ui->actionWire_Frame, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_wire_frame(bool)));
	connect(ui->actionFlat, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_flat(bool)));
	connect(ui->actionFlatLine, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_flat_line(bool)));
	connect(ui->actionTexure, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_texture(bool)));
	connect(ui->actionCameras, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_cameras(bool)));

	connect(this, SIGNAL(displayDensePoints(bool)), ui->viewer, SLOT(toggle_display_dense(bool)));
	connect(this, SIGNAL(displayTexture(bool)), ui->viewer, SLOT(toggle_display_texture(bool)));

	// show information from sfm
	connect(m_sfm_, SIGNAL(statusBar(QString)), this, SLOT(sfmMessageToStatusBar(QString)));
	connect(m_sfm_, SIGNAL(textEdit(QString)), this, SLOT(sfmMessageToTextEdit(QString)));

	// show information from cmvs
	connect(m_cmvs_, SIGNAL(statusBar(QString)), this, SLOT(cmvsMessageToStatusBar(QString)));
	connect(m_cmvs_, SIGNAL(textEdit(QString)), this, SLOT(cmvsMessageToTextEdit(QString)));

	// show information from viewer
	connect(ui->viewer, SIGNAL(statusBar(QString)), this, SLOT(viewerMessageToStatusBar(QString)));
	connect(ui->viewer, SIGNAL(textEdit(QString)), this, SLOT(viewerMessageToTextEdit(QString)));

	// show information from poisson
	connect(m_poisson_, SIGNAL(statusBar(QString)), this, SLOT(viewerMessageToStatusBar(QString)));
	connect(m_poisson_, SIGNAL(textEdit(QString)), this, SLOT(viewerMessageToTextEdit(QString)));

	// show information from textureMapping
	connect(m_texture_mapping_, SIGNAL(statusBar(QString)), this, SLOT(textureMappingToStatusBar(QString)));
	connect(m_texture_mapping_, SIGNAL(textEdit(QString)), this, SLOT(textureMappingToTextEdit(QString)));

	// enable actions
	connect(m_sfm_, SIGNAL(enableActionSparse()), this, SLOT(enableActionSparsePoints()));
	connect(m_cmvs_, SIGNAL(enableActionDense()), this, SLOT(enableActionDensePoints()));
	connect(m_poisson_, SIGNAL(enableActionMesh()), this, SLOT(enableActionMesh()));
	connect(m_texture_mapping_, SIGNAL(enableActionCameras()), this, SLOT(enableActionCameras()));
	connect(m_texture_mapping_, SIGNAL(enableActionTexture()), this, SLOT(enableActionTexture()));

	// play path
	connect(ui->actionPath, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_setKeyFrame(bool)));
	connect(ui->actionPlay_Path, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_play_path(bool)));

	// select points
	connect(ui->actionSelect_Points, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_select_points(bool)));

	// texture mapping
	connect(ui->actionTexture_Mapping, SIGNAL(triggered()), m_texture_mapping_, SLOT(run()), Qt::QueuedConnection);
	m_texture_mapping_->moveToThread(&m_thread_);
	m_thread_.start();

	// set texture image dir
	connect(m_texture_mapping_, SIGNAL(textureImageDir(QString)), ui->viewer, SLOT(setTextureImageDir(QString)));
	connect(this, SIGNAL(textureImageDir(QString)), ui->viewer, SLOT(setTextureImageDir(QString)));
	//----------------------------------------------------debug--------------------------------------------------//
	connect(ui->actionBase_Plane, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_debug_base_plane(bool)));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
	delete ui;
}
//////////////////////////////////////////////////loadImages///////////////////////////////////////////////////////
bool MainWindow::loadImages()
{
	int height_img = 0;
	int width_img = 0;
	int height_icon = 0;
	int width_icon = 0;

	bool params_setting = false;

	// get image names list
	QStringList img_names = QFileDialog::getOpenFileNames(this, tr("Load Images"));

	// create a progress dialog
	QProgressDialog progress(this);
	progress.setLabelText(tr("Loading Images..."));
	progress.setRange(0, img_names.size());
	progress.setWindowModality(Qt::WindowModal);

	int nSteps = 0;

	QImage img;

	//load images
	foreach(QString img_name, img_names)
	{
		// set the value of progress diaog
		progress.setValue(nSteps);
		qApp->processEvents();
		if (progress.wasCanceled())
		{
			img_names.clear();
			return false;
		}

		if (!img_name.isEmpty())
		{
			img.load(img_name);
			m_img_dirs_.append(img_name);

			height_img = img.height();
			width_img = img.width();

			// size of the item
			int width_icon = ui->imageListWidget->width() - 25;
			int height_icon = (float)height_img / (float)width_img*width_icon;
			if (params_setting == false)
			{
				ui->imageListWidget->setIconSize(QSize(width_icon, height_icon));
				params_setting = true;
			}

			QStringList fields = img_name.split("/");
			QListWidgetItem * pItem = new QListWidgetItem(QIcon(QPixmap::fromImage(img)), fields.takeLast());
			pItem->setSizeHint(QSize(width_icon, height_icon + 20));// +15 make the txt appear
			ui->imageListWidget->insertItem(nSteps, pItem);

			m_img_folder_ = fields.join("/");

			nSteps++;
			update();
		}
	}

	m_sfm_->setImgDirs(m_img_dirs_);
	m_sfm_->setImgFolder(m_img_folder_);
	m_cmvs_->setImgDirs(m_img_dirs_);
	m_cmvs_->setImgFolder(m_img_folder_);
	//    pItem->setStatusTip(fields.takeLast());

	return true;

}
///////////////////////////////////////////////////allinOne////////////////////////////////////////////////////////
void MainWindow::allinOne()
{
	statusBar()->showMessage(tr("Run All in One"));

	ui->outputTextEdit->append(tr("[Running SFM... "));
	m_sfm_->run();
	ui->outputTextEdit->append(tr("[Running SFM Done! "));

	ui->outputTextEdit->append(tr("[Running CMVS... "));
	m_cmvs_->run();
	ui->outputTextEdit->append(tr("[Running CMVS Done! "));

	statusBar()->showMessage(tr("Run All in One Done!"));
}
////////////////////////////////////////////////////paraSet////////////////////////////////////////////////////////
void MainWindow::paraSet()
{}
////////////////////////////////////////////////////paraSet////////////////////////////////////////////////////////
void MainWindow::sfmMessageToStatusBar(QString str)
{
	statusBar()->showMessage(str);
	update();
}
///////////////////////////////////////sfmMessageToTextEdit////////////////////////////////////////////////////////
void MainWindow::sfmMessageToTextEdit(QString str)
{
	ui->outputTextEdit->append(str);
	update();
}
//////////////////////////////////enableActionSparsePoints////////////////////////////////////////////////////////
void MainWindow::enableActionSparsePoints()
{
	ui->actionSparse_Points->setEnabled(true);
	update();
}
/////////////////////////////////////cmvsMessageToStatusBar////////////////////////////////////////////////////////
void MainWindow::cmvsMessageToStatusBar(QString str)
{
	statusBar()->showMessage(str);
	update();
}
//////////////////////////////////////cmvsMessageToTextEdit////////////////////////////////////////////////////////
void MainWindow::cmvsMessageToTextEdit(QString str)
{
	ui->outputTextEdit->append(str);
	update();
}
////////////////////////////enableActionDensePoints////////////////////////////////////////////////////////
void MainWindow::enableActionDensePoints()
{
	ui->actionDense_Points->setEnabled(true);
	update();
}
////////////////////////////viewerMessageToStatusBar////////////////////////////////////////////////////////
void MainWindow::viewerMessageToStatusBar(QString str)
{
	statusBar()->showMessage(str);
	update();
}
//////////////////////////// viewerMessageToTextEdit////////////////////////////////////////////////////////
void MainWindow::viewerMessageToTextEdit(QString str)
{
	ui->outputTextEdit->append(str);
	update();
}
////////////////////////////poissonMessageToStatusBar//////////////////////////////////////////////////////
void MainWindow::poissonMessageToStatusBar(QString str)
{
	statusBar()->showMessage(str);
	update();
}
/////////////////////////////poissonMessageToTextEdit//////////////////////////////////////////////////////
void MainWindow::poissonMessageToTextEdit(QString str)
{
	ui->outputTextEdit->append(str);
	update();
}
////////////////////////////enableActionMesh //////////////////////////////////////////////////////////////
void MainWindow::enableActionMesh()
{
	ui->actionVetices->setEnabled(true);
	ui->actionWire_Frame->setEnabled(true);
	ui->actionFlat->setEnabled(true);
	ui->actionFlatLine->setEnabled(true);
	ui->actionTexure->setEnabled(true);
	update();
	ui->viewer->updateGL();
}
////////////////////////////textureMappingToStatusBar////////////////////////////////////////////////////////
void MainWindow::textureMappingToStatusBar(QString str)
{
	statusBar()->showMessage(str);
	update();
}
////////////////////////////textureMappingToTextEdit////////////////////////////////////////////////////////
void MainWindow::textureMappingToTextEdit(QString str)
{
	ui->outputTextEdit->append(str);
	update();
}
////////////////////////////textureMappingToTextEdit////////////////////////////////////////////////////////
void MainWindow::enableActionTexture()
{
	ui->actionTexure->setEnabled(true);
	update();
}
////////////////////////////enableActionCameras//////////////////////////////////////////////////////////////
void MainWindow::enableActionCameras()
{
	ui->actionCameras->setEnabled(true);
	update();
}
//////////////////////////////////save2PLY//////////////////////////////////////////////////////////////////
bool MainWindow::save2PLY()
{
	QString file_name = QFileDialog::getSaveFileName(this, tr("Save PLY File"), ".", tr("PLY Files(*.ply)"));

	if (!file_name.isEmpty())
	{
		QFile file(file_name);
		if (!file.open(QIODevice::WriteOnly))
		{
			statusBar()->showMessage(tr("Fail to Save PLY!"));
			return false;
		}

		QTextStream out(&file);
		out << "ply" << endl;
		out << "format ascii 1.0" << endl;
		out << "element vertex " << m_points_ids_.size() << endl;
		out << "property float x" << endl;
		out << "property float y" << endl;
		out << "property float z" << endl;
		out << "property float nx" << endl;
		out << "property float ny" << endl;
		out << "property float nz" << endl;
		out << "property uchar diffuse_red" << endl;
		out << "property uchar diffuse_green" << endl;
		out << "property uchar diffuse_blue" << endl;
		out << "end_header" << endl;


		foreach(int id, m_points_ids_)
		{
			Point pt = m_dense_pts_.at(id);
			out << pt.x << " " << pt.y << " " << pt.z << " ";
			out << pt.normal_x << " " << pt.normal_y << " " << pt.normal_z << " ";
			out << pt.r << " " << pt.g << " " << pt.b << endl;
		}
	}
}
//////////////////////////////////Load points from/// //////////////////////////////////////////////////////
//---------------------------------------- thread can be used --------------------------------------------//
bool MainWindow::loadPointsFromPLY()
{

	m_dense_pts_.clear();

	QDir dir;
	if (!dir.exists(tr("pmvs/models"))) return false;

	QString path = QDir::currentPath() + tr("/pmvs/models");
	dir.setPath(path);
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QStringList filter;
	filter << "*.ply";

	// ply files
	QFileInfoList fileList = dir.entryInfoList(filter);
	int nFiles = fileList.size();

	statusBar()->showMessage(tr("Loading Dense Points..."));

	// create a progress dialog
	QProgressDialog progress(this);
	progress.setLabelText(tr("Loading Dense Points..."));
	progress.setWindowModality(Qt::WindowModal);
	progress.setRange(0, nFiles);

	int nSteps = 0;

	for (int i = 0; i < nFiles; i++)
	{
		progress.setValue(nSteps);
		qApp->processEvents();

		// the last file containes all the points	
		QString ply_file_name = fileList.at(i).filePath();

		ui->outputTextEdit->append("Loading Points From" + ply_file_name);
		//load points
		QVector<Point> tmp = loadPointsFromPLYFile_debug(ply_file_name);

		foreach(Point pt, tmp) m_dense_pts_.append(pt);
		tmp.clear();
		
		if (progress.wasCanceled())
		{
			m_dense_pts_.clear();
			return false;
		}
		nSteps++;
	}

	for (int i = 0; i < m_dense_pts_.size(); i++)
	{
		m_points_ids_.insert(i);
	}

	// enable action to show dense poitns
	ui->actionDense_Points->setEnabled(true);

	// set bounding box
	ui->viewer->viewAll();
	ui->viewer->showEntireScene();

	QString outputText = QString("%1 points").arg(m_dense_pts_.size());
	statusBar()->showMessage(outputText);

	// diaplay dense points
	emit displayDensePoints(true);
	ui->viewer->updateGL();

	return true;

#if 0
	QString file_name = QFileDialog::getOpenFileName(this, tr("Open PointCloud File"), ".",
		tr("Point Cloud files(*.ply)"));
	if (!file_name.isEmpty())
	{
		QFile file(file_name);
		if (!file.open(QIODevice::ReadOnly))
		{
			statusBar()->showMessage(tr("Fail to Load Dense Points!"));
			return false;
		}

		statusBar()->showMessage(tr("Loading Dense Points..."));

		// create a progress dialog
		QProgressDialog progress(this);
		progress.setLabelText(tr("Loading Dense Points..."));
		progress.setWindowModality(Qt::WindowModal);

		int nSteps = 0;
		bool start_to_read = false;

		QTextStream in(&file);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			QStringList fields = line.split(" ");

			// read number of vertices
			if (line.startsWith("element vertex"))
			{
				fields.takeFirst();
				fields.takeFirst();
				int pt_num = fields.takeFirst().toInt();

				progress.setRange(0, pt_num);
			}
			// begin to read points
			if (line.startsWith("end_header"))
			{
				start_to_read = true;
				continue;
			}

			// read points 
			if (start_to_read == true)
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

				m_dense_pts_.append(pt);
				nSteps++;

				progress.setValue(nSteps);
				qApp->processEvents();
			}
			if (progress.wasCanceled())
			{
				m_dense_pts_.clear();
				return false;
			}
		}


		// points indices
		for (int i = 0; i < m_dense_pts_.size(); i++)
		{
			m_points_ids_.insert(i);
		}

		// enable action to show dense poitns
		ui->actionDense_Points->setEnabled(true);

		// set bounding box
		ui->viewer->viewAll();
		ui->viewer->showEntireScene();

		QString outputText = QString("%1 points").arg(m_dense_pts_.size());
		statusBar()->showMessage(outputText);

		// diaplay dense points
		emit displayDensePoints(true);
		ui->viewer->updateGL();

		return true;

#endif
}
	//////////////////////////////////Load Mesh////////////////////////////////////////////////////////////////
	bool MainWindow::loadMesh()
	{
		QString file_name = QFileDialog::getOpenFileName(this, tr("Open Mesh File"), ".",
			tr("Mesh files(*.ply)"));

		if (file_name.isEmpty())
		{
			statusBar()->showMessage(tr("Fail to Load Mesh!"));
			return false;
		}
		QFile file(file_name);
		if (!file.open(QIODevice::ReadOnly))
		{
			statusBar()->showMessage(tr("Fail to Load Mesh!"));
			return false;
		}
		statusBar()->showMessage(tr("Loading Mesh..."));

		// create a progress dialog
		QProgressDialog progress(this);
		progress.setLabelText(tr("Loading Dense Points..."));
		progress.setWindowModality(Qt::WindowModal);

		QTextStream in(&file);

		int facet_num = 0;
		int vertex_num = 0;
		int counter = 0;
		bool start_read = false;
		while (!in.atEnd())
		{
			QString line = in.readLine();
			QStringList fields = line.split(" ");

			if (line.startsWith("ply") || line.startsWith("comment") || line.startsWith("format"))
			{
				continue;
			}

			// nummber of vetices
			if (line.startsWith("element vertex"))
			{
				fields.takeFirst();
				fields.takeFirst();
				vertex_num = fields.takeFirst().toInt();
				continue;
			}

			// nummber of facets
			if (line.startsWith("element face"))
			{
				fields.takeFirst();
				fields.takeFirst();
				facet_num = fields.takeFirst().toInt();
				continue;
			}
			// end of header
			if (line.startsWith("end_header"))
			{

				progress.setRange(0, vertex_num + facet_num);
				start_read = true;
				counter = 0;
				continue;
			}

			// read vertices
			if (start_read == true && counter < vertex_num)
			{
				Point pt;
				pt.x = fields.takeFirst().toFloat();
				pt.y = fields.takeFirst().toFloat();
				pt.z = fields.takeFirst().toFloat();
				pt.normal_x = fields.takeFirst().toFloat();
				pt.normal_y = fields.takeFirst().toFloat();
				pt.normal_z = fields.takeFirst().toFloat();

				QPointF coord;
				coord.setX(fields.takeFirst().toFloat());
				coord.setY(fields.takeFirst().toFloat());

				m_vertices_.append(pt);
				m_texture_coords_.append(coord);

				counter++;
			}
			// read facets
			if (start_read == true && counter >= vertex_num && counter < vertex_num + facet_num + 1)
			{
				if (counter == vertex_num)
				{
					counter++;
					continue;
				}
				QVector<int> facet;
				int num = fields.takeFirst().toInt();

				for (int i = 0; i < num; i++)
				{
					facet.append(fields.takeFirst().toInt());
				}
				m_facets_.append(facet);
				counter++;
			}
			progress.setValue(counter);
			qApp->processEvents();

			if (progress.wasCanceled())
			{
				m_vertices_.clear();
				m_facets_.clear();
				return false;
			}
		}
		//--------------------------------compute edges -----------------------------------------//
		// get edges from facets
		QSet<QPair<int, int> > e;
		foreach(QVector<int> facet, m_facets_)
		{
			int pt_num = facet.size();
			for (int i = 0; i < pt_num; i++)
			{
				int id0 = facet[i];
				int id1 = facet[(i + 1) % pt_num];

				if (id0 > id1)
				{
					e << qMakePair(id1, id0);
				}
				else{
					e << qMakePair(id0, id1);
				}
			}
		}
		QSet<QPair<int, int> > ::const_iterator iter = e.constBegin();
		while (iter != e.constEnd())
		{
			m_edges_ << *iter;
			iter++;
		}

		ui->actionVetices->setEnabled(true);
		ui->actionFlat->setEnabled(true);
		ui->actionFlatLine->setEnabled(true);
		ui->actionWire_Frame->setEnabled(true);
		ui->actionTexure->setEnabled(true);

		ui->viewer->viewAll();
		ui->viewer->showEntireScene();

		QString texture_image_dir = file_name.replace("ply", "png");
		emit textureImageDir(texture_image_dir);// set the texture image dir of glviewer
		emit displayTexture(true);
		ui->viewer->updateGL();

#if 0
		// enable action to show dense poitns
		ui->actionDense_Points->setEnabled(true);

		// set bounding box
		ui->viewer->viewAll();
		ui->viewer->showEntireScene();

		QString outputText = QString("%1 points").arg(m_dense_pts_.size());
		statusBar()->showMessage(outputText);

		// diaplay dense points
		ui->viewer->setDisplayDensePoints(true);
		ui->viewer->updateGL();
#endif

		return true;
	}
	//////////////////////////////////Save Mesh//////////////////////////////////////////////////////////////////
	bool MainWindow::saveMesh()
	{
		// 保存成off文件
#if 0
		// two files needed to be  created: the first is the file to store the mesh and the second is an auxiliary file
		// that stores the texture coordinates of the vertices
		QString file_name = QFileDialog::getSaveFileName(this, tr("Save OFF File"), ".", tr("PLY Files(*.OFF)"));

		if (file_name.isEmpty())
		{
			statusBar()->showMessage(tr("Fail to Save Mesh !"));
			return false;
		}
		QFile file(file_name);
		if (!file.open(QIODevice::WriteOnly))
		{
			statusBar()->showMessage(tr("Fail to Save Mesh !"));
			return false;
		}

		//*****************************************************************************************************//
		/*                                       Mesh need to be upated                                  */
		//*****************************************************************************************************//
		QTextStream out(&file);
		out << "OFF" << endl;
		out << m_vertices_.size() << " " << m_facets_.size() << " " << 0 << endl;
		foreach(Point pt, m_vertices_)
		{
			out << pt.x << " " << pt.y << " " << pt.z << endl;
		}
		foreach(QVector<int> facet, m_facets_)
		{
			out << facet.size() << " ";
			foreach(int id, facet)
			{
				out << id << " ";
			}
			out << endl;
	}

		//-------------------------------------cerate an append file for texture-----------------------------------//
		QString append_name = file_name + ".append";
		QFile file1(append_name);
		if (!file1.open(QIODevice::WriteOnly))
		{
			statusBar()->showMessage(tr("Fail to Save Mesh !"));
			return false;
		}

		QTextStream out1(&file1);
		out1 << m_texture_coords_.size() << endl;
		foreach(QPointF pt2D, m_texture_coords_)
		{
			out1 << pt2D.x() << pt2D.y() << endl;
		}
		return true;

#endif
		QString file_name = QFileDialog::getSaveFileName(this, tr("Save PLY File"), ".", tr("PLY Files(*.ply)"));

		if (file_name.isEmpty())
		{
			statusBar()->showMessage(tr("Fail to Save Mesh !"));
			return false;
		}
		QFile file(file_name);
		if (!file.open(QIODevice::WriteOnly))
		{
			statusBar()->showMessage(tr("Fail to Save Mesh !"));
			return false;
		}
		QString texture_dir = file_name.replace("ply", "png");

		QTextStream out(&file);
		out << "ply" << endl;
		out << "format ascii 1.0" << endl;
		out << "element vertex " << m_vertices_.size() << endl;
		out << "property float x" << endl;
		out << "property float y" << endl;
		out << "property float z" << endl;
		out << "property float nx" << endl;
		out << "property float ny" << endl;
		out << "property float nz" << endl;
		out << "property float tx" << endl;
		out << "property float ty" << endl;

		out << "element face " << m_facets_.size() << endl;
		out << "property list uchar int vertex_indices" << endl;
		out << "end_header" << endl;

		int counter = 0;
		foreach(Point pt, m_vertices_)
		{
			out << pt.x << " " << pt.y << " " << pt.z << " ";
			out << pt.normal_x << " " << pt.normal_y << " " << pt.normal_z << " ";
			out << m_texture_coords_[counter].x() << " " << m_texture_coords_[counter].y() << endl;
			counter++;
		}

		foreach(QVector<int> facet, m_facets_)
		{
			out << facet.size() << " ";
			foreach(int id, facet)
			{
				out << id << " ";
			}
			out << endl;
		}

		//--------------------------------------------save texture image---------------------------------------//
		m_texture_mapping_->textureImage().save(texture_dir);

		return true;
	}

