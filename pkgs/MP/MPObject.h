////////////////////////////////////////////////////////////////////////
// Include file for MP object
////////////////////////////////////////////////////////////////////////
#ifndef __MP__OBJECT__H__
#define __MP__OBJECT__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPObject {
  // Constructor/destructor
  MPObject(void);
  ~MPObject(void);

  // Manipulation stuff
  void InsertSegment(MPSegment *segment);
  void RemoveSegment(MPSegment *segment);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawBBox(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawLabel(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawSegments(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

public:
  struct MPHouse *house;
  int house_index;
  struct MPRegion *region;
  int region_index;
  struct MPCategory *category;
  int category_index;
  RNArray<MPSegment *> segments;
  R3Point position;
  R3OrientedBox obb;
};



// End namespace
}; 



// End include guard
#endif
