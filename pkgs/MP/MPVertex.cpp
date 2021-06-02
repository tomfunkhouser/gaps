////////////////////////////////////////////////////////////////////////
// Source file for MP image
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "MP.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {




////////////////////////////////////////////////////////////////////////
// Member functions
////////////////////////////////////////////////////////////////////////

MPVertex::
MPVertex(void)
  : house(NULL),
    house_index(-1),
    surface(NULL),
    surface_index(-1),
    position(0,0,0),
    normal(0,0,0),
    label(NULL)
{
}



MPVertex::
~MPVertex(void)
{
  // Remove from surface and house
  if (surface) surface->RemoveVertex(this);
  if (house) house->RemoveVertex(this);

  // Delete label
  if (label) free(label);
}



void MPVertex::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_SURFACE) && surface)
    LoadColor(surface->label);
  else if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_REGION) && surface && surface->region)
    LoadColor(surface->region->label);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_SURFACE) && surface)
    LoadColor(surface->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_REGION) && surface && surface->region)
    LoadColor(surface->region->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL) && surface && surface->region && surface->region->level)
    LoadColor(surface->region->level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_VERTEX_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_VERTICES] && draw_flags[MP_DRAW_DEPICTIONS]) DrawPosition(draw_flags);
}



void MPVertex::
DrawPosition(RNFlags draw_flags) const
{
  // Draw sphere at position
  if (draw_flags & MP_COLOR_FOR_PICK) glDisable(GL_LIGHTING);
  else glEnable(GL_LIGHTING);
  R3Sphere(position, 0.1).Draw();
}


  

// End namespace
};
