/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2006                                                \/)\/    *
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
Revision 1.5  2007/05/29 10:09:29  cignoni
Added a const (and reformatted)

Revision 1.4  2007/05/21 13:22:40  cignoni
Corrected gcc compiling issues

Revision 1.3  2006/02/16 15:16:51  corsini
Add reference plane support

Revision 1.2  2006/02/15 15:40:06  corsini
Decouple SVG properties and exporter for simmetry with the other exporter

Revision 1.1  2006/02/13 16:18:09  corsini
first working version


****************************************************************************/
#ifndef __VCG_LIB_EXPORTER_SVG
#define __VCG_LIB_EXPORTER_SVG

namespace vcg 
{
	namespace edge 
	{
		namespace io 
		{

static const char * DEFAULT_LINE_COLOR = "black";
static const char * DEFAULT_LINE_CAP= "round";
/**
 * SVG Properties.
 *
 * Support class to set the properties of the SVG exporter.
 */
class SVGProperties
{
	// definitions
public:

	//! Stroke colors.
	enum StrokeColor 
	{
		BLACK, 		SILVER,		GRAY,		WHITE,
		MAROON,		RED,		PURPLE,		FUCHSIA,
		GREEN,		LIME,		OLIVE,		YELLOW,
		NAVY,		BLUE,		TEAL,		AQUA
	};

	//! Stroke linecap types.
	enum StrokeLineCap
	{
		BUTT,		ROUND,		SQUARE
	};

	static const int DEFAULT_LINE_WIDTH=2;
	
	//in singlefile export make the grid of sessiones
	int numCol;
	int numRow;
	

// private data members
private:

	// Line width.
	int lwidth;

	// Stroke color (see StrokeColor).
	std::string stroke_color;

	// Stroke linecap (see StrokeLineCap).
	std::string stroke_linecap;

	// Plane where to project the edge mesh.
	Plane3d proj;

	// Scale of rdrawing respect coordinates of mesh  
	float scale;
    
	// Dimension of the drawing square
    Point2d ViewBox;

	int width,height; //Express in cm

	// Position of the drawing square
	Point2d position;
    
	

// construction
public:

	SVGProperties()
	{
		lwidth = DEFAULT_LINE_WIDTH;
		stroke_color = DEFAULT_LINE_COLOR;
		stroke_linecap = DEFAULT_LINE_CAP;

		// default projection plane (XZ plane)
		Point3d n(0.0, 1.0, 0.0);
		proj.SetDirection(n);
		proj.SetOffset(0.0);

		scale=0; //scale=0 it means expanded to the boards of the canvas   
        ViewBox=Point2d(1000, 1000); 
		position=Point2d(0, 0);
		width=10; //width of the windows
		height=10; //height of the windows
	
	}

// public methods
public:

	//! Set the line width.
	void setLineWidth(int width)
	{
		lwidth = width;
	}

	//! Set the stroke color.
	void setColor(enum StrokeColor color)
	{
	  switch (color)
		{
			case BLACK  : stroke_color = "black"; break;
			case SILVER : stroke_color = "silver"; break;
			case GRAY   : stroke_color = "gray"; break;
			case WHITE  : stroke_color = "white"; break;
			case MAROON : stroke_color = "maroon"; break;
			case RED    : stroke_color = "red"; break;
			case PURPLE : stroke_color = "purple"; break;
			case FUCHSIA: stroke_color = "fuchsia"; break;
			case GREEN  : stroke_color = "green"; break;
			case OLIVE  : stroke_color = "olive"; break;
			case LIME   : stroke_color = "lime"; break;
			case NAVY   : stroke_color = "navy"; break;
			case TEAL   : stroke_color = "teal"; break;
			case AQUA   : stroke_color = "aqua"; break;
			default: assert(0);
		}
	}

	//! Set the line cap style.
	void setLineCap(enum StrokeLineCap linecap)
	{
		if (linecap == BUTT)						stroke_linecap = "butt";
		else if (linecap == ROUND)			stroke_linecap = "round";
		else if (linecap == SQUARE)			stroke_linecap = "square";
	}

	void setPlane(double distance, const Point3d &direction)
	{
		proj.SetDirection(direction);
		proj.SetOffset(distance);
	}

	void setScale(float x){ scale=x; } //Define the scale between 2d coordinate and mesh

	void setViewBox(Point2d x) { ViewBox=x; }//Define the dimension of the square

