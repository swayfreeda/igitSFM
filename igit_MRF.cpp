#include "igit_MRF.h"
#include"igit_functions.h"

#include <limits>
#include <qglviewer.h>

#include<qfile.h>
#include<qtextstream.h>

#if 0
/////////////////////////////////////gradient of image/////////////////////////////////////////////////////////////
float gradient(int x, int y, QImage & image)
{
	float result = 0;

	QRgb rgb = image.pixel(x, y);
	float val0 = 0.299*qRed(rgb) + 0.587 * qGreen(rgb) + 0.114*qBlue(rgb);


	// 4-connected neighbours
	for (int xn = x - 1; xn < x + 1; xn++)
	{
		for (int yn = y - 1; yn < y + 1; yn++)
		{
			if (xn == x&& yn == y)continue;
			if (xn < 0 || xn >= image.width() || yn < 0 || yn >= image.height())continue;

			QRgb rgb = image.pixel(xn, yn);
			float val = 0.299*qRed(rgb) + 0.587 * qGreen(rgb) + 0.114*qBlue(rgb);			
			result += qAbs(val - val0);	
		}
	}

	return result;
}

/////////////////////////////////////generateDataArraySmoothTruncatedLinear/////////////////////////////////////////
void IGIT_MRF::generateDataArraySmoothTruncatedLinear()
{
	// for data function
	for (int i = 0; i < nSites_; i++)
	{
		for (int j = 0; j < nLabels_; j++)
		{
			if (candidate_labels_[i].contains(j))
			{
				D_[i* nLabels_ + j] =  -dataTerm(i, j);  // dataTerm 
			}
			else{
				D_[i*nLabels_ + j] = (MRF::CostVal) 100000;
			}
		}
	}

#if 0
	// generate function
	for (int i = 0; i < nLabels_; i++) {
		for (int j = i; j < nLabels_; j++) {
			V_[i*nLabels_ + j] = V_[j*nLabels_ + i] = (i == j) ? 0 : lambda_;
		}
	}
#endif
	// Use if the smoothness is V(l1,l2) = lambda * min ( |l1-l2|^m_smoothExp, m_smoothMax )
	MRF::CostVal smoothMax = (MRF::CostVal)20;


	// allocate energy
	DataCost *data = new DataCost(D_);
	//SmoothnessCost *smooth = new SmoothnessCost(V_);
	SmoothnessCost *smooth = new SmoothnessCost(1, smoothMax, lambda_*0);
	energy_ = new EnergyFunction(data, smooth);
}

