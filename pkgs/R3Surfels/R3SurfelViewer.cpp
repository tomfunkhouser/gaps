/* Source file for the surfel scene viewer class */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Graphics/R3Graphics.h"
#include "R3Surfels/R3Surfels.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Draw mode
////////////////////////////////////////////////////////////////////////

#define R3_SURFEL_VIEWER_DRAW_WITH_GLBEGIN   0
#define R3_SURFEL_VIEWER_DRAW_WITH_VBO       1
#define R3_SURFEL_VIEWER_DRAW_METHOD R3_SURFEL_VIEWER_DRAW_WITH_VBO

  
  
////////////////////////////////////////////////////////////////////////
// Surfel viewer constructor/destructor
////////////////////////////////////////////////////////////////////////

R3SurfelViewer::
R3SurfelViewer(R3SurfelScene *scene)
  : scene(NULL),
    resident_nodes(),
    viewer(),
    viewing_extent(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX),
    viewing_frustum(),
    center_point(0,0,0),
    selected_point(NULL),
    selected_image(NULL),
    current_image_texture(),
    image_inset_size(0.2),
    image_plane_depth(10),
    surfel_size(2),
    surfel_visibility(1),
    normal_visibility(0),
    backfacing_visibility(1),
    aerial_visibility(1),
    terrestrial_visibility(1),
    human_labeled_object_visibility(1),
    object_property_visibility(0),
    object_relationship_visibility(0),
    node_bbox_visibility(0),
    block_bbox_visibility(0),
    scan_viewpoint_visibility(0),
    image_viewpoint_visibility(1),
    image_plane_visibility(0),
    image_inset_visibility(1),
    image_points_visibility(0),
    center_point_visibility(0),
    viewing_extent_visibility(1),
    axes_visibility(0),
    label_visibilities(),
    surfel_color_scheme(R3_SURFEL_VIEWER_COLOR_BY_RGB),
    normal_color(0,1,0),
    background_color(0,0,0),
    object_property_color(0,1,1),
    node_bbox_color(0,0,1),
    block_bbox_color(0,1,0),
    scan_viewpoint_color(0,1,1),
    image_viewpoint_color(0,1,1),
    center_point_color(0,0,0.5),
    shape_draw_flags(0),
    adapt_working_set_automatically(0),
    target_resolution(30),
    last_target_resolution(0),
    focus_radius(0),
    last_focus_radius(0),
    adapt_subsampling_automatically(0),
    subsampling_factor(1),
    window_height(0),
    window_width(0),
    shift_down(0),
    ctrl_down(0),
    alt_down(0),
    start_timer(),
    frame_timer(),
    frame_time(-1),
    screenshot_name(NULL),
    vbo_position_buffer(0),
    vbo_normal_buffer(0),
    vbo_color_buffer(0),
    vbo_nsurfels(0)
{
  // Initialize mouse button state
  mouse_button[0] = 0;
  mouse_button[1] = 0;
  mouse_button[2] = 0;

  // Initialize mouse positions
  mouse_position[0] = 0;
  mouse_position[1] = 0;

  // Initialize mouse positions
  mouse_down_position[0] = 0;
  mouse_down_position[1] = 0;

  // Initialize mouse drag distance
  mouse_drag_distance_squared = 0;

  // Initialize timers
  start_timer.Read();
  frame_timer.Read();

  // Set the scene
  if (scene) SetScene(scene);
}



