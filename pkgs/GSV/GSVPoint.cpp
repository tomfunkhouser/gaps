// Source file for GSV point class



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "GSV.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Constructor/destructor functions
////////////////////////////////////////////////////////////////////////

GSVPoint::
GSVPoint(void)
  : timestamp(0),
    point_identifier(-1),
    cluster_identifier(-1),
    elevation(-1),
    beam_index(-1),
    reflectivity(1),
    category_identifier(0),
    category_confidence(0),
    flags(0)
{
  // Set position
  this->position[0] = 0;
  this->position[1] = 0;
  this->position[2] = 0;
  
  // Set normal
  this->normal[0] = 0;
  this->normal[1] = 0;
  this->normal[2] = 0;

  // Set tangent
  this->tangent[0] = 0;
  this->tangent[1] = 0;
  this->tangent[2] = 0;

  // Set radius
  this->radius[0] = 0;
  this->radius[1] = 0;

  // Set color
  this->color[0] = 0;
  this->color[1] = 0;
  this->color[2] = 0;
}


  
} // namespace gaps
