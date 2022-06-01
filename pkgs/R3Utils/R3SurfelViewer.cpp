/* Source file for the surfel scene viewer class */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils.h"



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
    elevation_range(FLT_MAX, -FLT_MAX),
    viewing_frustum(),
    center_point(0,0,0),
    selected_point(NULL),
    selected_image(NULL),
    image_inset_size(0.2),
    image_plane_depth(2),
    surfel_size(2),
    surfel_visibility(1),
    normal_visibility(0),
    backfacing_visibility(1),
    aerial_visibility(1),
    terrestrial_visibility(1),
    human_labeled_object_visibility(1),
    object_principal_axes_visibility(0),
    object_oriented_bbox_visibility(0),
    object_relationship_visibility(0),
    object_bbox_visibility(0),
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
    attribute_visibility_flags(0xFFFFFFFF),
    surfel_color_scheme(R3_SURFEL_VIEWER_COLOR_BY_RGB),
    normal_color(0,1,0),
    background_color(0,0,0),
    object_principal_axes_color(0,1,1),
    object_oriented_bbox_color(0.75,0.0,0.75),
    object_bbox_color(0,1,1),
    node_bbox_color(0,0,1),
    block_bbox_color(0,1,0),
    scan_viewpoint_color(0,1,1),
    image_viewpoint_color(0,1,1),
    center_point_color(0,0,0.5),
    shape_draw_flags(0),
    adapt_working_set_automatically(0),
    target_resolution(100),
    last_target_resolution(0),
    focus_radius(0),
    last_focus_radius(0),
    adapt_subsampling_automatically(0),
    subsampling_factor(1),
    subsampling_multiplier_when_mouse_down(1),
    window_height(0),
    window_width(0),
    shift_down(0),
    ctrl_down(0),
    alt_down(0),
    start_timer(),
    frame_timer(),
    frame_time(-1),
    screenshot_name(NULL),
    ground_z_grid(),
    selected_image_color_pixels(),
    previous_color_image(NULL),
    selected_image_texture_id(0),
    previous_texture_image(NULL),
    vbo_position_buffer(0),
    vbo_normal_buffer(0),
    vbo_color_buffer(0),
    vbo_nsurfels(0),
    shader_program(0),
    vertex_shader(0),
    fragment_shader(0)
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
  // Delete selected image texture
  if (selected_image_texture_id > 0) glDeleteTextures(1, &selected_image_texture_id);

