// Include file for R3 view frustum class
#ifndef __R3__FRUSTUM__H__
#define __R3__FRUSTUM__H__



/* Begin namespace */
namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct R3Frustum : public R3Solid {
public:
  // Constructor
  R3Frustum(void);
  R3Frustum(const R3Point& viewpoint,
    const R3Vector& towards, const R3Vector& up,
    RNAngle xfov, RNAngle yfov, 
    RNLength neardist, RNLength fardist);

  // Property functions
  const R3Halfspace& Halfspace(int dir, int dim) const;
  const R3CoordSystem& CoordSystem(void) const;
  const R3Point Corner(RNOctant octant) const;
  const R3Point Corner(int xdir, int ydir, int zdir) const;
  const RNAngle XFov(void) const; 
  const RNAngle YFov(void) const; 
  const RNLength Near(void) const; 
  const RNLength Far(void) const; 
  const RNBoolean IsEmpty(void) const;

  // Shape propetry functions/operators
  virtual const RNBoolean IsPoint(void) const;
  virtual const RNBoolean IsLinear(void) const ;
  virtual const RNBoolean IsPlanar(void) const ;
  virtual const RNBoolean IsConvex(void) const ;
  virtual const RNInterval NFacets(void) const;
  virtual const RNArea Area(void) const;
  virtual const RNVolume Volume(void) const;
  virtual const R3Point Centroid(void) const;
  virtual const R3Point ClosestPoint(const R3Point& point) const;
  virtual const R3Point FurthestPoint(const R3Point& point) const;
  virtual const R3Shape& BShape(void) const;
  virtual const R3Box BBox(void) const;
  virtual const R3Sphere BSphere(void) const;

  // Manipulation functions
  virtual void Empty(void);
  virtual void Transform(const R3Transformation& transformation);
  virtual void SetHalfspace(int dir, int dim, const R3Halfspace& halfspace);

  // Draw function
  virtual void Draw(const R3DrawFlags draw_flags = R3_DEFAULT_DRAW_FLAGS) const;
  virtual void Outline(const R3DrawFlags draw_flags = R3_DEFAULT_DRAW_FLAGS) const;

  // Standard shape definitions
  RN_CLASS_TYPE_DECLARATIONS(R3Frustum);
  R3_SHAPE_RELATIONSHIP_DECLARATIONS(R3Frustum);

public:
  R3CoordSystem cs;
  R3Halfspace halfspaces[2][3];
  RNAngle xfov, yfov;
  RNLength neardist, fardist;
};



// Inline functions

inline const R3CoordSystem& R3Frustum::
CoordSystem(void) const
{
  // Return coordinate system for frustum
  // (apex at origin, -z points towards, +x points right, +y points up)
  return cs;
}



inline const RNAngle R3Frustum::
XFov(void) const
{
  // Return half angle between planes
  return xfov;
}



inline const RNAngle R3Frustum::
YFov(void) const
{
  // Return half angle between planes
  return yfov;
}



inline const RNLength R3Frustum::
Near(void) const
{
  // Return near distance  
  return neardist;
}



inline const RNLength R3Frustum::
Far(void) const
{
  // Return near distance  
  return fardist;
}



inline const RNBoolean R3Frustum::
IsEmpty(void) const
{
  // Return whether frustum is empty
  return (neardist >= fardist) ? TRUE : FALSE;
}



inline const R3Halfspace& R3Frustum::
Halfspace(int dir, int dim) const
{
  // Return halfspace
  return halfspaces[dir][dim];
}



inline void R3Frustum::
SetHalfspace(int dir, int dim, const R3Halfspace& halfspace)
{
  // Set halfspace
  this->halfspaces[dir][dim] = halfspace;
}



// End namespace
}


// End include guard
#endif
