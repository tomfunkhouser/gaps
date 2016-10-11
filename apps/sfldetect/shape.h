/////////////////////////////////////////////////////////////////////////
// Include file for object shape class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectShape {
public:
  // Constructor destructor
  ObjectShape(void);
  ObjectShape(const R3Mesh *mesh, const R3CoordSystem *cs = NULL);
  ObjectShape(const R3Grid *grid, const R3CoordSystem *cs = NULL);
  ObjectShape(int npoints, const R3Point *points, const R3Vector *normals = NULL, const R3CoordSystem *cs = NULL);
  ~ObjectShape(void);

  // Point set properties
  int NPoints(void) const;
  const Point *Point(int k) const;

  // Coordinate system properties
  const R3Point& Origin(void) const;
  const R3Triad& Axes(void) const;
  const R3CoordSystem& CoordSystem(void) const;
  R3Affine ShapeToStandardTransformation(void) const;
  R3Affine StandardToShapeTransformation(void) const;

public:
  PointSet points;
  R3Grid voronoi_grid;
  R3Grid squared_distance_grid;
  R3CoordSystem cs;
  RNLength radius;
};



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline int ObjectShape::
NPoints(void) const
{
  // Return number of points
  return points.NPoints();
}



inline const Point *ObjectShape::
Point(int k) const
{
  // Return Kth point
  return points.Point(k);
}



inline const R3Point& ObjectShape::
Origin(void) const
{
  // Return coordinate system's origin
  return cs.Origin();
}



inline const R3Triad& ObjectShape::
Triad(void) const
{
  // Return coordinate system's triad
  return cs.Triad();
}



inline const R3CoordSystem& ObjectShape::
CoordSystem(void) const
{
  // Return coordinate system
  return cs;
}



inline R3Affine ObjectShape::
ShapeToStandardTransformation(void) const
{
  // Return transformation from shape's coordinate system to standard coordinate system
  return R3Affine(cs.InverseMatrix());
}



inline R3Affine ObjectShape::
StandardToShapeTransformation(void) const
{
  // Return transformation from standard coordinate system to shape's coordinate system 
  return R3Affine(cs.Matrix());
}