#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  // Delete VBO buffers
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
  case R3_SURFEL_VIEWER_COLOR_BY_OBJECT_ATTRIBUTES: return "Attributes";
  case R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX: return "Pick Index";
  default: return "Unknown";
  }
}


 
int R3SurfelViewer::
ObjectVisibility(R3SurfelObject *object) const
{
  // Check if any part is visible
  for (int i = 0; i < object->NParts(); i++) {
    R3SurfelObject *part = object->Part(i);
    if (ObjectVisibility(part)) return 1;
  }

  // Check if any node is visible
  for (int i = 0; i < object->NNodes(); i++) {
    R3SurfelNode *node = object->Node(i);
    if (NodeVisibility(node)) return 1;
  }

  // Not visible
  return 0;
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
    R3SurfelObject *object = node->Object(TRUE, TRUE);
    while (object) {
      if (object->HumanLabel()) return 0;
      object = object->Parent();
    }
  }

  // Check if label is visible
  R3SurfelObject *object = node->Object(TRUE, TRUE);
  while (object && object->Parent() && (object->Parent() != scene->RootObject())) object = object->Parent();
  if (object) {
    if (!AttributeVisibility(object->Flags())) return 0;
    R3SurfelLabel *label = object->CurrentLabel();
    if ((label) && !LabelVisibility(label)) return 0;
  }

  // Check elevation range
  if (!elevation_range.IsEmpty()) {
    RNInterval node_range = node->ElevationRange();
    if (!elevation_range.Contains(node_range)) return 0;
  }

  // Check viewing extent
  if (!viewing_extent.IsEmpty()) {
    if (!R3Intersects(viewing_extent, node->BBox())) return 0;
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


  
int R3SurfelViewer::
AttributeVisibility(RNFlags attribute_flags) const
{
  // Temporary hack
  if (attribute_flags == 0) attribute_flags.Add(R3_SURFEL_OBJECT_NO_ATTRIBUTES_FLAG);

  // Return whether attribute is visible
  return attribute_visibility_flags.Intersects(attribute_flags);
}


  
void R3SurfelViewer::
SetAttributeVisibility(RNFlags attribute_flags, int visibility)
{
  // Check if changing all flags
  if (attribute_flags == 0) attribute_flags = 0xFFFFFFFF;

  // Update attribute visibility flags
  if (visibility == -1) attribute_visibility_flags.XOR(attribute_flags);
  else if (!visibility) attribute_visibility_flags.Remove(attribute_flags);
  else attribute_visibility_flags.Add(attribute_flags);

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
    if (!label && object) label = object->CurrentLabel();
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
      
  case R3_SURFEL_VIEWER_COLOR_BY_NORMAL:
    if (surfel) {
      *color++ = 128 + 127 * surfel->NX();
      *color++ = 128 + 127 * surfel->NY();
      *color++ = 128 + 127 * surfel->NZ();
    }
    else {
      *color++ = 128;
      *color++ = 128;
      *color++ = 128;
    }
    break;

  case R3_SURFEL_VIEWER_COLOR_BY_Z: {
    double z = 0;
    if (block && surfel) z = block->PositionOrigin().Z() + surfel->Z();
    else if (node) z = node->Centroid().Z();
    else if (object) z = object->Centroid().Z();
    double dz = (z > center_point.Z()) ? z - center_point.Z() : 0;
    double value = 0.5 * sqrt(dz);
    CreateColor(color, value);
    break; }
      
  case R3_SURFEL_VIEWER_COLOR_BY_ELEVATION: {
    double elevation = 1;
    if (surfel) elevation = surfel->Elevation();
    double value = (elevation > 0) ? 0.5 * sqrt(elevation) : 0;
    CreateColor(color, value);
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_SURFEL_LABEL: {
    int label_identifier = 0;
    if (surfel) label_identifier = surfel->Attribute() & 0xFF;
    else if (label) label_identifier = label->Identifier();
    CreateColor(color, label_identifier);
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_CONFIDENCE: {
    double confidence = 0.5;
    if (surfel) confidence = ((surfel->Attribute() >> 8) & 0xFF) / 255.0;
    CreateColor(color, confidence);
    break; }

  case R3_SURFEL_VIEWER_COLOR_BY_OBJECT_ATTRIBUTES: {
    RNFlags flags = (object) ? object->Flags() : RNFlags(0);
    CreateColor(color, (int) flags);
    break; }
      
  default:
    if (surfel) {
      *color++ = surfel->R();
      *color++ = surfel->G();
      *color++ = surfel->B();
    }
    else {
      *color++ = 127;
      *color++ = 127;
      *color++ = 127;
    }
    break;
  }
}



void R3SurfelViewer::
LoadColor(int k) const
{
  // Load color
  unsigned char color[3];
  CreateColor(color, k);
  RNLoadRgb(color);
}



 
void R3SurfelViewer::
LoadColor(double value) const
{
  // Load color
  unsigned char color[3];
  CreateColor(color, value);
  RNLoadRgb(color);
}



 
void R3SurfelViewer::
LoadColor(int color_scheme,
  const R3Surfel *surfel, R3SurfelBlock *block, R3SurfelNode *node,
  R3SurfelObject *object, R3SurfelLabel *label) const
{
  // Load color
  unsigned char color[3];
  CreateColor(color, color_scheme, surfel, block, node, object, label);
  RNLoadRgb(color);
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
  RNLoadRgb(0.5, 0.5, 0.5);
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



void R3SurfelViewer::
UpdateGroundZGrid(void)
{
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return;
 
  // Initialize grids
  R3Box b = scene->BBox();
  R2Box grid_box(b.XMin(), b.YMin(), b.XMax(), b.YMax());
  RNScalar spacing = grid_box.DiagonalRadius() / 128.0;
  if (spacing < 0.1) spacing = 0.1;
  ground_z_grid = R2Grid(grid_box, spacing, 5, 512);
  R2Grid weight_grid = ground_z_grid;

  // Fill grids
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    if (node->NParts() > 0) continue;
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      database->ReadBlock(block);
      for (int k = 0; k < block->NSurfels(); k++) {
        double elevation = block->SurfelElevation(k);
        R3Point sfl_position = block->SurfelPosition(k);
        double ground_z = sfl_position.Z() - elevation;
        R2Point world_position(sfl_position.X(), sfl_position.Y());
        R2Point grid_position = ground_z_grid.GridPosition(world_position);
        int ix = (int) (grid_position.X() + 0.5);
        if ((ix < 0) || (ix >= ground_z_grid.XResolution())) continue;
        int iy = (int) (grid_position.Y() + 0.5);
        if ((iy < 0) || (iy >= ground_z_grid.YResolution())) continue;
        ground_z_grid.AddGridValue(ix, iy, ground_z);
        weight_grid.AddGridValue(ix, iy, 1.0);
      }
      database->ReleaseBlock(block);
    }
  }

  // Compute average Z at ground
  ground_z_grid.Divide(weight_grid);

  // Fill holes
  ground_z_grid.FillHoles();
}



void R3SurfelViewer::
UpdateSelectedImageColorPixels(void)
{
  // Check stuff
  if (!selected_image) return;
  if (selected_image == previous_color_image) return;

  // Get color image
  selected_image_color_pixels = selected_image->ColorChannels();
  if ((selected_image_color_pixels.Width() <= 0) ||
      (selected_image_color_pixels.Height() <= 0) ||
      (selected_image_color_pixels.Depth() != 3)) {
    selected_image_color_pixels = R2Image();
  }

  // Remember last selected image
  previous_color_image = selected_image;
}



void R3SurfelViewer::
UpdateSelectedImageTexture(void)
{
  // Check stuff
  if (!selected_image) return;
  if (selected_image == previous_texture_image) return;

  // Get color image
  UpdateSelectedImageColorPixels();
  if (selected_image_color_pixels.Width() <= 0) return;
  if (selected_image_color_pixels.Height() <= 0) return;
  if (selected_image_color_pixels.Depth() != 3) return;

  // Generate a texture id
  if (selected_image_texture_id == 0) {
    glGenTextures(1, &selected_image_texture_id);
    if (selected_image_texture_id <= 0) return;
  }

  // Load texture
  if (selected_image_texture_id > 0) {
    glBindTexture(GL_TEXTURE_2D, selected_image_texture_id);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, selected_image_color_pixels.Width(), selected_image_color_pixels.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, selected_image_color_pixels.Pixels() );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, selected_image_color_pixels.Width(), selected_image_color_pixels.Height(), GL_RGB, GL_UNSIGNED_BYTE, selected_image_color_pixels.Pixels());
  }

  // Remember last selected image
  previous_texture_image = selected_image;
}



