//===============================================================================================================//

// Author: Sui Wei
// Time: 10/17/2014
// Organization: Institue of Automation, Chinese Academy of Science
// Description: MRF framework for optimize the visible views of each facet. The sites follows are the facets and the 
// labels are the views that the facets can be viewed. The enery is minimized by BP.

//===============================================================================================================//

#ifndef IGIT_MRF_H
#define IGIT_MRF_H
#include "data_type.h"
#include<qmap.h>
#include<qimage.h>

typedef PointXYZRGBNormal Point;

#if 0
class IGIT_MRF
{
public:
	IGIT_MRF(int nSites, int nLabels): nSites_(nSites), nLabels_(nLabels){	 
		D_ = new MRF::CostVal[nSites * nLabels];
		V_ = new MRF::CostVal[nLabels * nLabels];
	}

	// we use array for data term and truncated linear function for smooth term
	void generateDataArraySmoothTruncatedLinear();

	// compute data term of assign label to site
	MRF::CostVal dataTerm(int site, int label);

	// BP algorithm for optimization
	void runBP();

	void setImagePtr(QMap<QString, QImage> * ptr){ images_ = ptr; }
	void setCandidateLabels(QVector<QVector<int> > & labels){ candidate_labels_ = labels; }
	void setCandidateTextureCoordinates(QVector<QVector<QVector<QPoint> >  >& ptr){ candidate_texture_coordinates_ = ptr; }
	void setNeighbours(QVector<QVector<int> > & neighs){ neighbours_ = neighs; }
	void setVerticesPtr(QVector<Point> * ptr){ vertices_ = ptr; }
	void setFacetsPtr(QVector<QVector<int> > * ptr){ facets_ = ptr; }
	void setLambda(MRF::CostVal lambda) { lambda_ = lambda; }
	QVector<int> getFinalLabels()
	{
		QVector<int>label;
		for (int i = 0; i < nSites_; i++)
		{
			label.append(mrf_->getLabel(i));
		}
		return label;
	}
private:

	// number of facets
	int nSites_;

	//number of views
	int nLabels_;

	// An arrray for data term
	MRF::CostVal * D_;
	MRF::CostVal *V_;

	//  MRF class from libMRF
	MRF* mrf_;

	// energy function containes data term and smooth term
	EnergyFunction *energy_;

	// energy in the optimiztion process
	MRF::EnergyVal E_;

	// images for gradient information
	QMap<QString, QImage> * images_;
	// vertices of the mesh
	QVector<Point> * vertices_;
	// facets of the mesh
	QVector<QVector<int> > * facets_;

	// each site can only be assigned several labels, and we call these labels as candidate labels
	QVector<QVector<int> > candidate_labels_;

	// textures of all the facets in all the corresponding views
	QVector<QVector<QVector<QPoint> > >candidate_texture_coordinates_;

	// neighboring relationship of each site
	QVector<QVector<int> > neighbours_;

	QVector<int> final_labels_;

	MRF::CostVal lambda_;
};


//USAGE:
//IGIT_MRF mrf(site_num, label_num);
//mrf.setLambda(10);
//mrf.setImagePtr(t_images_);
//mrf.setCandidateLabels(t_initial_facet_vis_);
//mrf.setCandidateTextureCoordinates(t_initial_facet_coordinates_);
//mrf.setNeighbours(neighbours);
//mrf.setFacetsPtr(t_facets_);
//mrf.setVerticesPtr(t_vertices_);
//mrf.runBP();
#endif


#include"energy.h"
#include"graph.h"
#include<qvector.h>
#include<qpoint.h>

class IGIT_MRF
{

public:

	IGIT_MRF(int nSites, int nLabels) : nSites_(nSites), nLabels_(nLabels){

		E_ = 0;
	}

	~IGIT_MRF(){
		D_.clear();
		labels_.clear();
		neighbors_.clear();
		candidate_labels_.clear();
		candidate_coords_.clear();
		gray_images_ = NULL;;
		vertices_ = NULL;
		facets_ = NULL;
	}

	Energy::Value computerDataTerm(int site, int label);

	void computeDataTermArray();

	Energy::Value data_term(int site, int label);

	Energy::Value smooth_term(int xid, int yid, int xlabel, int ylabel);

	void computeEnergy();

	Energy::Value expansion();

	void optimization();


	void setAlpha(int alpha){ alpha_ = alpha; }
	void setMaxIterNum(int num){ max_iter_num_ = num; }
	void setLambda(float lambda){ lambda_ = lambda; }
	void setNeighbours(QVector<QVector<int> > &neighbrs){ neighbors_ = neighbrs; }

	void setImagePtr(QMap<QString, cv::Mat> * ptr){ gray_images_ = ptr; }
	void setCandidateLabels(QVector<QVector<int> > & labels){ candidate_labels_ = labels; }
	void setCandidateTextureCoordinates(QVector<QVector<QVector<QPoint> >  >& ptr){ candidate_coords_ = ptr; }
	void setVerticesPtr(QVector<Point> * ptr){ vertices_ = ptr; }
	void setFacetsPtr(QVector<QVector<int> > * ptr){ facets_ = ptr; }
	QVector<int> getFinalLabels(){ return labels_; }

private:

	QVector<QVector<Energy::Value> > D_;

	QVector<int> labels_;
	QVector<QVector<int> > neighbors_;
	QVector<QVector<int> > candidate_labels_;
	QVector<QVector<QVector<QPoint> > > candidate_coords_;
	QMap<QString, cv::Mat> * gray_images_;
	QVector<Point> * vertices_;
	QVector<QVector<int> > * facets_;

	int nLabels_;
	int nSites_;
	int max_iter_num_;
	Energy::Value  E_;
	Energy::Value lambda_;
	int alpha_;
};


#endif
