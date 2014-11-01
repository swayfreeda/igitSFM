/* 
*  Copyright (c) 2008-2010  Noah Snavely (snavely (at) cs.cornell.edu)
*    and the University of Washington
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*/

/* keys2.cpp */
/* Class for SIFT keypoints */

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "keys2a.h"

extern "C"{
#include "sift.h"
};

int GetNumberOfKeysNormal(FILE *fp)
{
    int num, len;

    if (fscanf(fp, "%d %d", &num, &len) != 2) {
        printf("Invalid keypoint file.\n");
        return 0;
    }

    return num;
}

int GetNumberOfKeysGzip(gzFile fp)
{
    int num, len;

    char header[256];
    gzgets(fp, header, 256);

    if (sscanf(header, "%d %d", &num, &len) != 2) {
        printf("Invalid keypoint file.\n");
        return 0;
    }

    return num;
}

/* Returns the number of keys in a file */
int GetNumberOfKeys(const char *filename)
{
    FILE *file;

    file = fopen (filename, "r");
    if (! file) {
        /* Try to file a gzipped keyfile */
        char buf[1024];
        sprintf(buf, "%s.gz", filename);
        gzFile gzf = gzopen(buf, "rb");

        if (gzf == NULL) {
            printf("Could not open file: %s\n", filename);
            return 0;
        } else {
            int n = GetNumberOfKeysGzip(gzf);
            gzclose(gzf);
            return n;
        }
    }

    int n = GetNumberOfKeysNormal(file);
    fclose(file);
    return n;
}

/* This reads a keypoint file from a given filename and returns the list
* of keypoints. */
int ReadKeyFile(int &len, const char *filename, unsigned char **keys, keypt_t **info)
{
    FILE *file;

    file = fopen (filename, "r");
    if (! file) {
        /* Try to file a gzipped keyfile */
        char buf[1024];
        sprintf(buf, "%s.gz", filename);
        gzFile gzf = gzopen(buf, "rb");

        if (gzf == NULL) {
            printf("Could not open file: %s\n", filename);
            return 0;
        } else {
            int n = ReadKeysGzip(len, gzf, keys, info);
            gzclose(gzf);
            return n;
        }
    }

    int n = ReadKeys(len, file, keys, info);
    fclose(file);
    return n;

    // return ReadKeysMMAP(file);
}

#if 0
/* Read keys using MMAP to speed things up */
std::vector<Keypoint *> ReadKeysMMAP(FILE *fp) 
{    
    int i, j, num, len, val, n;

    std::vector<Keypoint *> kps;

    struct stat sb;

    /* Stat the file */
    if (fstat(fileno(fp), &sb) < 0) {
        printf("[ReadKeysMMAP] Error: could not stat file\n");
        return kps;
    }

    char *file = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, 
        fileno(fp), 0);

    char *file_start = file;

    if (sscanf(file, "%d %d%n", &num, &len, &n) != 2) {
        printf("[ReadKeysMMAP] Invalid keypoint file beginning.");
        return kps;
    }

    file += n;

    if (len != 128) {
        printf("[ReadKeysMMAP] Keypoint descriptor length invalid "
            "(should be 128).");
        return kps;
    }

    for (i = 0; i < num; i++) {
        /* Allocate memory for the keypoint. */
        unsigned char *d = new unsigned char[len];
        float x, y, scale, ori;

        if (sscanf(file, "%f %f %f %f%n", &y, &x, &scale, &ori, &n) != 4) {
            printf("[ReadKeysMMAP] Invalid keypoint file format.");
            return kps;
        }

        file += n;

        for (j = 0; j < len; j++) {
            if (sscanf(file, "%d%n", &val, &n) != 1 || val < 0 || val > 255) {
                printf("[ReadKeysMMAP] Invalid keypoint file value.");
                return kps;
            }
            d[j] = (unsigned char) val;
            file += n;
        }

        kps.push_back(new Keypoint(x, y, scale, ori, d));
    }

    /* Unmap */
    if (munmap(file_start, sb.st_size) < 0) {
        printf("[ReadKeysMMAP] Error: could not unmap memory\n");
        return kps;
    }

    return kps;    
}
#endif

