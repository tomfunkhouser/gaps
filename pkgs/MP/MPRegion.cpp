////////////////////////////////////////////////////////////////////////
// Source file for MP region
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

MPRegion::
MPRegion(void)
  : house(NULL),
    house_index(-1),
    level(NULL),
    level_index(-1),
    panoramas(),
    surfaces(),
    objects(),
    portals(),
    position(0,0,0),
    height(0),
    bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX),
    label(NULL)
{
}



MPRegion::
~MPRegion(void)
{
  // Remove panoramas, surfaces, and portals
  while (panoramas.NEntries() > 0) RemovePanorama(panoramas.Tail());
  while (surfaces.NEntries() > 0) RemoveSurface(surfaces.Tail());
  while (objects.NEntries() > 0) RemoveObject(objects.Tail());
  while (portals.NEntries() > 0) RemovePortal(portals.Tail());

  // Remove from level and house
  if (level) level->RemoveRegion(this);
  if (house) house->RemoveRegion(this);

  // Delete label
  if (label) free(label);
}



R2Polygon MPRegion::
FloorPolygon(void) const
{
  // Create wall grid
  RNArray<R2Point *> points;
  for (int i = 0; i < surfaces.NEntries(); i++) {
    MPSurface *surface = surfaces.Kth(i);
    if (surface->normal.Z() < 0.9) continue;
    for (int j = 0; j < surface->vertices.NEntries(); j++) 
      points.Insert((R2Point *) &(surface->vertices[j]->position));
    return R2Polygon(points);
  }

  // If no surface found, return empty polygon
  return R2Polygon();
}



void MPRegion::
InsertPanorama(MPPanorama *panorama)
{
  // Insert panorama
  panorama->region = this;
  panorama->region_index = panoramas.NEntries();
  panoramas.Insert(panorama);
}



void MPRegion::
RemovePanorama(MPPanorama *panorama)
{
  // Remove panorama
  MPPanorama *tail = panoramas.Tail();
  tail->region_index = panorama->region_index;
  panoramas[panorama->region_index] = tail;
  panoramas.RemoveTail();
  panorama->region = NULL;
  panorama->region_index = -1;
}



void MPRegion::
InsertSurface(MPSurface *surface)
{
  // Insert surface
  surface->region = this;
  surface->region_index = surfaces.NEntries();
  surfaces.Insert(surface);

  // Update bounding box
  bbox.Union(surface->bbox);
}



void MPRegion::
RemoveSurface(MPSurface *surface)
{
  // Remove surface
  MPSurface *tail = surfaces.Tail();
  tail->region_index = surface->region_index;
  surfaces[surface->region_index] = tail;
  surfaces.RemoveTail();
  surface->region = NULL;
  surface->region_index = -1;
}



void MPRegion::
InsertObject(MPObject *object)
{
  // Insert object
  object->region = this;
  object->region_index = objects.NEntries();
  objects.Insert(object);
}



void MPRegion::
RemoveObject(MPObject *object)
{
  // Remove object
  MPObject *tail = objects.Tail();
  tail->region_index = object->region_index;
  objects[object->region_index] = tail;
  objects.RemoveTail();
  object->region = NULL;
  object->region_index = -1;
}



void MPRegion::
InsertPortal(MPPortal *portal, int index)
{
  // Insert portal
  portal->regions[index] = this;
  portal->region_indices[index] = portals.NEntries();
  portals.Insert(portal);
}



void MPRegion::
RemovePortal(MPPortal *portal)
{
  // Find index
  int index = -1;
  if (portal->regions[0] == this) index = 0;
  else if (portal->regions[1] == this) index = 1;
  else RNAbort("portal not in region");
  
  // Remove portal
  MPPortal *tail = portals.Tail();
  tail->region_indices[index] = portal->region_indices[index];
  portals[portal->region_indices[index]] = tail;
  portals.RemoveTail();
  portal->regions[index] = NULL;
  portal->region_indices[index] = -1;
}



void MPRegion::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_REGION))
    LoadColor(label);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_REGION))
    LoadColor(house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL) && level)
    LoadColor(level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_REGION_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_REGIONS] && draw_flags[MP_DRAW_DEPICTIONS])
    DrawPosition(draw_flags);
  if (draw_flags[MP_SHOW_REGIONS] && draw_flags[MP_DRAW_BBOXES])
    DrawBBox(draw_flags);
  if (draw_flags[MP_SHOW_REGIONS] && draw_flags[MP_DRAW_LABELS])
    DrawLabel(draw_flags);

  // Draw contents
  // DrawPanoramas();
  DrawSurfaces(draw_flags & ~(MP_DRAW_BBOXES | MP_DRAW_DEPICTIONS));
  // DrawObjects();

}



void MPRegion::
DrawPosition(RNFlags draw_flags) const
{
  // Draw a sphere at position
  if (draw_flags & MP_COLOR_FOR_PICK) glDisable(GL_LIGHTING);
  else glEnable(GL_LIGHTING);
  R3Sphere(position, 0.2).Draw(R3_SURFACES_DRAW_FLAG);
}


 
void MPRegion::
DrawBBox(RNFlags draw_flags) const
{
  // Draw bounding box
  glDisable(GL_LIGHTING);
  bbox.Outline();
}



void MPRegion::
DrawLabel(RNFlags draw_flags) const
{
  // Draw sphere at position
  if (!label) return;
  glDisable(GL_LIGHTING);
  RNLoadRgb(1,1,1);
  R3DrawText(position + 0.25 * R3posz_vector, label);
}


  
void MPRegion::
DrawPanoramas(RNFlags draw_flags) const
{
  // Draw panoramas
  for (int i = 0; i < panoramas.NEntries(); i++) {
    MPPanorama *panorama = panoramas.Kth(i);
    panorama->Draw(draw_flags);
  }
}



void MPRegion::
DrawSurfaces(RNFlags draw_flags) const
{
  // Draw surfaces
  for (int i = 0; i < surfaces.NEntries(); i++) {
    MPSurface *surface = surfaces.Kth(i);
    surface->Draw(draw_flags);
  }
}



void MPRegion::
DrawObjects(RNFlags draw_flags) const
{
  // Draw objects
  for (int i = 0; i < objects.NEntries(); i++) {
    MPObject *object = objects.Kth(i);
    object->Draw(draw_flags);
  }
}



void MPRegion::
DrawPortals(RNFlags draw_flags) const
{
  // Draw portals
  for (int i = 0; i < portals.NEntries(); i++) {
    MPPortal *portal = portals.Kth(i);
    portal->Draw(draw_flags);
  }
}



void MPRegion::
RecomputeBBox(RNBoolean preserve_zmax)
{
  // Remember height
  RNScalar zmax = bbox[1][2];

  // Update bbox
  bbox = R3null_box;
  for (int i = 0; i < surfaces.NEntries(); i++) {
    MPSurface *surface = surfaces.Kth(i);
    bbox.Union(surface->bbox);
  }
  
  // Restore zextent
  if (preserve_zmax) bbox[1][2] = zmax;
}

  

// End namespace
};
