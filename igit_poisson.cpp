#include"igit_poisson.h"
#include<qdir.h>
#include<qmessagebox.h>
#include <qprocess.h>
#include<qtextstream.h>
#include<ctime>

/////////////////////////////////////loadMeshFromPLY////////////////////////////////////////////////////////////
bool Poisson::loadMeshFromPLY(QString file_name)
{

	//----------------------- read vertices and facets from file ---------------------//
	QFile file(file_name);
	if (!file.open(QIODevice::ReadOnly))
	{
		emit textEdit(tr("Fail to Load Sparse Points!"));
		return false;
	}
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
		}

		// end of header
		if (line.startsWith("end_header"))
		{
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
			p_vertices_->append(pt);
			counter++;
		}

		// read facets
		if (start_read == true && counter >= vertex_num && counter < vertex_num + facet_num +1)
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
			p_facets_->append(facet);
			counter++;
		}
	}
	//--------------------------------compute edges -----------------------------------------//
	// get edges from facets
	QSet<QPair<int, int> > e;
	foreach(QVector<int> facet, *p_facets_)
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
		(*p_edges_) << *iter;
		iter++;
	}

	return true;
}
////////////////////////////////////////savePointsToPLYFile////////////////////////////////////////////////
bool Poisson::savePointsToPLYFile(QString file_name)
{
	QFile file(file_name);
	if (!file.open(QIODevice::WriteOnly))
	{
		statusBar(tr("Fail to Save PLY!"));
		return false;
	}

	QTextStream out(&file);
	out << "ply" << endl;
	out << "format ascii 1.0" << endl;
	out << "element vertex " << p_points_ids_->size() << endl;
	out << "property float x" << endl;
	out << "property float y" << endl;
	out << "property float z" << endl;
	out << "property float nx" << endl;
	out << "property float ny" << endl;
	out << "property float nz" << endl;
	out << "end_header" << endl;

	foreach(int id, *p_points_ids_)
	{
		Point pt = p_dense_pts_->at(id);
		out << pt.x << " " << pt.y << " " << pt.z << " ";
		out << pt.normal_x << " " << pt.normal_y << " " << pt.normal_z << endl;
	}
	return true;
}
////////////////////////////////////////trimmer/////////////////////////////////////////////////////////////
void Poisson::trimmingFacets()
{

//-----------------------------elimiate vertices exceeding the bounding box--------------------------------//
	float min_x = 1000000;
	float min_y = 1000000;
	float min_z = 1000000;
	float max_x = -1000000;
	float max_y = -1000000;
	float max_z = -1000000;

	// bounding box are computed from the dense points
	foreach(Point pt, *p_dense_pts_)
	{
		if (min_x > pt.x) min_x = pt.x;
		if (min_y > pt.y) min_y = pt.y;
		if (min_z > pt.z) min_z = pt.z;

		if (max_x < pt.x) max_x = pt.x;
		if (max_y < pt.y) max_y = pt.y;
		if (max_z < pt.z) max_z = pt.z;
	}

	// trimmed facets
	QVector<int> trimmed_vertices;
	int counter = 0;
	foreach(Point pt, *p_vertices_)
	{
		if (pt.x<min_x || pt.x> max_x || pt.y < min_y || pt.y>max_y || pt.z < min_z || pt.z >max_z)
		{
			trimmed_vertices.append(counter);
		}
		counter++;
	}

	// check whether the facets contain eliminated vertice, and if true the facets are eliminated
	QVector<QVector<Point> > all_facets;
	foreach(QVector<int> facet, *p_facets_)
	{
		QVector<Point> facets;

		bool trimmed = false;
		foreach(int id, facet)
		{
			facets.append((*p_vertices_)[id]);
			if (trimmed_vertices.contains(id))
			{
				trimmed = true;
				break;
			}
		}

		if (trimmed == false)
		{
			all_facets.append(facets);
		}
	}

	//--------------------------------------------------update the mesh---------------------------------------//
	// reconstruct the mesh from the remaining facets
	updateMesh(all_facets);

}////////////////////////////////////updateMesh//////////////////////////////////////////////////////////////////
void Poisson::updateMesh(QVector<QVector<Point> > & facets)
{
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
		for (int i = 0; i< pt_num; i++)
		{
			int id0 = facet[i];
			int id1 = facet[(i + 1) % pt_num];

			if (id0< id1) e.insert(qMakePair(id0, id1));
			if (id1< id0) e.insert(qMakePair(id1, id0));
		}
	}
	QSet<QPair<int, int> > ::const_iterator iter = e.constBegin();
	while (iter != e.constEnd())
	{
		new_edges << (*iter);
		iter++;
	}


	p_vertices_->swap(new_vertices);
	p_edges_->swap(new_edges);
	p_facets_->swap(new_facets);
}
/////////////////////////////////////run ////////////////////////////////////////////////////////////////////////
void Poisson::run()
{
	const double begin = (double)clock() / CLOCKS_PER_SEC;

	emit textEdit("[Run Possion Surface Reconstruction\n");
	emit statusBar("Run Possion Surface Reconstruction");


	QDir dir;
	if (!dir.exists("all_points.ply"))
	{
		savePointsToPLYFile("all_points.ply");
		//QMessageBox::Warning(this, tr("Warning"), tr("No Input File for Posion Surface Resoncstuction"));
	}
	
	QString cmdAgu("PoissonRecon.exe --in all_points.ply  --out mesh.ply");
	emit textEdit(cmdAgu);

	system("PoissonRecon.exe --in all_points.ply --depth 10 --out mesh.ply --ascii");
#if 0
	QProcess cmd;
	cmd.start("Bundler.exe list.txt --options_file options.txt > bundler.out");
	while (cmd.waitForFinished())
	{
	}
# endif
	emit textEdit("  [Loading Meshes...");
	loadMeshFromPLY("mesh.ply");
	emit textEdit("  Done!] \n");


//---------------------------------------trimming facets--------------------------------------------------//
// therare many facets that are generated by interplotation and there are no points support them , and we trim
// these facets by first computing a bounding box of the dense points and any vertices exceed this bounding box
// are deleted. It is a very easy trick, and can not work for all the situations

	emit textEdit("  [Trimming Facets...");
	trimmingFacets();
	emit textEdit("  Done!]\n");

	emit textEdit("Done!]");
	emit statusBar("Done!");
	emit enableActionMesh();
	
	const double end = (double)clock() / CLOCKS_PER_SEC;
	QString txt = QString("Running time is %1 s").arg(end - begin);
	textEdit(txt);

}