/* Read keypoints from the given file pointer and return the list of
* keypoints.  The file format starts with 2 integers giving the total
* number of keypoints and the size of descriptor vector for each
* keypoint (currently assumed to be 128). Then each keypoint is
* specified by 4 floating point numbers giving subpixel row and
* column location, scale, and orientation (in radians from -PI to
* PI).  Then the descriptor vector for each keypoint is given as a
* list of integers in range [0,255]. */
int ReadKeys(int &len, FILE *fp, unsigned char **keys, keypt_t **info)
{
    int i, num;

    std::vector<Keypoint *> kps;

    if (fscanf(fp, "%d %d\n", &num, &len) != 2) {
        printf("Invalid keypoint file\n");
        return 0;
    }

    *keys = new unsigned char[len * num + 8];

    if (info != NULL) 
        *info = new keypt_t[num];

    unsigned char *p = *keys;
    for (i = 0; i < num; i++) {
        /* Allocate memory for the keypoint. */
        // short int *d = new short int[128];
        float x, y, scale, ori;

        if (fscanf(fp, "%f %f %f %f", &y, &x, &scale, &ori) != 4) {
            printf("Invalid keypoint file format.");
            return 0;
        }

		for (int j = 0;j < len;j++)
			fscanf(fp, " %hhu", p+j);
		fscanf(fp, "\n");

		p += len;

        if (info != NULL) {
            (*info)[i].x = x;
            (*info)[i].y = y;
            (*info)[i].scale = scale;
            (*info)[i].orient = ori;
        }

        /*char buf[1024];
        for (int line = 0; line < 7; line++) {
            fgets(buf, 1024, fp);

            if (line < 6) {
                sscanf(buf, 
                    "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu "
                    "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu", 
                    p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7, p+8, p+9, 
                    p+10, p+11, p+12, p+13, p+14, 
                    p+15, p+16, p+17, p+18, p+19);

                p += 20;
            } else {
                sscanf(buf, 
                    "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu",
                    p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7);
                p += 8;
            }
        }*/

    }

    return num; // kps;
}

int ReadKeysGzip(int &len, gzFile fp, unsigned char **keys, keypt_t **info)
{
    int i, num;

    std::vector<Keypoint *> kps;
    char header[256];
    gzgets(fp, header, 256);

    if (sscanf(header, "%d %d\n", &num, &len) != 2) {
        printf("Invalid keypoint file.\n");
        return 0;
    }

    *keys = new unsigned char[len * num + 8];

    if (info != NULL) 
        *info = new keypt_t[num];

    unsigned char *p = *keys;
    for (i = 0; i < num; i++) {
        /* Allocate memory for the keypoint. */
        // short int *d = new short int[128];
        float x, y, scale, ori;
        char buf[1024];
        gzgets(fp, buf, 1024);

        if (sscanf(buf, "%f %f %f %f\n", &y, &x, &scale, &ori) != 4) {
            printf("Invalid keypoint file format.");
            return 0;
        }
		for (int j = 0;j < len;j++)
			sscanf(buf, " %hhu", p+j);
		sscanf(buf, "\n");

		p += len;

        if (info != NULL) {
            (*info)[i].x = x;
            (*info)[i].y = y;
            (*info)[i].scale = scale;
            (*info)[i].orient = ori;
        }

        /*for (int line = 0; line < 7; line++) {
            char *str = gzgets(fp, buf, 1024);
            assert(str != Z_NULL);

            if (line < 6) {
                sscanf(buf, 
                    "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu "
                    "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu", 
                    p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7, p+8, p+9, 
                    p+10, p+11, p+12, p+13, p+14, 
                    p+15, p+16, p+17, p+18, p+19);

                p += 20;
            } else {
                sscanf(buf, 
                    "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu",
                    p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7);
                p += 8;
            }
        }*/

    }

    //assert(p == *keys + len * num);

    return num; // kps;
}

