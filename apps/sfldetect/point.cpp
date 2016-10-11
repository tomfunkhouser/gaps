////////////////////////////////////////////////////////////////////////
// Source file for object point class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////

ObjectPoint::
ObjectPoint(void)
  : position(0,0,0),
    normal(0,0,0),
    descriptor(),
    value(0)
{ 
  // Set rgba
  rgba[0] = 0;
  rgba[1] = 0;
  rgba[2] = 0;
  rgba[3] = 0;
}  

  

ObjectPoint::
ObjectPoint(const ObjectPoint& point)
  : position(point.position),
    normal(point.normal),
    descriptor(point.descriptor),
    value(point.value)
{ 
  // Set rgba
  rgba[0] = point.rgba[0];
  rgba[1] = point.rgba[1];
  rgba[2] = point.rgba[2];
  rgba[3] = point.rgba[3];
}  

  

ObjectPoint::
ObjectPoint(const R3Point& position)
  : position(position),
    normal(0,0,0),
    descriptor(),
    value(0)
{ 
  // Set color
  rgba[0] = 0;
  rgba[1] = 0;
  rgba[2] = 0;
  rgba[3] = 0;
}  

  

ObjectPoint::
ObjectPoint(const R3Point& position, const R3Vector& normal)
  : position(position),
    normal(normal),
    descriptor(),
    value(0)
{ 
  // Set color
  rgba[0] = 0;
  rgba[1] = 0;
  rgba[2] = 0;
  rgba[3] = 0;
}  

  

ObjectPoint::
ObjectPoint(const R3Point& position, const R3Vector& normal, const RNRgb& rgb)
  : position(position),
    normal(normal),
    descriptor(),
    value(0)
{ 
  // Set color
  SetColor(rgb);
}  

  

ObjectPoint::
ObjectPoint(const R3Point& position, const R3Vector& normal, const RNRgb& rgb, const ObjectDescriptor& descriptor)
  : position(position),
    normal(normal),
    descriptor(descriptor),
    value(0)
{ 
  // Set color
  SetColor(rgb);
}  

  

ObjectPoint& ObjectPoint::
operator=(const ObjectPoint& point)
{
  // Copy everything
  this->position = point.position;
  this->normal = point.normal;
  this->descriptor = point.descriptor;
  this->rgba[0] = point.rgba[0];
  this->rgba[1] = point.rgba[1];
  this->rgba[2] = point.rgba[2];
  this->rgba[3] = point.rgba[3];
  this->value = point.value;
  return *this;
}



