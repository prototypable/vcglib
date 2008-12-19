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

#include <vector>

/*include the base definition for the vertex */
#include <vcg/simplex/vertex/base.h>

/*include the base definition for the face */
#include <vcg/simplex/face/base.h>

/*include the base definition for the edge */
#include <vcg/simplex/edge/base.h>

/*include the base definition for the trimesh*/
#include <vcg/complex/trimesh/base.h>

/*include the algorithms for updating: */
#include <vcg/complex/trimesh/update/topology.h>	/* topology */
#include <vcg/complex/trimesh/update/bounding.h>	/* bounding box */
#include <vcg/complex/trimesh/update/normal.h>		/* normal */

/*include the algorithms for mesh fixing  */
#include <vcg/complex/trimesh/clean.h>

/*include the importer from disk*/
#include <wrap/io_trimesh/import.h>

/*include the exporter to disk (in ply)*/
#include <wrap/io_trimesh/export_ply.h>

/* include the support for polygon meshes (function to convert from/to trimesh)*/
#include <vcg/complex/trimesh/polygon_support.h>

/* include the support for polygon meshes (the component for the face )*/
#include <vcg/simplex/face/component_polygon.h>

/* include the support for half edges */
#include <vcg/complex/trimesh/edge_support.h>


using namespace vcg;
using namespace std;

// forward declarations
class CFace;
class CVertex;
class CEdge;
class DummyEdge;
class MyPolyVertex;

/* Definition of a mesh of triangles
*/
class CVertex : public VertexSimp2< CVertex, DummyEdge, CFace, 
	vertex::BitFlags,
	vertex::Coord3f, 
	vertex::Normal3f,
	vertex::Mark >{};

class CFace   : public FaceSimp2<   CVertex, DummyEdge, CFace,
	face::VertexRef,	// three pointers to vertices
	face::Normal3f,		// normal	
	face::BitFlags,		// flags
	face::FFAdj			// three pointers to adjacent faces
> {};

/* the mesh is a container of vertices and a container of faces */ 
class CMesh   : public vcg::tri::TriMesh< vector<CVertex>, vector<CFace> > {};


/* Definition of a mesh of polygons that also supports half-edges
*/
class MyPolyFace;

class MyPolyVertex:public vcg::VertexSimp2<MyPolyVertex , CEdge,MyPolyFace, 
	vcg::vertex::Coord3f,
	vcg::vertex::Normal3f,
	vcg::vertex::Mark,
	vcg::vertex::BitFlags, 
	vcg::vertex::VEAdj>{} ;

class CEdge : public EdgeSimp2< MyPolyVertex, CEdge, MyPolyFace, edge::BitFlags,
	//edge::EFAdj,		// pointer to the face
	//edge::HEOppAdj,	// pointer to the opposite edge
	//edge::HEVAdj,		// pointer to the vertex
	//edge::HENextAdj,	// pointer to the next halfedge
	edge::HEdgeData		// the previous 4 components (just more handy, you can comment this and uncomment the previous four lines)
	//,edge::HEPrevAdj	// pointer to the previous halfedge
>{};

class MyPolyFace:public vcg::FaceSimp2<
	 MyPolyVertex
	,CEdge  
	,MyPolyFace 
	,vcg::face::PolyInfo // this is necessary  if you use component in vcg/simplex/face/component_polygon.h
						 // It says "this class is a polygon and the memory for its components (e.g. pointer to its vertices
						 // will be allocated dynamically")	
	,vcg::face::PFVAdj	 // Pointer to the vertices (just like FVAdj )
	,vcg::face::PFFAdj	 // Pointer to edge-adjacent face (just like FFAdj )
	,vcg::face::PFHEAdj	 // Pointer its half -edges  ( you may need this if you use half edges)
	,vcg::face::BitFlags // bit flags
	,vcg::face::Normal3f // normal
> {};

class MyPolyMesh: public 
	vcg::tri::TriMesh< std::vector<MyPolyVertex>,	// the vector of vertices
	std::vector<MyPolyFace >,						// the vector of faces	
	std::vector<CEdge>								// the vector of edges 
	>{};

