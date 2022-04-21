// Source file for view frustum class



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Shapes.h"



// Namespace

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class type definitions
////////////////////////////////////////////////////////////////////////

RN_CLASS_TYPE_DEFINITIONS(R3Frustum);



////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////

R3Frustum::
R3Frustum(void)
{
  // Empty
  Empty();
}



R3Frustum::
R3Frustum(const R3Point& viewpoint, const R3Vector& towards, const R3Vector& up, 
  RNAngle xfov, RNAngle yfov, RNLength neardist, RNLength fardist)
  : cs(viewpoint, R3Triad(towards, up)),
    xfov(xfov),
    yfov(yfov),
    neardist(neardist),
    fardist(fardist)
{
  // Compute halfspaces
  for (int dir = 0; dir < 2; dir++) {
    for (int dim = 0; dim < 3; dim++) {
      R3Vector normal;
      switch (dim) {
      case RN_X:
        normal = cs.Axes().Axis(RN_X); 
        if (dir == RN_LO) normal.Rotate(cs.Axes().Axis(RN_Y), xfov);
        else normal.Rotate(cs.Axes().Axis(RN_Y), RN_PI - xfov);
        halfspaces[dir][dim] = R3Halfspace(cs.Origin(), normal);
        break;

      case RN_Y:
        normal = cs.Axes().Axis(RN_Y); 
        if (dir == RN_LO) normal.Rotate(cs.Axes().Axis(RN_X), -yfov);
        else normal.Rotate(cs.Axes().Axis(RN_X), yfov - RN_PI);
        halfspaces[dir][dim] = R3Halfspace(cs.Origin(), normal);
        break;

      case RN_Z:
        if (dir == RN_LO) {
          halfspaces[dir][dim] = R3Halfspace(cs.Origin() - cs.Axes().Axis(RN_Z) * neardist, -cs.Axes().Axis(RN_Z));
        }
        else {
          halfspaces[dir][dim] = R3Halfspace(cs.Origin() - cs.Axes().Axis(RN_Z) * fardist, cs.Axes().Axis(RN_Z));
        }
        break;

      default:
        RNAbort("Should never get here");
        break;
      }
    }
  }
}



////////////////////////////////////////////////////////////////////////
// Frustum property functions
////////////////////////////////////////////////////////////////////////

const R3Point R3Frustum::
Corner (RNOctant octant) const
{
  // Return corner point 
  switch (octant) {
  case RN_NNN_OCTANT: return Corner(RN_LO, RN_LO, RN_LO);
  case RN_NNP_OCTANT: return Corner(RN_LO, RN_LO, RN_HI);
  case RN_NPN_OCTANT: return Corner(RN_LO, RN_HI, RN_LO);
  case RN_NPP_OCTANT: return Corner(RN_LO, RN_HI, RN_HI);
  case RN_PNN_OCTANT: return Corner(RN_HI, RN_LO, RN_LO);
  case RN_PNP_OCTANT: return Corner(RN_HI, RN_LO, RN_HI);
  case RN_PPN_OCTANT: return Corner(RN_HI, RN_HI, RN_LO);
  case RN_PPP_OCTANT: return Corner(RN_HI, RN_HI, RN_HI);
  default: RNAbort("Invalid octant for frustum corner"); return R3null_point;
  }
}



const R3Point R3Frustum::
Corner(int xdir, int ydir, int zdir) const
{
  // Return corner point
  double xsign = (xdir == RN_LO) ? -1 : 1;
  double ysign = (ydir == RN_LO) ? -1 : 1;
  double depth = (zdir == RN_LO) ? Near() : Far();
  R3Point corner = cs.Origin() - cs.Axes().Axis(RN_Z) * depth;
  corner += cs.Axes().Axis(RN_X) * depth * tan(xfov) * xsign;
  corner += cs.Axes().Axis(RN_Y) * depth * tan(yfov) * ysign;
  return corner;
}
  

  
////////////////////////////////////////////////////////////////////////
// Shape property functions
////////////////////////////////////////////////////////////////////////

const RNBoolean R3Frustum::
IsPoint(void) const
{
  return FALSE;
}



const RNBoolean R3Frustum::
IsLinear(void) const
{
  return FALSE;
}


 
const RNBoolean R3Frustum::
IsPlanar(void) const
{
  return FALSE;
}



const RNBoolean R3Frustum::
IsConvex(void) const 
{
  return TRUE;
}



const RNInterval R3Frustum::
NFacets(void) const
{
  return RNInterval(6,6);
}



const RNArea R3Frustum::
Area(void) const
{
  // Todo
  return 0;
}



const RNVolume R3Frustum::
Volume(void) const
{
  // Todo
  return 0;
}



