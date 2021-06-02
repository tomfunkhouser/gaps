////////////////////////////////////////////////////////////////////////
// Include file for MP segment
////////////////////////////////////////////////////////////////////////
#ifndef __MP__SEGMENT__H__
#define __MP__SEGMENT__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPSegment {
  // Constructor/destructor
  MPSegment(void);
  ~MPSegment(void);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawMesh(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawBBox(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

public:
  struct MPHouse *house;
  int house_index;
  struct MPObject *object;
  int object_index;
  R3Mesh *mesh;
  RNArray<R3MeshFace *> faces;
  RNArea area;
  R3Point position;
  R3Box bbox;
  int id;
};



// End namespace
}; 



// End include guard
#endif
