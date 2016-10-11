////////////////////////////////////////////////////////////////////////
// Source file for object shape class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////

ObjectShape::
ObjectShape(void)
  : points(),
    voronoi_grid(),
    squared_distance_grid(),
    cs(),
    radius(0)
{ 
}  

  

ObjectShape::
ObjectShape(const R3Mesh *mesh, const R3CoordSystem *crdsys)
  : points(),
    voronoi_grid(),
    squared_distance_grid(),
    cs(),
    radius(0)
{ 
  // Compute coordinate system
  if (crdsys) {
    cs = *crdsys;
  }
  else {
    R3Point origin = mesh->Centroid();
    RNScalar radius = 3 * mesh->AverageRadius(&origin);
    R3Triad principle_axes = mesh->PrincipleAxes(&origin);
    XXX
  }

  // Create points
  XXX

  // Create grids
  XXX
}  

  

ObjectShape::
~ObjectShape(void) 
{ 
  // Delete the points
  XXX
}