/* Create a search tree for the given set of keypoints */
ANNkd_tree *CreateSearchTree(int num_keys, unsigned char *keys, int dim)
{
    // clock_t start = clock();

    /* Create a new array of points */
    ANNpointArray pts = annAllocPts(num_keys, dim);

    for (int i = 0; i < num_keys; i++) {
        memcpy(pts[i], keys + dim * i, sizeof(unsigned char) * dim);
    }

    /* Create a search tree for k2 */
    ANNkd_tree *tree = new ANNkd_tree(pts, num_keys, dim, 16);
    // clock_t end = clock();

    // printf("Building tree took %0.3fs\n", 
    //        (end - start) / ((double) CLOCKS_PER_SEC));    

    return tree;
}

std::vector<KeypointMatch> MatchKeys(int num_keys1, unsigned char *k1, int dim,
                                     ANNkd_tree *tree2,
                                     double ratio, int max_pts_visit)
{
    annMaxPtsVisit(max_pts_visit);
    std::vector<KeypointMatch> matches;

    /* Now do the search */
    // clock_t start = clock();
    for (int i = 0; i < num_keys1; i++) {
        ANNidx nn_idx[2];
        ANNdist dist[2];

        tree2->annkPriSearch(k1 + dim * i, 2, nn_idx, dist, 0.0);

        if (((double) dist[0]) < ratio * ratio * ((double) dist[1])) {
            matches.push_back(KeypointMatch(i, nn_idx[0]));
        }
    }
    // clock_t end = clock();

    // printf("Searching tree took %0.3fs\n", 
    //        (end - start) / ((double) CLOCKS_PER_SEC));

    return matches;    
}

/* Compute likely matches between two sets of keypoints */
std::vector<KeypointMatch> MatchKeys(int num_keys1, unsigned char *k1, 
                                     int num_keys2, unsigned char *k2, 
                                     double ratio, int max_pts_visit) 
{
    annMaxPtsVisit(max_pts_visit);

    int num_pts = 0;
    std::vector<KeypointMatch> matches;

    num_pts = num_keys2;
    clock_t start = clock();

    /* Create a new array of points */
    ANNpointArray pts = annAllocPts(num_pts, 128);

    for (int i = 0; i < num_pts; i++) {
        memcpy(pts[i], k2 + 128 * i, sizeof(unsigned char) * 128);
    }

    /* Create a search tree for k2 */
    ANNkd_tree *tree = new ANNkd_tree(pts, num_pts, 128, 16);
    clock_t end = clock();

    // printf("Building tree took %0.3fs\n", 
    //	      (end - start) / ((double) CLOCKS_PER_SEC));

    /* Now do the search */
    start = clock();
    for (int i = 0; i < num_keys1; i++) {
        ANNidx nn_idx[2];
        ANNdist dist[2];

        tree->annkPriSearch(k1 + 128 * i, 2, nn_idx, dist, 0.0);

        if (((double) dist[0]) < ratio * ratio * ((double) dist[1])) {
            matches.push_back(KeypointMatch(i, nn_idx[0]));
        }
    }
    end = clock();
    // printf("Searching tree took %0.3fs\n", 
    //        (end - start) / ((double) CLOCKS_PER_SEC));

    /* Cleanup */
    annDeallocPts(pts);
    // annDeallocPt(axis_weights);

    delete tree;

    return matches;
}

