////////////////////////////////////////////////////////////////////////
// Include file for MP level
////////////////////////////////////////////////////////////////////////
#ifndef __MP__LEVEL__H__
#define __MP__LEVEL__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPLevel {
  // Constructor/destructor
  MPLevel(void);
  ~MPLevel(void);

  // Manipulation stuff
  void InsertRegion(MPRegion *region);
  void RemoveRegion(MPRegion *region);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPosition(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawBBox(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawRegions(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

public:
  struct MPHouse *house;
  int house_index;
  RNArray<MPRegion *> regions;
  R3Point position;
  R3Box bbox;
  char *label;
};



// End namespace
}; 



// End include guard
#endif