R3SurfelViewer::
~R3SurfelViewer(void)
{
#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  if (vbo_position_buffer > 0) glDeleteBuffers(1, &vbo_position_buffer);
  if (vbo_normal_buffer > 0) glDeleteBuffers(1, &vbo_normal_buffer);
  if (vbo_color_buffer > 0) glDeleteBuffers(1, &vbo_color_buffer);
#endif
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

const char *R3SurfelViewer::
SurfelColorSchemeName(void) const
{
  // Return name of color scheme
  switch (surfel_color_scheme) {
  case R3_SURFEL_VIEWER_COLOR_BY_RGB: return "RGB";
  case R3_SURFEL_VIEWER_COLOR_BY_SHADING: return "Shading";
  case R3_SURFEL_VIEWER_COLOR_BY_Z: return "Z";
  case R3_SURFEL_VIEWER_COLOR_BY_NORMAL: return "Normal";
  case R3_SURFEL_VIEWER_COLOR_BY_SCAN: return "Scan";
  case R3_SURFEL_VIEWER_COLOR_BY_OBJECT: return "Object";
  case R3_SURFEL_VIEWER_COLOR_BY_NODE: return "Node";
  case R3_SURFEL_VIEWER_COLOR_BY_BLOCK: return "Block";
  case R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL: return "Current Label";
  case R3_SURFEL_VIEWER_COLOR_BY_GROUND_TRUTH_LABEL: return "Ground Truth Label";
  case R3_SURFEL_VIEWER_COLOR_BY_SURFEL_LABEL: return "Surfel Label";
  case R3_SURFEL_VIEWER_COLOR_BY_CONFIDENCE: return "Confidence";
  case R3_SURFEL_VIEWER_COLOR_BY_ELEVATION: return "Elevation";
  case R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX: return "Pick Index";
  default: return "Unknown";
  }
}


 
int R3SurfelViewer::
NodeVisibility(R3SurfelNode *node) const
{
  // Check node
  if (!node) return 0;

  // Check node complexity
  if (subsampling_factor > 1) {
    if (node->Complexity() < subsampling_factor) return 0;
  }
  
  // Check if drawing human labeled objects
  if (!human_labeled_object_visibility) {
    R3SurfelObject *object = node->Object();
    while (object) {
      if (object->HumanLabel()) return 0;
      object = object->Parent();
    }
  }

  // Check if label is visible
  R3SurfelObject *object = node->Object(TRUE);
  while (object && object->Parent() && (object->Parent() != scene->RootObject())) object = object->Parent();
  if (object) {
    R3SurfelLabel *label = object->CurrentLabel();
    if (label) {
      if (!LabelVisibility(label)) return 0;
    }
  }

#if (R3_SURFEL_VIEWER_DRAW_METHOD != R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  // Check viewing frustum
  if (node->Complexity() > 1000) {
    if (!viewing_frustum.IsEmpty()) {
      if (!R3Intersects(viewing_frustum, node->BBox())) return 0;
    }
  }
#endif
  
  // Passed all tests
  return 1;
}



int R3SurfelViewer::
LabelVisibility(int label_index) const
{
  // Check label index
  if (label_index < 0) return FALSE;
  if (label_index >= scene->NLabels()) return FALSE;

  // Label is visible if not in list
  if (label_visibilities.size() <= (unsigned int) label_index) 
    return TRUE;

  // Return whether label is visible
  return label_visibilities[label_index];
}


  
void R3SurfelViewer::
SetLabelVisibility(int label_index, int visibility)
{
  // Check label index
  if (label_index == -1) {
    if (visibility == 1) {
      // Turn all visibilities on
      label_visibilities.clear();
    }
    else {
      // Set all visibilities
      for (int i = 0; i < scene->NLabels(); i++) {
        SetLabelVisibility(i, visibility);
      }
    }
  }
  else {
    // Check label index
    if (label_index < 0) return;
    if (label_index >= scene->NLabels()) return;

    // Add label visibilities
    while (label_visibilities.size() <= (unsigned int) label_index) {
      label_visibilities.push_back(1);
    }

    // Set visibility
    if (visibility == -1) label_visibilities[label_index] = 1 - label_visibilities[label_index];
    else if (visibility == 0) label_visibilities[label_index] = 0;
    else label_visibilities[label_index] = 1;
  }

  // Invalidate VBO buffers
  InvalidateVBO();
}


  
////////////////////////////////////////////////////////////////////////
// Color utility functions
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
CreateColor(unsigned char *color, int k) const
{
  // Make array of colors
  static const int ncolors = 72;
  static const unsigned char colors[ncolors][3] = {
    { 128, 52, 52 }, { 0, 255, 0 }, { 0, 0, 255 }, 
    { 78, 156, 0 }, { 0, 255, 255 }, { 255, 0, 255 }, 
    { 255, 128, 0 }, { 0, 255, 128 }, { 128, 0, 255 }, 
    { 128, 255, 0 }, { 0, 128, 255 }, { 255, 0, 128 }, 
    { 128, 0, 0 }, { 0, 128, 0 }, { 0, 0, 128 }, 
    { 128, 128, 0 }, { 0, 128, 128 }, { 128, 0, 128 },
    { 182, 0, 0 }, { 0, 182, 0 }, { 0, 0, 182 }, 
    { 182, 182, 0 }, { 0, 182, 182 }, { 182, 0, 182 }, 
    { 182, 78, 0 }, { 0, 182, 78 }, { 78, 0, 182 }, 
    { 78, 182, 0 }, { 0, 78, 182 }, { 182, 0, 78 }, 
    { 78, 0, 0 }, { 0, 78, 0 }, { 0, 0, 78 }, 
    { 78, 78, 0 }, { 0, 78, 78 }, { 78, 0, 78 },
    { 255, 78, 78 }, { 78, 255, 78 }, { 78, 78, 255 }, 
    { 255, 0, 78 }, { 78, 255, 255 }, { 255, 78, 255 }, 
    { 255, 52, 182 }, { 78, 255, 128 }, { 128, 78, 255 }, 
    { 128, 255, 78 }, { 78, 128, 255 }, { 255, 78, 128 }, 
    { 128, 78, 78 }, { 78, 128, 78 }, { 78, 78, 128 }, 
    { 128, 128, 78 }, { 78, 128, 128 }, { 128, 78, 128 },
    { 78, 128, 128 }, { 128, 78, 128 }, { 128, 128, 78 }, 
    { 78, 78, 128 }, { 128, 78, 78 }, { 78, 128, 78 }, 
    { 78, 204, 128 }, { 128, 78, 204 }, { 204, 128, 78 }, 
    { 204, 78, 128 }, { 128, 204, 78 }, { 78, 128, 204 }, 
    { 204, 128, 128 }, { 128, 204, 128 }, { 128, 128, 204 }, 
    { 204, 204, 128 }, { 128, 204, 204 }, { 204, 128, 204 }
  };

  // Fill color
  int index = (k > 0) ? (k+1) % ncolors : 0;
  color[0] = colors[index][0];
  color[1] = colors[index][1];
  color[2] = colors[index][2];
}



void R3SurfelViewer::
CreateColor(unsigned char *color, double value) const
{
  // Compute rgb based on blue-green-red heatmap
  if (value <= 0) {
    color[0] = 0;
    color[1] = 0;
    color[2] = 255;
  }
  else if (value < 0.1) {
    value *= 10 * 255;
    color[0] = 0;
    color[1] = value;
    color[2] = 255;
  }
  else if (value < 0.5) {
    value = (value - 0.1) * 2.5 * 255;
    color[0] = 0;
    color[1] = 255;
    color[2] = 255 - value;
  }
  else if (value < 0.9) {
    value = (value - 0.5) * 2.5 * 255;
    color[0] = value;
    color[1] = 255;
    color[2] = 0;
  }
  else if (value < 1) {
    value = (value - 0.9) * 10 * 255;
    color[0] = 255;
    color[1] = 255 - value;
    color[2] = 0;
  }
  else {
    color[0] = 255;
    color[1] = 0;
    color[2] = 0;
  }
}



void R3SurfelViewer::
CreateColor(unsigned char *color, int color_scheme,
  const R3Surfel *surfel, R3SurfelBlock *block, R3SurfelNode *node,
  R3SurfelObject *object, R3SurfelLabel *label) const
{
  // Check color scheme
  switch (color_scheme) {    
  case R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX: {
    int node_index = (node) ? node->TreeIndex() + 1 : 0;
    *color++ = (node_index >> 16) & 0xFF;
    *color++ = (node_index >> 8) & 0xFF;
    *color++ = node_index & 0xFF;
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL: {
    const RNRgb& rgb = (label) ? label->Color() : RNwhite_rgb;
    *color++ = 255 * rgb.R();
    *color++ = 255 * rgb.G();
    *color++ = 255 * rgb.B();
    break; }
    
  case R3_SURFEL_VIEWER_COLOR_BY_GROUND_TRUTH_LABEL: {
    R3SurfelLabel *gtlabel = (object) ? object->GroundTruthLabel() : NULL;
    const RNRgb& rgb = (gtlabel) ? gtlabel->Color() : RNwhite_rgb;
    *color++ = 255 * rgb.R();
    *color++ = 255 * rgb.G();
    *color++ = 255 * rgb.B();
    break; }
    
  case R3_SURFEL_VIEWER_COLOR_BY_SCAN: {
    R3SurfelScan *scan = (node) ? node->Scan() : NULL;
    int scan_index = (scan) ? scan->SceneIndex() : 0;
    CreateColor(color, scan_index);
    break; }
    
  case R3_SURFEL_VIEWER_COLOR_BY_OBJECT: {
    int object_index = (object) ? object->SceneIndex() : 0;
    CreateColor(color, object_index);
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_NODE: {
    int node_index = (node) ? node->TreeIndex() : 0;
    CreateColor(color, node_index);
    break; }
    
  case R3_SURFEL_VIEWER_COLOR_BY_BLOCK: {
    int block_index = (block) ? block->DatabaseIndex() : 0;
    CreateColor(color, block_index);
    break; }
      
  case R3_SURFEL_VIEWER_COLOR_BY_NORMAL: {
    assert(surfel);
    *color++ = 128 + 127 * surfel->NX();
    *color++ = 128 + 127 * surfel->NY();
    *color++ = 128 + 127 * surfel->NZ();
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_Z: {
    assert(block && surfel);
    double z = block->PositionOrigin().Z() + surfel->Z();
    double dz = (z > center_point.Z()) ? z - center_point.Z() : 0;
    double value = 0.5 * sqrt(dz);
    CreateColor(color, value);
    break; }
      
  case R3_SURFEL_VIEWER_COLOR_BY_ELEVATION: {
    assert(surfel);
    unsigned int encoded_elevation = (surfel->Attribute() >> 16) & 0xFFFF;
    double elevation = (encoded_elevation - 32768.0) / 400.0;
    double value = (elevation > 0) ? 0.5 * sqrt(elevation) : 0;
    CreateColor(color, value);
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_SURFEL_LABEL: {
    assert(surfel);
    int label_identifier = surfel->Attribute() & 0xFF;
    CreateColor(color, label_identifier);
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_CONFIDENCE: {
    assert(surfel);
    double confidence = ((surfel->Attribute() >> 8) & 0xFF) / 255.0;
    CreateColor(color, confidence);
    break; }

  default: {
    assert(surfel);
    *color++ = surfel->R();
    *color++ = surfel->G();
    *color++ = surfel->B();
    break; }
  }
}



void R3SurfelViewer::
LoadColor(int k) const
{
  // Load color
  unsigned char color[3];
  CreateColor(color, k);
  glColor3ubv(color);
}



 
void R3SurfelViewer::
LoadColor(double value) const
{
  // Load color
  unsigned char color[3];
  CreateColor(color, value);
  glColor3ubv(color);
}



 
void R3SurfelViewer::
LoadColor(int color_scheme,
  const R3Surfel *surfel, R3SurfelBlock *block, R3SurfelNode *node,
  R3SurfelObject *object, R3SurfelLabel *label) const
{
  // Load color
  unsigned char color[3];
  CreateColor(color, color_scheme, surfel, block, node, object, label);
  glColor3ubv(color);
}



////////////////////////////////////////////////////////////////////////
// Viewing extent utility functions
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
DrawViewingExtent(void) const
{
  // Check extent visibility
  if (!viewing_extent_visibility) return;
  
  // Check extent
  const R3Box& bbox = scene->BBox();  
  const R3Box& extent = ViewingExtent();
  if (extent.IsEmpty() || R3Contains(extent, bbox)) return;

  // Draw extent
  glDisable(GL_LIGHTING);
  glColor3d(0.5, 0.5, 0.5);
  extent.Outline();
}

  

void R3SurfelViewer::
EnableViewingExtent(void) const
{
  // Check clip planes extent
  const R3Box& bbox = scene->BBox();  
  const R3Box& extent = ViewingExtent();
  if (extent.IsEmpty() || R3Contains(extent, bbox)) {
    DisableViewingExtent();
    return;
  }

  // Load lo clip planes
  for (int dim = RN_X; dim <= RN_Z; dim++) {
    if (extent[RN_LO][dim] > bbox[RN_LO][dim]) {
      GLdouble plane_equation[4] = { 0, 0, 0, 0 };
      plane_equation[dim] = 1.0;
      plane_equation[3] = -extent[RN_LO][dim];
      glClipPlane(GL_CLIP_PLANE0 + dim, plane_equation);
      glEnable(GL_CLIP_PLANE0 + dim);
    }
  }

  // Load hi clip planes
  for (int dim = RN_X; dim <= RN_Z; dim++) {
    if (extent[RN_HI][dim] < bbox[RN_HI][dim]) {
      GLdouble plane_equation[4] = { 0, 0, 0, 0 };
      plane_equation[dim] = -1.0;
      plane_equation[3] = extent[RN_HI][dim];
      glClipPlane(GL_CLIP_PLANE0 + 3 + dim, plane_equation);
      glEnable(GL_CLIP_PLANE0 + 3 + dim);
    }
  }
}  



void R3SurfelViewer::
DisableViewingExtent(void) const
{
  // Disable all clip planes
  for (int i = 0; i < 6; i++) {
    glDisable(GL_CLIP_PLANE0 + i);
  }
}



void R3SurfelViewer::
UpdateViewingFrustum(void)
{
  // Update viewing frustum
  const R3Camera& camera = viewer.Camera();
  viewing_frustum = R3Frustum(camera.Origin(), camera.Towards(), camera.Up(),
    camera.XFOV(), camera.YFOV(), camera.Near(), camera.Far());
}



////////////////////////////////////////////////////////////////////////
// Drawing utility functions
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
DrawSurfels(R3SurfelNode *node, RNFlags color_draw_flags) const
{
  // Check node
  if (!NodeVisibility(node)) return;

  // Draw all blocks
  for (int i = 0; i < node->NBlocks(); i++) {
    R3SurfelBlock *block = node->Block(i);
    block->Draw(shape_draw_flags | color_draw_flags, subsampling_factor);
  }
}



void R3SurfelViewer::
DrawSurfels(RNFlags color_draw_flags) const
{
  // Draw resident nodes
  switch (surfel_color_scheme) {
  case R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX:
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      LoadColor(surfel_color_scheme, NULL, NULL, node, NULL, NULL);
      DrawSurfels(node);
    }
    break;

  case R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL:
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      R3SurfelObject *object = node->Object();
      while (object && object->Parent() && (object->Parent() != scene->RootObject())) object = object->Parent();
      R3SurfelLabel *label = (object) ? object->CurrentLabel() : NULL;
      const RNRgb& rgb = (label) ? label->Color() : RNwhite_rgb;
      RNLoadRgb(rgb);
      DrawSurfels(node);
    }
    break;

  case R3_SURFEL_VIEWER_COLOR_BY_GROUND_TRUTH_LABEL:
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      R3SurfelObject *object = node->Object(TRUE);
      while (object && object->Parent() && (object->Parent() != scene->RootObject())) object = object->Parent();
      R3SurfelLabel *label = (object) ? object->GroundTruthLabel() : NULL;
      const RNRgb& rgb = (label) ? label->Color() : RNwhite_rgb;
      RNLoadRgb(rgb);
      DrawSurfels(node);
    }
    break;

  case R3_SURFEL_VIEWER_COLOR_BY_SCAN:
    // Draw with colors based on nodes
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      R3SurfelScan *scan = node->Scan();
      int scan_index = (scan) ? scan->SceneIndex() : 0;
      LoadColor(scan_index);
      DrawSurfels(node);
    }
    break;
    
  case R3_SURFEL_VIEWER_COLOR_BY_OBJECT:
    // Draw with colors based on nodes
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      R3SurfelObject *object = node->Object(TRUE);
      while (object && object->Parent() && (object->Parent() != scene->RootObject())) object = object->Parent();
      int object_index = (object) ? object->SceneIndex() : 0;
      LoadColor(object_index);
      DrawSurfels(node);
    }
    break;
    
  case R3_SURFEL_VIEWER_COLOR_BY_NODE:
    // Draw with colors based on nodes
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      LoadColor(i);
      DrawSurfels(node);
    }
    break;
    
  case R3_SURFEL_VIEWER_COLOR_BY_BLOCK:
    // Draw with colors based on blocks
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      if (!NodeVisibility(node)) continue;
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        LoadColor(block->DatabaseIndex());
        block->Draw(shape_draw_flags, subsampling_factor);
      }
    }
    break;
    
  case R3_SURFEL_VIEWER_COLOR_BY_SHADING:
    // Draw with colors based on shading
    glEnable(GL_LIGHTING);
    glColor3d(0.8, 0.8, 0.8);
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      DrawSurfels(node, R3_SURFEL_NORMAL_DRAW_FLAG);
    }
    glDisable(GL_LIGHTING);
    break;
    
  case R3_SURFEL_VIEWER_COLOR_BY_NORMAL:
    // Draw with colors based on normals
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      if (!NodeVisibility(node)) continue;
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        const R3Point& block_origin = block->PositionOrigin();
        glPushMatrix();
        glTranslated(block_origin.X(), block_origin.Y(), block_origin.Z());
        glBegin(GL_POINTS);
        for (int k = 0; k < block->NSurfels(); k += subsampling_factor) {
          const R3Surfel *surfel = block->Surfel(k);
          float r = 0.5F + 0.5F * surfel->NX();
          float g = 0.5F + 0.5F * surfel->NY();
          float b = 0.5F + 0.5F * surfel->NZ();
          glColor3f(r, g, b);
          glVertex3fv(surfel->PositionPtr());
        }
        glEnd();
        glPopMatrix();
      }
    }
    break;
    
  case R3_SURFEL_VIEWER_COLOR_BY_Z:
  case R3_SURFEL_VIEWER_COLOR_BY_ELEVATION:
  case R3_SURFEL_VIEWER_COLOR_BY_SURFEL_LABEL:
  case R3_SURFEL_VIEWER_COLOR_BY_CONFIDENCE:
    // Draw with colors based on surfel attributes
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      if (!NodeVisibility(node)) continue;
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        const R3Point& block_origin = block->PositionOrigin();
        glPushMatrix();
        glTranslated(block_origin.X(), block_origin.Y(), block_origin.Z());
        glBegin(GL_POINTS);
        for (int k = 0; k < block->NSurfels(); k += subsampling_factor) {
          const R3Surfel *surfel = block->Surfel(k);
          if (surfel_color_scheme == R3_SURFEL_VIEWER_COLOR_BY_Z) {
            double z = block_origin.Z() + surfel->Z();
            double dz = (z > center_point.Z()) ? z - center_point.Z() : 0;
            double value = 0.5 * sqrt(dz);
            LoadColor(value);
          }
          else if (surfel_color_scheme == R3_SURFEL_VIEWER_COLOR_BY_ELEVATION) {
            unsigned int encoded_elevation = (surfel->Attribute() >> 16) & 0xFFFF;
            double elevation = (encoded_elevation - 32768.0) / 400.0;
            double value = (elevation > 0) ? 0.5 * sqrt(elevation) : 0;
            LoadColor(value);
          }
          else if (surfel_color_scheme == R3_SURFEL_VIEWER_COLOR_BY_SURFEL_LABEL) {
            int label_identifier = surfel->Attribute() & 0xFF;
            LoadColor(label_identifier);
          }
          else if (surfel_color_scheme == R3_SURFEL_VIEWER_COLOR_BY_CONFIDENCE) {
            double confidence = ((surfel->Attribute() >> 8) & 0xFF) / 255.0;
            LoadColor(confidence);
          }
          glVertex3fv(surfel->PositionPtr());
        }
        glEnd();
        glPopMatrix();
      }
    }
    break;
    
  default:
    // Draw with RGB surfel colors
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      if (!NodeVisibility(node)) continue;
      DrawSurfels(node, R3_SURFEL_COLOR_DRAW_FLAG);
    }
    break;
  }
}



