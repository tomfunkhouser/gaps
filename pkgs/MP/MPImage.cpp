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

MPImage::
MPImage(void)
  : house(NULL),
    house_index(-1),
    panorama(NULL),
    panorama_index(-1),
    name(NULL),
    camera_index(-1),
    yaw_index(-1),
    rgbd(),
    extrinsics(1,0,0,0,0,1,0,0, 0,0,1,0,0,0,0,1),
    intrinsics(1,0,0,0,1,0,0,0,1),
    width(0), height(0),
    position(0,0,0)
{
}



MPImage::
~MPImage(void)
{
  // Remove from panorama and house
  if (panorama) panorama->RemoveImage(this);
  if (house) house->RemoveImage(this);

  // Delete names
  if (name) free(name);
}



void MPImage::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_LABEL) && panorama && panorama->region)
    LoadColor(panorama->region->label);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_IMAGE))
    LoadColor(house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_PANORAMA) && panorama)
    LoadColor(panorama->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_REGION) && panorama->region)
    LoadColor(panorama->region->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL) && panorama && panorama->region && panorama->region->level)
    LoadColor(panorama->region->level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_IMAGE_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_IMAGES] && draw_flags[MP_DRAW_DEPICTIONS]) DrawCamera(draw_flags);
  if (draw_flags[MP_SHOW_IMAGES] && draw_flags[MP_DRAW_BBOXES]) DrawBBox(draw_flags);
  if (draw_flags[MP_SHOW_IMAGES] && draw_flags[MP_DRAW_FACES]) DrawQuads(draw_flags);
  if (draw_flags[MP_SHOW_IMAGES] && draw_flags[MP_DRAW_VERTICES]) DrawPoints(draw_flags);
  if (draw_flags[MP_SHOW_IMAGES] && draw_flags[MP_DRAW_IMAGES]) DrawImage(draw_flags);
}



void MPImage::
DrawCamera(RNFlags draw_flags) const
{
#if 0
  // Determine camera parameters in world coordinates
  R4Matrix camera_to_world = extrinsics.Inverse();
  R3Point eye = camera_to_world * R3zero_point;
  R3Vector towards = camera_to_world * R3negz_vector;
  R3Vector up = camera_to_world * R3posy_vector;

  // Draw vectors along view directions
  glDisable(GL_LIGHTING);
  R3Span(eye, eye+0.5*towards).Draw();
  R3Span(eye, eye+0.3*up).Draw();
#else
  rgbd.DrawCamera(0);
#endif
}



void MPImage::
DrawBBox(RNFlags draw_flags) const
{
  // Draw bounding box
  glDisable(GL_LIGHTING);
  rgbd.DrawBBox(0);
}



void MPImage::
DrawPoints(RNFlags draw_flags) const
{
  // Draw points
  glDisable(GL_LIGHTING);
  rgbd.DrawPoints(RGBD_PHOTO_COLOR_SCHEME);
}



void MPImage::
DrawQuads(RNFlags draw_flags) const
{
  // Draw points
  glEnable(GL_LIGHTING);
  rgbd.DrawQuads(RGBD_RENDER_COLOR_SCHEME);
}



void MPImage::
DrawImage(RNFlags draw_flags) const
{
  // Draw image
  rgbd.DrawImage(RGBD_PHOTO_COLOR_SCHEME, 0.25);
}



// End namespace
};
