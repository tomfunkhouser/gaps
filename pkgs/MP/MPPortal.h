////////////////////////////////////////////////////////////////////////
// Include file for MP portal
////////////////////////////////////////////////////////////////////////
#ifndef __MP__PORTAL__H__
#define __MP__PORTAL__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPPortal {
  // Constructor/destructor
  MPPortal(void);
  ~MPPortal(void);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawSpan(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawLabel(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  

public:
  struct MPHouse *house;
  int house_index;
  MPRegion *regions[2];
  int region_indices[2];
  R3Span span;
  char *label;
};

  

// End namespace
}; 



// End include guard
#endif