void KepMatchFull(char *list_in, char *file_out)
{
	double ratio = 0.9;// 0.6 for sift

	clock_t start = clock();

	unsigned char **keys;
	int *num_keys;

	/* Read the list of files */
	std::vector<std::string> key_files;

	FILE *f = fopen(list_in, "r");
	if (f == NULL) {
		printf("Error opening file %s for reading\n", list_in);
		return;
	}

	char buf[512];
	while (fgets(buf, 512, f)) {
		/* Remove trailing newline */
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0;

		key_files.push_back(std::string(buf));
	}

	fclose(f);

	f = fopen(file_out, "w");
	assert(f != NULL);

	int num_images = (int) key_files.size();
	int dim = 0;

	keys = new unsigned char *[num_images];
	num_keys = new int[num_images];

	/* Read all keys */
	for (int i = 0; i < num_images; i++) {
		keys[i] = NULL;
		num_keys[i] = ReadKeyFile(dim, key_files[i].c_str(), keys+i);
	}

	clock_t end = clock();    
	printf("[KeyMatchFull] Reading keys took %0.3fs\n", 
		(end - start) / ((double) CLOCKS_PER_SEC));

	for (int i = 1; i < num_images; i++) {
		if (num_keys[i] == 0)
			continue;

		printf("[KeyMatchFull] Matching to image %d\n", i);

		start = clock();

		/* Create a tree from the keys */
		ANNkd_tree *tree = CreateSearchTree(num_keys[i], keys[i], dim);

		for (int j = 0; j < i; j++) {
			if (num_keys[j] == 0)
				continue;

			/* Compute likely matches between two sets of keypoints */
			std::vector<KeypointMatch> matches = MatchKeys(num_keys[j], keys[j], dim, tree, ratio);

			int num_matches = (int) matches.size();

			if (num_matches >= 16) {
				/* Write the pair */
				fprintf(f, "%d %d\n", j, i);

				/* Write the number of matches */
				fprintf(f, "%d\n", (int) matches.size());

				for (int i = 0; i < num_matches; i++) {
					fprintf(f, "%d %d\n", 
						matches[i].m_idx1, matches[i].m_idx2);
				}
			}
		}

		end = clock();    
		printf("[KeyMatchFull] Matching took %0.3fs\n", 
			(end - start) / ((double) CLOCKS_PER_SEC));
		fflush(stdout);

		// annDeallocPts(tree->pts);
		delete tree;
	}

	/* Free keypoints */
	for (int i = 0; i < num_images; i++) {
		if (keys[i] != NULL)
			delete [] keys[i];
	}
	delete [] keys;
	delete [] num_keys;

	fclose(f);
	return ;
}


