////////////////////////////////////////////////////////////////////////
// Include file for MP panorama
////////////////////////////////////////////////////////////////////////
#ifndef __MP__PANORAMA__H__
#define __MP__PANORAMA__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPPanorama {
  // Constructor/destructor
  MPPanorama(void);
  ~MPPanorama(void);

  // Manipulation stuff
  void InsertImage(MPImage *image);
  void RemoveImage(MPImage *image);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPosition(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawName(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawImages(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

public:
  struct MPHouse *house;
  int house_index;
  struct MPRegion *region;
  int region_index;
  char *name;
  RNArray<MPImage *> images;
  R3Point position;
};



// End namespace
}; 



// End include guard
#endif