	void setPosition(Point2d x) { position=x;}//Define the starting position of the canvas
    
	void setDimension(int width, int height){ this->width=width; this->height=height;}

// accessors
public:

	int lineWidth(){return lwidth;}
	const char * lineColor(){return stroke_color.c_str();}
	const char * lineCapStyle(){return stroke_linecap.c_str();}
	const Plane3d * projPlane(){return &proj;}
	float getScale(){return scale;}
	Point2d getViewBox(){return ViewBox;}
	Point2d getPosition(){return position;}
	int getWidth(){return width;}
	int getHeight(){return height;}

};
	    //const int SVGProperties::DEFAULT_LINE_WIDTH = 2;
		//const char * SVGProperties::DEFAULT_LINE_COLOR = "black";
        //const char * SVGProperties::DEFAULT_LINE_CAP = "round";

/**
 * SVG exporter.
 *
 * This exporter save a mesh of EdgeMesh type in the SVG format.
 * Most of the features of the SVG format are not supported. 
 * The given EdgeMesh is saved as a set lines. The properties
 * of the SVG export can be set through the SVGProp class.
 */
template <class EdgeMeshType>
class ExporterSVG
{

public:

	//! Save with the default SVG properties.
	static bool Save(EdgeMeshType *mp, const char *filename)
	{
		SVGProperties properties;
		return Save(mp, filename, properties);
	}
	static bool Save(vector<EdgeMeshType*> *vp, const char *filename, SVGProperties & pro){
	    //Function that export a vector of EdgeMesh in an unic single SVG file. 
        FILE * o = fopen(filename,"w");  
	    if (o==NULL)
			return false;
		int num = (*vp).size(); //number of square to draw
		//pro.setDimension((math::Sqrt(num)*10), (math::Sqrt(num)*10));
		//pro.setViewBox((math::Sqrt(num)*1000));
		WriteXmlHead(o,pro.getWidth(),pro.getHeight(),pro.getViewBox(),Point2d(0,0));
		float scale= pro.getScale();
		vector<EdgeMeshType*>::iterator it;
        int i=0;
		
		for(it=(*vp).begin(); it!=(*vp).end(); it++){
            
			EdgeMeshType* ed;
		    ed=(*it);
            
			pro.setPosition(Point2d((pro.getViewBox().V(0)/pro.numCol)*i,40));
			
			int x=pro.getViewBox().V(0);
			int y=pro.getViewBox().V(1);
			
			fprintf(o, "<rect width= \" %d \" height= \" %d \" x=\"%d \" y=\" %d \" style= \" stroke-width:1; fill-opacity:0.0; stroke:rgb(0,0,0)\" /> \n",x/pro.numCol,y,  (x/pro.numCol)*i, 30);
			
			Save(ed,o,pro);


		    fprintf(o,"<text x= \" %d \" y= \"20 \" font-family= \"Verdana \" font-size= \"20 \" >",(x/pro.numCol)*i);
			fprintf(o,"Slice num:%d, </text>", i);
			i++;
		}
		fprintf(o, "</svg>");
		fclose(o);
		return true;
	}
		 