void R3SurfelViewer::
AdaptWorkingSet(void)
{
  // Check if adapting automatically
  if (!adapt_working_set_automatically) return;

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
DrawSurfels(int color_scheme) const
{
  // Check surfel visibility
  if (!SurfelVisibility()) return;

  // Set default lighting
  glDisable(GL_LIGHTING);

  // Set point size
  if (color_scheme != R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX) glPointSize(surfel_size);
  else glPointSize(0.01 * Viewport().Width() + 1);

#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  if (!shape_draw_flags[R3_SURFEL_DISC_DRAW_FLAG]) {
    // Draw with VBO
    DrawVBO(color_scheme);
    glPointSize(1);    
    return;
  }
#endif

  // Draw resident nodes
  switch (color_scheme) {
  case R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX:
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      LoadColor(color_scheme, NULL, NULL, node, NULL, NULL);
      DrawSurfels(node);
    }
    break;

  case R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL:
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      R3SurfelObject *object = node->Object(TRUE, TRUE);
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
      R3SurfelObject *object = node->Object(TRUE, TRUE);
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
      R3SurfelObject *object = node->Object(TRUE, TRUE);
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
    
  case R3_SURFEL_VIEWER_COLOR_BY_OBJECT_ATTRIBUTES:
    // Draw with colors based on flags encoding object attributes
    for (int i = 0; i < resident_nodes.NNodes(); i++) {
      R3SurfelNode *node = resident_nodes.Node(i);
      if (!NodeVisibility(node)) continue;
      R3SurfelObject *object = node->Object(TRUE, TRUE);
      while (object && object->Parent() && (object->Parent() != scene->RootObject())) object = object->Parent();
      RNFlags flags = (object) ? object->Flags() : RNFlags(0);
      LoadColor((int) flags);
      DrawSurfels(node);
    }
    break;
    
  case R3_SURFEL_VIEWER_COLOR_BY_SHADING:
    // Draw with colors based on shading
    glEnable(GL_LIGHTING);
    RNLoadRgb(0.8, 0.8, 0.8);
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
        RNGrfxBegin(RN_GRFX_POINTS);
        for (int k = 0; k < block->NSurfels(); k += subsampling_factor) {
          const R3Surfel *surfel = block->Surfel(k);
          float r = 0.5F + 0.5F * surfel->NX();
          float g = 0.5F + 0.5F * surfel->NY();
          float b = 0.5F + 0.5F * surfel->NZ();
          RNLoadRgb(r, g, b);
          R3LoadPoint(surfel->PositionPtr());
        }
        RNGrfxEnd();
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
        RNGrfxBegin(RN_GRFX_POINTS);
        for (int k = 0; k < block->NSurfels(); k += subsampling_factor) {
          const R3Surfel *surfel = block->Surfel(k);
          if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_Z) {
            double z = block_origin.Z() + surfel->Z();
            double dz = (z > center_point.Z()) ? z - center_point.Z() : 0;
            double value = 0.5 * sqrt(dz);
            LoadColor(value);
          }
          else if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_ELEVATION) {
            double elevation = surfel->Elevation();
            double value = (elevation > 0) ? 0.5 * sqrt(elevation) : 0;
            LoadColor(value);
          }
          else if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_SURFEL_LABEL) {
            int label_identifier = surfel->Attribute() & 0xFF;
            LoadColor(label_identifier);
          }
          else if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_CONFIDENCE) {
            double confidence = ((surfel->Attribute() >> 8) & 0xFF) / 255.0;
            LoadColor(confidence);
          }
          R3LoadPoint(surfel->PositionPtr());
        }
        RNGrfxEnd();
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

  // Reset point size
  glPointSize(1);    
}



void R3SurfelViewer::
DrawNormals(void) const
{
  // Check stuff
  if (!scene) return;
  if (!normal_visibility) return;

  // Draw normals
  glDisable(GL_LIGHTING);
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
      RNGrfxBegin(RN_GRFX_LINES);
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
      RNGrfxEnd();
      glPopMatrix();
    }
  }
}



void R3SurfelViewer::
DrawObjectProperties(int property_type) const
{
  // Check stuff
  if (!scene) return;
  
  // Draw object properties
  for (int i = 0; i < scene->NObjectProperties(); i++) {
    R3SurfelObjectProperty *property = scene->ObjectProperty(i);
    if (property->Type() != property_type) continue;
    R3SurfelObject *object = property->Object();
    if (!object->Parent()) continue;
    if (object->Parent() != scene->RootObject()) continue;
    if (!ObjectVisibility(object)) continue;
    if (!object->HasSurfels(TRUE)) continue;
    if ((surfel_color_scheme == R3_SURFEL_VIEWER_COLOR_BY_OBJECT) ||
        (surfel_color_scheme == R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL) ||
        (surfel_color_scheme == R3_SURFEL_VIEWER_COLOR_BY_OBJECT_ATTRIBUTES)) {
      unsigned char color[3];
      CreateColor(color, surfel_color_scheme, NULL, NULL, NULL, object, NULL);
      color[0] += 0.5 * (255 - color[0]);
      color[1] += 0.5 * (255 - color[1]);
      color[2] += 0.5 * (255 - color[2]);
      RNLoadRgb(color);
    }
    property->Draw(0);
  }
}



