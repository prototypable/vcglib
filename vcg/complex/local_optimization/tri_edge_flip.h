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

#include <vcg/complex/local_optimization.h>
#include <vcg/simplex/face/topology.h>
//#include <vcg/space/point3.h>
#include <vcg/space/triangle3.h>

namespace vcg
{
namespace tri
{
/** \addtogroup trimesh */
/* @{ */

/*!
 *	This Class is specialization of LocalModification for the edge flip
 *	It wraps the atomic operation EdgeFlip to be used in a optimization routine.
 * Note that it has knowledge of the heap of the class LocalOptimization because
 * it is responsible of updating it after a flip has been performed
 * This is the simplest edge flipping class. 
 * It flips an edge only if two adjacent faces are coplanar and the 
 * quality of the faces improves after the flip.
 */
template <class TRIMESH_TYPE, class MYTYPE> class PlanarEdgeFlip :
	public LocalOptimization< TRIMESH_TYPE>::LocModType
{
protected:
	typedef typename TRIMESH_TYPE::FaceType FaceType;
	typedef typename TRIMESH_TYPE::FacePointer FacePointer;
	typedef typename TRIMESH_TYPE::FaceIterator FaceIterator;
	typedef typename TRIMESH_TYPE::VertexType VertexType;
	typedef typename TRIMESH_TYPE::ScalarType ScalarType;
	typedef typename TRIMESH_TYPE::VertexPointer VertexPointer;
	typedef typename TRIMESH_TYPE::CoordType CoordType;
	typedef vcg::face::Pos<FaceType> PosType;
	typedef typename LocalOptimization<TRIMESH_TYPE>::HeapElem HeapElem;
	typedef typename LocalOptimization<TRIMESH_TYPE>::HeapType HeapType;

	/*! 
	 *	the pos of the flipping
	 */
	PosType _pos;

	/*!
	 * priority in the heap
	 */
	ScalarType _priority;

	/*!
	 * Mark for updating
	 */
	int _localMark;

	/*!
	 *	mark for up_dating
	 */
	static int& GlobalMark()
	{
		static int im = 0;
		return im;
	}
	
public:
	/*!
	 *	Default constructor
	 */
	inline PlanarEdgeFlip()
	{
	}
	
	
	/*!
	 *	Constructor with <I>pos</I> type
	 */
	inline PlanarEdgeFlip(PosType pos, int mark)
	{
		_pos = pos;
		_localMark = mark;
		_priority = ComputePriority();
	}
	
	
	/*!
	 *	Copy Constructor 
	 */
	inline PlanarEdgeFlip(const PlanarEdgeFlip &par)
	{
		_pos = par.GetPos();
		_localMark = par.GetMark();
		_priority = par.Priority();
	}
	
	
	/*!
	 */
	~PlanarEdgeFlip()
	{
	}
	
	
	/*!
	 * Parameter 
	 */
	static ScalarType &CoplanarAngleThresholdDeg()
	{
		static ScalarType _CoplanarAngleThresholdDeg = 0.01f;
		return _CoplanarAngleThresholdDeg;
	}
	
	inline PosType GetPos()
	{
		return _pos;
	}
	
	inline int GetMark()
	{
		return _localMark;
	}
	
	
	/*!
	 *	Return the LocalOptimization type
	 */
	ModifierType IsOfType()
	{
		return TriEdgeFlipOp;
	}
	
	
	/*!
	 * Check if the pos is updated
	 */
	bool IsUpToDate()
	{
		int MostRecentVertexMark = _pos.F()->V(0)->IMark();
		MostRecentVertexMark = vcg::math::Max<int>(MostRecentVertexMark, _pos.F()->V(1)->IMark());
		MostRecentVertexMark = vcg::math::Max<int>(MostRecentVertexMark, _pos.F()->V(2)->IMark());

		return ( _localMark >= MostRecentVertexMark );
	}

	/*!
	 * 
	 Check if this flipping operation can be performed.
	 It is a topological and geometrical check. 
	 */
	virtual bool IsFeasible()
	{
		if( math::ToDeg( Angle(_pos.FFlip()->cN(), _pos.F()->cN()) ) > CoplanarAngleThresholdDeg() )
			return false;

		CoordType v0, v1, v2, v3;
		int i = _pos.I();
		/*v0 = _pos.F()->V0(i)->P();
		v1 = _pos.F()->V1(i)->P();
		v2 = _pos.F()->V2(i)->P();
		v3 = _pos.F()->FFp(i)->V2(_pos.F()->FFi(i))->P();*/
		
		v0 = _pos.F()->P0(i);
		v1 = _pos.F()->P1(i);
		v2 = _pos.F()->P2(i);
		v3 = _pos.F()->FFp(i)->P2(_pos.F()->FFi(i));

		// Take the parallelogram formed by the adjacent faces of edge
		// If a corner of the parallelogram on extreme of edge to flip is >= 180
		// the flip produce two identical faces - avoid this
		if( (Angle(v2 - v0, v1 - v0) + Angle(v3 - v0, v1 - v0) >= M_PI) ||
		    (Angle(v2 - v1, v0 - v1) + Angle(v3 - v1, v0 - v1) >= M_PI))
			return false;

		return vcg::face::CheckFlipEdge(*_pos.f, _pos.z);
	}

	/*!
	 *	Compute the priority of this optimization
	 */
	/*
	     1  
	    /|\
	   / | \
	  2  |  3 
	   \ | /
	    \|/
	     0
	 */
	virtual ScalarType ComputePriority()
	{
		CoordType v0, v1, v2, v3;
		int i = _pos.I();
		v0 = _pos.F()->P0(i);
		v1 = _pos.F()->P1(i);
		v2 = _pos.F()->P2(i);
		v3 = _pos.F()->FFp(i)->P2(_pos.F()->FFi(i));

		ScalarType Qa = Quality(v0,v1,v2);
		ScalarType Qb = Quality(v0,v3,v1);

		ScalarType QaAfter = Quality(v1,v2,v3);
		ScalarType QbAfter = Quality(v0,v3,v2);

		// higher the quality better the triangle.
		// swaps that improve the worst quality more are performed before 
		// (e.g. they have an higher priority)
		/*_priority = vcg::math::Max<ScalarType>(QaAfter,QbAfter) - vcg::math::Min<ScalarType>(Qa,Qb) ;
		 _priority *= -1;*/

		// < 0 if the average quality of faces improves after flip
		//_priority = ((Qa + Qb) / 2.0) - ((QaAfter + QbAfter) / 2.0);
		_priority = (Qa + Qb - QaAfter - QbAfter) / 2.0;

		return _priority;
	}

	/*!
	 * Return the priority of this optimization
	 */
	virtual ScalarType Priority() const
	{
		return _priority;
	}

	/*!
	 * Execute the flipping of the edge
	 */
	void Execute(TRIMESH_TYPE &/*m*/)
	{
		int z = _pos.z;
		vcg::face::FlipEdge(*_pos.f, z);
	}

	/*!
	 */
	const char* Info(TRIMESH_TYPE &m)
	{
		static char dump[60];
		sprintf(dump,"%ld -> %ld %g\n", _pos.F()->V(0)-&m.vert[0], _pos.F()->V(1)-&m.vert[0],-_priority);
		return dump;
	}

	/*!
	 */
	static void Init(TRIMESH_TYPE &mesh, HeapType &heap)
	{
		heap.clear();
		FaceIterator fi;
		for(fi = mesh.face.begin(); fi != mesh.face.end(); ++fi) {
			if(!(*fi).IsD() && (*fi).V(0)->IsW() && (*fi).V(1)->IsW() && (*fi).V(2)->IsW()) {
				for(unsigned int i = 0; i < 3; i++) {
					if( !(*fi).IsB(i) && (*fi).FFp(i)->V2((*fi).FFi(i))->IsW() ) {
						if((*fi).V1(i) - (*fi).V0(i) > 0)
							heap.push_back( HeapElem( new MYTYPE(PosType(&*fi, i), mesh.IMark() )) );
					} //endif
				} //endfor
			}
		} //endfor
	}

	/*!
	 */
	virtual void UpdateHeap(HeapType &heap)
	{
		GlobalMark()++;
		PosType pos(_pos.f, _pos.z);
		pos.FlipF();

		_pos.F()->V(0)->IMark() = GlobalMark();
		_pos.F()->V(1)->IMark() = GlobalMark();
		_pos.F()->V(2)->IMark() = GlobalMark();
		pos.F()->V2(_pos.F()->FFi(_pos.z))->IMark() = GlobalMark();

		PosType poss(_pos.f, _pos.z);

		poss.FlipV(); poss.FlipE();
		if(!poss.IsBorder())
			heap.push_back(HeapElem(new MYTYPE(poss, GlobalMark())));

		poss.FlipV(); poss.FlipE();
		if(!poss.IsBorder())
			heap.push_back(HeapElem(new MYTYPE(poss, GlobalMark())));

		poss.FlipV(); poss.FlipE();
		poss.FlipF(); poss.FlipE();
		if(!poss.IsBorder())
			heap.push_back(HeapElem(new MYTYPE(poss, GlobalMark())));

		poss.FlipV(); poss.FlipE();
		if(!poss.IsBorder())
			heap.push_back(HeapElem(new MYTYPE(poss, GlobalMark())));

		std::push_heap(heap.begin(),heap.end());
	}
}; // end of PlanarEdgeFlip class


template <class TRIMESH_TYPE, class MYTYPE>
class TriEdgeFlip : public PlanarEdgeFlip<TRIMESH_TYPE, MYTYPE>
{
protected:
	typedef typename TRIMESH_TYPE::FaceType FaceType;
	typedef typename TRIMESH_TYPE::FacePointer FacePointer;
	typedef typename TRIMESH_TYPE::FaceIterator FaceIterator;
	typedef typename TRIMESH_TYPE::VertexType VertexType;
	typedef typename TRIMESH_TYPE::VertexPointer VertexPointer;
	typedef typename TRIMESH_TYPE::ScalarType ScalarType;
	typedef typename TRIMESH_TYPE::CoordType CoordType;
	typedef vcg::face::Pos<FaceType> PosType;
	typedef typename LocalOptimization<TRIMESH_TYPE>::HeapElem HeapElem;
	typedef typename LocalOptimization<TRIMESH_TYPE>::HeapType HeapType;

