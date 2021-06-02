////////////////////////////////////////////////////////////////////////
// Include file for MP house
////////////////////////////////////////////////////////////////////////
#ifndef __MP__HOUSE__H__
#define __MP__HOUSE__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPHouse {
  // Constructor/destructor
  MPHouse(const char *name = NULL, const char *label = NULL);
  ~MPHouse(void);

  // Manipulation stuff
  void InsertImage(MPImage *image);
  void RemoveImage(MPImage *image);
  void InsertPanorama(MPPanorama *panorama);
  void RemovePanorama(MPPanorama *panorama);
  void InsertCategory(MPCategory *category);
  void RemoveCategory(MPCategory *category);
  void InsertSegment(MPSegment *segment);
  void RemoveSegment(MPSegment *segment);
  void InsertObject(MPObject *object);
  void RemoveObject(MPObject *object);
  void InsertVertex(MPVertex *vertex);
  void RemoveVertex(MPVertex *vertex);
  void InsertSurface(MPSurface *surface);
  void RemoveSurface(MPSurface *surface);
  void InsertRegion(MPRegion *region);
  void RemoveRegion(MPRegion *region);
  void InsertPortal(MPPortal *portal);
  void RemovePortal(MPPortal *portal);
  void InsertLevel(MPLevel *level);
  void RemoveLevel(MPLevel *level);

  // Input/output stuff
  int ReadFile(const char *filename);
  int ReadAsciiFile(const char *filename);
  int WriteFile(const char *filename) const;
  int WriteAsciiFile(const char *filename) const;

  // Other input stuff
  int ReadMeshFile(const char *filename);
  int ReadSceneFile(const char *filename);
  int ReadSegmentFile(const char *filename);
  int ReadObjectFile(const char *filename);
  int ReadCategoryFile(const char *filename);
  int ReadConfigurationFile(const char *filename);

  // Drawing stuff
  void Draw(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawLevels(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPortals(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawRegions(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawSurfaces(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawVertices(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawObjects(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawSegments(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawPanoramas(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawImages(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawScene(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;
  void DrawMesh(RNFlags draw_flags = MP_DEFAULT_DRAW_FLAGS) const;

public:
  // Utility stuff
  MPRegion *FindRegion(const R3Point& position, RNLength max_distance = 1) const;
  MPLevel *FindLevel(const R3Point& position, RNLength max_dz = 1.0) const;
  MPCategory *FindCategory(const char *label_name) const;
  MPCategory *FindCategory(int label_id) const;
  MPSegment *FindSegment(int segment_id) const;
  
  // Geometric query stuff
  MPImage *FindClosestImage(const R3Point& query_position, const R3Vector& query_normal = R3zero_vector,
    RNLength max_distance = FLT_MAX, RNBoolean check_normal = FALSE, RNBoolean check_visibility = FALSE) const;
  MPVertex *FindClosestVertex(const R3Point& query_position, const R3Vector& query_normal = R3zero_vector,
    RNLength max_distance = FLT_MAX, RNBoolean check_normal = FALSE, RNBoolean check_visibility = FALSE) const;
  MPRegion *FindClosestRegion(const R3Point& query_position, const R3Vector& query_normal = R3zero_vector, 
    RNLength max_distance = FLT_MAX, RNBoolean check_normal = FALSE, RNBoolean check_visibility = FALSE) const;

public:
  RNArray<MPImage *> images;
  RNArray<MPPanorama *> panoramas;
  RNArray<MPCategory *> categories;
  RNArray<MPSegment *> segments;
  RNArray<MPObject *> objects;
  RNArray<MPVertex *> vertices;
  RNArray<MPSurface *> surfaces;
  RNArray<MPRegion *> regions;
  RNArray<MPPortal *> portals;
  RNArray<MPLevel *> levels;
  RGBDConfiguration rgbd;
  R3Scene *scene;
  R3Mesh *mesh;
  R3Box bbox;
  char *label;
  char *name;
};



// End namespace
}; 



// End include guard
#endif