void R3SurfelViewer::
DrawObjectPrincipalAxes(void) const
{
  // Check stuff
  if (!object_principal_axes_visibility) return;
  
  // Draw object principal axes
  glDisable(GL_LIGHTING);
  RNLoadRgb(object_principal_axes_color);
  DrawObjectProperties(R3_SURFEL_OBJECT_PCA_PROPERTY);
}



void R3SurfelViewer::
DrawObjectOrientedBBoxes(void) const
{
  // Check stuff
  if (!object_oriented_bbox_visibility) return;

  // Draw object oriented boxes
  glLineWidth(2);
  glDisable(GL_LIGHTING);
  RNLoadRgb(object_oriented_bbox_color);
  DrawObjectProperties(R3_SURFEL_OBJECT_AMODAL_OBB_PROPERTY);
  glLineWidth(1);
}



void R3SurfelViewer::
DrawObjectRelationships(void) const
{
  // Check stuff
  if (!scene) return;
  if (!object_relationship_visibility) return;
  
  // Draw object relationships
  glDisable(GL_LIGHTING);
  RNLoadRgb(1.0, 1.0, 1.0);
  for (int i = 0; i < scene->NObjectRelationships(); i++) {
    R3SurfelObjectRelationship *relationship = scene->ObjectRelationship(i);
    relationship->Draw();
  }
}



void R3SurfelViewer::
DrawObjectBBoxes(void) const
{
  // Check stuff
  if (!scene) return;
  if (!object_bbox_visibility) return;
  
  // Draw node bounding boxes
  glDisable(GL_LIGHTING);
  RNLoadRgb(object_bbox_color);
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (!object->Parent()) continue;
    if (object->Parent() != scene->RootObject()) continue;
    if ((object->NParts() == 0) && (object->NNodes() == 0)) continue;    
    object->BBox().Outline();
  }
}



void R3SurfelViewer::
DrawNodeBBoxes(void) const
{
  // Check stuff
  if (!scene) return;
  if (!node_bbox_visibility) return;
  
  // Draw node bounding boxes
  glDisable(GL_LIGHTING);
  RNLoadRgb(node_bbox_color);
  for (int i = 0; i < resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = resident_nodes.Node(i);
    if (node->NParts() > 0) RNLoadRgb(0, 1, 0);
    else RNLoadRgb(1, 0, 0);
    node->BBox().Outline();
  }
}



void R3SurfelViewer::
DrawBlockBBoxes(void) const
{
  // Check stuff
  if (!scene) return;
  if (!block_bbox_visibility) return;

  // Draw block bounding boxes
  glDisable(GL_LIGHTING);
  RNLoadRgb(block_bbox_color);
  for (int i = 0; i < resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = resident_nodes.Node(i);
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      block->BBox().Outline();
    }
  }
}



void R3SurfelViewer::
DrawCenterPoint(void) const
{
  // Check stuff
  if (!center_point_visibility) return;
  
  // Draw center point
  glEnable(GL_LIGHTING);
  RNLoadRgb(center_point_color);
  R3Sphere(center_point, 0.1).Draw();
  glDisable(GL_LIGHTING);
}



void R3SurfelViewer::
DrawSelectedPoint(void) const
{
  // Check stuff
  if (!selected_point) return;
  // if (!selected_point_visibility) return;

  // Draw selected point
  glEnable(GL_LIGHTING);
  RNLoadRgb(RNred_rgb);
  R3Sphere(selected_point->Position(), 0.05).Draw();
  glDisable(GL_LIGHTING);
}



void R3SurfelViewer::
DrawScanViewpoints(void) const
{
  // Check stuff
  if (!scene) return;
  if (!scan_viewpoint_visibility) return;
  
  // Draw scan viewpoints
  glDisable(GL_LIGHTING);
  RNLoadRgb(scan_viewpoint_color);
  RNGrfxBegin(RN_GRFX_LINES);
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
  RNGrfxEnd();
}



void R3SurfelViewer::
DrawImageViewpoints(void) const
{
  // Check stuff
  if (!scene) return;
  if (!image_viewpoint_visibility) return;
  
  // Draw image viewpoints
  glDisable(GL_LIGHTING);
  RNLoadRgb(image_viewpoint_color);
  for (int i = 0; i < scene->NImages(); i++) {
    R3SurfelImage *image = scene->Image(i);
    if (image == selected_image) glLineWidth(5);
    image->Draw();
    if (image == selected_image) glLineWidth(1);
  }
}