////////////////////////////////////////////////////////////////////////
// UI event handler functions
////////////////////////////////////////////////////////////////////////

int R3SurfelViewer::
Redraw(void)
{
  // Check scene
  if (!scene) return 0;

  // Set viewing transformation
  viewer.Camera().Load();

  // Clear window 
  glClearColor(background_color[0], background_color[1], background_color[2], 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set lights
  static GLfloat light0_position[] = { 3.0, 4.0, 5.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  static GLfloat light1_position[] = { -3.0, -2.0, -3.0, 0.0 };
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

  // Set viewing extent
  DrawViewingExtent();
  EnableViewingExtent();
  UpdateViewingFrustum();
  
  // Set draw modes
  glDisable(GL_LIGHTING);
  glPointSize(surfel_size);
  glLineWidth(1);
  if (backfacing_visibility) glDisable(GL_CULL_FACE);
  else glEnable(GL_CULL_FACE);


#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  if (surfel_visibility) {
    if (shape_draw_flags[R3_SURFEL_DISC_DRAW_FLAG]) {
      DrawSurfels(surfel_color_scheme);
    }
    else {
      DrawVBO(surfel_color_scheme);
    }
  }
#else
  // Draw surfels
  DrawSurfels(surfel_color_scheme);
#endif
  
  // Reset opengl modes
  glDisable(GL_LIGHTING);
  glPointSize(1);
  glLineWidth(1);

  // Draw normals
  if (normal_visibility) {
    RNLoadRgb(normal_color);
    RNLength r = 0.00025 * scene->BBox().DiagonalRadius();
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      if (!NodeVisibility(node)) continue;
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        const R3Point& block_origin = block->PositionOrigin();
        glPushMatrix();
        glTranslated(block_origin.X(), block_origin.Y(), block_origin.Z());
        glBegin(GL_LINES);
        for (int k = 0; k < block->NSurfels(); k++) {
          const R3Surfel *surfel = block->Surfel(k);
          double px = surfel->X();
          double py = surfel->Y();
          double pz = surfel->Z();
          double nx = surfel->NX();
          double ny = surfel->NY();
          double nz = surfel->NZ();
          R3LoadPoint(px, py, pz);
          R3LoadPoint(px + r * nx, py + r * ny, pz + r * nz);
        }
        glEnd();
        glPopMatrix();
      }
    }
  }

  // Draw object properties
  if (object_property_visibility) {
    RNLoadRgb(object_property_color);
    for (int i = 0; i < scene->NObjectProperties(); i++) {
      R3SurfelObjectProperty *property = scene->ObjectProperty(i);
      property->Draw(0);
    }
  }

  // Draw object relationships
  if (object_relationship_visibility) {
    RNLoadRgb(1.0, 1.0, 1.0);
    for (int i = 0; i < scene->NObjectRelationships(); i++) {
      R3SurfelObjectRelationship *relationship = scene->ObjectRelationship(i);
      relationship->Draw();
    }
  }

  // Draw node bounding boxes
  if (node_bbox_visibility) {
    RNLoadRgb(node_bbox_color);
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      node->BBox().Outline();
      if (node->NParts() > 0) glColor3d(0, 1, 0);
      else glColor3d(1, 0, 0);
    }
  }

  // Draw block bounding boxes
  if (block_bbox_visibility) {
    RNLoadRgb(block_bbox_color);
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        block->BBox().Outline();
      }
    }
  }

  // Draw image points
  if (image_points_visibility && selected_image) {
    glDisable(GL_LIGHTING);
    glPointSize(surfel_size);
    glBegin(GL_POINTS);
    for (int i = 0; i < scene->NImages(); i++) {
      R3SurfelImage *image = scene->Image(i);
      if (selected_image && (image != selected_image)) continue;
      const R2Grid *depth_channel = image->DepthChannel();
      if (!depth_channel) continue;
      const R2Image color_channels = image->ColorChannels();
      for (int iy = 0; iy < image->ImageHeight(); iy++) {
        for (int ix = 0; ix < image->ImageWidth(); ix++) {
          RNScalar depth = depth_channel->GridValue(ix, iy);
          if (RNIsZero(depth) || (depth == R2_GRID_UNKNOWN_VALUE)) continue;
          R2Point image_position(ix, iy);
          R3Point world_position = image->TransformFromImageToWorld(image_position);
          RNLoadRgb(color_channels.PixelRGB(ix, iy));
          R3LoadPoint(world_position);
        }
      }
    }
    glEnd();
    glPointSize(1);
  }
  
  // Draw center point
  if (center_point_visibility) {
    glEnable(GL_LIGHTING);
    RNLoadRgb(center_point_color);
    R3Sphere(center_point, 0.1).Draw();
    glDisable(GL_LIGHTING);
  }

  // Reset viewing modes
  DisableViewingExtent();

  // Draw selected point
  if (selected_point) {
    glEnable(GL_LIGHTING);
    RNLoadRgb(RNred_rgb);
    R3Sphere(selected_point->Position(), 0.05).Draw();
    glDisable(GL_LIGHTING);
  }

  // Draw scan viewpoints
  if (scan_viewpoint_visibility) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(scan_viewpoint_color);
    glBegin(GL_LINES);
    for (int i = 0; i < scene->NScans(); i++) {
      R3SurfelScan *scan = scene->Scan(i);
      const R3Point& viewpoint = scan->Viewpoint();
      const R3Vector towards = scan->Towards();
      const R3Vector up = scan->Up();
      R3LoadPoint(viewpoint);
      R3LoadPoint(viewpoint + towards);
      R3LoadPoint(viewpoint);
      R3LoadPoint(viewpoint + 0.5 * up);
    }
    glEnd();
  }

  // Draw image viewpoints
  if (image_viewpoint_visibility) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(image_viewpoint_color);
    for (int i = 0; i < scene->NImages(); i++) {
      R3SurfelImage *image = scene->Image(i);
      if (image == selected_image) glLineWidth(5);
      image->Draw();
      if (image == selected_image) glLineWidth(1);
    }
  }

  // Draw image plane
  if ((image_plane_visibility) && (current_image_texture.Image()) && selected_image) {
    RNLength depth = image_plane_depth;
    glDisable(GL_LIGHTING);
    RNLoadRgb(1.0, 1.0, 1.0);
    R3SurfelImage *image = selected_image;
    if ((image->ImageWidth() > 0) && (image->ImageHeight() > 0)) {
      R3Point c = image->Viewpoint() + depth * image->Towards();
      c -= ((image->ImageCenter().X() - 0.5*image->ImageWidth()) / (0.5*image->ImageWidth())) * image->Right() * depth;
      c -= ((image->ImageCenter().Y() - 0.5*image->ImageHeight()) / (0.5*image->ImageHeight())) * image->Up() * depth;
      R3Vector dx = depth * tan(image->XFOV()) * image->Right();
      R3Vector dy = depth * tan(image->YFOV()) * image->Up();
      current_image_texture.Draw();
      glBegin(GL_QUADS);
      R3LoadTextureCoords(0.0, 0.0);
      R3LoadPoint(c - dx - dy);
      R3LoadTextureCoords(1.0, 0.0);
      R3LoadPoint(c + dx - dy);
      R3LoadTextureCoords(1.0, 1.0);
      R3LoadPoint(c + dx + dy);
      R3LoadTextureCoords(0.0, 1.0);
      R3LoadPoint(c - dx + dy);
      glEnd();
      R2null_texture.Draw();
    }
  }

  // Draw image inset
  if ((image_inset_visibility) && (current_image_texture.Image()) && selected_image) {
    R3SurfelImage *image = selected_image;
    if ((image->ImageWidth() > 0) && (image->ImageHeight() > 0)) {
      int w = viewer.Viewport().Width();
      int h = viewer.Viewport().Height();

      // Determine image coordinates
      double x2 = w-1;
      double y2 = h-1;
      double aspect = (double) image->ImageHeight() / (double)  image->ImageWidth();
      double x1 = x2 - image_inset_size * w;
      double y1 = y2 - image_inset_size * w * aspect;

      // Determine focal point
      R2Point focal_point(0.5*image->ImageWidth(), 0.5*image->ImageHeight());
      if (selected_point) {
        focal_point = image->ImagePosition(selected_point->Position());
        if ((focal_point.X() >= 0) && (focal_point.Y() >= 0) &&
            (focal_point.X() <= image->ImageWidth()-0.5) &&
            (focal_point.Y() <= image->ImageHeight()-0.5)) {
          focal_point[0] = x1 + (x2 - x1) * focal_point.X()/image->ImageWidth();
          focal_point[1] = y1 + (y2 - y1) * focal_point.Y()/image->ImageHeight();
        }
      }

      // Push ortho viewing matrices
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0, w-1, 0, h-1, 0.1, 1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();

      // Translate so that focal point is in view
      if (focal_point.Y() < 0.25*h) {
        double dy = 0.25*h - focal_point.Y();
        glTranslated(0.0, dy, 0.0);
      }
     
      // Draw image as textured quad
      glDisable(GL_LIGHTING);
      RNLoadRgb(1.0, 1.0, 1.0);
      current_image_texture.Draw();
      glBegin(GL_QUADS);
      R3LoadTextureCoords(0.0, 0.0);
      R3LoadPoint(x1, y1, -0.5);
      R3LoadTextureCoords(1.0, 0.0);
      R3LoadPoint(x2, y1, -0.5);
      R3LoadTextureCoords(1.0, 1.0);
      R3LoadPoint(x2, y2, -0.5);
      R3LoadTextureCoords(0.0, 1.0);
      R3LoadPoint(x1, y2, -0.5);
      glEnd();
      R2null_texture.Draw();

      // Draw projected center point
      if (selected_point) {
        glPointSize(8);
        RNLoadRgb(1.0, 0.0, 0.0);
        glBegin(GL_POINTS);
        R3LoadPoint(focal_point.X(), focal_point.Y(), -0.25);
        glEnd();
        glPointSize(1);
      }
     
      // Pop ortho viewing matrices
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }
  }

  // Draw axes
  if (axes_visibility) {
    RNScalar d = 1.0;
    glDisable(GL_LIGHTING);
    glLineWidth(3);
    R3BeginLine();
    glColor3f(1, 0, 0);
    R3LoadPoint(R3zero_point);
    R3LoadPoint(R3zero_point + d * R3posx_vector);
    R3EndLine();
    R3BeginLine();
    glColor3f(0, 1, 0);
    R3LoadPoint(R3zero_point);
    R3LoadPoint(R3zero_point + d * R3posy_vector);
    R3EndLine();
    R3BeginLine();
    glColor3f(0, 0, 1);
    R3LoadPoint(R3zero_point);
    R3LoadPoint(R3zero_point + d * R3posz_vector);
    R3EndLine();
    glLineWidth(1);
  }

  // Capture image
  if (screenshot_name) {
    R2Image image(viewer.Viewport().Width(), viewer.Viewport().Height(), 3);
    image.Capture();
    image.Write(screenshot_name);
    free(screenshot_name);
    screenshot_name = NULL;
  }

  // Update the frame time
  if (frame_time < 0) frame_time = 0.05;
  else frame_time = frame_timer.Elapsed();
  frame_timer.Read();

  // Adapt working set
  if (adapt_working_set_automatically) {
    // Adjust target resolution based on frame time
    if (frame_time > 0.05) {
      SetTargetResolution(0.9 * TargetResolution());
    }
    else if (frame_time < 0.025) {
      SetTargetResolution(1.1 * TargetResolution());
    }

    // Make gross estimate of visible radius
    RNLength camera_height = viewer.Camera().Origin().Z();
    RNLength visible_radius = camera_height;

    // Adjust focus radius 
    SetFocusRadius(visible_radius);

    // Adjust surfel size based on visible radius
    if ((TargetResolution() > 0) && (visible_radius > 0)) {
      RNLength window_width = viewer.Viewport().Width();
      RNLength npixels = window_width / (visible_radius * TargetResolution());
      SetSurfelSize(npixels);
    }
  }

  // Return whether need redraw
  return 0;
}    



