////////////////////////////////////////////////////////////////////////
// Source file for MP portal
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

MPPortal::
MPPortal(void)
  : house(NULL),
    house_index(-1),
    span(R3Point(0,0,0), R3Point(0,0,0)),
    label(NULL)
{
  // Initialize regions
  regions[0] = regions[1] = NULL;
  region_indices[0] = region_indices[1] = -1;
}



MPPortal::
~MPPortal(void)
{
  // Remove from region and house
  if (regions[0]) regions[0]->RemovePortal(this);
  if (regions[1]) regions[1]->RemovePortal(this);
  if (house) house->RemovePortal(this);

  // Delete label
  if (label) free(label);
}



void MPPortal::
Draw(RNFlags draw_flags) const
{
  // Set the color
  MPRegion *region = regions[0];
  if ((draw_flags & MP_COLOR_BY_PORTAL) && (draw_flags & MP_COLOR_BY_LABEL))
    LoadColor(label);
  else if ((draw_flags & MP_COLOR_BY_PORTAL) && (draw_flags & MP_COLOR_BY_INDEX))
    LoadColor(house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_REGION) && (draw_flags & MP_COLOR_BY_INDEX) && region)
    LoadColor(region->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_REGION) && (draw_flags & MP_COLOR_BY_LABEL) && region)
    LoadColor(region->label);
  else if ((draw_flags & MP_COLOR_BY_LEVEL) && (draw_flags & MP_COLOR_BY_INDEX) && region && region->level)
    LoadColor(region->level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_PORTAL_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_PORTALS] && draw_flags[MP_DRAW_DEPICTIONS]) DrawSpan(draw_flags);
  if (draw_flags[MP_SHOW_PORTALS] && draw_flags[MP_DRAW_LABELS]) DrawLabel(draw_flags);
}



void MPPortal::
DrawSpan(RNFlags draw_flags) const
{
  // Draw span
  glDisable(GL_LIGHTING);
  span.Draw();

  // Draw spheres
  R3Sphere(span.Start(), 0.1).Draw();  
  R3Sphere(span.End(), 0.1).Draw();  
}



void MPPortal::
DrawLabel(RNFlags draw_flags) const
{
  // Draw label above midpoint
  if (!label) return;
  glDisable(GL_LIGHTING);
  RNLoadRgb(1, 1, 1);
  R3DrawText(span.Midpoint() + 0.5 * R3posz_vector, label);
}


  
// End namespace
};