int extract_vlsift(const char *imFile, const char* outFile, bool bCalcuOri)
{
	// ori in [0, 360], auto calculate dominant orientation if ori = -1
	/* algorithm parameters */
	int omin = -1;
	int ori = -1;
	if( !bCalcuOri )	ori = 0;
	double peak_thresh = 3;
	double   edge_thresh  = -1 ;
	double   magnif       = -1 ;
	int      O = -1, S = 3;
	vl_bool  err    = VL_ERR_OK ;
    vl_sift_pix     *fdata = 0 ;
    VlSiftFilt      *filt = 0 ;
    vl_size          q ;
    vl_bool          first ;
    int              nikeys = 0;

	FILE *fout = fopen(outFile, "wt");
	if( !fout ){
		printf("Incorrect outFile path.\n");
		return 0;
	}

	cv::Mat im = cv::imread(imFile, cv::IMREAD_GRAYSCALE);
	if( im.empty() ){
		printf("Incorrect input image.\n");
		return 0;
	}
    
    /* allocate buffer */
    fdata = (vl_sift_pix *) malloc(im.cols * im.rows * sizeof (vl_sift_pix)) ;
	for (int y = 0;y < im.rows;y++)
		for (int x = 0;x < im.cols;x++)
			fdata[y * im.cols + x] = im.at<uchar>(y,x);

	/* ...............................................................
     *                                                     Make filter
     * ............................................................ */

    filt = vl_sift_new (im.cols, im.rows, O, S, omin) ;

    if (edge_thresh >= 0) vl_sift_set_edge_thresh (filt, edge_thresh) ;
    if (peak_thresh >= 0) vl_sift_set_peak_thresh (filt, peak_thresh) ;
    if (magnif      >= 0) vl_sift_set_magnif      (filt, magnif) ;

    if (!filt) {
	  free(fdata);
      return 0;
    }
		
    /* ...............................................................
     *                                             Process each octave
     * ............................................................ */
	std::vector<vl_sift_pix*> desc_list;
	std::vector<cv::KeyPoint> kpt_list;
    first = 1 ;
    while (1) {
		VlSiftKeypoint const *keys = 0 ;
		int                   nkeys ;

		/* calculate the GSS for the next octave .................... */
		if (first) {
			first = 0 ;
			err = vl_sift_process_first_octave (filt, fdata) ;
		} else {
			err = vl_sift_process_next_octave  (filt) ;
		}

		if (err) {
			err = VL_ERR_OK ;
			break ;
		}

		/* run detector ............................................. */
		vl_sift_detect (filt) ;

		keys  = vl_sift_get_keypoints  (filt) ;
		nkeys = vl_sift_get_nkeypoints (filt) ;

		/* for each keypoint ........................................ */
		for (int i = 0; i < nkeys ; ++i) {
			double                angles [4] ;
			int                   nangles ;
			VlSiftKeypoint const *k ;

			/* obtain keypoint orientations ........................... */
			k = keys + i ;
			if( bCalcuOri )
				nangles = vl_sift_calc_keypoint_orientations(filt, angles, k) ;
			else{
				nangles = 1;
				angles[0] = 0;  // 转化为弧度
			}

			/* for each orientation ................................... */
			for (q = 0 ; q < (unsigned) nangles ; ++q) {    
				cv::KeyPoint kpt(k->x, k->y, k->sigma, angles[q]);
				kpt_list.push_back(kpt);
				vl_sift_pix *descr = new vl_sift_pix[128];
				vl_sift_calc_keypoint_descriptor(filt, descr, k, angles[q]) ;
				desc_list.push_back(descr);
			}        
		}
	}

	fprintf(fout, "%d %d\n", kpt_list.size(), 128);
	for (int i = 0;i < kpt_list.size();i++)
	{
		fprintf(fout, "%f %f %f %f", kpt_list[i].pt.y, kpt_list[i].pt.x, kpt_list[i].size, kpt_list[i].angle);
		for (int j = 0;j < 128;j++)
		{
			int x = int(512.0 * desc_list[i][j]);
			x = (x < 255) ? x : 255;
			fprintf(fout, " %hhu", x);
		}
		fprintf(fout, "\n");

		delete [] desc_list[i];
	}
	fclose(fout);


    /* ...............................................................
     *                                                       Finish up
     * ............................................................ */

    /* release filter */
    if (filt) {
		vl_sift_delete (filt) ;
		filt = 0 ;
    }

    /* release image data */
    if (fdata) {
		free (fdata) ;
		fdata = 0 ;
    }

	return (int) kpt_list.size();
}