int R3SurfelViewer::
Resize(int w, int h)
{
  // Resize window
  glViewport(0, 0, w, h);

  // Resize viewer viewport
  viewer.ResizeViewport(0, 0, w, h);

  // Remember window size
  window_width = w;
  window_height = h;

  // Return whether need redraw
  return 1;
}



int R3SurfelViewer::
MouseMotion(int x, int y)
{
  // Initialize
  int redraw = 0;

  // Compute mouse movement
  int dx = x - mouse_position[0];
  int dy = y - mouse_position[1];

  // Set viewing center point
  R3Point viewing_center_point = center_point;
  const R3Camera& camera = viewer.Camera();
  R3Plane camera_plane(camera.Origin(), camera.Towards());
  RNScalar signed_distance = R3SignedDistance(camera_plane, viewing_center_point);
  if (signed_distance < 0) viewing_center_point -= (signed_distance - 1) * camera.Towards();
  
  // World in hand navigation 
  if (shift_down && mouse_button[0]) viewer.ScaleWorld(2.0, viewing_center_point, x, y, dx, dy);
  else if (ctrl_down && mouse_button[0]) viewer.TranslateWorld(2.0, viewing_center_point, x, y, dx, dy);
  else if (mouse_button[0]) RotateWorld(1.0, viewing_center_point, x, y, dx, dy);
  else if (mouse_button[1]) viewer.ScaleWorld(2.0, viewing_center_point, x, y, dx, dy);
  else if (mouse_button[2]) viewer.TranslateWorld(2.0, viewing_center_point, x, y, dx, dy);
  if (mouse_button[0] || mouse_button[1] || mouse_button[2]) redraw = 1;

  // Remember mouse position 
  mouse_position[0] = x;
  mouse_position[1] = y;

  // Update mouse drag movement
  mouse_drag_distance_squared += dx*dx + dy*dy;

  // Return whether need redraw
  return redraw;
}



int R3SurfelViewer::
MouseButton(int x, int y, int button, int state, int shift, int ctrl, int alt)
{
  // Initialize
  int redraw = 0;

  // Process mouse button event
  if (state == 1) {
    // Button is going down
    mouse_drag_distance_squared = 0;

    // Remember mouse down position 
    mouse_down_position[0] = x;
    mouse_down_position[1] = y;

    // Process thumbwheel
    if ((button == 3) || (button == 4)) {
      R3Point viewing_center_point = center_point;
      const R3Camera& camera = viewer.Camera();
      R3Plane camera_plane(camera.Origin(), camera.Towards());
      RNScalar signed_distance = R3SignedDistance(camera_plane, viewing_center_point);
      if (signed_distance < 0) viewing_center_point -= (signed_distance - 1) * camera.Towards();
      if (button == 3) viewer.ScaleWorld(viewing_center_point, 0.9);
      else if (button == 4) viewer.ScaleWorld(viewing_center_point, 1.1);
      redraw = TRUE;
    }
  }
  else {
    // Check for drag
    RNBoolean drag = (mouse_drag_distance_squared > 10 * 10);

    // Check for double click
    static RNBoolean double_click = FALSE;
    static RNTime last_mouse_down_time;
    double_click = !drag && !double_click && (last_mouse_down_time.Elapsed() < 0.4);
    last_mouse_down_time.Read();

    // Select stuff on left click 
    if (!drag) {
      // Select image
      R3Point pick_position;
      R3SurfelImage *picked_image = PickImage(x, y, &pick_position);
      if (picked_image) {
        if (button == 0) SelectImage(picked_image, FALSE, FALSE);
        SetCenterPoint(pick_position);
        redraw = TRUE;
      }
      else {
        // Select point
        int picked_surfel_index = -1;
        R3SurfelBlock *picked_block = NULL;
        if (PickNode(x, y, &pick_position, &picked_block, &picked_surfel_index)) {
          // Hit
          if ((button == 0) && (picked_surfel_index >= 0)) {
            SelectPoint(picked_block, picked_surfel_index);
            if (selected_point && !image_plane_visibility) {
              R3SurfelImage *image = scene->FindImageByBestView(selected_point->Position(), selected_point->Normal());
              SelectImage(image, FALSE, FALSE);
            }
          }
          SetCenterPoint(pick_position);
          redraw = TRUE;
        }
        else {
          // Miss
          SelectPoint(NULL, -1);
          redraw = TRUE;
        }
      }
    }
  }

  // Remember mouse position 
  mouse_position[0] = x;
  mouse_position[1] = y;

  // Remember button state 
  mouse_button[button] = state;

  // Remember modifiers 
  shift_down = shift;
  ctrl_down = ctrl;
  alt_down = alt;

  // Return whether need redraw
  return redraw;
}



