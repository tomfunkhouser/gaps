/* Include file for the R3 surfel viewer class */
#ifndef __R3__SURFEL__VIEWER__H__
#define __R3__SURFEL__VIEWER__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE 
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class R3SurfelViewer {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor function
  R3SurfelViewer(R3SurfelScene *scene);

  // Destructor function
  virtual ~R3SurfelViewer(void);


  ////////////////////////////////////
  //// UI EVENT HANDLER FUNCTIONS ////
  ////////////////////////////////////

  // Call these at beginning and end of execution
  virtual void Initialize(void);
  virtual void Terminate(void);

  // Call these to handle user input events
  virtual int Redraw(void);
  virtual int Resize(int width, int height);
  virtual int MouseMotion(int x, int y);
  virtual int MouseButton(int x, int y, int button, int state, int shift, int ctrl, int alt, int update_center_point = 1);
  virtual int Keyboard(int x, int y, int key, int shift, int ctrl, int alt);
  virtual int Idle(void);


  ////////////////////////////////
  //// SCENE ACCESS FUNCTIONS ////
  ////////////////////////////////

  // Scene properties
  R3SurfelScene *Scene(void) const;


  ///////////////////////////////////
  //// PROPERTY ACCESS FUNCTIONS ////
  ///////////////////////////////////

  // Viewing properties
  const R3Camera& Camera(void) const;
  const R2Viewport& Viewport(void) const;
  const R3Box& ViewingExtent(void) const;
  const RNInterval& ElevationRange(void) const;
  const R3Point& CenterPoint(void) const;
  RNScalar ImageInsetSize(void) const;
  RNLength ImagePlaneDepth(void) const;
  RNScalar SurfelSize(void) const;

  // Visibility properties (0=off, 1=on)
  int SurfelVisibility(void) const;
  int NormalVisibility(void) const;
  int BackfacingVisibility(void) const;
  int AerialVisibility(void) const;
  int TerrestrialVisibility(void) const;
  int HumanLabeledObjectVisibility(void) const;
  int ObjectPrincipalAxesVisibility(void) const;
  int ObjectOrientedBBoxVisibility(void) const;
  int ObjectRelationshipVisibility(void) const;
  int ObjectBBoxVisibility(void) const;
  int NodeBBoxVisibility(void) const;
  int BlockBBoxVisibility(void) const;
  int ScanViewpointVisibility(void) const;
  int ImageViewpointVisibility(void) const;
  int ImagePlaneVisibility(void) const;
  int ImageInsetVisibility(void) const;
  int ImagePointsVisibility(void) const;
  int CenterPointVisibility(void) const;
  int ViewingExtentVisibility(void) const;
  int AxesVisibility(void) const;
  int LabelVisibility(R3SurfelLabel *label) const;
  int LabelVisibility(int label_index) const;
  int AttributeVisibility(RNFlags attribute) const;
  int ObjectVisibility(R3SurfelObject *object) const;
  int NodeVisibility(R3SurfelNode *node) const;
  
  // Color properties
  int SurfelColorScheme(void) const;
  const char *SurfelColorSchemeName(void) const;
  const RNRgb& NormalColor(void) const;
  const RNRgb& BackgroundColor(void) const;
  const RNRgb& ObjectPrincipalAxesColor(void) const;
  const RNRgb& ObjectOrientedBBoxColor(void) const;
  const RNRgb& ObjectBBoxColor(void) const;
  const RNRgb& NodeBBoxColor(void) const;
  const RNRgb& BlockBBoxColor(void) const;
  const RNRgb& ScanViewpointColor(void) const;
  const RNRgb& ImageViewpointColor(void) const;
  const RNRgb& CenterPointColor(void) const;

  // Viewing parameters
  RNScalar TargetResolution(void) const;
  RNScalar FocusRadius(void) const;
  int SubsamplingFactor(void) const;

  // Elevation properties
  RNScalar Elevation(const R3Point& position) const;
  RNCoord GroundZ(const R3Point& position) const;
  RNCoord GroundZ(const R2Point& position) const;
  
  // Selection properties
  R3SurfelPoint *SelectedPoint(void) const;
  R3SurfelImage *SelectedImage(void) const;
  
  // Frame rate statistics
  RNScalar FrameRate(void) const;
  RNScalar FrameTime(void) const;
  RNScalar CurrentTime(void) const;

  
  /////////////////////////////////////////
  //// PROPERTY MANIPULATION FUNCTIONS ////
  /////////////////////////////////////////

  // Viewing property manipulation
  virtual void ResetCamera(void);
  virtual void SetCamera(const R3Camera& camera);
  virtual void ZoomCamera(RNScalar scale = 10);
  virtual void SetViewport(const R2Viewport& viewport);
  virtual void SetViewingExtent(const R3Box& box);
  virtual void SetElevationRange(const RNInterval& range);
  virtual void SetCenterPoint(const R3Point& point);
  virtual void SetImageInsetSize(RNScalar fraction);
  virtual void SetImagePlaneDepth(RNLength depth);
  virtual void SetSurfelSize(RNScalar npixels);

  // Visibility manipulation (0=off, 1=on, -1=toggle)
  virtual void SetSurfelVisibility(int visibility);
  virtual void SetNormalVisibility(int visibility);
  virtual void SetBackfacingVisibility(int visibility);
  virtual void SetAerialVisibility(int visibility);
  virtual void SetTerrestrialVisibility(int visibility);
  virtual void SetHumanLabeledObjectVisibility(int visibility);
  virtual void SetObjectPrincipalAxesVisibility(int visibility);
  virtual void SetObjectOrientedBBoxVisibility(int visibility);
  virtual void SetObjectRelationshipVisibility(int visibility);
  virtual void SetObjectBBoxVisibility(int visibility);
  virtual void SetNodeBBoxVisibility(int visibility);
  virtual void SetBlockBBoxVisibility(int visibility);
  virtual void SetScanViewpointVisibility(int visibility);
  virtual void SetImageViewpointVisibility(int visibility);
  virtual void SetImagePlaneVisibility(int visibility);
  virtual void SetImageInsetVisibility(int visibility);
  virtual void SetImagePointsVisibility(int visibility);
  virtual void SetCenterPointVisibility(int visibility);
  virtual void SetViewingExtentVisibility(int visibility);
  virtual void SetAxesVisibility(int visibility);
  virtual void SetLabelVisibility(R3SurfelLabel *label, int visibility);
  virtual void SetLabelVisibility(int label_index, int visibility);
  virtual void SetAttributeVisibility(RNFlags attribute, int visibility);
  
  // Color manipulation
  virtual void SetSurfelColorScheme(int scheme);
  virtual void SetNormalColor(const RNRgb& color);
  virtual void SetBackgroundColor(const RNRgb& color);
  virtual void SetObjectPrincipalAxesColor(const RNRgb& color);
  virtual void SetObjectOrientedBBoxColor(const RNRgb& color);
  virtual void SetObjectBBoxColor(const RNRgb& color);
  virtual void SetNodeBBoxColor(const RNRgb& color);
  virtual void SetBlockBBoxColor(const RNRgb& color);
  virtual void SetScanViewpointColor(const RNRgb& color);
  virtual void SetImageViewpointColor(const RNRgb& color);
  virtual void SetCenterPointColor(const RNRgb& color);

  // Selection manipulation
  virtual void SelectPoint(R3SurfelObject *object);
  virtual void SelectPoint(R3SurfelBlock *block, int surfel_index);
  virtual void SelectImage(R3SurfelImage *image,
    RNBoolean update_working_set = TRUE,
    RNBoolean jump_to_viewpoint = FALSE);

  // Display parameters
  virtual void SetTargetResolution(RNScalar resolution);
  virtual void SetFocusRadius(RNScalar radius);
  virtual void SetSubsamplingFactor(int subsampling_factor);

  // Image input/output
  virtual int WriteImage(const char *filename);


  /////////////////////////////////////////
  //// PICKING FUNCTIONS               ////
  /////////////////////////////////////////

  // Pick utility functions
  virtual R3SurfelImage *PickImage(int x, int y,
    R3Point *picked_position = NULL);
  virtual R3SurfelObject *PickObject(int x, int y,
    R3Point *picked_position = NULL);
  virtual R3SurfelNode *PickNode(int x, int y,
    R3Point *picked_position = NULL,
    R3SurfelBlock **picked_block = NULL, int *picked_surfel_index = NULL,
    RNBoolean exclude_nonobjects = FALSE);


  /////////////////////////////////////////
  //// DRAWING UTILITY FUNCTIONS       ////
  /////////////////////////////////////////

  // Color creation
  virtual void CreateColor(unsigned char *color, int k) const;
  virtual void CreateColor(unsigned char *color, double value) const;
  virtual void CreateColor(unsigned char *color, int color_scheme,
    const R3Surfel *surfel, R3SurfelBlock *block, R3SurfelNode *node,
    R3SurfelObject *object, R3SurfelLabel *label) const;

  // Color loading
  virtual void LoadColor(int k) const;
  virtual void LoadColor(double value) const;
  virtual void LoadColor(int color_scheme,
    const R3Surfel *surfel, R3SurfelBlock *block, R3SurfelNode *node,
    R3SurfelObject *object, R3SurfelLabel *label) const;

  // Viewing extent
  virtual void EnableViewingExtent(void) const;
  virtual void DisableViewingExtent(void) const;
  virtual void DrawViewingExtent(void) const;

  // Update
  virtual void UpdateViewingFrustum(void);
  virtual void UpdateGroundZGrid(void);
  virtual void UpdateSelectedImageColorPixels(void);
  virtual void UpdateSelectedImageTexture(void);
  virtual void AdaptWorkingSet(void);
 
  // Drawing 
  virtual void DrawSurfels(int color_scheme = 0) const;
  virtual void DrawSurfels(R3SurfelNode *node, RNFlags color_draw_flags = 0) const;
  virtual void DrawNormals(void) const;
  virtual void DrawObjectPrincipalAxes(void) const;
  virtual void DrawObjectOrientedBBoxes(void) const;
  virtual void DrawObjectRelationships(void) const;
  virtual void DrawObjectBBoxes(void) const;
  virtual void DrawNodeBBoxes(void) const;
  virtual void DrawBlockBBoxes(void) const;
  virtual void DrawCenterPoint(void) const;
  virtual void DrawSelectedPoint(void) const;
  virtual void DrawScanViewpoints(void) const;
  virtual void DrawImageViewpoints(void) const;
  virtual void DrawImageInset(void) const;
  virtual void DrawImagePlane(void) const;
  virtual void DrawImagePoints(void) const;
  virtual void DrawAxes(void) const;

  // Image capture
  virtual void CaptureImage(const char *filename) const;
  
  
////////////////////////////////////////////////////////////////////////
// INTERNAL STUFF BELOW HERE
////////////////////////////////////////////////////////////////////////

  // Object editing 
  virtual int SplitLeafNodes(R3SurfelNode *source_node, const R3SurfelConstraint& constraint, 
    RNArray<R3SurfelNode *> *nodesA = NULL, RNArray<R3SurfelNode *> *nodesB = NULL);

  // Working set management
  virtual void EmptyWorkingSet(void);
  virtual void UpdateWorkingSet(void);
  virtual void UpdateWorkingSet(const R3Viewer& view);
  virtual void UpdateWorkingSet(const R3Point& center, RNScalar target_resolution, RNScalar focus_radius);
  virtual void InsertIntoWorkingSet(R3SurfelNode *node, RNBoolean full_resolution = FALSE);
  virtual void RemoveFromWorkingSet(R3SurfelNode *node, RNBoolean full_resolution = FALSE);

  // Memory management functions
  virtual void ReadCoarsestBlocks(RNScalar max_complexity);
  virtual void ReleaseCoarsestBlocks(RNScalar max_complexity);

protected:
  // Scene manipulation functions
  virtual void SetScene(R3SurfelScene *scene);

  // Viewing utility functions
  virtual void RotateWorld(RNScalar factor, const R3Point& origin, int, int, int dx, int dy);

  // Draw functions
  virtual void DrawObject(R3SurfelObject *object, RNFlags flags = R3_SURFEL_DEFAULT_DRAW_FLAGS) const;
  virtual void DrawObjectProperties(int property_type) const;

  // VBO management functions
  virtual void DrawVBO(int color_scheme) const;
  virtual void InvalidateVBO(void);
  virtual void UpdateVBO(void);

  // Shader management functions
  virtual void CompileShaders(void);
  virtual void DeleteShaders(void);

protected:
  // Tree properties
  R3SurfelScene *scene;

  // Node working set
  R3SurfelNodeSet resident_nodes;

  // Viewing properties
  R3Viewer viewer;
  R3Box viewing_extent;
  RNInterval elevation_range;
  R3Frustum viewing_frustum;
  R3Point center_point;
  R3SurfelPoint *selected_point;
  R3SurfelImage *selected_image;
  RNScalar image_inset_size;
  RNLength image_plane_depth;
  RNScalar surfel_size;

  // Visibility properties
  int surfel_visibility;
  int normal_visibility;
  int backfacing_visibility;
  int aerial_visibility;
  int terrestrial_visibility;
  int human_labeled_object_visibility;
  int object_principal_axes_visibility;
  int object_oriented_bbox_visibility;
  int object_relationship_visibility;
  int object_bbox_visibility;
  int node_bbox_visibility;
  int block_bbox_visibility;
  int scan_viewpoint_visibility;
  int image_viewpoint_visibility;
  int image_plane_visibility;
  int image_inset_visibility;
  int image_points_visibility;
  int center_point_visibility;
  int viewing_extent_visibility;
  int axes_visibility;
  std::vector<int> label_visibilities;
  RNFlags attribute_visibility_flags;

  // Color properties
  int surfel_color_scheme;
  RNRgb normal_color;
  RNRgb background_color;
  RNRgb object_principal_axes_color;
  RNRgb object_oriented_bbox_color;
  RNRgb object_bbox_color;
  RNRgb node_bbox_color;
  RNRgb block_bbox_color;
  RNRgb scan_viewpoint_color;
  RNRgb image_viewpoint_color;
  RNRgb center_point_color;

  // Shape primitive properties
  RNFlags shape_draw_flags;
  
  // Working set parameters
  RNBoolean adapt_working_set_automatically;
  RNScalar target_resolution, last_target_resolution;
  RNScalar focus_radius, last_focus_radius;

  // Subsampling parameters
  RNBoolean adapt_subsampling_automatically;
  int subsampling_factor;
  int subsampling_multiplier_when_mouse_down;

  // UI state
  int window_height;
  int window_width;
  int mouse_button[3];
  int mouse_position[2];
  int mouse_down_position[2];
  int mouse_drag_distance_squared;
  int shift_down;
  int ctrl_down;
  int alt_down;

  // Timing
  RNTime start_timer;
  RNTime frame_timer;
  RNScalar frame_time;

  // Screen capture
  char *screenshot_name;

  // Ground height
  R2Grid ground_z_grid;

  // Image drawing stuff
  R2Image selected_image_color_pixels;
  R3SurfelImage *previous_color_image;
  GLuint selected_image_texture_id;
  R3SurfelImage *previous_texture_image;
  
  // Vertex buffer objects
  GLuint vbo_position_buffer;
  GLuint vbo_normal_buffer;
  GLuint vbo_color_buffer;
  unsigned int vbo_nsurfels;

  // Shader objects
  GLuint shader_program;
  GLuint vertex_shader;
  GLuint fragment_shader;
};



