////////////////////////////////////////////////////////////////////////
// Include file for MP region
////////////////////////////////////////////////////////////////////////
#ifndef __MP__REGION__H__
#define __MP__REGION__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPRegion {
  // Constructor/destructor
  MPRegion(void);
  ~MPRegion(void);

  // Property stuff
  R2Polygon FloorPolygon(void) const;

  // Manipulation stuff
  void InsertPanorama(MPPanorama *panorama);
  void RemovePanorama(MPPanorama *panorama);
  void InsertSurface(MPSurface *surface);
  void RemoveSurface(MPSurface *surface);
  void InsertObject(struct MPObject *object);
  void RemoveObject(struct MPObject *object);
  void InsertPortal(struct MPPortal *portal, int index);
  void RemovePortal(struct MPPortal *portal);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPosition(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawBBox(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawLabel(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPortals(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawObjects(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawSurfaces(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPanoramas(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

  // Processing stuff
  void RecomputeBBox(RNBoolean preserve_zmax = FALSE);

public:
  struct MPHouse *house;
  int house_index;
  struct MPLevel *level;
  int level_index;
  RNArray<MPPanorama *> panoramas;
  RNArray<MPSurface *> surfaces;
  RNArray<struct MPObject *> objects;
  RNArray<struct MPPortal *> portals;
  R3Point position;
  RNLength height;
  R3Box bbox;
  char *label;
};



// End namespace
}; 



// End include guard
#endif