void R3SurfelViewer::
DrawImageInset(void) const
{
  // Get/check stuff
  if (!selected_image) return;
  if (selected_image->ImageWidth() <= 0) return;
  if (selected_image->ImageHeight() <= 0) return;
  if (!image_inset_visibility) return;
  if (image_inset_size <= 0) return;
  
  // Get/check color image
  ((R3SurfelViewer *) this)->UpdateSelectedImageColorPixels();
  int color_width = selected_image_color_pixels.Width();
  int color_height = selected_image_color_pixels.Height();
  if ((color_width <= 0) || (color_height <= 0)) return;
  const unsigned char *color_pixels = selected_image_color_pixels.Pixels();
  if (!color_pixels) return;

  // Determine image coordinates
  int w = viewer.Viewport().Width();
  int h = viewer.Viewport().Height();
  double x2 = w;
  double y2 = h;
  double aspect = (double) selected_image->ImageHeight() / (double) selected_image->ImageWidth();
  double x1 = x2 - image_inset_size * w;
  double y1 = y2 - image_inset_size * w * aspect;

  // Determine focal point
  R2Point focal_point(0.5*selected_image->ImageWidth(), 0.5*selected_image->ImageHeight());
  if (selected_point) {
    focal_point = selected_image->ImagePosition(selected_point->Position());
    if ((focal_point.X() >= 0) && (focal_point.Y() >= 0) &&
        (focal_point.X() <= selected_image->ImageWidth()-0.5) &&
        (focal_point.Y() <= selected_image->ImageHeight()-0.5)) {
      focal_point[0] = x1 + (x2 - x1) * focal_point.X()/selected_image->ImageWidth();
      focal_point[1] = y1 + (y2 - y1) * focal_point.Y()/selected_image->ImageHeight();
    }
  }

  // Push ortho viewing matrices
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, w, 0, h, 0.1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Get image translation (so that focal point is in view)
  double tx = 0;
  double ty = 0;
  if (x1 < 0) {
    if (focal_point.X() < 0.25*w) {
      tx = 0.25*w - focal_point.X();
    }
  }
  if (y1 < 0) {
    if (focal_point.Y() < 0.25*h) {
      ty = 0.25*h - focal_point.Y();
    }
  }

#if (RN_OS == RN_MAC)
  // For some unknown reason, drawing a textured quad hangs the GPU for some images
  // (very rarely, but it is repeatable).   So, draw the with glPixelDraw instead.
  
  // Set viewport 
  glViewport(x1, y1, x2 - x1, y2 - y1);

  // Set image scale 
  GLfloat sx = (x2 - x1) / color_width;
  GLfloat sy = (y2 - y1) / color_height;
  glPixelZoom(sx, sy);

  // Set image translation
  // Can't send point outside viewport to glRasterPos, so add translation this way
  glRasterPos3d(0, 0, -0.5);
  glBitmap(0, 0, 0, 0, tx, ty, NULL);
  
  // Draw image
  glDrawPixels(color_width, color_height, GL_RGB, GL_UNSIGNED_BYTE, color_pixels);

  // Reset everything
  glPixelZoom(1, 1);
  glViewport(0, 0, w, h);
#else
  // Update selected image texture
  ((R3SurfelViewer *) this)->UpdateSelectedImageTexture();

  // Draw image as textured quad
  if (selected_image_texture_id > 0) {
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, selected_image_texture_id);
    glEnable(GL_TEXTURE_2D);

    // Draw quad
    glDisable(GL_LIGHTING);
    RNLoadRgb(1.0, 1.0, 1.0);
    RNGrfxBegin(RN_GRFX_QUADS);
    R3LoadTextureCoords(0.0, 0.0);
    R3LoadPoint(x1 + tx, y1 + ty, -0.5);
    R3LoadTextureCoords(1.0, 0.0);
    R3LoadPoint(x2 + tx, y1 + ty, -0.5);
    R3LoadTextureCoords(1.0, 1.0);
    R3LoadPoint(x2 + tx, y2 + ty, -0.5);
    R3LoadTextureCoords(0.0, 1.0);
    R3LoadPoint(x1 + tx, y2 + ty, -0.5);
    RNGrfxEnd();

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }
#endif
  
  // Draw projected center point
  if (selected_point) {
    RNLoadRgb(1.0, 0.0, 0.0);
    R3Point p(focal_point.X() + tx, focal_point.Y() + ty, -0.25);
    R3Box(p[0]-4, p[1]-4, -0.25, p[0]+4, p[1]+4, -0.25).Draw();
  }

  // Pop ortho viewing matrices
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}



void R3SurfelViewer::
DrawImagePlane(void) const
{
  // Get/check stuff
  if (!selected_image) return;
  if (selected_image->ImageWidth() <= 0) return;
  if (selected_image->ImageHeight() <= 0) return;
  if (!image_plane_visibility) return;

#if (RN_OS != RN_MAC)
  // For some unknown reason, drawing a textured quad hangs the GPU for some images
  // (very rarely, but it is repeatable). 

  // Get convenient variables
  RNLength depth = image_plane_depth;
  R3SurfelImage *image = selected_image;
  R3Point c = image->Viewpoint() + depth * image->Towards();
  c -= ((image->ImageCenter().X() - 0.5*image->ImageWidth()) / (0.5*image->ImageWidth())) * image->Right() * depth;
  c -= ((image->ImageCenter().Y() - 0.5*image->ImageHeight()) / (0.5*image->ImageHeight())) * image->Up() * depth;
  R3Vector dx = depth * tan(image->XFOV()) * image->Right();
  R3Vector dy = depth * tan(image->YFOV()) * image->Up();

  // Update selected image texture
  ((R3SurfelViewer *) this)->UpdateSelectedImageTexture();

  // Draw image as textured quad
  if (selected_image_texture_id > 0) {
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, selected_image_texture_id);
    glEnable(GL_TEXTURE_2D);

    // Draw quad
    glDisable(GL_LIGHTING);
    RNLoadRgb(1.0, 1.0, 1.0);
    RNGrfxBegin(RN_GRFX_QUADS);
    R3LoadTextureCoords(0.0, 0.0);
    R3LoadPoint(c - dx - dy);
    R3LoadTextureCoords(1.0, 0.0);
    R3LoadPoint(c + dx - dy);
    R3LoadTextureCoords(1.0, 1.0);
    R3LoadPoint(c + dx + dy);
    R3LoadTextureCoords(0.0, 1.0);
    R3LoadPoint(c - dx + dy);
    RNGrfxEnd();

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }
#endif
}