int R3SurfelViewer::
Keyboard(int x, int y, int key, int shift, int ctrl, int alt)
{
  // Initialize redraw status
  int redraw = 1;

  // Process debugging commands
  if (alt) {
    // Process debugging commands
    switch (key) {
    case 'A':
    case 'a':
      SetAxesVisibility(-1);
      break;
      
    case 'B':
      SetBlockBBoxVisibility(-1);
      break;

    case 'b':
      SetNodeBBoxVisibility(-1);
      break;

    case 'C':
    case 'c':
      if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_RGB)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_ELEVATION);
      else if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_ELEVATION)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL);
      else SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_RGB);
      break;
      
    case 'D':
    case 'd':
      SetImagePointsVisibility(-1);
      SelectImage(selected_image, FALSE, FALSE);
      break;
      
    case 'E':
    case 'e':
      if (shape_draw_flags != 0) shape_draw_flags = 0;
      else shape_draw_flags = R3_SURFEL_DISC_DRAW_FLAG | R3_SURFEL_NORMAL_DRAW_FLAG;
      break;
      
    case 'F':
    case 'f':
      SetBackfacingVisibility(-1);
      break;
      
    case 'G':
    case 'g':
      SetSurfelColorScheme((surfel_color_scheme + 1) % R3_SURFEL_VIEWER_NUM_COLOR_SCHEMES);
      break;

    case 'I':
      SetImagePlaneVisibility(-1);
      SelectImage(selected_image, FALSE, FALSE);
      break;

    case 'i':
      SetImageInsetVisibility(-1);
      SelectImage(selected_image, FALSE, FALSE);
      break;

    case 'L': 
    case 'l':
      SetHumanLabeledObjectVisibility(-1);
      break;
      
    // case 'M': // Reserve for toggling "menu" in sfllabel
    // case 'm':
    //  break;
      
    case 'N':
    case 'n':
      SetNormalVisibility(-1);
      break;
      
    case 'O':
    case 'o':
      SetCenterPointVisibility(-1);
      break;
      
    case 'P':
    case 'p':
      SetSurfelVisibility(-1);
      break;

    case 'R':
    case 'r':
      SetObjectRelationshipVisibility(-1);
      break;

    // case 'S': // Save for toggling "selection" in sfllabel
    // case 's':
    //   break;

    // case 'T': // Reserve for toggling object label text in sfllabel
    // case 't':
    //  break;
      
    case 'V':
    case 'v':
      SetImageViewpointVisibility(-1);
      break;
      
    case 'W':
      UpdateWorkingSet(viewer);
      break;

    case 'w': {
      R3Point pick_position;
      R3SurfelNode *node = PickNode(x, y, &pick_position);
      if (node) SetCenterPoint(pick_position);
      break; }

    case 'X':
    case 'x':
      SetObjectPropertyVisibility(-1);
      break;
      
    case 'Y':
    case 'y':
      SetScanViewpointVisibility(-1);
      break;
      
    case 'Z':
    case 'z':
      SetViewingExtentVisibility(-1);
      break;
      
    case 'Q': 
    case 'q': {
      R3Point pick_position(0,0,0);
      R3SurfelNode *node = PickNode(x, y, &pick_position);
      if (node) {
        SetCenterPoint(pick_position);
        printf("%g %g %g\n", pick_position[0], pick_position[1], pick_position[2]);
        while (node) {
          const char *node_name = (node->Name()) ? node->Name() : "-";
          R3SurfelObject *object = node->Object();
          const char *object_name = (object && (object->Name())) ? object->Name() : "-";
          char object_index[128];
          if (object) sprintf(object_index, "%d", object->SceneIndex());
          else sprintf(object_index, "%s", "-");
          R3SurfelLabel *ground_truth_label = (object) ? object->GroundTruthLabel() : NULL;
          const char *ground_truth_label_name = (ground_truth_label && (ground_truth_label->Name())) ? ground_truth_label->Name() : "-";
          R3SurfelLabel *current_label = (object) ? object->CurrentLabel() : NULL;
          const char *current_label_name = (current_label && (current_label->Name())) ? current_label->Name() : "-";
          printf("  %4d %4d %-30s  :  %-6s %-30s  :  %-30s %-30s\n",  
                 node->TreeLevel(), node->NParts(), node_name, 
                 object_index, object_name, 
                 current_label_name, ground_truth_label_name);
          node = node->Parent();        
        }
      }
      break; }

    case '.':
      SetFocusRadius(1.1 * FocusRadius());
      break;
      
    case ',':
      SetFocusRadius(0.9 * FocusRadius());
      break;

    case '<':
      SetTargetResolution(0.9 * TargetResolution());
      break;

    case '>':
      SetTargetResolution(1.1 * TargetResolution());
      break;

    case ';': 
    case '\'': {
      // Select next/prev image
      int image_index = (selected_image) ? selected_image->SceneIndex() : 0;
      if (key == '\'') image_index++;
      else if (key == ';') image_index--;
      if (image_index < 0) image_index = 0;
      if (image_index > scene->NImages()-1) image_index = scene->NImages()-1;
      SelectImage(scene->Image(image_index), TRUE, TRUE);
      break; }

    case '_': 
      SetImagePlaneDepth(0.9 * ImagePlaneDepth());
      break;

    case '+': 
      SetImagePlaneDepth(1.1 * ImagePlaneDepth());
      break;

    case '-': 
      SetSurfelSize(0.9 * SurfelSize());
      break;

    case '=': 
      SetSurfelSize(1.1 * SurfelSize());
      break;

    default:
      redraw = 0;
      break;
    }
  }
  else if (ctrl) {
    switch(key) {
    default:
      redraw = 0;
      break;
    }
  }
  else {
    // Process other commands
    switch (key) {
    case R3_SURFEL_VIEWER_F1_KEY:
    case R3_SURFEL_VIEWER_F2_KEY:
    case R3_SURFEL_VIEWER_F3_KEY:
    case R3_SURFEL_VIEWER_F4_KEY: {
      int scale = key - R3_SURFEL_VIEWER_F1_KEY + 1;
      ZoomCamera(0.05 + 0.25*scale*scale);
      break; }

    case R3_SURFEL_VIEWER_F7_KEY:
      if (selected_image) SelectImage(selected_image, TRUE, TRUE);
      break;

    case R3_SURFEL_VIEWER_F8_KEY:
      ResetCamera();
      break;
      
    case R3_SURFEL_VIEWER_DOWN_KEY:
      SetImageInsetSize(0.9 * ImageInsetSize());
      break;

    case R3_SURFEL_VIEWER_UP_KEY:
      SetImageInsetSize(1.1 * ImageInsetSize());
      break;

    case R3_SURFEL_VIEWER_LEFT_KEY: 
      SetSubsamplingFactor(SubsamplingFactor() - 1);
      break;

    case R3_SURFEL_VIEWER_RIGHT_KEY: 
      SetSubsamplingFactor(SubsamplingFactor() + 1);
      break;

    case R3_SURFEL_VIEWER_PAGE_UP_KEY: 
      if (viewing_extent.IsEmpty()) viewing_extent = scene->BBox();
      if (shift) viewing_extent[RN_LO][RN_Z] += 0.01 * scene->BBox().ZLength();
      else viewing_extent[RN_HI][RN_Z] += 0.01 * scene->BBox().ZLength();
      if (R3Contains(viewing_extent, scene->BBox())) viewing_extent = R3null_box;
      break;

    case R3_SURFEL_VIEWER_PAGE_DOWN_KEY: 
      if (viewing_extent.IsEmpty()) viewing_extent = scene->BBox();
      if (shift) viewing_extent[RN_LO][RN_Z] -= 0.01 * scene->BBox().ZLength();
      else viewing_extent[RN_HI][RN_Z] -= 0.01 * scene->BBox().ZLength();
      if (R3Contains(viewing_extent, scene->BBox())) viewing_extent = R3null_box;
      break;

    default:
      redraw = 0;
      break;
    }
  }

  // Remember mouse position 
  mouse_position[0] = x;
  mouse_position[1] = y;

  // Remember modifiers 
  shift_down = shift;
  ctrl_down = ctrl;
  alt_down = alt;

  // Return whether need redraw
  return redraw;
}



int R3SurfelViewer::
Idle(void)
{
  // Return whether need redraw
  return FALSE;
}



