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
Revision 1.20  2006/12/18 09:46:39  callieri
camera+shot revamp: changed field names to something with more sense, cleaning of various functions, correction of minor bugs/incongruences, removal of the infamous reference in shot.

Revision 1.19  2006/01/22 17:01:40  cignoni
Corrected intialization of flag, must be zero.

Revision 1.18  2005/12/12 16:53:43  callieri
corrected UnProject, it's necessary also a ZDepth value to perform inverse projection

Revision 1.17  2005/12/07 10:57:52  callieri
added commodity function ProjectWorldtoViewport() to obtain directly pixel indices without calling two separate function of two different objects

Revision 1.16  2005/12/02 16:14:35  callieri
in Shot<S>::Axis changed Row3  to  GetRow3 . row3 was the old method name of Matrix44

Revision 1.15  2005/12/01 01:03:37  cignoni
Removed excess ';' from end of template functions, for gcc compiling

Revision 1.14  2005/11/23 14:18:35  ganovelli
added access to similarity (just for symmetry with Camera() )

Revision 1.13  2005/11/23 11:58:52  ganovelli
Empty constructor added, untemplated class Shotf and Shotd added
usage: Shotf myShot;
corrected member access rights

Revision 1.12  2005/07/11 13:12:35  cignoni
small gcc-related compiling issues (typenames,ending cr, initialization order)

Revision 1.11  2005/01/22 11:20:20  ponchio
<...Point3.h> -> <...point3.h>

Revision 1.10  2005/01/05 13:26:15  ganovelli
corretto cambiamento di sistema di rif.

Revision 1.9  2004/12/15 18:45:50  tommyfranken
*** empty log message ***

Revision 1.4  2004/10/07 14:41:31  fasano
Little fix on ViewPoint() method

Revision 1.3  2004/10/07 14:24:53  ganovelli
added LookAt,LookToward

Revision 1.2  2004/10/05 19:04:25  ganovelli
version 5-10-2004 in progress

Revision 1.1  2004/09/15 22:58:05  ganovelli
re-creation

Revision 1.2  2004/09/06 21:41:30  ganovelli
*** empty log message ***

Revision 1.1  2004/09/03 13:01:51  ganovelli
creation

****************************************************************************/


#ifndef __VCGLIB_SHOT
#define __VCGLIB_SHOT

#include <vcg/space/point2.h>
#include <vcg/space/point3.h>
#include <vcg/math/similarity.h>
#include <vcg/math/camera.h>

namespace vcg{

template <class S>
class Shot {
public:
	typedef Camera<S> CameraType;
	typedef S ScalarType;

	Camera<S>							Intrinsics;		// the camera that made the shot
	Similarity<S,vcg::Matrix44<S> >		Extrinsics;		// the position and orientation of the camera 


	Shot(Camera<S> c)
	{
		Intrinsics = c;
		Extrinsics.SetIdentity();
	}

	Shot()
	{
		Extrinsics.SetIdentity();
	}


	/// GET the i-th axis of the coordinate system of the camera
	vcg::Point3<S> Axis(const int & i)const;

	/// GET the viewpoint
	const vcg::Point3<S> GetViewPoint()const;
	/// SET the viewpoint
	void SetViewPoint(const vcg::Point3<S> & viewpoint);

	/// look at (dir+up)
	void LookAt(const vcg::Point3<S> & z_dir,const vcg::Point3<S> & up);

	/// look at (opengl-like)
	void LookAt(const S & eye_x,const S & eye_y,const S & eye_z,
				const S & at_x,const S & at_y,const S & at_z,
				const S & up_x,const S & up_y,const S & up_z);

	/// look towards
	void LookTowards(const vcg::Point3<S> & z_dir,const vcg::Point3<S> & up);

	/// convert a 3d point from world to camera coordinates
	vcg::Point3<S>  ConvertWorldToCameraCoordinates(const vcg::Point3<S> & p) const;

	/// convert a 3d point from camera to world coordinates
	vcg::Point3<S>  ConvertCameraToWorldCoordinates(const vcg::Point3<S> & p) const;

	/// project a 3d point from world coordinates to 2d camera viewport (pixel) 
	vcg::Point2<S> Project(const vcg::Point3<S> & p) const;

	/// inverse projection from 2d camera viewport (pixel) + Zdepth to 3d world coordinates
	vcg::Point3<S> UnProject(const vcg::Point2<S> & p, const S & d) const;