void R3SurfelViewer::
DrawImagePoints(void) const
{
  // Get/check stuff
  if (!selected_image) return;
  if (selected_image->ImageWidth() <= 0) return;
  if (selected_image->ImageHeight()<= 0) return;
  if (!image_points_visibility) return;

  // Get depth and color channels
  const R2Grid *depth_channel = selected_image->DepthChannel();
  if (!depth_channel) return;
  if (depth_channel->XResolution() <= 0) return;
  if (depth_channel->YResolution() <= 0) return;
  const R2Image color_channels = selected_image->ColorChannels();
  if (color_channels.Width() <= 0) return;
  if (color_channels.Height() <= 0) return;

  // Draw image points
  glDisable(GL_LIGHTING);
  glPointSize(surfel_size);
  RNGrfxBegin(RN_GRFX_POINTS);
  for (int iy = 0; iy < selected_image->ImageHeight(); iy++) {
    for (int ix = 0; ix < selected_image->ImageWidth(); ix++) {
      RNScalar depth = depth_channel->GridValue(ix, iy);
      if (RNIsZero(depth) || (depth == R2_GRID_UNKNOWN_VALUE)) return;
      R2Point image_position(ix, iy);
      R3Point world_position = selected_image->TransformFromImageToWorld(image_position);
      RNLoadRgb(color_channels.PixelRGB(ix, iy));
      R3LoadPoint(world_position);
    }
  }
  RNGrfxEnd();
  glPointSize(1);
}



void R3SurfelViewer::
DrawAxes(void) const
{
  // Check stuff
  if (!axes_visibility) return;

  // Draw axes
  RNScalar d = 1.0;
  glDisable(GL_LIGHTING);
  glLineWidth(3);
  R3BeginLine();
  RNLoadRgb(1, 0, 0);
  R3LoadPoint(R3zero_point);
  R3LoadPoint(R3zero_point + d * R3posx_vector);
  R3EndLine();
  R3BeginLine();
  RNLoadRgb(0, 1, 0);
  R3LoadPoint(R3zero_point);
  R3LoadPoint(R3zero_point + d * R3posy_vector);
  R3EndLine();
  R3BeginLine();
  RNLoadRgb(0, 0, 1);
  R3LoadPoint(R3zero_point);
  R3LoadPoint(R3zero_point + d * R3posz_vector);
  R3EndLine();
  glLineWidth(1);
}