////////////////////////////////////////////////////////////////////////
// KEY CONSTANT DEFINITIONS
////////////////////////////////////////////////////////////////////////

#define R3_SURFEL_VIEWER_ESC_KEY           27
#define R3_SURFEL_VIEWER_SPACE_KEY         32
#define R3_SURFEL_VIEWER_DEL_KEY          127

#define R3_SURFEL_VIEWER_LEFT_KEY        1024
#define R3_SURFEL_VIEWER_RIGHT_KEY       1025
#define R3_SURFEL_VIEWER_DOWN_KEY        1026
#define R3_SURFEL_VIEWER_UP_KEY          1027
#define R3_SURFEL_VIEWER_HOME_KEY        1028
#define R3_SURFEL_VIEWER_END_KEY         1029
#define R3_SURFEL_VIEWER_INSERT_KEY      1030
#define R3_SURFEL_VIEWER_PAGE_DOWN_KEY   1031
#define R3_SURFEL_VIEWER_PAGE_UP_KEY     1032

#define R3_SURFEL_VIEWER_F1_KEY          1064
#define R3_SURFEL_VIEWER_F2_KEY          1065
#define R3_SURFEL_VIEWER_F3_KEY          1066
#define R3_SURFEL_VIEWER_F4_KEY          1067
#define R3_SURFEL_VIEWER_F5_KEY          1068
#define R3_SURFEL_VIEWER_F6_KEY          1069
#define R3_SURFEL_VIEWER_F7_KEY          1070
#define R3_SURFEL_VIEWER_F8_KEY          1071
#define R3_SURFEL_VIEWER_F9_KEY          1072
#define R3_SURFEL_VIEWER_F10_KEY         1073
#define R3_SURFEL_VIEWER_F11_KEY         1074
#define R3_SURFEL_VIEWER_F12_KEY         1075