////////////////////////////////////////////////////////////////////////
// COMMANDS
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
Initialize(void)
{
  // Initialize lights
  static GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  static GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  static GLfloat light0_position[] = { 0.0, 0.0, -1.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glEnable(GL_LIGHT0);
  static GLfloat light1_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
  static GLfloat light1_position[] = { 0.0, -1.0, 0.0, 0.0 };
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glEnable(GL_LIGHT1);
  glEnable(GL_NORMALIZE);

  // Initialize color settings
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  // Initialize graphics modes
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
}



void R3SurfelViewer::
Terminate(void)
{
}



void R3SurfelViewer::
ResetCamera(void)
{
  // Move center point to scene centroid
  if (!scene) return;
  SetCenterPoint(scene->Centroid());
  R3Point eye = CenterPoint() + 2 * scene->BBox().DiagonalRadius() * R3posz_vector;
  viewer.RepositionCamera(eye);
  viewer.ReorientCamera(R3negz_vector, R3posy_vector);
}



void R3SurfelViewer::
ZoomCamera(RNScalar scale)
{
  // Zoom into center point so that an area of radius scale is visible
  if (!scene) return;
  R3Point eye = center_point - scale * scene->BBox().DiagonalRadius() * viewer.Camera().Towards();
  viewer.RepositionCamera(eye);
}



void R3SurfelViewer::
SelectPoint(R3SurfelBlock *block, int surfel_index)
{
  // Remember previously selected point
  R3SurfelPoint *previous_selected_point = selected_point;

  // Select point
  selected_point = NULL;
  if (block && (surfel_index >= 0) && (surfel_index < block->NSurfels())) {
    selected_point = new R3SurfelPoint(block, surfel_index);
  }

  // Delete previously selected point
  if (previous_selected_point) delete previous_selected_point;
}



void R3SurfelViewer::
SelectImage(R3SurfelImage *image, RNBoolean update_working_set, RNBoolean jump_to_viewpoint)
{
  // Jump to viewpoint
  if (image && jump_to_viewpoint) {
    R3Camera camera(image->Viewpoint(), image->Towards(), image->Up(),
      image->XFOV(), image->YFOV(), viewer.Camera().Near(), viewer.Camera().Far());
    viewer.SetCamera(camera);
  }

  // Update working set
  if (image && update_working_set) {
    R3SurfelScan *scan = image->Scan();
    R3SurfelNode *node = (scan) ? scan->Node() : NULL;
    if (!node) {
      SetCenterPoint(image->Viewpoint() + 2.0 * image->Towards());
    }
    else {
      EmptyWorkingSet();
      center_point = node->Centroid();
      InsertIntoWorkingSet(node, TRUE);
    }
  }
  
  // Update image texture
  static R3SurfelImage *previous_image = NULL;
  if (image && (image != previous_image) && (image_plane_visibility || image_inset_visibility)) {
    previous_image = image;
    static R2Image color_image;
    color_image = image->ColorChannels();
    if (color_image.Depth() == 3) {
      if ((color_image.Width() == image->ImageWidth()) && (color_image.Height() == image->ImageHeight())) {
        current_image_texture.SetImage(&color_image);
      }
    }
  }

  // Remember selected image
  selected_image = image;
}



int R3SurfelViewer::
WriteImage(const char *filename)
{
  // Check if can write file
  FILE *fp = fopen(filename, "w");
  if (!fp) return 0;
  else fclose(fp);

  // Remember screenshot image name -- capture image next redraw
  if (screenshot_name) free(screenshot_name);
  if (!filename) screenshot_name = NULL;
  else screenshot_name = RNStrdup(filename);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Scene manipulation utility functions
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
SetScene(R3SurfelScene *scene)
{
  // Remember scene
  this->scene = scene;

  // Set center point
  center_point = scene->Centroid();
  // center_point[1] = scene->BBox().YMin();

  // Set focus radius
  focus_radius = 400;
  if (focus_radius > 0.5 * scene->BBox().DiagonalRadius()) {
    focus_radius = 0.5 * scene->BBox().DiagonalRadius();
  }

  // Set camera and viewport
  R3Box bbox = scene->BBox();
  RNLength r = bbox.DiagonalRadius();
  static const R3Vector up(0, 1, 0);
  static const R3Vector towards(0, 0, -1);
  R3Point eye = scene->Centroid() - towards * (2 * r); 
  R3Camera camera(eye, towards, up, 0.4, 0.4, 0.01, 100000.0);
  R2Viewport viewport(0, 0, window_width, window_height);
  viewer.SetViewport(viewport);
  viewer.SetCamera(camera);

  // Lock coarsest blocks in memory (~500MB)
  // ReadCoarsestBlocks(1 * 1024 * 1024);

  // Update working set
  UpdateWorkingSet();
}



////////////////////////////////////////////////////////////////////////
// Working set utility functions
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
ReadCoarsestBlocks(RNScalar max_complexity)
{
  // Just checking
  if (!scene) return;

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return;

  // Seed breadth first search with root nodes
  RNQueue<R3SurfelNode *> queue;
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    queue.Insert(node);
  }

  // Visit nodes in breadth first search reading blocks
  RNScalar total_complexity = 0;
  while (!queue.IsEmpty()) {
    R3SurfelNode *node = queue.Pop();
    if (total_complexity + node->Complexity() > max_complexity) break;
    total_complexity += node->Complexity();
    node->ReadBlocks();
    for (int i = 0; i < node->NParts(); i++) {
      R3SurfelNode *part = node->Part(i);
      queue.Push(part);
    }
  }

  // Invalidate VBO buffers
  InvalidateVBO();
}



void R3SurfelViewer::
ReleaseCoarsestBlocks(RNScalar max_complexity)
{
  // Just checking
  if (!scene) return;

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return;

  // Seed breadth first search with root nodes
  RNQueue<R3SurfelNode *> queue;
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    queue.Insert(node);
  }

  // Visit nodes in breadth first search reading blocks
  RNScalar total_complexity = 0;
  while (!queue.IsEmpty()) {
    R3SurfelNode *node = queue.Pop();
    if (total_complexity + node->Complexity() > max_complexity) break;
    total_complexity += node->Complexity();
    node->ReleaseBlocks();
    for (int i = 0; i < node->NParts(); i++) {
      R3SurfelNode *part = node->Part(i);
      queue.Push(part);
    }
  }

  // Invalidate VBO buffers
  InvalidateVBO();
}



void R3SurfelViewer::
EmptyWorkingSet(void)
{
  // Just checking
  if (!scene) return;

  // Release blocks from resident nodes
  resident_nodes.ReleaseBlocks();

  // Empty resident nodes
  resident_nodes.Empty();

  // Invalidate VBO buffers
  InvalidateVBO();
}



void R3SurfelViewer::
UpdateWorkingSet(const R3Point& center, RNScalar resolution, RNScalar radius)
{
  // Just checking
  if (!scene) return;

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return;

  // Check if already at full resolution
  if ((resolution == last_target_resolution) && (resolution == RN_INFINITY) && 
      (radius == last_focus_radius) && (radius == RN_INFINITY)) return;

  // Find new set of nodes
  R3SurfelNodeSet new_resident_nodes;
  new_resident_nodes.InsertNodes(tree, center, radius, -FLT_MAX, FLT_MAX, resolution, RN_EPSILON);

  // Read new working set
  new_resident_nodes.ReadBlocks();

  // Release old working set
  resident_nodes.ReleaseBlocks();

  // Now use newnodes 
  resident_nodes = new_resident_nodes;

  // Invalidate VBO buffers
  InvalidateVBO();

  // Remember parameters
  last_target_resolution = resolution;
  last_focus_radius = radius;
}



void R3SurfelViewer::
UpdateWorkingSet(const R3Viewer& view)
{
  // Just checking
  if (!scene) return;
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return;
 
  // Get convenient variables
  R3Point eye = view.Camera().Origin();
  R3Vector towards = view.Camera().Towards();
  R2Box viewport_box = view.Viewport().BBox();
  if (viewport_box.Area() == 0) return;

  // Allocate temporary memory
  int xres = viewport_box.XLength();
  int yres = int (xres * (double) viewport_box.YLength() / (double) viewport_box.XLength());
  int *visible_marks = new int [ tree->NNodes() ];
  int *resident_marks = new int [ tree->NNodes() ];
  int *item_buffer = new int [ xres * yres ];
  RNScalar *depth_buffer = new RNScalar [ xres * yres ];
  RNScalar *viewpoint_distance_buffer = new RNScalar [ xres * yres ];
  for (int i = 0; i < tree->NNodes(); i++) { resident_marks[i] = visible_marks[i] = 0; }
  for (int i = 0; i < resident_nodes.NNodes(); i++) resident_marks[resident_nodes.Node(i)->TreeIndex()] = 1;

  // Create viewer
  R3Viewer tmp_viewer(view);
  tmp_viewer.ResizeViewport(0, 0, xres, yres);

  RNBoolean done = FALSE;
  const int max_iterations = 8;
  for (int iteration = 0; iteration < max_iterations; iteration++) {
    // Check if done
    if (done) break;
    done = TRUE;

    // Initialize buffers
    for (int i = 0; i < xres * yres; i++) {
      item_buffer[i] = -1;
      depth_buffer[i] = FLT_MAX;
      viewpoint_distance_buffer[i] = FLT_MAX;
    }
    for (int i = 0; i < tree->NNodes(); i++) {
      visible_marks[i] = 0;
    }

    // Render surfels into item buffer
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        const R3Point& block_origin = block->PositionOrigin();
        for (int k = 0; k < block->NSurfels(); k++) {
          const R3Surfel *surfel = block->Surfel(k);
          double wx = surfel->X() + block_origin.X();
          double wy = surfel->Y() + block_origin.Y();
          double wz = surfel->Z() + block_origin.Z();
          R3Point wp(wx, wy, wz);
          RNScalar depth = towards.Dot(wp - eye);
          if (depth <= 0) continue; 
          R2Point vp = tmp_viewer.ViewportPoint(wp);
          if (vp == R2infinite_point) continue; 
          int vx = (int) (vp.X() + 0.5);
          if ((vx < 0) || (vx >= xres)) continue; 
          int vy = (int) (vp.Y() + 0.5);
          if ((vy < 0) || (vy >= yres)) continue; 
          int index = xres*vy + vx;
          if (RNIsGreater(depth, depth_buffer[index])) continue;
          RNLength viewpoint_distance = (node->Scan()) ? R3Distance(tmp_viewer.Camera().Origin(), node->Scan()->Viewpoint()) : RN_INFINITY;
          if (RNIsEqual(depth, depth_buffer[index], 1E-3) && RNIsGreater(viewpoint_distance, viewpoint_distance_buffer[index])) continue;
          item_buffer[index] = node->TreeIndex();
          depth_buffer[index] = depth;
          viewpoint_distance_buffer[index] = viewpoint_distance;
        }
      }
    }

    // Mark visible nodes
    for (int i = 0; i < xres * yres; i++) {
      int node_index = item_buffer[i];
      if (node_index < 0) continue;
      if (visible_marks[node_index] == 0) {
        visible_marks[node_index] = 1;
      }
    }

    // Remove invisible nodes from working set
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      assert(resident_marks[node->TreeIndex()]);
      if (visible_marks[node->TreeIndex()] == 1) continue;
      resident_marks[node->TreeIndex()] = 0;
      RemoveFromWorkingSet(node);
    }
    
    // Add parts of visible nodes to working set
    for (int i = 0; i < xres * yres; i++) {
      int node_index = item_buffer[i];
      if (node_index < 0) continue;
      assert(depth_buffer[i] < FLT_MAX);
      assert(visible_marks[node_index] == 1);
      if (resident_marks[node_index] == 0) continue;
      R3SurfelNode *node = tree->Node(node_index);
      if (node->NParts() == 0) continue;
      resident_marks[node_index] = 0;
      RemoveFromWorkingSet(node);
      for (int j = 0; j < node->NParts(); j++) {
        R3SurfelNode *part = node->Part(j);
        resident_marks[part->TreeIndex()] = 1;
        InsertIntoWorkingSet(part);
        done = FALSE;
      }
    }

    if (done) break;
  }

  // Delete temporary memory
  delete [] visible_marks;
  delete [] resident_marks;
  delete [] item_buffer;
  delete [] depth_buffer;
  delete [] viewpoint_distance_buffer;

  // Invalidate VBO buffers
  InvalidateVBO();
}