	/// returns distance of point p from camera plane (z depth), used for unprojection 
	S Depth(const vcg::Point3<S> & p)const;

}; // end class definition



//---
/// GET the viewpoint
template <class S>
const vcg::Point3<S> Shot<S>::GetViewPoint() const 
{
	return -Extrinsics.tra;
}
/// SET the viewpoint
template <class S>
void Shot<S>::SetViewPoint(const vcg::Point3<S> & viewpoint)
{
	Extrinsics.tra = -viewpoint;
}
//---

/// GET the i-th axis of the coordinate system of the camera
template <class S>
vcg::Point3<S>  Shot<S>::Axis(const int & i) const 
{	
	vcg::Matrix44<S> m; 
	Extrinsics.rot.ToMatrix(m); 
	vcg::Point3<S> aa = m.GetRow3(i);
	return aa;
}

/// look at (dir+up)
template <class S>
void Shot<S>::LookAt(const vcg::Point3<S> & z_dir,const vcg::Point3<S> & up)
{
	  LookTowards(z_dir-GetViewPoint(),up);
}

/// look at (opengl-like)
template <class S>
void Shot<S>::LookAt(const S & eye_x, const S & eye_y, const S & eye_z,
					 const S & at_x, const S & at_y, const S & at_z,
					 const S & up_x,const S & up_y,const S & up_z)
{
	SetViewPoint(Point3<S>(eye_x,eye_y,eye_z));
	LookAt(Point3<S>(at_x,at_y,at_z),Point3<S>(up_x,up_y,up_z));
}

/// look towards
template <class S>
void Shot<S>::LookTowards(const vcg::Point3<S> & z_dir,const vcg::Point3<S> & up)
{
	vcg::Point3<S> x_dir = up ^-z_dir;
	vcg::Point3<S> y_dir = -z_dir ^x_dir;
	
	Matrix44<S> m;
	m.SetIdentity();
	*(vcg::Point3<S> *)&m[0][0] =  x_dir/x_dir.Norm();
	*(vcg::Point3<S> *)&m[1][0] =  y_dir/y_dir.Norm();
	*(vcg::Point3<S> *)&m[2][0] = -z_dir/z_dir.Norm();

	Extrinsics.rot.FromMatrix(m);
}

//--- Space transformation methods

/// convert a 3d point from world to camera coordinates
template <class S>
vcg::Point3<S> Shot<S>::ConvertWorldToCameraCoordinates(const vcg::Point3<S> & p) const
{
	Matrix44<S> rotM;
	Extrinsics.rot.ToMatrix(rotM);
	vcg::Point3<S> cp = rotM * (p + Extrinsics.tra);
	cp[2]=-cp[2]; 	// note: camera reference system is right handed
	return cp;
	}

/// convert a 3d point from camera to world coordinates
template <class S>
vcg::Point3<S> Shot<S>::ConvertCameraToWorldCoordinates(const vcg::Point3<S> & p) const
{
	Matrix44<S> rotM;
	vcg::Point3<S> cp = p;
	cp[2]=-cp[2];	// note: World reference system is left handed
	Extrinsics.rot.ToMatrix(rotM);
	cp = Inverse(rotM) * cp-Extrinsics.tra;
	return cp;
}

/// project a 3d point from world coordinates to 2d camera viewport (pixel) 
template <class S>
vcg::Point2<S> Shot<S>::Project(const vcg::Point3<S> & p) const
{
	Point3<S> cp = ConvertWorldToCameraCoordinates(p);
	Point2<S> pp = Intrinsics.Project(cp);
	Point2<S> vp = Intrinsics.LocalToViewportPx(pp);
	return vp;
}

/// inverse projection from 2d camera viewport (pixel) + Zdepth to 3d world coordinates
template <class S>
vcg::Point3<S> Shot<S>::UnProject(const vcg::Point2<S> & p, const S & d) const
{
	Point2<S> lp = Intrinsics.ViewportPxToLocal(p);
	Point3<S> cp = Intrinsics.UnProject(lp,d);
	Point3<S> wp = ConvertCameraToWorldCoordinates(cp);
	return wp;
}

/// returns distance of point p from camera plane (z depth), used for unprojection 
template <class S>
S Shot<S>::Depth(const vcg::Point3<S> & p)const 
{
	return ConvertWorldToCameraCoordinates(p).Z();
}



//--------------------------------


//--- utility definitions
class Shotf: public Shot<float>{};
class Shotd: public Shot<double>{};
//-----------------------

} // end name space

#endif




