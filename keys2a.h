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

/* keys2a.h */
/* Class for SIFT keypoints */

#ifndef __keys2a_h__
#define __keys2a_h__

#include <highgui.h>
#include <cxcore.h>
#include <cv.h>
#include <vector>
#include <string>
#include <bitset>

#if 0
#ifdef   _DEBUG 
#pragma comment(lib,"opencv_highgui243d.lib")
#pragma comment(lib,"opencv_core243d.lib")
#pragma comment(lib,"opencv_imgproc243d.lib")
#pragma comment(lib, "opencv_features2d243d.lib")
#else
#pragma comment(lib,"opencv_highgui243.lib")
#pragma comment(lib,"opencv_core243.lib")
#pragma comment(lib,"opencv_imgproc243.lib")
#pragma comment(lib, "opencv_features2d243.lib")
#endif
#endif

#include <vector>
#include <stdio.h>

#include <zlib.h>

#include "ANN/ANN.h"

using namespace ann_1_1_char;

class Keypoint {
public:    
    Keypoint(float x, float y, float scale, float ori, short int *d) :
	m_x(x), m_y(y), m_scale(scale), m_ori(ori), m_d(d)
    { }

    float m_x, m_y;        /* Subpixel location of keypoint. */
    float m_scale, m_ori;  /* Scale and orientation (range [-PI,PI]) */
    short int *m_d;    /* Vector of descriptor values */
};


/* Data struct for matches */
class KeypointMatch {
public:
    KeypointMatch() 
    { }

#if 0
    KeypointMatch(int idx1, int idx2, float x1, float y1, float x2, float y2) :
	m_idx1(idx1), m_idx2(idx2), m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
    { }
#endif

    KeypointMatch(int idx1, int idx2) :
	m_idx1(idx1), m_idx2(idx2)
    { }

    int m_idx1, m_idx2;
    // float m_x1, m_y1;
    // float m_x2, m_y2;
};

typedef struct {
    float x, y;
    float scale;
    float orient;
} keypt_t;

/* Returns the number of keys in a file */
int GetNumberOfKeys(const char *filename);

/* This reads a keypoint file from a given filename and returns the list
 * of keypoints. */
int ReadKeyFile(int &len, const char *filename, unsigned char **keys, keypt_t **info = NULL);

int ReadKeyPositions(const char *filename, keypt_t **info);

/* Read keypoints from the given file pointer and return the list of
 * keypoints.  The file format starts with 2 integers giving the total
 * number of keypoints and the size of descriptor vector for each
 * keypoint (currently assumed to be 128). Then each keypoint is
 * specified by 4 floating point numbers giving subpixel row and
 * column location, scale, and orientation (in radians from -PI to
 * PI).  Then the descriptor vector for each keypoint is given as a
 * list of integers in range [0,255]. */
int ReadKeys(int &len, FILE *fp, unsigned char **keys, keypt_t **info = NULL);
int ReadKeysGzip(int &len, gzFile fp, unsigned char **keys, keypt_t **info = NULL);

/* Read keys using MMAP to speed things up */
std::vector<Keypoint *> ReadKeysMMAP(FILE *fp);

/* Create a search tree for the given set of keypoints */
ANNkd_tree *CreateSearchTree(int num_keys, unsigned char *keys, int dim);

/* Compute likely matches between two sets of keypoints */
std::vector<KeypointMatch> MatchKeys(int num_keys1, unsigned char *k1, 
				     int num_keys2, unsigned char *k2,
				     double ratio = 0.6, 
                                     int max_pts_visit = 200);

std::vector<KeypointMatch> MatchKeys(int num_keys1, unsigned char *k1, int dim,
                                     ANNkd_tree *tree2,
								     double ratio = 0.6, 
                                     int max_pts_visit = 200);

void KepMatchFull(char *list_in, char *file_out);

int extract_vlsift(const char *imFile, const char* outFile, bool bCalcuOri = true);

#if 0
int extract_vlmrogh(const char *imFile, const char* outFile);
#endif
#endif /* __keys2_h__ */