/////////////////////////////////////////////////data Term //////////////////////////////////////////////////////////
MRF::CostVal  IGIT_MRF::dataTerm(int site, int label)
{

	// get image, note that label is the view index
	QString name;
	name.sprintf("%08d.jpg", label);
	QImage ref_img = (*images_)[name];

	int imageH = ref_img.height();
	int imageW = ref_img.width();

	// not that the structure of candidate_texture_coordinates, in each row the 
	// label is not the index, and the index need to be calculated
	int ind = 0; 
	for (int i = 0; i < candidate_labels_[site].size(); i++)
	{
		if (candidate_labels_[site][i] == label)
		{
			ind = i;
			break;
		}
	}
	// get the coordinates of the  projected vertices
	QVector<QPoint> coords_ref = candidate_texture_coordinates_[site][ind];

	//**********************************************************************************//
	/*                              计算面积进行加权                                   */
	//**********************************************************************************//
	// gennerate some  area coordinate and each area coordinate represents a point
	QVector <float> interval;
	int size = 6;
	float step = 1.0 / (float)(size -1);
	for (int i = 0; i < size; i++)
	{
		interval.append(i*step);
	}
	QVector<Point> area_coordinates;
	for (int i = 0; i < interval.size(); i++)
	{
		for (int j = 0; j < interval.size(); j++)
		{
			if (interval[i] + interval[j] > 1) continue;
			area_coordinates.append(Point(interval[i], interval[j], 1 - interval[i]- interval[j]));
		}
	}

	// We use set structure to ensure that each point id  only calculated once
	QVector<QPoint> points2D;
	foreach(Point coord_area, area_coordinates)
	{
		int ww = (int)(coord_area.x * coords_ref[0].x() + coord_area.y * coords_ref[1].x() + coord_area.z * coords_ref[2].x() + 0.5);
		int hh = (int)(coord_area.x * coords_ref[0].y() + coord_area.y * coords_ref[1].y() + coord_area.z * coords_ref[2].y() + 0.5);

		points2D.append(QPoint(ww, hh));
	}
	//********************************************************************************//
	/*做一下处理，去除重复的点*/
	//**********************************************************************************//
	// calculate the mean gradient

	float gradient_sum = 0;
	foreach(QPoint coord, points2D)
	{
		gradient_sum += gradient(coord.x(), coord.y(), ref_img);
	}
	float gradient_mean = gradient_sum / (float)points2D.size();
	return (MRF::CostVal)gradient_mean;
}
//////////////////////////////////////////////////run////////////////////////////////////////////////////////////////
void  IGIT_MRF::runBP()
{

	float t, tot_t;
	int iter;

	//------------------------------------------generate energy function -------------------------------------//
	generateDataArraySmoothTruncatedLinear();

	QFile file1("dataTerm.txt");
	file1.open(QIODevice::WriteOnly);
	QTextStream out1(&file1);

	for (int i = 0; i < nSites_; i++)
	{
		for (int j = 0; j < nLabels_; j++)
		{
			out1 << (float)D_[i* nLabels_ + j] << " ";
		}
		out1 << endl;
	}

	//------------------------------------------MaxProd Belief Propagation -----------------------------------//
	//printf("\n*******  Started MaxProd Belief Propagation *****\n");
	//mrf_ = new MaxProdBP(nSites_, nLabels_, energy_);
	mrf_ = new Swap(nSites_, nLabels_, energy_);
#if 1
	// set neighbours
	for (int i = 0; i < neighbours_.size(); i++)
	{
		for (int j = 0; j < neighbours_[i].size(); j++)
		{
			int id0 = i; 
			int id1 = neighbours_[i][j];
			if (id0 == id1) continue;  // will not happen

			if (id0 < id1) mrf_->setNeighbors(id0, id1, (MRF::CostVal) 1);
			else{
				mrf_->setNeighbors(id1, id0, (MRF::CostVal)1);
			}
		}
	}
	mrf_->initialize();
	mrf_->clearAnswer();
	E_ = mrf_->totalEnergy();

	QFile file("energy.txt");
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);

	out << "Energy at the Start: " << (float)E_<<"  ";
	out << "smoothTerm: " << (float)mrf_->smoothnessEnergy()<<" ";
	out << "dateTerm: " << (float)mrf_->dataEnergy() << endl;
	//printf("Energy at the Start= %g (%g,%g)\n", (float)E,
	//	(float)mrf->smoothnessEnergy(), (float)mrf->dataEnergy());

	tot_t = 0;
	for (iter = 0; iter < 10; iter++) {
		mrf_->optimize(1, t);

		E_ = mrf_->totalEnergy();
		tot_t = tot_t + t;
		out << iter << " th iteration: " << endl;
		out << "Energy: " << (float)E_ << "  ";
		out << "smoothTerm: " << (float)mrf_->smoothnessEnergy() << " ";
		out << "dateTerm: " << (float)mrf_->dataEnergy() << endl;
	}

	delete mrf_;
#endif
}
#endif

#define VAR_ACTIVE     ((Energy::Var)0)
#define IS_VAR(var) (var >(Energy::Var) 1)
#define VALUE0 0
#define VALUE1 1


