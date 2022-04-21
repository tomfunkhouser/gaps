////////////////////////////////////////////////////////////////////////
// Source file for MP level
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

MPLevel::
MPLevel(void)
  : house(NULL),
    house_index(-1),
    regions(),
    position(0,0,0),
    bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX),
    label(NULL)  
{
}



MPLevel::
~MPLevel(void)
{
  // Remove regions
  while (regions.NEntries() > 0) RemoveRegion(regions.Tail());

  // Remove from house
  if (house) house->RemoveLevel(this);

  // Delete label
  if (label) free(label);
}



void MPLevel::
InsertRegion(MPRegion *region)
{
  // Insert region
  region->level = this;
  region->level_index = regions.NEntries();
  regions.Insert(region);

  // Update bounding box
  bbox.Union(region->bbox);
}



void MPLevel::
RemoveRegion(MPRegion *region)
{
  // Remove region
  MPRegion *tail = regions.Tail();
  tail->level_index = region->level_index;
  regions[region->level_index] = tail;
  regions.RemoveTail();
  region->level = NULL;
  region->level_index = -1;
}



void MPLevel::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL))
    LoadColor(house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_LEVEL_TAG);
  else RNLoadRgb(0.1, 0.5, 0.9);

  // Draw this
  if (draw_flags[MP_SHOW_LEVELS] && draw_flags[MP_DRAW_DEPICTIONS])
    DrawPosition(draw_flags);

  // Draw contents
  // DrawRegions(draw_flags);
}



void MPLevel::
DrawPosition(RNFlags draw_flags) const
{
  // Draw a sphere at position
  if (draw_flags[MP_DRAW_DEPICTIONS]) {
    if (draw_flags & MP_COLOR_FOR_PICK) glDisable(GL_LIGHTING);
    else glEnable(GL_LIGHTING);
    R3Sphere(position, 0.2).Draw(R3_SURFACES_DRAW_FLAG);
  }
}


 
void MPLevel::
DrawBBox(RNFlags draw_flags) const
{
  // Draw bounding box
  if (draw_flags & MP_DRAW_BBOXES) {
    glDisable(GL_LIGHTING);
    bbox.Outline();
  }
}



void MPLevel::
DrawRegions(RNFlags draw_flags) const
{
  // Draw regions
  for (int i = 0; i < regions.NEntries(); i++) {
    MPRegion *region = regions.Kth(i);
    region->Draw(draw_flags);
  }
}



// End namespace
};
