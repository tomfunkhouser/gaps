////////////////////////////////////////////////////////////////////////
// Source file for MP object
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

MPObject::
MPObject(void)
  : house(NULL),
    house_index(-1),
    region(NULL),
    region_index(-1),
    category(NULL),
    segments(),
    position(0,0,0),
    obb(R3Point(0.0, 0.0, 0.0), R3Vector(1.0, 0.0, 0.0), R3Vector(0.0, 1.0, 0.0), -1.0, -1.0, -1.0)
{
}



MPObject::
~MPObject(void)
{
  // Remove all segments
  while (!segments.IsEmpty()) RemoveSegment(segments.Tail());
  
  // Remove from category, region and house
  if (category) category->RemoveObject(this);
  if (region) region->RemoveObject(this);
  if (house) house->RemoveObject(this);
}



void MPObject::
InsertSegment(MPSegment *segment)
{
  // Insert segment
  segment->object = this;
  segment->object_index = segments.NEntries();
  segments.Insert(segment);
}



void MPObject::
RemoveSegment(MPSegment *segment)
{
  // Remove segment
  MPSegment *tail = segments.Tail();
  tail->object_index = segment->object_index;
  segments[segment->object_index] = segments.Tail();
  segments.RemoveTail();
  segment->object = NULL;
  segment->object_index = -1;
}



void MPObject::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_OBJECT) && category)
    LoadColor(category->mpcat40_id);
  else if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_REGION) && region && region->label)
    LoadColor(region->label);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_OBJECT))
    LoadColor(house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_REGION) && region)
    LoadColor(region->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL) && region && region->level)
    LoadColor(region->level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_OBJECT_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_OBJECTS] && draw_flags[MP_DRAW_BBOXES]) DrawBBox(draw_flags);
  if (draw_flags[MP_SHOW_OBJECTS] && draw_flags[MP_DRAW_LABELS]) DrawLabel(draw_flags);

  // Draw contents
  DrawSegments(draw_flags & ~(MP_DRAW_BBOXES | MP_DRAW_LABELS));
}



void MPObject::
DrawBBox(RNFlags draw_flags) const
{
  // Draw the oriented box
  glDisable(GL_LIGHTING);
  obb.Outline();
}



void MPObject::
DrawLabel(RNFlags draw_flags) const
{
  // Draw sphere at position
  if (!category) return;
  if (!category->mpcat40_name) return;
  glDisable(GL_LIGHTING);
  RNLoadRgb(1,1,1);
  R3DrawText(position + 0.25 * R3posz_vector, category->mpcat40_name);
}


  
void MPObject::
DrawSegments(RNFlags draw_flags) const
{
  // Draw segments
  for (int i = 0; i < segments.NEntries(); i++) {
    MPSegment *segment = segments.Kth(i);
    segment->Draw(draw_flags);
  }
}



// End namespace
};