////////////////////////////////////////////////////////////////////////
// COLOR SCHEME CONSTANT DEFINITIONS
////////////////////////////////////////////////////////////////////////

enum {
  R3_SURFEL_VIEWER_COLOR_BY_RGB,
  R3_SURFEL_VIEWER_COLOR_BY_SHADING,
  R3_SURFEL_VIEWER_COLOR_BY_Z,
  R3_SURFEL_VIEWER_COLOR_BY_NORMAL,
  R3_SURFEL_VIEWER_COLOR_BY_SCAN,
  R3_SURFEL_VIEWER_COLOR_BY_OBJECT,
  R3_SURFEL_VIEWER_COLOR_BY_NODE,
  R3_SURFEL_VIEWER_COLOR_BY_BLOCK,
  R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL,
  R3_SURFEL_VIEWER_COLOR_BY_GROUND_TRUTH_LABEL,
  R3_SURFEL_VIEWER_COLOR_BY_SURFEL_LABEL,
  R3_SURFEL_VIEWER_COLOR_BY_CONFIDENCE,
  R3_SURFEL_VIEWER_COLOR_BY_ELEVATION,
  R3_SURFEL_VIEWER_COLOR_BY_OBJECT_ATTRIBUTES,
  R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX,
  R3_SURFEL_VIEWER_NUM_COLOR_SCHEMES
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTIONS
////////////////////////////////////////////////////////////////////////

inline const R3Camera& R3SurfelViewer::
Camera(void) const
{
  // Return camera
  return viewer.Camera();
}



inline const R2Viewport& R3SurfelViewer::
Viewport(void) const
{
  // Return viewport
  return viewer.Viewport();
}



inline const R3Box& R3SurfelViewer::
ViewingExtent(void) const
{
  // Return viewing extent
  return viewing_extent;
}



inline const RNInterval& R3SurfelViewer::
ElevationRange(void) const
{
  // Return elevation range
  return elevation_range;
}



inline const R3Point& R3SurfelViewer::
CenterPoint(void) const
{
  // Return center point
  return center_point;
}



inline RNScalar R3SurfelViewer::
ImageInsetSize(void) const
{
  // Return size of inset image (as fraction of window size)
  return image_inset_size;
}



inline RNLength R3SurfelViewer::
ImagePlaneDepth(void) const
{
  // Return depth at which image plane will be drawn
  return image_plane_depth;
}



inline RNScalar R3SurfelViewer::
SurfelSize(void) const
{
  // Return surfel size
  return surfel_size;
}



inline int R3SurfelViewer::
SurfelVisibility(void) const
{
  // Return surfel visibililty
  return surfel_visibility;
}



inline int R3SurfelViewer::
NormalVisibility(void) const
{
  // Return normal visibililty
  return normal_visibility;
}



inline int R3SurfelViewer::
BackfacingVisibility(void) const
{
  // Return backfacing visibililty
  return backfacing_visibility;
}



inline int R3SurfelViewer::
AerialVisibility(void) const
{
  // Return background visibililty
  return aerial_visibility;
}



inline int R3SurfelViewer::
TerrestrialVisibility(void) const
{
  // Return background visibililty
  return terrestrial_visibility;
}



inline int R3SurfelViewer::
HumanLabeledObjectVisibility(void) const
{
  // Return human labeled object visibililty
  return human_labeled_object_visibility;
}



inline int R3SurfelViewer::
ObjectPrincipalAxesVisibility(void) const
{
  // Return object principal axes visibililty
  return object_principal_axes_visibility;
}



inline int R3SurfelViewer::
ObjectOrientedBBoxVisibility(void) const
{
  // Return object oriented bbox visibililty
  return object_oriented_bbox_visibility;
}



inline int R3SurfelViewer::
ObjectRelationshipVisibility(void) const
{
  // Return object relationship visibililty
  return object_relationship_visibility;
}



inline int R3SurfelViewer::
ObjectBBoxVisibility(void) const
{
  // Return object bbox visibililty
  return object_bbox_visibility;
}



inline int R3SurfelViewer::
NodeBBoxVisibility(void) const
{
  // Return node bbox visibililty
  return node_bbox_visibility;
}



inline int R3SurfelViewer::
BlockBBoxVisibility(void) const
{
  // Return block bbox visibililty
  return block_bbox_visibility;
}



inline int R3SurfelViewer::
ScanViewpointVisibility(void) const
{
  // Return scan viewpoint visibililty
  return scan_viewpoint_visibility;
}



inline int R3SurfelViewer::
ImageViewpointVisibility(void) const
{
  // Return image viewpoint visibililty
  return image_viewpoint_visibility;
}



inline int R3SurfelViewer::
ImagePlaneVisibility(void) const
{
  // Return image plane visibililty
  return image_plane_visibility;
}



inline int R3SurfelViewer::
ImageInsetVisibility(void) const
{
  // Return image inset visibililty
  return image_inset_visibility;
}



inline int R3SurfelViewer::
ImagePointsVisibility(void) const
{
  // Return image points visibililty
  return image_points_visibility;
}



inline int R3SurfelViewer::
CenterPointVisibility(void) const
{
  // Return center point visibililty
  return center_point_visibility;
}



inline int R3SurfelViewer::
AxesVisibility(void) const
{
  // Return axes visibililty
  return axes_visibility;
}



inline int R3SurfelViewer::
LabelVisibility(R3SurfelLabel *label) const
{
  // Check label
  if (!label) return 0;

  // Return visibility
  return LabelVisibility(label->SceneIndex());
}



inline int R3SurfelViewer::
ViewingExtentVisibility(void) const
{
  // Return viewing extent visibililty
  return viewing_extent_visibility;
}



inline int R3SurfelViewer::
SurfelColorScheme(void) const
{
  // Return color scheme for drawing surfels
  return surfel_color_scheme;
}



inline const RNRgb& R3SurfelViewer::
NormalColor(void) const
{
  // Return normal color
  return normal_color;
}



inline const RNRgb& R3SurfelViewer::
BackgroundColor(void) const
{
  // Return background color
  return background_color;
}



inline const RNRgb& R3SurfelViewer::
ObjectPrincipalAxesColor(void) const
{
  // Return object principal axes color
  return object_principal_axes_color;
}



inline const RNRgb& R3SurfelViewer::
ObjectOrientedBBoxColor(void) const
{
  // Return object oriented bbox color
  return object_oriented_bbox_color;
}



inline const RNRgb& R3SurfelViewer::
ObjectBBoxColor(void) const
{
  // Return object bbox color
  return object_bbox_color;
}



inline const RNRgb& R3SurfelViewer::
NodeBBoxColor(void) const
{
  // Return node bbox color
  return node_bbox_color;
}



inline const RNRgb& R3SurfelViewer::
BlockBBoxColor(void) const
{
  // Return block bbox color
  return block_bbox_color;
}



inline const RNRgb& R3SurfelViewer::
ScanViewpointColor(void) const
{
  // Return scan viewpoint color
  return scan_viewpoint_color;
}



inline const RNRgb& R3SurfelViewer::
ImageViewpointColor(void) const
{
  // Return image viewpoint color
  return image_viewpoint_color;
}



inline const RNRgb& R3SurfelViewer::
CenterPointColor(void) const
{
  // Return center point color
  return center_point_color;
}



inline RNScalar R3SurfelViewer::
TargetResolution(void) const
{
  // Return target resolution
  return target_resolution;
}



inline RNScalar R3SurfelViewer::
FocusRadius(void) const
{
  // Return focus radius
  return focus_radius;
}



inline int R3SurfelViewer::
SubsamplingFactor(void) const
{
  // Return subsampling factor
  return subsampling_factor;
}



inline RNCoord R3SurfelViewer::
GroundZ(const R2Point& position) const
{
  // Update ground z grid
  if (ground_z_grid.NEntries() == 0) {
    ((R3SurfelViewer *) this)->UpdateGroundZGrid();
  }

  // Return z coordinate of ground at position
  if (ground_z_grid.NEntries() == 0) return 0;
  return ground_z_grid.WorldValue(position);
}



inline RNCoord R3SurfelViewer::
GroundZ(const R3Point& position) const
{
  // Return z coordinate of ground at position
  return GroundZ(R2Point(position.X(), position.Y()));
}



inline RNScalar R3SurfelViewer::
Elevation(const R3Point& position) const
{
  // Return elevation at position
  return position.Z() - GroundZ(position);
}



inline RNScalar R3SurfelViewer::
FrameRate(void) const
{
  // Return frame rate
  if (frame_time == 0) return 0;
  else return 1.0 / frame_time;
}



inline RNScalar R3SurfelViewer::
FrameTime(void) const
{
  // Return number of seconds per frame refresh
  return frame_time;
}



inline RNScalar R3SurfelViewer::
CurrentTime(void) const
{
  // Return number of seconds since start up
  return start_timer.Elapsed();
}



inline R3SurfelScene *R3SurfelViewer::
Scene(void) const
{
  // Return scene
  return scene;
}


inline void R3SurfelViewer::
SetCamera(const R3Camera& camera)
{
  // Set camera
  viewer.SetCamera(camera);
}



inline void R3SurfelViewer::
SetViewport(const R2Viewport& viewport)
{
  // Set viewport
  viewer.SetViewport(viewport);
}



inline void R3SurfelViewer::
SetViewingExtent(const R3Box& box)
{
  // Set viewing extent
  viewing_extent = box;
}



inline void R3SurfelViewer::
SetElevationRange(const RNInterval& range)
{
  // Set elevation range
  elevation_range = range;

  // Invalidate VBO buffers
  InvalidateVBO();
}



inline void R3SurfelViewer::
SetCenterPoint(const R3Point& point)
{
  // Set center point
  center_point = point;

  // Update working set
  UpdateWorkingSet(center_point, target_resolution, focus_radius);
}



inline void R3SurfelViewer::
SetImageInsetSize(RNScalar fraction)
{
  // Set size of inset image as fraction of window size
  if (fraction < 0.01) fraction = 0.01;
  else if (fraction > 2) fraction = 2;
  this->image_inset_size = fraction;
}



inline void R3SurfelViewer::
SetImagePlaneDepth(RNLength depth)
{
  // Set depth at which image plane will be drawn
  this->image_plane_depth = depth;
}



inline void R3SurfelViewer::
SetSurfelSize(RNScalar npixels)
{
  // Set surfel size
  if (npixels < 1) npixels = 1;
  this->surfel_size = npixels;
}



inline void R3SurfelViewer::
SetSurfelVisibility(int visibility)
{
  // Set surfel visibililty
  if (visibility == -1) surfel_visibility = 1 - surfel_visibility;
  else if (visibility == 0) surfel_visibility = 0;
  else surfel_visibility = 1;
}



inline void R3SurfelViewer::
SetNormalVisibility(int visibility)
{
  // Set normal visibililty
  if (visibility == -1) normal_visibility = 1 - normal_visibility;
  else if (visibility == 0) normal_visibility = 0;
  else normal_visibility = 1;
}



inline void R3SurfelViewer::
SetBackfacingVisibility(int visibility)
{
  // Set backfacing visibililty
  if (visibility == -1) backfacing_visibility = 1 - backfacing_visibility;
  else if (visibility == 0) backfacing_visibility = 0;
  else backfacing_visibility = 1;
}



inline void R3SurfelViewer::
SetAerialVisibility(int visibility)
{
  // Set background visibililty
  if (visibility == -1) aerial_visibility = 1 - aerial_visibility;
  else if (visibility == 0) aerial_visibility = 0;
  else aerial_visibility = 1;

  // Invalidate VBO buffers
  InvalidateVBO();
}



inline void R3SurfelViewer::
SetTerrestrialVisibility(int visibility)
{
  // Set background visibililty
  if (visibility == -1) terrestrial_visibility = 1 - terrestrial_visibility;
  else if (visibility == 0) terrestrial_visibility = 0;
  else terrestrial_visibility = 1;

  // Invalidate VBO buffers
  InvalidateVBO();
}



inline void R3SurfelViewer::
SetHumanLabeledObjectVisibility(int visibility)
{
  // Set human labeled object visibililty
  if (visibility == -1) human_labeled_object_visibility = 1 - human_labeled_object_visibility;
  else if (visibility == 0) human_labeled_object_visibility = 0;
  else human_labeled_object_visibility = 1;

  // Invalidate VBO buffers
  InvalidateVBO();
}



inline void R3SurfelViewer::
SetObjectPrincipalAxesVisibility(int visibility)
{
  // Set object principal axes visibililty
  if (visibility == -1) object_principal_axes_visibility = 1 - object_principal_axes_visibility;
  else if (visibility == 0) object_principal_axes_visibility = 0;
  else object_principal_axes_visibility = 1;
}



inline void R3SurfelViewer::
SetObjectOrientedBBoxVisibility(int visibility)
{
  // Set object oriented bbox visibililty
  if (visibility == -1) object_oriented_bbox_visibility = 1 - object_oriented_bbox_visibility;
  else if (visibility == 0) object_oriented_bbox_visibility = 0;
  else object_oriented_bbox_visibility = 1;
}



inline void R3SurfelViewer::
SetObjectRelationshipVisibility(int visibility)
{
  // Set object relationship visibililty
  if (visibility == -1) object_relationship_visibility = 1 - object_relationship_visibility;
  else if (visibility == 0) object_relationship_visibility = 0;
  else object_relationship_visibility = 1;
}



inline void R3SurfelViewer::
SetObjectBBoxVisibility(int visibility)
{
  // Set object bbox visibililty
  if (visibility == -1) object_bbox_visibility = 1 - object_bbox_visibility;
  else if (visibility == 0) object_bbox_visibility = 0;
  else object_bbox_visibility = 1;
}



inline void R3SurfelViewer::
SetNodeBBoxVisibility(int visibility)
{
  // Set node bbox visibililty
  if (visibility == -1) node_bbox_visibility = 1 - node_bbox_visibility;
  else if (visibility == 0) node_bbox_visibility = 0;
  else node_bbox_visibility = 1;
}



inline void R3SurfelViewer::
SetBlockBBoxVisibility(int visibility)
{
  // Set block bbox visibililty
  if (visibility == -1) block_bbox_visibility = 1 - block_bbox_visibility;
  else if (visibility == 0) block_bbox_visibility = 0;
  else block_bbox_visibility = 1;
}



inline void R3SurfelViewer::
SetScanViewpointVisibility(int visibility)
{
  // Set scan viewpoint visibililty
  if (visibility == -1) scan_viewpoint_visibility = 1 - scan_viewpoint_visibility;
  else if (visibility == 0) scan_viewpoint_visibility = 0;
  else scan_viewpoint_visibility = 1;
}



inline void R3SurfelViewer::
SetImageViewpointVisibility(int visibility)
{
  // Set image viewpoint visibililty
  if (visibility == -1) image_viewpoint_visibility = 1 - image_viewpoint_visibility;
  else if (visibility == 0) image_viewpoint_visibility = 0;
  else image_viewpoint_visibility = 1;
}



inline void R3SurfelViewer::
SetImagePlaneVisibility(int visibility)
{
  // Set image plane visibililty
  if (visibility == -1) image_plane_visibility = 1 - image_plane_visibility;
  else if (visibility == 0) image_plane_visibility = 0;
  else image_plane_visibility = 1;
}



inline void R3SurfelViewer::
SetImageInsetVisibility(int visibility)
{
  // Set image inset visibililty
  if (visibility == -1) image_inset_visibility = 1 - image_inset_visibility;
  else if (visibility == 0) image_inset_visibility = 0;
  else image_inset_visibility = 1;
}



inline void R3SurfelViewer::
SetImagePointsVisibility(int visibility)
{
  // Set image points visibililty
  if (visibility == -1) image_points_visibility = 1 - image_points_visibility;
  else if (visibility == 0) image_points_visibility = 0;
  else image_points_visibility = 1;
}



inline void R3SurfelViewer::
SetCenterPointVisibility(int visibility)
{
  // Set center point visibililty
  if (visibility == -1) center_point_visibility = 1 - center_point_visibility;
  else if (visibility == 0) center_point_visibility = 0;
  else center_point_visibility = 1;
}



inline void R3SurfelViewer::
SetViewingExtentVisibility(int visibility)
{
  // Set viewing extent visibililty
  if (visibility == -1) viewing_extent_visibility = 1 - viewing_extent_visibility;
  else if (visibility == 0) viewing_extent_visibility = 0;
  else viewing_extent_visibility = 1;
}



inline void R3SurfelViewer::
SetAxesVisibility(int visibility)
{
  // Set axes visibililty
  if (visibility == -1) axes_visibility = 1 - axes_visibility;
  else if (visibility == 0) axes_visibility = 0;
  else axes_visibility = 1;
}



inline void R3SurfelViewer::
SetLabelVisibility(R3SurfelLabel *label, int visibility)
{
  // Check label
  if (!label) return;

  // Set visibility
  SetLabelVisibility(label->SceneIndex(), visibility);
}



inline void R3SurfelViewer::
SetSurfelColorScheme(int scheme)
{
  // Set color scheme for drawing surfels
  surfel_color_scheme = scheme;

  // Invalidate VBO buffers
  InvalidateVBO();
}



inline void R3SurfelViewer::
SetNormalColor(const RNRgb& color)
{
  // Set normal color
  normal_color = color;
}



inline void R3SurfelViewer::
SetBackgroundColor(const RNRgb& color)
{
  // Set background color
  background_color = color;
}



inline void R3SurfelViewer::
SetObjectPrincipalAxesColor(const RNRgb& color)
{
  // Set object principal axes color
  object_principal_axes_color = color;
}



inline void R3SurfelViewer::
SetObjectOrientedBBoxColor(const RNRgb& color)
{
  // Set object oriented bbox color
  object_oriented_bbox_color = color;
}



inline void R3SurfelViewer::
SetObjectBBoxColor(const RNRgb& color)
{
  // Set object bbox color
  object_bbox_color = color;
}



inline void R3SurfelViewer::
SetNodeBBoxColor(const RNRgb& color)
{
  // Set node bbox color
  node_bbox_color = color;
}



inline void R3SurfelViewer::
SetBlockBBoxColor(const RNRgb& color)
{
  // Set block bbox color
  block_bbox_color = color;
}



inline void R3SurfelViewer::
SetScanViewpointColor(const RNRgb& color)
{
  // Set scan viewpoint color
  scan_viewpoint_color = color;
}



inline void R3SurfelViewer::
SetImageViewpointColor(const RNRgb& color)
{
  // Set image viewpoint color
  image_viewpoint_color = color;
}



inline void R3SurfelViewer::
SetCenterPointColor(const RNRgb& color)
{
  // Set center point color
  center_point_color = color;
}



inline void R3SurfelViewer::
SetTargetResolution(RNScalar target_resolution)
{
  // Set target resolution
  this->target_resolution = target_resolution;

  // Update working set
  UpdateWorkingSet(center_point, target_resolution, focus_radius);
}



inline void R3SurfelViewer::
SetFocusRadius(RNScalar focus_radius)
{
  // Set focus radius
  this->focus_radius = focus_radius;

  // Update working set
  UpdateWorkingSet(center_point, target_resolution, focus_radius);
}



inline void R3SurfelViewer::
SetSubsamplingFactor(int subsampling_factor)
{
  // Just checking
  if (subsampling_factor < 1) subsampling_factor = 1;
  static const int max_subsampling_factor = 1024*1024;
  if (subsampling_factor > max_subsampling_factor) subsampling_factor = max_subsampling_factor;

  // Set subsampling factor
  this->subsampling_factor = subsampling_factor;

  // Invalidate VBO buffers
  InvalidateVBO();
}



inline R3SurfelPoint *R3SurfelViewer::
SelectedPoint(void) const
{
  // Return currently selected point
  return selected_point;
}



inline R3SurfelImage *R3SurfelViewer::
SelectedImage(void) const
{
  // Return currently selected image
  return selected_image;
}



inline void R3SurfelViewer::
UpdateWorkingSet(void)
{
  // Update working set
  UpdateWorkingSet(center_point, target_resolution, focus_radius);
}



inline void
LoadUnsignedInt(unsigned int value)
{
  // Load identifer
  unsigned char rgba[4];
  rgba[0] = value & 0xFF;
  rgba[1] = (value >> 8) & 0xFF;
  rgba[2] = (value >> 16) & 0xFF;
  rgba[3] = (value >> 24) & 0xFF;
  RNLoadRgba(rgba);
}



inline unsigned int
DecodeUnsignedInt(unsigned char rgba[4])
{
  // Decode identifer
  int r = rgba[0] & 0xFF;
  int g = rgba[1] & 0xFF;
  int b = rgba[2] & 0xFF;
  int a = rgba[3] & 0xFF;
  int value = r | (g << 8) | (b << 16) | (a << 24);
  return value;
}



// End namespace
}



#endif