MyPolyMesh pm;


////////////////////////////////////////////////////////////////////////////
// Globals: the mesh, the OpenGL wrapper to draw the mesh and the trackball.
CMesh mesh,mesh1;




int main(int argc, char *argv[]) {

	int loadmask;
	
	vcg::tri::io::PlyInfo pi;

if(false){
	/*
	first way: 
	1) read a polygon mesh that will be automatically converted in a triangle mesh tagging
	  the internal edges (i.e. the edges that have been added for triangulating the polygons)
    2) make some cleaning
	3) import the tagged triangle mesh in a polygon mesh
	*/
	vcg::tri::io::ImporterOBJ<CMesh>::Open(mesh,argv[1],loadmask);

	vcg::tri::Clean<CMesh>::RemoveUnreferencedVertex(mesh);
	vcg::tri::Clean<CMesh>::RemoveZeroAreaFace(mesh);
	vcg::tri::UpdateTopology<CMesh>::FaceFace(mesh);
	vcg::tri::Clean<CMesh>::RemoveNonManifoldFace(mesh);
	vcg::tri::UpdateTopology<CMesh>::FaceFace(mesh);
	assert(vcg::tri::Clean<CMesh>::IsTwoManifoldFace(mesh));
	
	// create a polygon meshe from a trimesh with tagged faces
	vcg::tri::PolygonSupport<CMesh,MyPolyMesh>::ImportFromTriMesh(pm,mesh);
}
else
{
	/* second way: 
	Load into a polygon mesh straight away.
	*/
	vcg::tri::io::ImporterOBJ<MyPolyMesh>::Open(pm,argv[1],loadmask);
	vcg::tri::UpdateTopology<MyPolyMesh>::FaceFace(pm);
	vcg::tri::Clean<MyPolyMesh>::RemoveNonManifoldFace(pm);
	vcg::tri::UpdateTopology<MyPolyMesh>::FaceFace(pm);
	assert(vcg::tri::Clean<MyPolyMesh>::IsTwoManifoldFace(pm));

}


	// compute the half edges because I'm a half-edge programmer
	vcg::tri::EdgeSupport<MyPolyMesh>::ComputeHalfEdgeFromIndexed(pm);

	// .... my half edge based code ......

	// check for consistency
	assert(vcg::tri::EdgeSupport<MyPolyMesh>::CheckConsistency(pm));

	int size =  pm.face.size();

	// add a face to each face with more than 3 vertices ( just one pass)
	for(int i = 0; i < size; ++i)
		if(!(pm.face[i].IsD()))
		if(pm.face[i].VN()>3){
 			MyPolyMesh::EdgePointer ef =  pm.face[i].FHEp();
			MyPolyMesh::EdgePointer ef1 = ef -> HENp();
			ef1 = ef1->HENp();
			vcg::tri::EdgeSupport<MyPolyMesh>::AddEdge(pm, ef, ef1 );
 		}
	assert(vcg::tri::EdgeSupport<MyPolyMesh>::CheckConsistency(pm));
	size =  pm.face.size();

	// remove an edge for each face
	for(int i = 0; i < size; ++i)
		if(!(pm.face[i].IsD() ))
		{
 			MyPolyMesh::EdgePointer ef =  pm.face[i].FHEp();
			if( ef->HEOp()->EFp() !=NULL){
				vcg::tri::EdgeSupport<MyPolyMesh>::RemoveEdge(pm,ef);
			}
 		}

	// check for consistency
	assert(vcg::tri::EdgeSupport<MyPolyMesh>::CheckConsistency(pm));

	// recompute indexed data structure from the half edge data structure
	vcg::tri::EdgeSupport<MyPolyMesh>::ComputeIndexedFromHalfEdge(pm );
 
	// create a triangle mesh from a polygon mesh
  	vcg::tri::PolygonSupport<CMesh,MyPolyMesh>::ImportFromPolyMesh(mesh1,pm);

	// write out the triangle mesh
	vcg::tri::io::ExporterPLY<CMesh>::Save(mesh1,"converted_out.ply",true,pi);
}