void R3SurfelViewer::
InsertIntoWorkingSet(R3SurfelNode *node, RNBoolean full_resolution)
{
  // Just checking
  if (!scene) return;

  // Recurse to children
  if (full_resolution) {
    if (node->NParts() == 0) {
      InsertIntoWorkingSet(node, FALSE);
    }
    else {
      for (int i = 0; i < node->NParts(); i++) {
        R3SurfelNode *part = node->Part(i);
        InsertIntoWorkingSet(part, full_resolution);
      }
    }
  }
  else {
    // Read blocks 
    node->ReadBlocks();

    // Insert into resident nodes
    resident_nodes.InsertNode(node);
  }
}



void R3SurfelViewer::
RemoveFromWorkingSet(R3SurfelNode *node, RNBoolean full_resolution)
{
  // Just checking
  if (!scene) return;

  // Recurse to children
  if (full_resolution) {
    if (node->NParts() == 0) {
      RemoveFromWorkingSet(node, FALSE);
    }
    else {
      for (int i = 0; i < node->NParts(); i++) {
        R3SurfelNode *part = node->Part(i);
        RemoveFromWorkingSet(part, full_resolution);
      }
    }
  }
  else {
    // Release blocks 
    node->ReleaseBlocks();

    // Remove from resident nodes
    resident_nodes.RemoveNode(node);
  }
}



void R3SurfelViewer::
RotateWorld(RNScalar factor, const R3Point& origin, int, int, int dx, int dy)
{
  // Rotate world based on mouse (dx)
  if ((dx == 0) && (dy == 0)) return;
  RNLength vx = (RNLength) dx / (RNLength) viewer.Viewport().Width();
  RNLength vy = (RNLength) dy / (RNLength) viewer.Viewport().Height();
  RNAngle theta = -1 * factor * 4.0 * vx;
  viewer.RotateWorld(origin, viewer.Camera().Up(), theta);
  RNAngle phi = factor * 4.0 * vy;
  // RNAngle max_phi = R3InteriorAngle(viewer.Camera().Towards(), R3posy_vector) - RN_PI_OVER_TWO;
  // RNAngle min_phi = -1.0 * R3InteriorAngle(viewer.Camera().Towards(), R3negz_vector);
  // if (phi < min_phi) phi = min_phi;
  // if (phi > max_phi) phi = max_phi;
  viewer.RotateWorld(origin, viewer.Camera().Right(), phi);
}



