////////////////////////////////////////////////////////////////////////
// Include file for MP surface
////////////////////////////////////////////////////////////////////////
#ifndef __MP__SURFACE__H__
#define __MP__SURFACE__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPSurface {
  // Constructor/destructor
  MPSurface(void);
  ~MPSurface(void);

  // Manipulation stuff
  void InsertVertex(MPVertex *vertex, RNBoolean search_for_best_index = FALSE);
  void RemoveVertex(MPVertex *vertex);
  void FlipOrientation(void);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPolygon(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawVertices(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

  // Processing stuff
  void RecomputeBBox(void);

public:
  struct MPHouse *house;
  int house_index;
  struct MPRegion *region;
  int region_index;
  RNArray<MPVertex *> vertices;
  R3Point position;
  R3Vector normal;
  R3Box bbox;
  char *label;
};

  

// End namespace
}; 



// End include guard
#endif