#if 0
int extract_vlmrogh(const char *imFile, const char* outFile)
{
	// ori in [0, 360], auto calculate dominant orientation if ori = -1
	/* algorithm parameters */
	int omin = -1;
	double peak_thresh = 3;
	double   edge_thresh  = -1 ;
	double   magnif       = -1 ;
	int      O = -1, S = 3;
	vl_bool  err    = VL_ERR_OK ;
    vl_sift_pix     *fdata = 0 ;
    VlSiftFilt      *filt = 0 ;
    vl_size          q ;
    vl_bool          first ;
    int              nikeys = 0;

	FILE *fout = fopen(outFile, "wt");
	if( !fout ){
		printf("Incorrect outFile path.\n");
		return 0;
	}

	cv::Mat im = cv::imread(imFile, cv::IMREAD_GRAYSCALE);
	if( im.empty() ){
		printf("Incorrect input image.\n");
		return 0;
	}
    
    /* allocate buffer */
    fdata = (vl_sift_pix *) malloc(im.cols * im.rows * sizeof (vl_sift_pix)) ;
	for (int y = 0;y < im.rows;y++)
		for (int x = 0;x < im.cols;x++)
			fdata[y * im.cols + x] = im.at<uchar>(y,x);

	/* ...............................................................
     *                                                     Make filter
     * ............................................................ */

    filt = vl_sift_new (im.cols, im.rows, O, S, omin) ;

    if (edge_thresh >= 0) vl_sift_set_edge_thresh (filt, edge_thresh) ;
    if (peak_thresh >= 0) vl_sift_set_peak_thresh (filt, peak_thresh) ;
    if (magnif      >= 0) vl_sift_set_magnif      (filt, magnif) ;

    if (!filt) {
	  free(fdata);
      return 0;
    }
		
    /* ...............................................................
     *                                             Process each octave
     * ............................................................ */
	std::vector<vl_sift_pix*> desc_list;
	std::vector<cv::KeyPoint> kpt_list;
    first = 1 ;
    while (1) {
		VlSiftKeypoint const *keys = 0 ;
		int                   nkeys ;

		/* calculate the GSS for the next octave .................... */
		if (first) {
			first = 0 ;
			err = vl_sift_process_first_octave (filt, fdata) ;
		} else {
			err = vl_sift_process_next_octave  (filt) ;
		}

		if (err) {
			err = VL_ERR_OK ;
			break ;
		}

		/* run detector ............................................. */
		vl_sift_detect (filt) ;

		keys  = vl_sift_get_keypoints  (filt) ;
		nkeys = vl_sift_get_nkeypoints (filt) ;

		/* for each keypoint ........................................ */
		for (int i = 0; i < nkeys ; ++i) {
			double                angles [4] ;
			int                   nangles ;
			VlSiftKeypoint const *k ;

			k = keys + i ;

			vl_sift_pix *descr = new vl_sift_pix[192];
			vl_sift_pix tmp_descr[48];
			vl_sift_pix norm = 0;
			mrogh_calc_keypoint_descriptor(filt, tmp_descr, k, 1.0) ;
			for( int j = 0;j < 48;j++) 
				descr[j] = tmp_descr[j];
			mrogh_calc_keypoint_descriptor(filt, tmp_descr, k, 2.0) ;
			for( int j = 0;j < 48;j++) 
				descr[48+j] = tmp_descr[j];
			mrogh_calc_keypoint_descriptor(filt, tmp_descr, k, 3.0) ;
			for( int j = 0;j < 48;j++) 
				descr[96+j] = tmp_descr[j];
			mrogh_calc_keypoint_descriptor(filt, tmp_descr, k, 4.0) ;
			for( int j = 0;j < 48;j++) 
				descr[144+j] = tmp_descr[j];
			
			for( int j = 0;j < 192;j++ )
				norm += descr[j] * descr[j];
			norm = sqrt(norm);
			for( int j = 0;j < 192;j++)
				descr[j] /= norm;

			cv::KeyPoint kpt(k->x, k->y, k->sigma, 0);
			kpt_list.push_back(kpt);
			desc_list.push_back(descr);

		}
	}

	fprintf(fout, "%d %d\n", kpt_list.size(), 192);
	for (int i = 0;i < kpt_list.size();i++)
	{
		fprintf(fout, "%f %f %f %f", kpt_list[i].pt.y, kpt_list[i].pt.x, kpt_list[i].size, kpt_list[i].angle);
		for (int j = 0;j < 192;j++)
		{
			int x = int(512.0 * desc_list[i][j]);
			x = (x < 255) ? x : 255;
			fprintf(fout, " %hhu", x);
		}
		fprintf(fout, "\n");

		delete [] desc_list[i];
	}
	fclose(fout);


    /* ...............................................................
     *                                                       Finish up
     * ............................................................ */

    /* release filter */
    if (filt) {
		vl_sift_delete (filt) ;
		filt = 0 ;
    }

    /* release image data */
    if (fdata) {
		free (fdata) ;
		fdata = 0 ;
    }

	return (int)kpt_list.size();
}
#endif