////////////////////////////////////////////////////////////////////////
// Screen capture functions
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
CaptureImage(const char *filename) const
{
  // Check stuff
  if (!filename) return;

  // Capture iamge
  R2Image image(viewer.Viewport().Width(), viewer.Viewport().Height(), 3);
  image.Capture();

  // Wrimte to file
  image.Write(filename);
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

  // Set opengl modes
  if (backfacing_visibility) glDisable(GL_CULL_FACE);
  else glEnable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glPointSize(1);
  glLineWidth(1);

  // Draw everything (without culling to viewing extent)
  DrawSelectedPoint();
  DrawScanViewpoints();
  DrawImageViewpoints();
  DrawImagePlane();
  DrawImageInset();
  DrawAxes();

  // Set viewing extent
  DrawViewingExtent();
  EnableViewingExtent();
  UpdateViewingFrustum();

  // Draw everything else (with culling to viewing extent)
  DrawSurfels(surfel_color_scheme);
  DrawNormals();
  DrawObjectPrincipalAxes();
  DrawObjectOrientedBBoxes();
  DrawObjectRelationships();
  DrawObjectBBoxes();
  DrawNodeBBoxes();
  DrawBlockBBoxes();
  DrawImagePoints();
  DrawCenterPoint();

  // Reset viewing modes
  DisableViewingExtent();

  // Capture image
  if (screenshot_name) {
    CaptureImage(screenshot_name);
    free(screenshot_name);
    screenshot_name = NULL;
  }

  // Update the frame time
  if (frame_time < 0) frame_time = 0.05;
  else frame_time = frame_timer.Elapsed();
  frame_timer.Read();

  // Adapt working set
  AdaptWorkingSet();

  // Check if there are any GL errors
  GLenum status = glGetError();
  if (status != GL_NO_ERROR) {
    printf("Error in OpenGL: %d\n", (int) status);
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
MouseButton(int x, int y, int button, int state, int shift, int ctrl, int alt, int update_center_point)
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
      if (button == 3) viewer.ScaleWorld(viewing_center_point, 1.1);
      else if (button == 4) viewer.ScaleWorld(viewing_center_point, 0.9);
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
    if (!drag && update_center_point) {
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

    // Redraw if was subsampling
    if (subsampling_multiplier_when_mouse_down > 1) redraw = TRUE;
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
      SetNodeBBoxVisibility(-1);
      break;

    case 'b':
      SetObjectOrientedBBoxVisibility(-1);
      break;

    case 'C':
      SetSurfelColorScheme((surfel_color_scheme + 1) % R3_SURFEL_VIEWER_NUM_COLOR_SCHEMES);
      break;

    case 'c':
      if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_RGB)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_ELEVATION);
      else if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_ELEVATION)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_OBJECT);
      else if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_OBJECT)
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
      if (ElevationRange().IsEmpty()) SetElevationRange(RNInterval(-FLT_MAX, 0.25));
      else SetElevationRange(RNnull_interval);
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
      
    case 'M':
    case 'm':
      subsampling_multiplier_when_mouse_down = (subsampling_multiplier_when_mouse_down <= 1) ? 8 : 1;
      break;
      
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
      SetObjectPrincipalAxesVisibility(-1);
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

    case ';':
      SetFocusRadius(0.9 * FocusRadius());
      break;
      
    case '\'':
      SetFocusRadius(1.1 * FocusRadius());
      break;

    case ':':
      SetTargetResolution(0.9 * TargetResolution());
      break;

    case '"':
      SetTargetResolution(1.1 * TargetResolution());
      break;

    case ',': 
    case '.':
      if (!ElevationRange().IsEmpty()) {
        RNInterval range = ElevationRange();
        if (key == ',') range.SetMax(0.9 * range.Max());
        else if (key == '.') range.SetMax(1.1 * range.Max());
        SetElevationRange(range);
      }
      break;
      
    case '{':
    case '}': {
      // Set billboard depth
      RNScalar factor = (key == '{') ? 0.8 : 1.2;
      SetImagePlaneDepth(factor * ImagePlaneDepth());
      break; }

    case '<':
    case '>': {
      // Select next/prev image
      int image_index = (selected_image) ? selected_image->SceneIndex() : 0;
      if (key == '>') image_index++;
      else if (key == '<') image_index--;
      if (image_index < 0) image_index = 0;
      if (image_index > scene->NImages()-1) image_index = scene->NImages()-1;
      SelectImage(scene->Image(image_index), FALSE, FALSE);
      break; }

    case '_': 
      SetSubsamplingFactor(SubsamplingFactor() / 2);
      break;

    case '+': 
      SetSubsamplingFactor(SubsamplingFactor() * 2);
      break;

    case '-': 
      SetSurfelSize(0.8 * SurfelSize());
      break;

    case '=': 
      SetSurfelSize(1.25 * SurfelSize());
      break;

    default:
      redraw = 0;
      break;
    }
  }
  else if (ctrl) {
    switch(key) {
    case 'M':
    case 'm': // Enter
      if (image_inset_size < 0.5) SetImageInsetSize(0.8);
      else SetImageInsetSize(0.2);
      break;

    case 'R':
    case 'r':
      // Reset camera
      ResetCamera();
      break;

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
      SetImageInsetSize(0.8 * ImageInsetSize());
      break;

    case R3_SURFEL_VIEWER_UP_KEY:
      SetImageInsetSize(1.25 * ImageInsetSize());
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

  // Initialize shaders
  CompileShaders();
}



void R3SurfelViewer::
Terminate(void)
{
  // Delete shaders
  DeleteShaders();
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
SelectPoint(R3SurfelObject *object)
{
  // Create pointset
  R3SurfelPointSet *pointset = object->PointSet(TRUE);
  if (!pointset) {
    SelectPoint(NULL, -1);
    return;
  }

  // Get object centroid
  R3Point centroid = object->Centroid();

  // Find point nearest object centroid
  RNLength best_dd = FLT_MAX;
  R3SurfelPoint *best_point = NULL;
  for (int i = 0; i < pointset->NPoints(); i++) {
    R3SurfelPoint *point = pointset->Point(i);
    RNLength dd = R3SquaredDistance(centroid, point->Position());
    if (dd < best_dd) {
      best_point = point;
      best_dd = dd;
    }
  }

  // Select point 
  R3SurfelBlock *block = (best_point) ? best_point->Block() : NULL;
  int surfel_index = (best_point) ? best_point->BlockIndex() : -1;
  SelectPoint(block, surfel_index);

  // Delete pointset
  delete pointset;
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
  if (focus_radius > scene->BBox().DiagonalRadius()) {
    focus_radius = scene->BBox().DiagonalRadius();
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

  // Update ground z grid
  // UpdateGroundZGrid();

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

  // Seed breadth first search with root node
  RNQueue<R3SurfelNode *> queue;
  queue.Insert(tree->RootNode());

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
  if ((resolution == last_target_resolution) && (resolution >= RN_INFINITY) && 
      (radius == last_focus_radius) && (radius >= RN_INFINITY)) return;

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
  RNAngle max_phi = R3InteriorAngle(viewer.Camera().Towards(), R3posz_vector) - RN_PI_OVER_TWO;
  if (phi > max_phi) phi = max_phi;
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
      RNLoadRgba(rgba);
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




R3SurfelObject *R3SurfelViewer::
PickObject(int x, int y, R3Point *picked_position)
{
  // Pick node
  R3SurfelNode *node = PickNode(x, y, picked_position, NULL, NULL, TRUE);
  if (!node) return NULL;

  // Get picked object
  R3SurfelObject *picked_object = node->Object(TRUE);
  if (!picked_object) return NULL;

  // Adjust picked object to be ancestor directly under root object
  while (picked_object && picked_object->Parent() && (picked_object->Parent() != scene->RootObject())) {
    picked_object = picked_object->Parent();
  }

  // Return picked object
  return picked_object;
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

  // Draw surfels
  DrawSurfels(R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX);

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
        if (resident_index >= 0) {
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
      R3SurfelObject *object = node->Object(TRUE, TRUE);
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
DrawVBO(int color_scheme) const
{
#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  // Update VBO
  ((R3SurfelViewer *) this)->UpdateVBO();

  // Check VBO
  if (vbo_nsurfels == 0) return;

  // Determine stride and point size
  int subsampling = 1;
  float pointSize = SurfelSize();
  if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX)
    pointSize = 0.01 * Viewport().Width() + 1;
  if (subsampling_multiplier_when_mouse_down > 1) {
    if (mouse_button[0] || mouse_button[1] || mouse_button[2]) {
      subsampling = subsampling_multiplier_when_mouse_down;
      pointSize *= sqrt(subsampling_multiplier_when_mouse_down);
      glPointSize(pointSize);
    }
  }

  // Assign shader and its variables
  if (shader_program > 0) {
    glUseProgram(shader_program);
    int pointSizeLocation = glGetUniformLocation(shader_program, "pointSize");
    glUniform1f(pointSizeLocation, pointSize);
  }

  // Enable position buffer
  if (vbo_position_buffer > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_buffer);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat) * subsampling, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
  }

  // Enable color/normal buffers
  if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX) {
    // Enable color buffer to draw pick colors 
    if (vbo_color_buffer > 0) {
      // Set color pointer to second half of buffer where pick colors are
      void *pick_color_offset = (void *) (3 * vbo_nsurfels * sizeof(GLubyte));
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_buffer);
      glColorPointer(3, GL_UNSIGNED_BYTE, 3 * sizeof(GLubyte) * subsampling, pick_color_offset);
      glEnableClientState(GL_COLOR_ARRAY);
    }

    // Draw points with pick color
    glDrawArrays(GL_POINTS, 0, vbo_nsurfels);

    // Reset color pointer to start of buffer where regular colors are
    if (vbo_color_buffer > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_buffer);
      glColorPointer(3, GL_UNSIGNED_BYTE,  3 * sizeof(GLubyte) * subsampling, 0);
    }
  }
  else if (color_scheme == R3_SURFEL_VIEWER_COLOR_BY_SHADING) {
    // Enable normal buffer
    if (vbo_normal_buffer > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_buffer);
      glNormalPointer(GL_FLOAT,  3 * sizeof(GLfloat) * subsampling, 0);
      glEnableClientState(GL_NORMAL_ARRAY);
    }

    // Draw points with shading
    glEnable(GL_LIGHTING);
    RNLoadRgb(0.8, 0.8, 0.8);
    glDrawArrays(GL_POINTS, 0, vbo_nsurfels);
    glDisable(GL_LIGHTING);
  }
  else {
    // Enable color buffer
    if (vbo_color_buffer > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_buffer);
      glColorPointer(3, GL_UNSIGNED_BYTE, 3 * sizeof(GLubyte) * subsampling, 0);
      glEnableClientState(GL_COLOR_ARRAY);
    }

    // Draw points with color
    glDrawArrays(GL_POINTS, 0, vbo_nsurfels);
  }

  // Disable client state
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  // Reset shader
  glUseProgram(0);
