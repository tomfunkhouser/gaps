////////////////////////////////////////////////////////////////////////
// Source file for object instance class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////

ObjectInstance::
ObjectInstance(const R3Point& origin, RNScalar radius, RNScalar height, const char *name)
  : pointset(),
    assignments(),
    parse(NULL),
    parse_index(-1),
    origin(origin),
    radius(radius),
    height(height),
    name((name) ? strdup(name) : NULL)
{ 
}  

  

ObjectInstance::
ObjectInstance(const ObjectPointSet& pset,
  const R3Point& origin, RNLength radius, RNLength height,
  const char *name)
  : pointset(pset),
    assignments(),
    parse(NULL),
    parse_index(-1),
    origin(0,0,0),
    radius(0),
    height(0),
    name((name) ? strdup(name) : NULL)
{ 
  // Update properties
  if (origin != R3unknown_point) SetOrigin(origin);
  if (radius > 0) SetRadius(radius);
  else SetRadius(pointset.Radius());
  if (height > 0) SetHeight(height);
  else SetHeight(pointset.Height());
}  



ObjectInstance::
~ObjectInstance(void) 
{ 
  // Delete name
  if (name) free(name); 

  // Delete assignments
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    delete assignment;
  }

  // Remove from parse
  if (parse) parse->RemoveInstance(this);
}
