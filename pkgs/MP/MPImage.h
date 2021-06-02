////////////////////////////////////////////////////////////////////////
// Include file for MP image
////////////////////////////////////////////////////////////////////////
#ifndef __MP__IMAGE__H__
#define __MP__IMAGE__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPImage {
  // Constructor/destructor
  MPImage(void);
  ~MPImage(void);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawCamera(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawBBox(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawImage(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPoints(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawQuads(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

public:
  struct MPHouse *house;
  int house_index;
  struct MPPanorama *panorama;
  int panorama_index;
  char *name;
  int camera_index;
  int yaw_index;
  RGBDImage rgbd;
  R4Matrix extrinsics;
  R3Matrix intrinsics;
  int width, height;
  R3Point position;
};



// End namespace
}; 



// End include guard
#endif