	typedef typename vcg::Triangle3<ScalarType> TriangleType;

public:
	/*!
	 *	Default constructor
	 */
	inline TriEdgeFlip() {}

	/*!
	 *	Constructor with <I>pos</I> type
	 */
	inline TriEdgeFlip(const PosType pos, int mark)
	{
		this->_pos = pos;
		this->_localMark = mark;
		this->_priority = ComputePriority();
	}

	/*!
	 *	Copy Constructor 
	 */
	inline TriEdgeFlip(const TriEdgeFlip &par)
	{
		this->_pos = par.GetPos();
		this->_localMark = par.GetMark();
		this->_priority = par.Priority();
	}

	//only topology check 
	/*bool IsFeasible()
	 {
	 return vcg::face::CheckFlipEdge(*this->_pos.f, this->_pos.z);
	 }*/

	ScalarType ComputePriority()
	{
		/*
		     1  
		    /|\
		   / | \
		  2  |  3 
		   \ | /
		    \|/
		     0
		 */
		CoordType v0, v1, v2, v3;
		int i = this->_pos.I();
		v0 = this->_pos.F()->P0(i);
		v1 = this->_pos.F()->P1(i);
		v2 = this->_pos.F()->P2(i);
		v3 = this->_pos.F()->FFp(i)->P2(this->_pos.F()->FFi(i));

		//CoordType CircumCenter = vcg::Circumcenter(*(app.F()));
		CoordType circumcenter = vcg::Circumcenter(*(this->_pos.F()));

		ScalarType radius  = Distance(v0, circumcenter);
		ScalarType radius1 = Distance(v1, circumcenter);
		ScalarType radius2 = Distance(v2, circumcenter);

		assert( fabs(radius - radius1) < 0.1 );
		assert( fabs(radius - radius2) < 0.1 );

		///Return the difference of radius and the distance of v3 and the CircumCenter
		/*this->_priority =  (radius2 - Distance(v3, circumcenter));
		 this->_priority *= -1;*/

		this->_priority = (Distance(v3, circumcenter) - radius2);
		return this->_priority;
	}

};

/*! @} */
}; // end of namespace tri
}; // end of namespace vcg
