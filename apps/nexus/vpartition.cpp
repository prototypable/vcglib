/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

$Log: not supported by cvs2svn $

****************************************************************************/

#include <stdio.h>

#include "vpartition.h"
#include <ANN/ANN.h>

using namespace std;
using namespace vcg;
using namespace nxs;

void VPartition::Init() {  
  if(bd) delete bd;
  buffer.resize(size() * 3);
  for(unsigned int i = 0; i < size(); i++) {
    for(int k = 0; k < 3; k++)
      buffer[i*3+k] = operator[](i)[k];
  }
  points.resize(size());
  for(unsigned int i = 0; i < size(); i++) {
    points[i] = &buffer[i*3];
  }
  bd = new ANNkd_tree(&*points.begin(), size(), 3);
}

void VPartition::Closest(const vcg::Point3f &p, unsigned int nsize, 
			       vector<int> &nears, 
			       vector<float> &dist) {
  double point[3];
  point[0] = p[0];
  point[1] = p[1];
  point[2] = p[2];

  nears.resize(nsize);
  dist.resize(nsize);
  vector<double> dists;
  dists.resize(nsize);
  bd->annkSearch(&point[0], nsize, &*nears.begin(), &*dists.begin());
  for(unsigned int i = 0; i < nsize; i++)
    dist[i] = (float)dists[i];
}

void VPartition::Closest(const vcg::Point3f &p, 
			       int &target, float &dist) {
  double point[3];
  point[0] = p[0];
  point[1] = p[1];
  point[2] = p[2];
  double dists;
  bd->annkSearch(&point[0], 1, &target, &dists, 1);
  assert(target >= 0);
  assert(target < size());

  dist = (float)dists;
}

int VPartition::Locate(const vcg::Point3f &p) {

  double point[3];
  point[0] = p[0];
  point[1] = p[1];
  point[2] = p[2];

  int target = -1;
  double dists;
  bd->annkSearch(&point[0], 1, &target, &dists, 1);

  return target;
} 

float VPartition::Radius(unsigned int seed) {
  assert(size() > 1);
  int nears[2];
  double dists[2];

  double point[3];
  Point3f &p = operator[](seed);
  point[0] = p[0];
  point[1] = p[1];
  point[2] = p[2];

  bd->annkSearch(&point[0], 2, nears, dists);

  if(dists[1] == 0) return 0.0f;
  assert(nears[0] == seed);
  assert(dists[0] == 0);
  
  return (float)sqrt(dists[1]);
}
