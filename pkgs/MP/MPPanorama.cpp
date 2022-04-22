////////////////////////////////////////////////////////////////////////
// Source file for MP panorama
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

MPPanorama::
MPPanorama(void)
  : house(NULL),
    house_index(-1),
    region(NULL),
    region_index(-1),
    name(NULL),
    images(),
    position(0,0,0)
{
}



MPPanorama::
~MPPanorama(void)
{
  // Remove all images
  while (!images.IsEmpty()) RemoveImage(images.Tail());
  
  // Remove from region and house
  if (region) region->RemovePanorama(this);
  if (house) house->RemovePanorama(this);
}



void MPPanorama::
InsertImage(MPImage *image)
{
  // Insert image
  image->panorama = this;
  image->panorama_index = images.NEntries();
  images.Insert(image);
}



void MPPanorama::
RemoveImage(MPImage *image)
{
  // Remove image
  MPImage *tail = images.Tail();
  tail->panorama_index = image->panorama_index;
  images[image->panorama_index] = tail;
  images.RemoveTail();
  image->panorama = NULL;
  image->panorama_index = -1;
}



void MPPanorama::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_LABEL) && region)
    LoadColor(region->label);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_PANORAMA))
    LoadColor(house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_REGION) && region)
    LoadColor(region->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL) && region && region->level)
    LoadColor(region->level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_PANORAMA_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_PANORAMAS] && draw_flags[MP_DRAW_DEPICTIONS]) DrawPosition(draw_flags);
  if (draw_flags[MP_SHOW_PANORAMAS] && draw_flags[MP_DRAW_LABELS]) DrawName(draw_flags);

  // Draw contents
  // DrawImages(draw_flags);
}



void MPPanorama::
DrawPosition(RNFlags draw_flags) const
{
  // Draw sphere at position
  if (draw_flags & MP_COLOR_FOR_PICK) glDisable(GL_LIGHTING);
  else glEnable(GL_LIGHTING);
  R3Sphere(position, 0.2).Draw();
}


  
void MPPanorama::
DrawName(RNFlags draw_flags) const
{
  // Draw name
  if (!name) return;
  glDisable(GL_LIGHTING);
  RNLoadRgb(1,1,1);
  R3DrawText(position + 0.25 * R3posz_vector, name);
}


  
void MPPanorama::
DrawImages(RNFlags draw_flags) const
{
  // Draw all images
  for (int i = 0; i < images.NEntries(); i++) {
    MPImage *image = images.Kth(i);
    image->Draw(draw_flags);
  }
}



// End namespace
};