const R3Point R3Frustum::
Centroid(void) const
{
  // Return centroid
  double middepth = 0.5 * (Near() + Far());
  return cs.Origin() - middepth * cs.Axes().Axis(RN_Z);
}



const R3Point R3Frustum::
ClosestPoint(const R3Point& point) const
{
  // Todo
  RNAbort("Not implemented");
  return R3unknown_point;
}



const R3Point R3Frustum::
FurthestPoint(const R3Point& point) const
{
  // Todo
  RNAbort("Not implemented");
  return R3unknown_point;
}



const R3Shape& R3Frustum::
BShape(void) const
{
  // Return self
  return *this;
}



const R3Box R3Frustum::
BBox(void) const
{
  // Return bounding box
  R3Box bbox = R3null_box;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        bbox.Union(Corner(i, j, k));
      }
    }
  }
  return bbox;
}




const R3Sphere R3Frustum::
BSphere(void) const
{
  // Return bounding sphere
  R3Point centroid = Centroid();
  R3Point corner = Corner(RN_HI, RN_HI, RN_HI);
  RNLength radius = R3Distance(centroid, corner);
  return R3Sphere(centroid, radius);
}


 
////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void R3Frustum::
Empty(void)
{
  // Reset coordinate system
  cs = R3xyz_coordinate_system;

  // Reset fovs
  xfov = 0;
  yfov = 0;

  // Reset near and far dists
  neardist = 0;
  fardist = 0;
  
  // Reset halfspaces
  for (int dir = 0; dir < 2; dir++) {
    for (int dim = 0; dim < 3; dim++) {
      halfspaces[dir][dim] = R3null_halfspace;
    }
  }
}



void R3Frustum::
Transform(const R3Transformation& transformation)
{
  // Assumes rigid transformation
  // xfov, yfov stay same

  // Transform coordinate system
  cs.Transform(transformation);

  // Transform halfspaces
  for (int dir = 0; dir < 2; dir++) {
    for (int dim = 0; dim < 3; dim++) {
      halfspaces[dir][dim].Transform(transformation);
    }
  }
}



void R3Frustum::
Draw(const R3DrawFlags draw_flags) const
{
  // Draw quads
  RNGrfxBegin(RN_GRFX_QUADS);
  R3LoadPoint(Corner(0,0,0));  // Front
  R3LoadPoint(Corner(1,0,0));
  R3LoadPoint(Corner(1,1,0));
  R3LoadPoint(Corner(0,1,0));
  R3LoadPoint(Corner(0,0,1));  // Back
  R3LoadPoint(Corner(0,1,1));
  R3LoadPoint(Corner(1,1,1));
  R3LoadPoint(Corner(1,0,1));
  R3LoadPoint(Corner(1,0,0)); // Right
  R3LoadPoint(Corner(1,0,1));
  R3LoadPoint(Corner(1,1,1));
  R3LoadPoint(Corner(1,1,0));
  R3LoadPoint(Corner(0,0,0));  // Left
  R3LoadPoint(Corner(0,0,1));
  R3LoadPoint(Corner(0,1,1));
  R3LoadPoint(Corner(0,1,0));
  R3LoadPoint(Corner(0,1,0));  // Top
  R3LoadPoint(Corner(1,1,0));
  R3LoadPoint(Corner(1,1,1));
  R3LoadPoint(Corner(0,1,1));
  R3LoadPoint(Corner(0,0,0));  // Bottom
  R3LoadPoint(Corner(0,0,1));
  R3LoadPoint(Corner(1,0,1));
  R3LoadPoint(Corner(1,0,0));
  RNGrfxEnd();
}



void R3Frustum::
Outline(const R3DrawFlags draw_flags) const
{
  // Draw wireframe
  R3BeginLine();
  R3LoadPoint(Corner(0,0,0));
  R3LoadPoint(Corner(1,0,0));
  R3LoadPoint(Corner(1,1,0));
  R3LoadPoint(Corner(0,1,0));
  R3LoadPoint(Corner(0,0,0));
  R3LoadPoint(Corner(0,0,1));
  R3LoadPoint(Corner(1,0,1));
  R3LoadPoint(Corner(1,1,1));
  R3LoadPoint(Corner(0,1,1));
  R3LoadPoint(Corner(0,0,1));
  R3LoadPoint(Corner(0,0,0));
  R3LoadPoint(Corner(1,0,0));
  R3LoadPoint(Corner(1,0,1));
  R3LoadPoint(Corner(1,1,1));
  R3LoadPoint(Corner(1,1,0));
  R3LoadPoint(Corner(0,1,0));
  R3LoadPoint(Corner(0,1,1));
  R3EndLine();
}



} // namespace gaps