void error_function(char *msg)
{
	//fprintf(stderr, "%s\n", msg);
	exit(1);
}
/////////////////////////////////////////////////gradient of image////////////////////////////////////////////////
Energy::Value gradient(int x, int y, const cv::Mat & image)
{
	Energy::Value result = 0;

	Energy::Value val0 = (Energy::Value)image.at<uchar>(y, x);
	// 4-connected neighbours
	for (int xn = x - 1; xn < x + 1; xn++)
	{
		for (int yn = y - 1; yn < y + 1; yn++)
		{
			if (xn == x&& yn == y)continue;
			if (xn < 0 || xn >= image.cols || yn < 0 || yn >= image.rows)continue;

			Energy::Value val = (Energy::Value)image.at<uchar>(yn, xn);
			result += qAbs(val - val0);
		}
	}

	return result/(Energy::Value)255.0;
}
/////////////////////////////////////////////computerDataTerm/////////////////////////////////////////////////////
Energy::Value IGIT_MRF::computerDataTerm(int site, int label)
{
	// get image, note that label is the view index
	QString name;
	name.sprintf("%08d.jpg", label);
	cv::Mat ref_img = (*gray_images_)[name];

	int imageH = ref_img.rows;
	int imageW = ref_img.cols;

	// note that the structure of candidate_texture_coordinates, in each row the 
	// label is not the index, and therefore the index need to be calculated
	int ind = 0;
	for (int i = 0; i < candidate_labels_[site].size(); i++)
	{
		if (candidate_labels_[site][i] == label)
		{
			ind = i;
			break;
		}
	}
	// get the coordinates of the  projected vertices
	QVector<QPoint> coords_ref = candidate_coords_[site][ind];

	// gennerate some  area coordinate and each area coordinate represents a point
	QVector <float> interval;
	int size = 15;
	float step = 1.0 / (float)(size - 1);
	for (int i = 0; i < size; i++)
	{
		interval.append(i*step);
	}
	QVector<Point> area_coordinates;
	for (int i = 0; i < interval.size(); i++)
	{
		for (int j = 0; j < interval.size(); j++)
		{
			if (interval[i] + interval[j] > 1) continue;
			area_coordinates.append(Point(interval[i], interval[j], 1 - interval[i] - interval[j]));
		}
	}

	// We use set structure to ensure that each point id  only calculated once
	QVector<QPoint> points2D;
	foreach(Point coord_area, area_coordinates)
	{
		int ww = (int)(coord_area.x * coords_ref[0].x() + coord_area.y * coords_ref[1].x() + coord_area.z * coords_ref[2].x() + 0.5);
		int hh = (int)(coord_area.x * coords_ref[0].y() + coord_area.y * coords_ref[1].y() + coord_area.z * coords_ref[2].y() + 0.5);

		points2D.append(QPoint(ww, hh));
	}
	//********************************************************************************//
	/*做一下处理，去除重复的点*/
	//**********************************************************************************//
	// calculate the mean gradient

	Energy::Value gradient_sum = 0;
	foreach(QPoint coord, points2D)
	{
		gradient_sum += gradient(coord.x(), coord.y(), ref_img);
	}
	Energy::Value gradient_mean = (Energy::Value)(float)gradient_sum / (float)points2D.size();

	//---------------------------------------------area of the triangulate----------------------------------------//
	// triangulation in default
	QPoint p0 = coords_ref[0];
	QPoint p1 = coords_ref[1];
	QPoint p2 = coords_ref[2];

	// area of the triangulation
	qglviewer::Vec v0(p1.x() - p0.x(), p1.y() - p0.y(), 0);
	qglviewer::Vec v1(p2.x() - p0.x(), p2.y() - p0.y(), 0);
	qglviewer::Vec vv = cross(v0, v1);
	float area = 0.5 * qAbs(vv.norm());
	
	return (Energy::Value) area*gradient_mean;
}
////////////////////////////////////////////computeDateTermArray//////////////////////////////////////////////////
void IGIT_MRF::computeDataTermArray()
{
	D_.resize(nSites_);
	// for data function
	for (int i = 0; i < nSites_; i++)
	{
		for (int j = 0; j < nLabels_; j++)
		{
			if (candidate_labels_[i].contains(j))
			{
				D_[i].append( -computerDataTerm(i, j) );  // dataTerm 
			}
			else{
				D_[i].append( 1) ;
			}
		}
	}

#if DEBUG_
	QFile file1("DEBUG_dataTerm.txt");
	file1.open(QIODevice::WriteOnly);
	QTextStream out1(&file1);

	for (int i = 0; i < nSites_; i++)
	{
		for (int j = 0; j < nLabels_; j++)
		{
			out1 << (Energy::Value)D_[i][j] << " ";
		}
		out1 << endl;
	}
#endif 
}
//////////////////////////////////////////////////data term //////////////////////////////////////////////////////
Energy::Value IGIT_MRF::data_term(int site, int label)
{
	if (D_[site][label] == (Energy::Value)1)
	{
		return 10000;
	}
	else
	{
		return D_[site][label];
	}	
}
//////////////////////////////////////////////////smooth term ////////////////////////////////////////////////////
Energy::Value IGIT_MRF::smooth_term(int xid, int yid, int xlabel, int ylabel)
{
	if (xlabel == ylabel) return 0;
	else {
		return lambda_;
	}
}
///////////////////////////////////////////////////computeEnergy/////////////////////////////////////////////////
void IGIT_MRF::computeEnergy()
{
	E_ = 0;

	for (int i = 0; i < nSites_; i++)
	{
		E_ += data_term(i, labels_[i]);
	}

	for (int i = 0; i < neighbors_.size(); i++)
	{
		int xid = i;
		int xlabel = labels_[xid];

		for (int j = 0; j < neighbors_[i].size(); j++)
		{
			int yid = neighbors_[i][j];
			int ylabel = labels_[yid];

			E_ += smooth_term(xid, yid, xlabel, ylabel);
		}
	}
}
/////////////////////////////////////////////////////expansion/////////////////////////////////////////////////
Energy::Value IGIT_MRF::expansion()
{
	Energy *e = new Energy(error_function);
	Energy::Var Var_p, Var_q;
	Energy::Var * variables = new Energy::Var[nSites_];

	computeEnergy();


	Energy::Value E_old = E_;

	int newl = alpha_;

	// data_term
	for (int i = 0; i< nSites_; i++)
	{
		int oldl = labels_[i];

		Energy::Value E0 = data_term(i, oldl);
		Energy::Value E1 = data_term(i, newl);

		if (oldl == newl)
		{
			variables[i] = VAR_ACTIVE;
			e->add_constant((Energy::Value) E0);
		}
		else{

			Var_p = e->add_variable();
			variables[i] = Var_p;

			e->add_term1(Var_p, E0, E1);
		}
	}

	// smooth term
	Energy::Value E00, E01, E10, E11;
	for (int i = 0; i< nSites_; i++)
	{
		int xid = i;
		int oldl_x = labels_[xid];

		Var_p = variables[i];

		for (int j = 0; j< neighbors_[i].size(); j++)
		{
			int yid = neighbors_[i][j];
			int oldl_y = labels_[yid];

			Var_q = variables[yid];

			E00 = smooth_term(xid, yid, oldl_x, oldl_y);
			E01 = smooth_term(xid, yid, oldl_x, newl);
			E10 = smooth_term(xid, yid, newl, oldl_y);
			E11 = smooth_term(xid, yid, newl, newl);

			if (Var_p != VAR_ACTIVE)
			{
				if (Var_q != VAR_ACTIVE)
				{
					e->add_term2(Var_p, Var_q, E00, E01, E10, E11);
				}
				else
				{
					e->add_term1(Var_p, E01, E11);
				}
			}
			else
			{
				if (Var_q != VAR_ACTIVE)
				{
					e->add_term1(Var_q, E10, E11);
				}
				else
				{
				}
			}
		}
	}

	Energy::Value E_new = e->minimize();

	if (E_new < E_old)
	{
		for (int i = 0; i< nSites_; i++)
		{
			Var_p = variables[i];

			if (Var_p != VAR_ACTIVE&& e->get_var(Var_p) == VALUE1)
			{
				labels_[i] = newl;
			}
		}
	}

	delete e;
	delete [] variables;

	e = NULL;
	variables = NULL;
	return E_new;
}
/////////////////////////////////////////////////////optimization//////////////////////////////////////////////
void IGIT_MRF::optimization()
{
#if DEBUG_
	QFile file("energy.txt");
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);
#endif

	labels_.clear();
	for (int i = 0; i < nSites_; i++) labels_.append(0);

	// initial energy
	computeEnergy();
	
	Energy::Value E = E_;
	int effect_num = nLabels_;

#if DEBUG_
	out << "E-Start = " << (Energy::Value)E << endl;
#endif

	// alpha expansion
	for (int iterOuter = 0; iterOuter< max_iter_num_&&effect_num>0; iterOuter++)
	{

#if DEBUG_
		out << iterOuter << "  th iteration " << endl;
#endif
		for (int iterInner = 0; iterInner< nLabels_; iterInner++)
		{
			Energy::Value E_old = E;
			setAlpha(iterInner);
			E = expansion();

			computeEnergy();
			Energy::Value E_tmp = E_;
			if (abs(E_tmp - E)> 10)
			{
#if DEBUG_
				out << "E and E_tmp are different! " << " E: " << E << "  E_tmp:  " << E_tmp << endl;
#endif
			}

			if (abs(E_old - E)< 0.01)
			{
				effect_num--;
			}
			else{
				effect_num = nLabels_;
			}
#if DEBUG_
			out << "E = " << E << endl;
#endif
		}
	}

}