#endif
}



////////////////////////////////////////////////////////////////////////
// SHADERS
////////////////////////////////////////////////////////////////////////

void R3SurfelViewer::
CompileShaders(void)
{
  // Inline GLSL 1.1 vertex shader definition
  const GLchar* vertex_shader_source =
       "#version 110\n"
       "attribute vec3 inPosition;"
       "attribute vec3 inColor;"
       "uniform float pointSize;"
       "varying vec4 vertexColor;"
       "void main()"
       "{"
       "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
       "    vertexColor = gl_Color;"
       "    gl_PointSize = pointSize;" // Replicate the current functionality
       // Alternative: uncomment to divide by clip space w to scale based on distance.
       // If we do this, we also need to increase the pointSize to compensate:
       //"    gl_PointSize = pointSize / gl_Position.w;"
       "}";
                                                                                  // Inline GLSL 1.1 fragment shader definition
  const GLchar* fragment_shader_source =
       "#version 110\n"
       "varying vec4 vertexColor;"
       "void main()"
       "{"
       "        gl_FragColor = vertexColor;"
       "}";

  // Compile the vertex shader
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, 0);
  glCompileShader(vertex_shader);

  // Check if vertex shader is compiled
  int is_compiled = 0;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_compiled);
  if (!is_compiled) {
    int log_length = 0;
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
    char* log_contents = new char[log_length + 1];
    glGetShaderInfoLog(vertex_shader, log_length, &log_length, log_contents);
    RNFail("Error: Failed to compile the vertex shader: %s\n", log_contents);
    delete[] log_contents;
    DeleteShaders();
    return;
  }
                                                                                     
  // Compile the fragment shader
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, 0);
  glCompileShader(fragment_shader);

  // Check if fragment shader is compiled
  is_compiled = 0;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_compiled);
  if (!is_compiled) {
    int log_length = 0;
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
    char* log_contents = new char[log_length + 1];
    glGetShaderInfoLog(fragment_shader, log_length, &log_length, log_contents);
    RNFail("Error: Failed to compile the fragment shader: %s\n", log_contents);
    delete[] log_contents;
    DeleteShaders();
    return;
  }
                                                                                     
  // Link the shader program
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  // Check if shader is linked
  int is_linked = 0;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &is_linked);
  if (!is_linked) {
    int log_length = 0;
    glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);
    char* log_contents = new char[log_length + 1];
    glGetProgramInfoLog(shader_program, log_length, &log_length, log_contents);
    RNFail("Error: Failed to link the shader program: %s\n", log_contents);
    delete[] log_contents;
    DeleteShaders();
    return;
  }
}



void R3SurfelViewer::
DeleteShaders(void)
{
  // Delete shader program
  if (shader_program > 0) {
    glDetachShader(shader_program, vertex_shader);
    glDetachShader(shader_program, fragment_shader);
    glDeleteProgram(shader_program);
    shader_program = 0;
  }

  // Delete vertex shader
  if (vertex_shader > 0) {
    glDeleteShader(vertex_shader);
    vertex_shader = 0;
  }

  // Delete fragment shader
  if (fragment_shader > 0) {
    glDeleteShader(fragment_shader);
    fragment_shader = 0;
  }
}



} // namespace gaps