	static void WriteXmlHead(FILE *o,int width, int height, Point2d viewBox, Point2d position){
	 
	   int Vx=viewBox[0];
		int Vy=viewBox[1];
	  // initial xml tags
		fprintf(o, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
		fprintf(o, "<!-- Created with vcg library -->\n");
		fprintf(o, "<svg width=\"%d cm\" height=\"%d cm\" viewBox=\"0 0 %d %d \" \n",width, height, Vx, Vy+50);
		fprintf(o, "  xmlns:dc=\"http://purl.org/dc/elements/1.1/\" \n");
		fprintf(o, "  xmlns:cc=\"http://web.resource.org/cc/\" \n");
		fprintf(o, "  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" \n");
		fprintf(o, "  xmlns:svg=\"http://www.w3.org/2000/svg\" \n");
		fprintf(o, "  xmlns=\"http://www.w3.org/2000/svg\" \n");
		fprintf(o, "  id=\"svg2\"> \n");
		fprintf(o, "  <defs id=\"defs4\"/> \n");
		fprintf(o, "  <metadata id=\"metadata7\"> \n");
		fprintf(o, "    <rdf:RDF> \n");
		fprintf(o, "    <cc:Work rdf:about=\"\"> \n");
		fprintf(o, "    <dc:format>image/svg+xml</dc:format> \n");
		fprintf(o, "    <dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" /> \n");
		fprintf(o, "    </cc:Work> \n");
		fprintf(o, "    </rdf:RDF> \n");
		fprintf(o, "  </metadata> \n");
	}
	//! Save with the given SVG properties.
	static bool Save(EdgeMeshType *mp, const char *filename, SVGProperties & props )
	{
	  FILE * o = fopen(filename,"w");
	  if (o==NULL)
			return false;
	
	  WriteXmlHead(o, props.getWidth(),props.getHeight(), props.getViewBox(), props.getPosition());
			

		Save(mp, o, props);

		fprintf(o, "</svg>");

		// final xml tags

		fclose(o);
		return true;
	}

	 static void Save(EdgeMeshType *mp, FILE* o, SVGProperties & props)
	{
		// build vector basis (n, v1, v2)
		Point3d p1(0.0,0.0,0.0);
		Point3d p2(1.0,0.0,0.0);

		Point3d d = props.projPlane()->Direction() - p2;

		Point3d v1;
		if (d.Norm() < 0.00001)
			v1 = Point3d(0.0,0.0,1.0) - p1;
		else
			v1 = p2 - p1;

		v1.Normalize();
		Point3d v2 = v1 ^ props.projPlane()->Direction();

		std::vector<Point2f> pts;
	    pts.clear();
		Point2f pmin(100000000.0f,  100000000.0f);
		Point2f pmax(-100000000.0f, -100000000.0f);
       
		
		typename EdgeMeshType::EdgeIterator i;
		
		for (i = mp->edges.begin(); i != mp->edges.end(); ++i)
		{
			Point3<typename EdgeMeshType::ScalarType> p1;
		    Point3<typename EdgeMeshType::ScalarType> p2;
			
			p1 = (*i).V(0)->P();
			p2 = (*i).V(1)->P();

			Point3d p1d(p1[0], p1[1], p1[2]);
			Point3d p2d(p2[0], p2[1], p2[2]);

			// Project the line on the reference plane
			Point3d p1proj = props.projPlane()->Projection(p1d);
			Point3d p2proj = props.projPlane()->Projection(p2d);

			// Convert the 3D coordinates of the line to the uv coordinates of the plane
			Point2f pt1(static_cast<float>(p1proj * v1), static_cast<float>(p1proj * v2));
			Point2f pt2(static_cast<float>(p2proj * v1), static_cast<float>(p2proj * v2));

			pts.push_back(pt1);
			pts.push_back(pt2);

			pmin[0]=math::Min(math::Min(pt1[0],pmin[0]), pt2[0]);
			pmin[1]=math::Min(math::Min(pt1[1],pmin[1]), pt2[1]);
			pmax[0]=math::Max(math::Max(pt1[0],pmax[0]), pt2[0]);
			pmax[1]=math::Max(math::Max(pt1[1],pmax[1]), pt2[1]); 

		
		}
	
		
		float scale=props.getScale();

		// line settings
		 Point2d pos=props.getPosition();
		 fprintf(o,"<text x= \" %f \" y= \"50 \" font-family= \"Verdana \" font-size= \"20 \" >",pos[0]);
		 fprintf(o,"Scale 1: %f, </text>", scale);
		fprintf(o, "      <g stroke=\"%s\" stroke-linecap=\"%s\" > \n", 
			props.lineColor(), props.lineCapStyle());

		std::vector<Point2f>::iterator itPoints;
		for(itPoints = pts.begin(); itPoints != pts.end(); ++itPoints)
		{
			Point2f p1 = *itPoints;
			++itPoints;
			Point2f p2 = *itPoints;
			
			

			
			fprintf(o, "        <line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" \n", 
				pos.X()+((p1[0] - pmin[0]) * scale), pos.Y()+((p1[1] - pmin[1]) * scale),
				pos.X()+((p2[0] - pmin[0]) * scale), pos.Y()+((p2[1] - pmin[1]) * scale ));

			fprintf(o, "          stroke-width = \"%d\" ",props.lineWidth());
			fprintf(o, "/>\n");
		}

		fprintf(o, "  </g>\n");
	}

	
};


		};  // namespace io
	};  // namespace edge
};  // namespace vcg
#endif  // __VCG_LIB_EXPORTER_SVG