R3SurfelImage *R3SurfelViewer::
PickImage(int x, int y, R3Point *picked_position) 
{
  // How close the cursor has to be to a point (in pixels)
  int pick_tolerance = 0.01 * Viewport().Width() + 1;

  // Clear window 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set viewing transformation
  viewer.Camera().Load();

  // Set viewing extent
  EnableViewingExtent();

  // Set OpenGL stuff
  glLineWidth(pick_tolerance);    

  // Draw image viewpoints
  if (image_viewpoint_visibility) {
    glDisable(GL_LIGHTING);
    for (int i = 0; i < scene->NImages(); i++) {
      R3SurfelImage *image = scene->Image(i);
      unsigned char rgba[4];
      int image_index = i + 1;
      rgba[0] = (image_index >> 16) & 0xFF;
      rgba[1] = (image_index >> 8) & 0xFF;
      rgba[2] = image_index & 0xFF;
      rgba[3] = 0xFD;
      glColor4ubv(rgba);
      image->Draw(0);
    }
  }

  // Reset OpenGL stuff
  glLineWidth(1);
  glFinish();

  // Reset viewing modes
  DisableViewingExtent();

  // Read color buffer at cursor position
  unsigned char rgba[4];
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  if (rgba[3] == 0) return NULL;

  // Determine image index
  int r = rgba[0] & 0xFF;
  int g = rgba[1] & 0xFF;
  int b = rgba[2] & 0xFF;
  int a = rgba[3] & 0xFF;
  if (a != 0xFD) return NULL;
  int image_index = (r << 16) | (g << 8) | b;
  image_index--;

  // Determine image
  if (image_index < 0) return NULL;
  if (image_index >= scene->NImages()) return NULL;
  R3SurfelImage *picked_image = scene->Image(image_index);

  // Find hit position
  if (picked_position) {
    GLfloat depth;
    GLdouble p[3];
    GLint viewport[4];
    GLdouble modelview_matrix[16];
    GLdouble projection_matrix[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    gluUnProject(x, y, depth, modelview_matrix, projection_matrix, viewport, &(p[0]), &(p[1]), &(p[2]));
    R3Point position(p[0], p[1], p[2]);
    *picked_position = position;
  }

  // Return picked image
  return picked_image;
}




R3SurfelNode *R3SurfelViewer::
PickNode(int x, int y, R3Point *picked_position, 
  R3SurfelBlock **picked_block, int *picked_surfel_index,
  RNBoolean exclude_nonobjects) 
{
  // How close the cursor has to be to a point (in pixels)
  int pick_tolerance = 0.005 * Viewport().Width() + 1;

  // Clear window 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set viewing transformation
  viewer.Camera().Load();

  // Set viewing extent
  EnableViewingExtent();

  // Set OpenGL stuff
  glPointSize(pick_tolerance);    

#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  // Draw with VBO
  DrawVBO(R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX);
#else
  // Draw with glBegin ... glEnd
  for (int i = 0; i < resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = resident_nodes.Node(i);
    if (!NodeVisibility(node)) continue;

    // Set color
    unsigned char rgba[4] = { 0, 0, 0, 0xFE };
    CreateColor(rgba, R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX, NULL, NULL, node, NULL, NULL);
    glColor4ubv(rgba);

    // Draw node
    node->Draw(shape_draw_flags);
  }
#endif

  // Reset OpenGL stuff
  glPointSize(1);
  glFinish();

  // Reset viewing modes
  DisableViewingExtent();

  // Read color buffer at cursor position
  unsigned char rgba[4];
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  if (rgba[3] == 0) return NULL;

  // Determine node index
  int r = rgba[0] & 0xFF;
  int g = rgba[1] & 0xFF;
  int b = rgba[2] & 0xFF;
  // int a = rgba[3] & 0xFF;
  // if (a != 0xFE) return NULL;
  int node_index = (r << 16) | (g << 8) | b;
  if (node_index == 0) return NULL;
  node_index--;

  // Determine node
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return NULL;
  if (node_index < 0) return NULL;
  if (node_index >= tree->NNodes()) return NULL;
  R3SurfelNode *hit_node = tree->Node(node_index);

  // Find node part of an object
  R3SurfelNode *picked_node = hit_node;
  if (exclude_nonobjects) {
    // Find node associated with object
    picked_node = NULL;

    // Check if hit node is part of an object
    if (hit_node->Object()) {
      picked_node = hit_node;
    }

    // Check if hit node has ancestor that is part of an object
    if (picked_node == NULL) {
      R3SurfelNode *ancestor = hit_node->Parent();
      while (ancestor) {
        if (ancestor->Object()) { picked_node = ancestor; break; }
        ancestor = ancestor->Parent();
      }
    }
    
    // Check if hit node has descendent that is part of an object
    if (picked_node == NULL) {
      R3Ray ray = viewer.WorldRay(x, y);
      RNScalar t, picked_t = FLT_MAX;
      RNArray<R3SurfelNode *> stack;
      stack.Insert(hit_node);
      while (!stack.IsEmpty()) {
        R3SurfelNode *node = stack.Tail();
        stack.RemoveTail();
        for (int i = 0; i < node->NParts(); i++) {
          stack.Insert(node->Part(i));
        }
        if (node->Object()) {
          if (R3Intersects(ray, node->BBox(), NULL, NULL, &t)) {
            if (t < picked_t) {
              picked_node = node;
              picked_t = t;
            }
          }
        }
      }
    }
  }
    
  // Find hit position
  GLfloat depth;
  GLdouble p[3];
  GLint viewport[4];
  GLdouble modelview_matrix[16];
  GLdouble projection_matrix[16];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
  glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
  gluUnProject(x, y, depth, modelview_matrix, projection_matrix, viewport, &(p[0]), &(p[1]), &(p[2]));
  R3Point position(p[0], p[1], p[2]);
  if (picked_position) *picked_position = position;

  // Find hit surfel
  if (picked_block || picked_surfel_index) {
    // Create pointset in vicinity of picked position
    R3Point position(p[0], p[1], p[2]);
    RNScalar radius = 0.1 * R3Distance(position, viewer.Camera().Origin());
    R3SurfelSphereConstraint sphere_constraint(R3Sphere(position, radius));
    R3SurfelPointSet *pointset = CreatePointSet(scene, hit_node, &sphere_constraint);
    if (pointset) {
      // Find surfel point closest to picked position
      R3SurfelPoint *closest_point = NULL;
      RNLength closest_distance = FLT_MAX;
      for (int i = 0; i < pointset->NPoints(); i++) {
        R3SurfelPoint *point = pointset->Point(i);
        RNLength distance = R3SquaredDistance(point->Position(), position);
        if (distance < closest_distance) {
          closest_distance = distance;
          closest_point = point;
        }
      }

      // Return closest point
      if (closest_point) {
        if (picked_position) *picked_position = closest_point->Position();
        if (picked_block) *picked_block = closest_point->Block();
        if (picked_surfel_index) *picked_surfel_index = closest_point->BlockIndex();
      }

      // Delete point set
      delete pointset;
    }
  }

  // Return picked node
  return picked_node;
}



void R3SurfelViewer::
DrawObject(R3SurfelObject *object, RNFlags draw_flags) const
{
  // Draw nodes
  for (int i = 0; i < object->NNodes(); i++) {
    R3SurfelNode *node = object->Node(i);
    if (!node->DrawResidentDescendents(draw_flags)) {
      if (!node->DrawResidentAncestor(draw_flags)) {
        // const char *object_name = (object->Name()) ? object->Name() : "None";
        // RNFail("Did not draw object %s\n", object_name);
      }
    }
  }

  // Draw parts
  for (int i = 0; i < object->NParts(); i++) {
    R3SurfelObject *part = object->Part(i);
    DrawObject(part, draw_flags);
  }
}
  
  
  
////////////////////////////////////////////////////////////////////////
// OBJECT EDITING 
////////////////////////////////////////////////////////////////////////

int R3SurfelViewer::
SplitLeafNodes(R3SurfelNode *source_node, const R3SurfelConstraint& constraint, 
  RNArray<R3SurfelNode *> *nodesA, RNArray<R3SurfelNode *> *nodesB)
{
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  if (!source_node) source_node = tree->RootNode();
  if (!source_node) return 0;

  // Split leaf nodes, WHILE UPDATING NODES IN VIEWER'S RESIDENT SET
  int countA = 0;
  int countB = 0;
  RNArray<R3SurfelNode *> stack;
  stack.Insert(source_node);
  while (!stack.IsEmpty()) {
    R3SurfelNode *node = stack.Tail();
    stack.RemoveTail();
    if (node->NParts() == 0) {
      // Check if node is resident in working set
      int resident_index = resident_nodes.NodeIndex(node);
    
      // Split leaf node
      RNArray<R3SurfelNode *> partsA, partsB;
      if (tree->SplitLeafNodes(node, constraint, &partsA, &partsB)) {
        // Update resident set
        if (resident_index > 0) {
          node->ReleaseBlocks(); // ???
          resident_nodes.RemoveNode(resident_index);
          for (int j = 0; j < partsA.NEntries(); j++) {
            R3SurfelNode *partA = partsA.Kth(j);
            resident_nodes.InsertNode(partA);
          }
          for (int j = 0; j < partsB.NEntries(); j++) {
            R3SurfelNode *partB = partsB.Kth(j);
            resident_nodes.InsertNode(partB);
          }
        }
      }

      // Insert parts into result
      if (nodesA) nodesA->Append(partsA);
      if (nodesB) nodesB->Append(partsB);
      countA += partsA.NEntries();
      countB += partsB.NEntries();
    }
    else {
      for (int i = 0; i < node->NParts(); i++) {
        R3SurfelNode *part = node->Part(i);
        stack.Insert(part);
      }
    } 
  }

  // Invalidate VBO buffers
  InvalidateVBO();

  // Return status
  if (countA == 0) return 0;
  if (countB == 0) return 0;
  return 1;
}



////////////////////////////////////////////////////////////////////////
// VERTEX BUFFER OBJECT UPDATES
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
InvalidateVBO(void)
{
  // Ensure VBO is updated
  vbo_nsurfels = 0;
}



void R3SurfelViewer::
UpdateVBO(void)
{
#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  // Check if VBO is uptodate
  if (vbo_nsurfels > 0) return;

  // Count surfels
  vbo_nsurfels = 0;
  for (int i = 0; i < resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = resident_nodes.Node(i);
    if (!NodeVisibility(node)) continue;
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      vbo_nsurfels += block->NSurfels() / subsampling_factor;
      if (block->NSurfels() % subsampling_factor > 0) vbo_nsurfels++;
    }
  }

  // Allocate in-memory buffers
  GLfloat *surfel_positions = new GLfloat [ 3 * vbo_nsurfels ];
  GLfloat *surfel_normals = new GLfloat [ 3 * vbo_nsurfels ];
  GLubyte *surfel_colors = new GLubyte [  2 * 3 * vbo_nsurfels ];

  // Fill in-memory buffers 
  if (surfel_positions && surfel_normals && surfel_colors) {
    GLfloat *surfel_positionsp = surfel_positions;
    GLfloat *surfel_normalsp = surfel_normals;
    GLubyte *surfel_colorsp = surfel_colors;
    unsigned int pick_color_offset = 3 * vbo_nsurfels;
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      if (!NodeVisibility(node)) continue;
      R3SurfelObject *object = node->Object();
      while (object && object->Parent() && (object->Parent() != scene->RootObject())) object = object->Parent();
      R3SurfelLabel *label = (object) ? object->CurrentLabel() : NULL;
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        const R3Point& block_origin = block->PositionOrigin();
        for (int k = 0; k < block->NSurfels(); k += subsampling_factor) {
          const R3Surfel *surfel = block->Surfel(k);
          assert(surfel_positionsp - surfel_positions < 3*vbo_nsurfels);
          assert(surfel_normalsp - surfel_normals < 3*vbo_nsurfels);
          assert(surfel_colorsp - surfel_colors < 3*vbo_nsurfels);
          *surfel_positionsp++ = block_origin.X() + surfel->X();
          *surfel_positionsp++ = block_origin.Y() + surfel->Y();
          *surfel_positionsp++ = block_origin.Z() + surfel->Z();
          *surfel_normalsp++ = surfel->NX();
          *surfel_normalsp++ = surfel->NY();
          *surfel_normalsp++ = surfel->NZ();
          CreateColor(surfel_colorsp, surfel_color_scheme,
            surfel, block, node, object, label);
          CreateColor(surfel_colorsp + pick_color_offset, 
            R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX,
            surfel, block, node, object, label);
          surfel_colorsp += 3;
        }
      }
    }
  
    // Just checking
    assert(surfel_positionsp - surfel_positions == 3*vbo_nsurfels);
    assert(surfel_normalsp - surfel_normals == 3*vbo_nsurfels);
    assert(surfel_colorsp - surfel_colors == 3*vbo_nsurfels);

    // Generate VBO buffers (first time only)
    if (vbo_position_buffer == 0) glGenBuffers(1, &vbo_position_buffer);
    if (vbo_normal_buffer == 0) glGenBuffers(1, &vbo_normal_buffer);
    if (vbo_color_buffer == 0) glGenBuffers(1, &vbo_color_buffer);

    // Load VBO buffers
    if (vbo_position_buffer && surfel_positions) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_position_buffer);
      glBufferData(GL_ARRAY_BUFFER, 3 * vbo_nsurfels * sizeof(GLfloat), surfel_positions, GL_STATIC_DRAW);
      glVertexPointer(3, GL_FLOAT, 0, 0);
    }
    if (vbo_normal_buffer && surfel_normals) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_buffer);
      glBufferData(GL_ARRAY_BUFFER, 3 * vbo_nsurfels * sizeof(GLfloat), surfel_normals, GL_STATIC_DRAW);
      glNormalPointer(GL_FLOAT, 0, 0);
    }
    if (vbo_color_buffer && surfel_colors) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_buffer);
      glBufferData(GL_ARRAY_BUFFER, 2 * 3 * vbo_nsurfels * sizeof(GLubyte), surfel_colors, GL_STATIC_DRAW);
      glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);
    }
  }

  // Delete in-memory buffers
  if (surfel_positions) delete [] surfel_positions;
  if (surfel_normals) delete [] surfel_normals;
  if (surfel_colors) delete [] surfel_colors;
#endif
}



void R3SurfelViewer::
DrawVBO(int color_scheme)
{
#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  // Update VBO
  UpdateVBO();

  // Check VBO
  if (vbo_nsurfels == 0) return;

  // Enable position buffer
  if (vbo_position_buffer > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
  }

  // Enable color/normal buffers
  if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX) {
    // Enable color buffer to draw pick colors 
    if (vbo_color_buffer > 0) {
      // Set color pointer to second half of buffer where pick colors are
      void *pick_color_offset = (void *) (3 * vbo_nsurfels * sizeof(GLubyte));
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_buffer);
      glColorPointer(3, GL_UNSIGNED_BYTE, 0, pick_color_offset);
      glEnableClientState(GL_COLOR_ARRAY);
    }

    // Draw points with pick color
    glDrawArrays(GL_POINTS, 0, vbo_nsurfels);

    // Reset color pointer to start of buffer where regular colors are
    if (vbo_color_buffer > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_buffer);
      glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);
    }
  }
  else if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_SHADING) {
    // Enable normal buffer
    if (vbo_normal_buffer > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_buffer);
      glEnableClientState(GL_NORMAL_ARRAY);
    }

    // Draw points with shading
    glEnable(GL_LIGHTING);
    glColor3d(0.8, 0.8, 0.8);
    glDrawArrays(GL_POINTS, 0, vbo_nsurfels);
    glDisable(GL_LIGHTING);
  }
  else {
    // Enable color buffer
    if (vbo_color_buffer > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_buffer);
      glEnableClientState(GL_COLOR_ARRAY);
    }

    // Draw points with color
    glDrawArrays(GL_POINTS, 0, vbo_nsurfels);
  }

  // Disable client state
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
#endif
}



} // namespace gaps
