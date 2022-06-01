/* Source file for the surfel scene labeler class */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// Surfel labeler constructor/destructor
////////////////////////////////////////////////////////////////////////

R3SurfelLabeler::
R3SurfelLabeler(R3SurfelScene *scene, const char *logging_filename)
  : R3SurfelViewer(scene),
    object_label_visibility(0),
    classifier(scene),
    classify_after_change_label(0),
    segmenter(scene),
    segment_after_change_label(0),
    obb_manipulator(),
    obb_manipulator_visibility(0),
    selection_objects(),
    selection_visibility(1),
    current_command(NULL),
    undo_stack(),
    undo_index(-1),
    logging_filename(NULL),
    logging_fp(NULL),
    message(NULL),
    message_visibility(1),
    status_visibility(1),
    command_menu_visibility(0),
    attribute_menu_visibility(1),
    attribute_menu_flags(),
    attribute_menu_names(),
    attribute_menu_item_width(190),
    attribute_menu_item_height(14),
    attribute_menu_font(RN_GRFX_BITMAP_HELVETICA_12),
    label_menu_visibility(1),
    label_menu_list(),
    label_menu_item_width(190),
    label_menu_item_height(14),
    label_menu_font(RN_GRFX_BITMAP_HELVETICA_12),
    snapshot_directory(NULL),
    object_selection_times()
{
  // Get/create unknown label
  R3SurfelLabel *unknown_label = scene->FindLabelByName("Unknown");
  if (!unknown_label) {
    unknown_label = new R3SurfelLabel("Unknown");
    unknown_label->SetColor(RNred_rgb);
    scene->InsertLabel(unknown_label, scene->RootLabel());
  }

  // Initialize object selection times
  object_selection_times.resize(scene->NObjects());
  for (int i = 0; i < scene->NObjects(); i++) {
    object_selection_times.push_back(-1.0);
  }

  // Initialize color scheme
  surfel_color_scheme = R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL;
  
  // Initialize select box
  rubber_box_corners[0] = R2zero_point;
  rubber_box_corners[1] = R2zero_point;
  select_box_active = FALSE;

  // Initialize select polygon
  num_rubber_polygon_points = 0;
  for (int i = 0; i < max_rubber_polygon_points; i++) rubber_polygon_points[i].Reset(0,0);
  select_polygon_active = FALSE;
  split_polygon_active = FALSE;
  click_polygon_active = FALSE;

  // Initialize split line
  rubber_line_points[0] = R2zero_point;
  rubber_line_points[1] = R2zero_point;
  split_line_active = FALSE;

  // Initialize attribute menu items
  attribute_menu_names.push_back("Attributes");
  attribute_menu_names.push_back("Group");
  attribute_menu_names.push_back("Moving");
  attribute_menu_flags.push_back(R3_SURFEL_OBJECT_NO_ATTRIBUTES_FLAG);
  attribute_menu_flags.push_back(R3_SURFEL_GROUP_ATTRIBUTE);
  attribute_menu_flags.push_back(R3_SURFEL_MOVING_ATTRIBUTE);
  attribute_menu_keystrokes.push_back(' ');
  attribute_menu_keystrokes.push_back('b');
  attribute_menu_keystrokes.push_back('v');

  // Initialize labels
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->NParts() > 0) continue;
    if (object->HumanLabel()) continue;
    if (object->PredictedLabel()) continue;
    R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, unknown_label, 0, R3_SURFEL_MACHINE_ORIGINATOR);
    scene->InsertLabelAssignment(assignment);
  }

  // Update object oriented bounding boxes
  UpdateObjectOrientedBBoxes();

  // Open logging file
  if (logging_filename) {
    this->logging_filename = RNStrdup(logging_filename);
    if (!strcmp(logging_filename, "stdout")) this->logging_fp = stdout;
    else {
      this->logging_fp = fopen(logging_filename, "w");
      if (!this->logging_fp) {
        RNFail("Unble to open logging file %s\n", logging_filename);
        abort();
      }
    }
  }
}



R3SurfelLabeler::
~R3SurfelLabeler(void)
{
  // Empty working set
  EmptyWorkingSet();

  // Close logging file
  if (logging_fp) fclose(logging_fp);
  if (logging_filename) free(logging_filename);
    
  // Delete message
  if (message) free(message);
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
SetMessage(const char *fmt, ...)
{
  // Free previous message
  if (message) free(message);
  message = NULL;

  // Set message (displayed at bottom of screen)
  if (fmt) {
    message = new char [4096];
    va_list args;
    va_start(args, fmt);
    vsprintf(message, fmt, args);
    va_end(args);
  }
}



void R3SurfelLabeler::
SetOBBManipulator(const R3OrientedBoxManipulator& obb_manipulator)
{
  // Set OBB manipulator
  this->obb_manipulator = obb_manipulator;
}


  
void R3SurfelLabeler::
SetSelectionVisibility(int visibility)
{
  // Set selection visibililty
  if (visibility == -1) selection_visibility = 1 - selection_visibility;
  else if (visibility == 0) selection_visibility = 0;
  else selection_visibility = 1;
}



void R3SurfelLabeler::
SetOBBManipulatorVisibility(int visibility)
{
  // Set obb manipulator visibililty
  if (visibility == -1) obb_manipulator_visibility = 1 - obb_manipulator_visibility;
  else if (visibility == 0) obb_manipulator_visibility = 0;
  else obb_manipulator_visibility = 1;

  // Update obb manipulator
  UpdateOBBManipulator(TRUE, TRUE);
}



void R3SurfelLabeler::
SetMessageVisibility(int visibility)
{
  // Set message visibililty
  if (visibility == -1) message_visibility = 1 - message_visibility;
  else if (visibility == 0) message_visibility = 0;
  else message_visibility = 1;
}



void R3SurfelLabeler::
SetStatusVisibility(int visibility)
{
  // Set status visibililty
  if (visibility == -1) status_visibility = 1 - status_visibility;
  else if (visibility == 0) status_visibility = 0;
  else status_visibility = 1;
}



  void R3SurfelLabeler::
SetCommandMenuVisibility(int visibility)
{
  // Set command menu visibililty
  if (visibility == -1) command_menu_visibility = 1 - command_menu_visibility;
  else if (visibility == 0) command_menu_visibility = 0;
  else command_menu_visibility = 1;
}



void R3SurfelLabeler::
SetLabelMenuVisibility(int visibility)
{
  // Set label menu visibililty
  if (visibility == -1) label_menu_visibility = 1 - label_menu_visibility;
  else if (visibility == 0) label_menu_visibility = 0;
  else label_menu_visibility = 1;
}



void R3SurfelLabeler::
SetAttributeMenuVisibility(int visibility)
{
  // Set attribute menu visibililty
  if (visibility == -1) attribute_menu_visibility = 1 - attribute_menu_visibility;
  else if (visibility == 0) attribute_menu_visibility = 0;
  else attribute_menu_visibility = 1;
}



void R3SurfelLabeler::
SetSnapshotDirectory(const char *directory_name)
{
  // Delete previous snapshot directory
  if (this->snapshot_directory) {
    free(this->snapshot_directory);
    this->snapshot_directory = NULL;
  }

  // Set new snapshot directory
  if (directory_name) {
    this->snapshot_directory = strdup(directory_name);
  }
}



////////////////////////////////////////////////////////////////////////
// OBB estimation functions
////////////////////////////////////////////////////////////////////////

static void
OrientOBB(R3OrientedBox& obb, R3SurfelObject *object = NULL)
{
  // Swap XY axes to make axis0 the longer axis
  if (obb.Radius(0) < obb.Radius(1)) {
    obb.Reset(obb.Center(), obb.Axis(1), -(obb.Axis(0)),
      obb.Radius(1), obb.Radius(0), obb.Radius(2));
  }

  // Rotate around Z by 180 degrees to make positive side point towards closest image viewpoint
  if (object) {
    R3SurfelScene *scene = object->Scene();
    if (scene && (scene->NImages() > 0)) {
      R3SurfelImage *image = ClosestImage(scene, obb.Centroid());
      R3Vector sensor_direction = image->Viewpoint() - obb.Centroid();
      RNScalar dot0 = sensor_direction.Dot(obb.Axis(0));
      RNScalar dot1 = sensor_direction.Dot(obb.Axis(1));
      if (((fabs(dot0) > fabs(dot1)) && (dot0 < 0)) ||
          ((fabs(dot1) > fabs(dot0)) && (dot1 < 0))) {
        obb.Reset(obb.Center(), -(obb.Axis(0)), -(obb.Axis(1)),
          obb.Radius(0), obb.Radius(1), obb.Radius(2));
      }
    }
  }

  // Swap XY axes to make axis1 the longer axis for some classes
  if (object) {
    R3SurfelLabel *label = object->CurrentLabel();
    if (label) {
      if (label->Flags()[R3_SURFEL_LABEL_SHORT_AXIS_TOWARDS_FRONT_FLAG]) {
        obb.Reset(obb.Center(), obb.Axis(1), -(obb.Axis(0)),
          obb.Radius(1), obb.Radius(0), obb.Radius(2));
      }
    }
  }
}



static R3OrientedBox 
EstimateOrientedBBox(R3SurfelObject *object,
  const R3Point& centroid, const R3Triad& axes)
{
  // Initialize result
  R3OrientedBox obb = R3null_oriented_box;

  // Create pointset
  R3SurfelPointSet pointset;
  static const int max_points = 1024;
  RNVolume volume = object->BBox().Volume();
  RNScalar max_resolution = (volume > 0) ? max_points / volume : 0;
  object->InsertIntoPointSet(&pointset, TRUE, max_resolution);
  if (pointset.NPoints() == 0) return R3null_oriented_box;

  // Estimate oriented box of pointset
  if (axes == R3null_triad) {
    // Orient oriented box based on heuristics
    obb = EstimateOrientedBBox(&pointset);
    OrientOBB(obb, object);
  }
  else {
    // Orient oriented box according to given axes
    obb = EstimateOrientedBBox(&pointset, centroid, axes);
  }

  // Return estimated oriented bbox
  return obb;
}
  


static void
UpdateObjectOrientedBBox(R3SurfelObject *object,
  const R3Point& centroid = R3zero_point, const R3Triad& axes = R3null_triad)
{
  // Check if already have OBB property
  if (object->FindObjectProperty(R3_SURFEL_OBJECT_AMODAL_OBB_PROPERTY)) return;

  // Estimate OBB
  R3OrientedBox obb = EstimateOrientedBBox(object, centroid, axes);

  // Set OBB property
  SetObjectOBBProperty(object, obb, 0, R3_SURFEL_MACHINE_ORIGINATOR);
}



void R3SurfelLabeler::
UpdateObjectOrientedBBoxes(void)
{
  // Update oriented box for all top-level objects
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    UpdateObjectOrientedBBox(object);
  }
}



////////////////////////////////////////////////////////////////////////
// OBB manipulation functions
////////////////////////////////////////////////////////////////////////

RNBoolean R3SurfelLabeler::
IsOBBManipulatorVisible(void) const
{
  // Check visibility
  if (!obb_manipulator_visibility &&
      !object_oriented_bbox_visibility) return FALSE;

  // Check oriented box
  if (obb_manipulator.OrientedBox().IsEmpty()) return FALSE;

  // Check number of object selections
  if (NObjectSelections() != 1) return FALSE;

  // Passed all tests
  return TRUE;
}



void R3SurfelLabeler::
UpdateOBBManipulator(RNBoolean reset_manipulation,
  RNBoolean update_oriented_bbox,
  RNBoolean keep_orientation)
{
  // Check visibility
  if (!obb_manipulator_visibility &&
      !object_oriented_bbox_visibility) return;

  // Check object selection
  if (NObjectSelections() != 1) return;
  R3SurfelObject *object = ObjectSelection(0);

  // Reset obb manipulation
  if (reset_manipulation) {
    obb_manipulator.ResetManipulation();
  }

  // Update obb
  if (update_oriented_bbox) {
    R3OrientedBox obb = obb_manipulator.OrientedBox();
    if (keep_orientation) {
      obb = EstimateOrientedBBox(object, obb.Center(), obb.Axes());
    }
    else {
      if (!GetObjectOBBProperty(object, &obb)) {
        obb = EstimateOrientedBBox(object);
      }
    }
    obb_manipulator.SetOrientedBox(obb);
  }

  // Update whether rotation is allowed
  R3SurfelLabel *label = object->CurrentLabel();
  RNBoolean rotating_allowed = TRUE;
  if (label && label->Flags()[R3_SURFEL_LABEL_UNORIENTABLE_FLAG])
    rotating_allowed = FALSE;
  if (object->Flags()[R3_SURFEL_GROUP_ATTRIBUTE])
    rotating_allowed = FALSE;
  obb_manipulator.SetRotatingAllowed(rotating_allowed);
}



////////////////////////////////////////////////////////////////////////
// Drawing utility functions
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
DrawObjectSelections(void) const
{
  // Check if need to draw
  if (!surfel_visibility) return;
  if (!selection_visibility) return;
  if (NObjectSelections() == 0) return;

  // Set opengl draw modes to make sure selection is visible
  // glDisable(GL_DEPTH_TEST);
  // glEnable(GL_POLYGON_OFFSET_FILL);
  // glPolygonOffset(-2, -1);
  RNLoadRgb(1.0, 1.0, 0.0);
  glDepthRange(0, 0.9999);

  // Increase point size
  glPointSize(2 + surfel_size);

  // Assign shader and its variables
  if (shader_program > 0) {
    glUseProgram(shader_program);
    int pointSizeLocation = glGetUniformLocation(shader_program, "pointSize");
    glUniform1f(pointSizeLocation, 2 + surfel_size);
  }
  
  // Draw objects
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    DrawObject(object, shape_draw_flags);
  }

  // Unassign shader
  glUseProgram(0);

  // Reset point size
  glPointSize(surfel_size);
  
  // Reset draw modes
  glDepthRange(0, 1);
  // glDisable(GL_POLYGON_OFFSET_FILL);
  // glEnable(GL_DEPTH_TEST);
}



void R3SurfelLabeler::
DrawObjectLabels(void) const
{
  // Check stuff
  if (!object_label_visibility) return;
  
  // Set opengl stuff
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, viewer.Viewport().Width(), 0, viewer.Viewport().Height());
  glDisable(GL_DEPTH_TEST);
  glDepthMask(FALSE);

  // Draw labels
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    for (int i = 0; i < object->NLabelAssignments(); i++) {
      R3SurfelLabelAssignment *assignment = object->LabelAssignment(i);
      if (assignment->Originator() == R3_SURFEL_GROUND_TRUTH_ORIGINATOR) continue;
      RNBoolean confirmed = (assignment->Originator() == R3_SURFEL_HUMAN_ORIGINATOR) ? 1 : 0;
      R3SurfelLabel *label = assignment->Label();
      if (!strcmp(label->Name(), "Unknown")) continue;
      if (!LabelVisibility(label)) continue;
      R3Point position = object->Centroid();
      position[2] = object->BBox().ZMax() + 0.01;
      R2Point p = viewer.ViewportPoint(position);
      void *font = (confirmed) ? RN_GRFX_BITMAP_HELVETICA_18 : RN_GRFX_BITMAP_HELVETICA_12;
      int width = RNTextWidth(label->Name(), font);
      p[0] -= width / 2;
      RNLoadRgb(label->Color());
      R2DrawText(p, label->Name(), font);
      break;
    }
  }

  // Reset opengl stuff
  glDepthMask(TRUE);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}



void R3SurfelLabeler::
DrawOBBManipulator(void) const
{
  // Check visibility
  if (!IsOBBManipulatorVisible()) return;

  // Set opengl modes
  glDisable(GL_LIGHTING);
  glLineWidth(5);
  RNLoadRgb(1.0, 0.5, 1.0);
  glDepthRange(0, 0.9999);

  // Draw the oriented box manipulator
  obb_manipulator.Draw();

  // Reset opengl modes
  glDepthRange(0, 1);
  glLineWidth(1);
}



////////////////////////////////////////////////////////////////////////
// Rasterization utilities (for interaction processing)
////////////////////////////////////////////////////////////////////////

static int
RasterizeSDF(R2Grid& grid, const R2Polygon& polygon, int end_condition)
{
  // Get viewport dimensions
  int width = grid.XResolution();
  int height = grid.YResolution();
  if (width*height == 0) return 0;

  // Check number of points
  int n = polygon.NPoints();
  if (n < 2) return 0;

  // Compute distance longer than any line on screen
  double max_distance = width;

  // Set identity transformation
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, 0, height, 0.1, max_distance);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -0.11);

  // Clear color and depth buffer
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Determine if polygon is closed
  RNBoolean closed = TRUE;
  if (polygon.NPoints() < 3) closed = FALSE;
  else if (end_condition == 1) closed = TRUE;
  else if (end_condition == 2) closed = FALSE;
  else {
    RNScalar closed_endpoint_distance = 0.05 * grid.XResolution();
    RNScalar endpoint_distance = R2Distance(polygon.Point(0), polygon.Point(n-1));
    if (endpoint_distance > closed_endpoint_distance) {
      R2Vector v1 = polygon.Point(1) - polygon.Point(0); 
      R2Vector v2 = polygon.Point(n-1) - polygon.Point(n-2); 
      if (v1.Dot(v2) > 0.707) closed = FALSE;
    }
  }

  // Draw cones at points and wedges at segments
  // Color indicates side (r=left, g=right) and depth indicates distance
  for (int i = 0; i < polygon.NPoints(); i++) {
    const R2Point *p0 = &polygon.Point((i-1+n)%n);
    const R2Point *p1 = &polygon.Point(i);
    const R2Point *p2 = &polygon.Point((i+1)%n);
    R2Vector v1 = *p1 - *p0;
    R2Vector v2 = *p2 - *p1;
    if (!closed && (i == 0)) v1 = v2;
    if (!closed && (i == n-1)) v2 = v1;
    if (!closed && (i == 0)) p0 = p1;
    if (!closed && (i == n-1)) p2 = p1;
    RNLength length1 = v1.Length();
    RNLength length2 = v2.Length();
    if (RNIsZero(length1) || RNIsZero(length2)) continue;
    v1 /= length1;
    v2 /= length2;
    R2Vector n2(-v2[1], v2[0]);

    // Determine angles
    RNAngle a1 = atan2(v1.Y(), v1.X()) - RN_PI;
    while (a1 < 0) a1 += RN_TWO_PI;
    RNAngle a2 = atan2(v2.Y(), v2.X());
    while (a2 < a1) a2 += RN_TWO_PI;
    while (a2 > (a1 + RN_TWO_PI)) a2 -= RN_TWO_PI;

    // Draw right side of cone at p1
    RNLoadRgb(0, 1, 0);
    R3BeginPolygon();
    double da = (a2 - a1);
    double desired_step = (RN_PI / 60.0);
    int nsteps = da / desired_step;
    double step = da / nsteps;
    for (int j = 1; j <= nsteps; j++) {
      double angle1 = a1 + j * step;
      double angle2 = a1 + (j+1) * step;
      double dx1 = cos(angle1);
      double dy1 = sin(angle1);
      double dx2 = cos(angle2);
      double dy2 = sin(angle2);
      R3LoadPoint(p1->X(), p1->Y(), 0.0);
      R3LoadPoint(p1->X() + max_distance * dx1, p1->Y() + max_distance * dy1, -max_distance);
      R3LoadPoint(p1->X() + max_distance * dx2, p1->Y() + max_distance * dy2, -max_distance);
      dx1 = dx2;
      dy1 = dy2;
    }
    R3EndPolygon();

    // Draw left side of cone at p1
    RNLoadRgb(1, 0, 0);
    R3BeginPolygon();
    da = RN_TWO_PI - (a2 - a1);
    nsteps = da / desired_step;
    step = da / nsteps;
    for (int j = 1; j <= nsteps; j++) {
      double angle1 = a2 + j * step;
      double angle2 = a2 + (j+1) * step;
      double dx1 = cos(angle1);
      double dy1 = sin(angle1);
      double dx2 = cos(angle2);
      double dy2 = sin(angle2);
      R3LoadPoint(p1->X(), p1->Y(), 0.0);
      R3LoadPoint(p1->X() + max_distance * dx1, p1->Y() + max_distance * dy1, -max_distance);
      R3LoadPoint(p1->X() + max_distance * dx2, p1->Y() + max_distance * dy2, -max_distance);
      dx1 = dx2;
      dy1 = dy2;
    }
    R3EndPolygon();

    // Draw wedge 
    if (closed || (i < n-1)) {
      // Draw right side of wedge
      RNLoadRgb(0, 1, 0);
      R3BeginPolygon();
      R3LoadPoint(p1->X(), p1->Y(), 0.0);
      R3LoadPoint(p1->X() - max_distance * n2.X(), p1->Y() - max_distance * n2.Y(), -max_distance);
      R3LoadPoint(p2->X() - max_distance * n2.X(), p2->Y() - max_distance * n2.Y(), -max_distance);
      R3LoadPoint(p2->X(), p2->Y(), 0.0);
      R3EndPolygon();

      // Draw left side of wedge
      RNLoadRgb(1, 0, 0);
      R3BeginPolygon();
      R3LoadPoint(p1->X(), p1->Y(), 0.0);
      R3LoadPoint(p2->X(), p2->Y(), 0.0);
      R3LoadPoint(p2->X() + max_distance * n2.X(), p2->Y() + max_distance * n2.Y(), -max_distance);
      R3LoadPoint(p1->X() + max_distance * n2.X(), p1->Y() + max_distance * n2.Y(), -max_distance);
      R3EndPolygon();
    }
  }

  // Read color and depth buffers
  unsigned char *color_pixels = new unsigned char [ 4 * width * height ];
  float *distance_pixels = new float [ width * height ];
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, color_pixels);
  glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, distance_pixels);

  // Reset OpenGL modes
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  // Fill sdf grid
  for (int i = 0; i < width*height; i++) {
    RNScalar distance = distance_pixels[i];
    RNScalar signed_distance = (color_pixels[4*i]) ? distance : -distance;
    grid.SetGridValue(i, signed_distance);
  }

  // Delete buffers
  delete [] color_pixels;
  delete [] distance_pixels;

  // Return success
  return 1;
}



int R3SurfelLabeler::
RasterizeObjectMask(unsigned int *object_mask) 
{
  // Get convenient variables
  if (!scene) return 0;
  if (scene->NObjects() == 0) return 0;
  if (!object_mask) return 0;
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  int width = viewer.Viewport().Width();
  if (width <= 0) return 0;
  int height = viewer.Viewport().Height();
  if (height <= 0) return 0;

  // Clear window 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set viewing transformation
  viewer.Camera().Load();

  // Set viewing extent
  EnableViewingExtent();

#if (R3_SURFEL_VIEWER_DRAW_METHOD == R3_SURFEL_VIEWER_DRAW_WITH_VBO)
  // Draw with VBO
  DrawVBO(R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX);
#else
  // Draw with RNGrfxBegin ... RNGrfxEnd
  for (int i = 0; i < resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = resident_nodes.Node(i);
    if (!NodeVisibility(node)) continue;

    // Set color
    unsigned char rgba[4] = { 0, 0, 0, 0xFE };
    CreateColor(rgba, R3_SURFEL_VIEWER_COLOR_BY_PICK_INDEX, NULL, NULL, node, NULL, NULL);
    RNLoadRgba(rgba);

    // Draw node
    node->Draw(shape_draw_flags);
  }
#endif

  // Reset OpenGL stuff
  glFinish();

  // Reset viewing modes
  DisableViewingExtent();

  // Read color buffer
  unsigned char *pixels = new unsigned char [ 4 * width * height ];
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // Process color buffer to fill returned mask with object indices
  unsigned char *pixelsp = pixels;
  for (int i = 0; i < width*height; i++) {
    // Initialize object mask
    object_mask[i] = -1;
    
    // Determine node index
    int r = *(pixelsp++);
    int g = *(pixelsp++);
    int b = *(pixelsp++);
    int a = *(pixelsp++);
    if (a == 0) continue;
    int node_index = (r << 16) | (g << 8) | b;
    node_index--;    

    // Determine node
    if (node_index < 0) continue;
    if (node_index >= tree->NNodes()) continue;
    R3SurfelNode *node = tree->Node(node_index);

    // Determine object
    R3SurfelObject *object = node->Object(TRUE);
    if (!object) continue;
    while (object->Parent() && (object->Parent() != scene->RootObject()))
      object = object->Parent();

    // Update object mask
    int object_index = object->SceneIndex();
    object_mask[i] = object_index;
  }

  // Free pixels
  delete [] pixels;

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// UI event handler functions
////////////////////////////////////////////////////////////////////////

int R3SurfelLabeler::
Redraw(void)
{
  // Check scene
  if (!scene) return 0;

  // Draw everything 
  R3SurfelViewer::Redraw();
  
  // Set draw modes
  glDisable(GL_LIGHTING);
  glPointSize(surfel_size);
  glLineWidth(1);

  // Draw labeling stuff
  DrawObjectSelections();
  DrawObjectLabels();
  DrawOBBManipulator();
  DrawStatus();
  DrawMessage();
  DrawCommandMenu();
  DrawLabelMenu();
  DrawAttributeMenu();
  
  // Draw select stuff
  DrawRubberBox(FALSE, TRUE);
  DrawRubberLine(FALSE, TRUE);
  DrawRubberPolygon(FALSE, TRUE);

  // Return whether need redraw
  return 0;
}    



int R3SurfelLabeler::
Resize(int w, int h)
{
  // Send event to viewer
  return R3SurfelViewer::Resize(w, h);
}



int R3SurfelLabeler::
MouseMotion(int x, int y)
{
  // Initialize
  int redraw = 0;

  // Compute mouse movement
  int dx = x - mouse_position[0];
  int dy = y - mouse_position[1];

  // Process mouse movement
  if (ctrl_down || alt_down || shift_down) {
    // Check for drag
    RNBoolean drag = (mouse_drag_distance_squared > 10 * 10);
    if (drag) {
      if (click_polygon_active && (num_rubber_polygon_points > 1)) {
        // Update click polygon
        R2Point p(x, y);
        if (R2Distance(p, rubber_polygon_points[num_rubber_polygon_points-2]) > 10) {
          DrawRubberPolygon();
          rubber_polygon_points[num_rubber_polygon_points-1] = p;
          DrawRubberPolygon();
        }
      }
      else if ((select_polygon_active || split_polygon_active) && (num_rubber_polygon_points > 0)) {
        // Update rubber polygon
        R2Point p(x, y);
        if (R2Distance(p, rubber_polygon_points[num_rubber_polygon_points-1]) > 10) {
          DrawRubberPolygon();
          rubber_polygon_points[num_rubber_polygon_points++] = p;
          DrawRubberPolygon();
        }
      }
      else if (split_line_active) {
        // Update rubber line
        DrawRubberLine();
        rubber_line_points[1] = R2Point(x, y);
        DrawRubberLine();
      }
      else if (select_box_active) {
        // Update rubber box
        DrawRubberBox();
        rubber_box_corners[1] = R2Point(x, y);
        DrawRubberBox();
      }
    }
  }
  else if (mouse_button[0] || mouse_button[1] || mouse_button[2]) {
    if (obb_manipulator.IsManipulating()) {
      if (IsOBBManipulatorVisible()) {
        if (obb_manipulator.UpdateManipulation(viewer, x, y)) {
          R3OrientedBox obb = obb_manipulator.OrientedBox();
          UpdateOBBManipulator(FALSE, TRUE, TRUE);
          if (obb_manipulator.IsScaling()) {
            // Make sure obb contains entire object
            obb.Union(obb_manipulator.OrientedBox());
            obb_manipulator.SetOrientedBox(obb);
          }
          redraw = 1;
        }
      }
    }
    else {
      // Set viewing center point
      R3Point viewing_center_point = center_point;
      const R3Camera& camera = viewer.Camera();
      const R2Viewport& viewport = viewer.Viewport();
      R3Plane camera_plane(camera.Origin(), camera.Towards());
      RNScalar signed_distance = R3SignedDistance(camera_plane, viewing_center_point);
      if (signed_distance < 0) viewing_center_point -= (signed_distance - 1) * camera.Towards();
      R2Point screen_point = viewer.ViewportPoint(viewing_center_point);
      if ((NObjectSelections() == 0) || !R2Contains(viewer.Viewport().BBox(), screen_point)) {
        R3Ray world_ray = viewer.WorldRay(viewport.XCenter(), viewport.YCenter());
        R3Plane world_plane(0, 0, 1, -viewing_center_point[2]);
        R3Intersects(world_ray.Line(), world_plane, &viewing_center_point);
        viewing_center_point = scene->BBox().ClosestPoint(viewing_center_point);
      }
  
      // World in hand navigation 
      if (mouse_button[0]) RotateWorld(1.0, viewing_center_point, x, y, dx, dy);
      // else if (shift_down && (mouse_button[1] || mouse_button[2])) viewer.ScaleWorld(2.0, viewing_center_point, x, y, dx, dy);
      // else if (ctrl_down && (mouse_button[1] || mouse_button[2])) viewer.TranslateWorld(2.0, viewing_center_point, x, y, dx, dy);
      else if (mouse_button[1]) viewer.ScaleWorld(2.0, viewing_center_point, x, y, dx, dy);
      else if (mouse_button[2]) viewer.TranslateWorld(2.0, viewing_center_point, x, y, dx, dy);
      if (mouse_button[0] || mouse_button[1] || mouse_button[2]) redraw = 1;
    }
  }
  
  // Remember mouse position 
  mouse_position[0] = x;
  mouse_position[1] = y;

  // Update mouse drag movement
  mouse_drag_distance_squared += dx*dx + dy*dy;

  // Return whether need redraw
  return redraw;
}



int R3SurfelLabeler::
MouseButton(int x, int y, int button, int state, int shift, int ctrl, int alt, int update_center_point)
{
  // Send event to viewer
  update_center_point &= !R2Contains(LabelMenuBBox(), R2Point(x,y));
  int redraw = R3SurfelViewer::MouseButton(x, y, button, state, shift, ctrl, alt, update_center_point);

  // Process mouse button event
  if (state == 1) { // Down
    // Process command
    obb_manipulator.ResetManipulation();
    if (!click_polygon_active) {
      if ((button == 0) && IsOBBManipulatorVisible() &&
          obb_manipulator.BeginManipulation(viewer, x, y)) {
        redraw = 1;
      }
      else if ((button == 0) && (shift || ctrl)) {
        // Start select polygon
        select_polygon_active = TRUE;
        rubber_polygon_points[0] = R2Point(x, y);
        num_rubber_polygon_points = 1;
        DrawRubberPolygon();
        redraw = 0;
      }
      else if ((button == 2) && alt) {
        // Start split polygon
        split_polygon_active = TRUE;
        rubber_polygon_points[0] = R2Point(x, y);
        num_rubber_polygon_points = 1;
        DrawRubberPolygon();
        redraw = 0;
      }
      else if ((button == 0) && alt) {
        // Start split line
        split_line_active = TRUE;
        rubber_line_points[0] = R2Point(x, y);
        rubber_line_points[1] = R2Point(x, y);
        DrawRubberLine();
        redraw = 0;
      }
      else if (0) {
        // Start select box
        select_box_active = TRUE;
        rubber_box_corners[0] = R2Point(x, y);
        rubber_box_corners[1] = R2Point(x, y);
        DrawRubberBox();
        redraw = 0;
      }
    }
  }
  else if (state == 0) { // Up
    // Check for drag
    RNBoolean drag = (mouse_drag_distance_squared > 10 * 10);

    // Check for double click
    static RNBoolean double_click = FALSE;
    static RNTime last_mouse_down_time;
    double_click = !drag && !double_click && (last_mouse_down_time.Elapsed() < 0.4);
    last_mouse_down_time.Read();

    // Process command
    if (obb_manipulator.IsManipulating()) {
      if (IsOBBManipulatorVisible()) {
        if (obb_manipulator.IsDirty()) {
          AssignOBBToSelectedObject(obb_manipulator.OrientedBox(), 1.0, R3_SURFEL_HUMAN_ORIGINATOR);
        }
        redraw = 1;
      }
    }
    else if (select_box_active) {
      if (0 && drag && !double_click) {
        // Get box
        R2Box box = R2null_box;
        box.Union(rubber_box_corners[0]);
        box.Union(rubber_box_corners[1]);

        // Process command
        if (alt) {
          // Process split box
          SplitSelectedObjects();
          redraw = 1;
        }
        else if (shift || ctrl) {
          // Process select box
          SelectEnclosedObjects(box, shift, ctrl, alt);
          redraw = 1;
        }
      }

      // Reset rubber box
      DrawRubberBox();
      select_box_active = FALSE;
      rubber_box_corners[0] = R2zero_point;
      rubber_box_corners[1] = R2zero_point;
    }
    else if (select_polygon_active) {
      if ((button == 0) && (shift || ctrl) && drag && !double_click) {
        // Process select polygon
        R2Polygon polygon(rubber_polygon_points, num_rubber_polygon_points);
        SelectEnclosedObjects(polygon, shift, ctrl, alt);
        redraw = 1;
      }

      // Reset rubber polygon
      DrawRubberPolygon();
      num_rubber_polygon_points = 0;
      select_polygon_active = FALSE;
    }
    else if (split_polygon_active) {
      if ((button == 2) && alt && drag && !double_click) {
        // Process split polygon
        SplitSelectedObjects();
        redraw = 1;
      }

      // Reset rubber polygon
      DrawRubberPolygon();
      num_rubber_polygon_points = 0;
      split_polygon_active = FALSE;
    }
    else if (split_line_active) {
      if ((button == 0) && alt && drag && !double_click) {
        // Process split line
        SplitSelectedObjects();
        redraw = 1;
      }

      // Reset rubber line
      DrawRubberLine();
      rubber_line_points[0] = R2zero_point;
      rubber_line_points[1] = R2zero_point;
      split_line_active = FALSE;
    }
    else if (click_polygon_active) {
      DrawRubberPolygon();
      if (((button == 0) || (button == 2)) && alt) {
        if (double_click) {
          // Process click polygon
          if (num_rubber_polygon_points > 2) {
            num_rubber_polygon_points--;
            SplitSelectedObjects();
            num_rubber_polygon_points = 0;
            click_polygon_active = FALSE;
            redraw = 1;
          }
        }
        else {
          // Extend click polygon
          if (num_rubber_polygon_points < max_rubber_polygon_points) {
            if (!click_polygon_active) num_rubber_polygon_points = 1;
            rubber_polygon_points[num_rubber_polygon_points-1] = R2Point(x, y);
            rubber_polygon_points[num_rubber_polygon_points++] = R2Point(x+1, y+1);
            DrawRubberPolygon();
            redraw = 0;
          }
        }
      }
      else {
        // Reset click polygon
        num_rubber_polygon_points = 0;
        click_polygon_active = FALSE;
      }
    }

    // Process click commands
    if (button == 0) {
      if (!drag && !double_click && !click_polygon_active &&
          (!IsOBBManipulatorVisible() || !obb_manipulator.IsManipulating())) {
        if (alt) {
          // Start click polygon
          num_rubber_polygon_points = 2;
          rubber_polygon_points[0] = R2Point(x, y);
          rubber_polygon_points[1] = R2Point(x, y);
          click_polygon_active = TRUE;
          DrawRubberPolygon();
          redraw = 0;
        }
        else {
          if (PickCommandMenu(x, y, button, state, shift, ctrl, alt)) redraw = 1;
          else if (PickLabelMenu(x, y, button, state, shift, ctrl, alt)) redraw = 1;
          else if (PickAttributeMenu(x, y, button, state, shift, ctrl, alt)) redraw = 1;
          else if (PickImage(x, y)) redraw = 1;
          else if (SelectPickedObject(x, y, shift, ctrl, alt)) redraw = 1;
        }
      }
    }

    // Reset obb manipulation
    obb_manipulator.ResetManipulation();
  }

  // Return whether need redraw
  return redraw;
}



int R3SurfelLabeler::
Keyboard(int x, int y, int key, int shift, int ctrl, int alt)
{
  // For compatibility with R3SurfelViewer
  return Keyboard(x, y, key, shift, ctrl, alt, 0);
}



int R3SurfelLabeler::
Keyboard(int x, int y, int key, int shift, int ctrl, int alt, int tab)
{
  // Do not redraw by default
  int redraw = 0;
  
  // Process mouse button event
  if (alt) {
    // Make sure does not conflict with keys used by R3SurfelViewer
    switch(key) {
    case 'B':
      SetObjectOrientedBBoxVisibility(-1);
      redraw = 1;
      break;

    case 'b':
      SetOBBManipulatorVisibility(-1);
      redraw = 1;
      break;

    case 'C':
      // Copied from R3SurfelViewer
      SetSurfelColorScheme((surfel_color_scheme + 1) % R3_SURFEL_VIEWER_NUM_COLOR_SCHEMES);
      redraw = 1;
      break;

    case 'c':
      // Copied from R3SurfelViewer
      if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_RGB)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_ELEVATION);
      else if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_ELEVATION)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_OBJECT);
      else if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_OBJECT)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_OBJECT_ATTRIBUTES);
      else if (SurfelColorScheme() == R3_SURFEL_VIEWER_COLOR_BY_OBJECT_ATTRIBUTES)
        SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_CURRENT_LABEL);
      else SetSurfelColorScheme(R3_SURFEL_VIEWER_COLOR_BY_RGB);
      redraw = 1;
      break;

    case 'G':
    case 'g': 
      if (ElevationRange().IsEmpty()) SetElevationRange(RNInterval(-FLT_MAX, 0.25));
      else SetElevationRange(RNnull_interval);
      redraw = 1;
      break;
      
    case 'I':
    case 'i':
      // Copied from R3SurfelViewer
      SetImageInsetVisibility(-1);
      SelectImage(selected_image, FALSE, FALSE);
      redraw = 1;
      break;

    case 'L': 
    case 'l':
      // Copied from R3SurfelViewer
      SetHumanLabeledObjectVisibility(-1);
      redraw = 1;
      break;
      
    case 'M':
    case 'm':
      subsampling_multiplier_when_mouse_down = (subsampling_multiplier_when_mouse_down <= 1) ? 8 : 1;
      redraw = 1;
      break;
      
    case 'P':
    case 'p':
      // Copied from R3SurfelViewer
      SetSurfelVisibility(-1);
      redraw = 1;
      break;

    case 'S':
    case 's': 
      SetSelectionVisibility(-1);
      redraw = 1;
      break;

    case 'T':
    case 't':
      // Copied from R3SurfelViewer
      // object_label_visibility = !object_label_visibility;
      redraw = 1;
      break;

    case 'V':
    case 'v':
      // Copied from R3SurfelViewer
      SetImageViewpointVisibility(-1);
      redraw = 1;
      break;
      
    case 'Z':
    case 'z':
      // Copied from R3SurfelViewer
      SetViewingExtentVisibility(-1);
      redraw = 1;
      break;
      
    case '1': 
    case '!': 
      SetStatusVisibility(-1);
      redraw = 1;
      break; 

    case '2': 
    case '@': 
      SetLabelMenuVisibility(-1);
      redraw = 1;
      break; 

    case '3': 
    case '#': 
      SetAttributeMenuVisibility(-1);
      redraw = 1;
      break; 

    case ',': 
    case '.':
      // Copied from R3SurfelViewer
      if (!ElevationRange().IsEmpty()) {
        RNInterval range = ElevationRange();
        if (key == ',') range.SetMax(0.9 * range.Max());
        else if (key == '.') range.SetMax(1.1 * range.Max());
        SetElevationRange(range);
        redraw = 1;
      }
      break;

    case '{':
    case '}': {
      // Copied from R3SurfelViewer
      RNScalar factor = (key == '{') ? 0.8 : 1.2;
      SetImagePlaneDepth(factor * ImagePlaneDepth());
      redraw = 1;
      break; }

#if 0      
    case '<': 
    case '>': {
      // Select next/prev image
      int image_index = (selected_image) ? selected_image->SceneIndex() : 0;
      if (key == ',') image_index++;
      else if (key == '.') image_index--;
      if (image_index < 0) image_index = 0;
      if (image_index > scene->NImages()-1) image_index = scene->NImages()-1;
      SelectImage(scene->Image(image_index), FALSE, FALSE);
      redraw = 1;
      break; }
#endif
      
    case '_': 
      // Copied from R3SurfelViewer
      SetSubsamplingFactor(SubsamplingFactor() / 2);
      redraw = 1;
      break;

    case '+': 
      // Copied from R3SurfelViewer
      SetSubsamplingFactor(SubsamplingFactor() * 2);
      redraw = 1;
      break;

    case '-': 
      // Copied from R3SurfelViewer
      SetSurfelSize(0.8 * SurfelSize());
      redraw = 1;
      break;

    case '=': 
      // Copied from R3SurfelViewer
      SetSurfelSize(1.25 * SurfelSize());
      redraw = 1;
      break;

    case ';':
      // Copied from R3SurfelViewer
      if (FocusRadius() < RN_INFINITY) {
        SetFocusRadius(0.9 * FocusRadius());
        redraw = 1;
      }
      break;
      
    case '\'':
      // Copied from R3SurfelViewer
      if (FocusRadius() < RN_INFINITY) {
        SetFocusRadius(1.1 * FocusRadius());
        redraw = 1;
      }
      break;

    case ':':
      // Copied from R3SurfelViewer
      if (TargetResolution() < RN_INFINITY) {
        SetTargetResolution(0.9 * TargetResolution());
        redraw = 1;
      }
      break;

      case '"':
      // Copied from R3SurfelViewer
      if (TargetResolution() < RN_INFINITY) {
        SetTargetResolution(1.1 * TargetResolution());
        redraw = 1;
      }
      break;
    }
  }
  else if (ctrl) {
    // Process accelerator key commands
    switch(key) {
    case 'A':
    case 'a': 
      SelectAllObjects();
      redraw = 1;
      break;

    // case 'B':
    // case 'b':
      // Save for attribute "Group"
      // break;
      
    case 'E':
    case 'e':
      if (IsOBBManipulatorVisible()) {
        obb_manipulator.ResetManipulation();
        R3OrientedBox obb = obb_manipulator.OrientedBox();
        SelectOverlappedObjects(obb);
        obb_manipulator.SetOrientedBox(obb);
        redraw = 1;
      }
      else {
        SelectOverlappedObjects();
        redraw = 1;
      }
      break; 
      
    //case XXX:
    //  segmenter.PredictObjectSegmentations(selection_objects);
    //  redraw = 1;
    // break;

    case 'G': {
      UnmergeSelectedObjects();
      redraw = 1;
      break; }

    case 'g': {
      MergeSelectedObjects();
      redraw = 1;
      break; }

    case 'M':
    case 'm': // Enter
      // Copied from R3SurfelViewer
      if (image_inset_size < 0.5) SetImageInsetSize(0.8);
      else SetImageInsetSize(0.2);
      redraw = 1;
      break;

    // case 'xxx':
    // case 'xxx':
    //   Assign new label (copy label from closest other object)
    //   if (NObjectSelections() > 0) AssignNewLabelToSelectedObjects();
    //   else if (!AssignNewLabelToPickedObject(x, y)) SelectPickedObject(x, y);
    //   redraw = 1;
    //   break;
      
    // case 'O':
    // case 'o':
    //   // Split selected objects by OBB
    //   if (IsOBBManipulatorVisible()) {
    //     if (NObjectSelections() == 1) {
    //       R3OrientedBox obb;
    //       RNScalar confidence;
    //       int originator;
    //       R3SurfelObject *object = ObjectSelection(0);
    //       if (GetObjectOBBProperty(object, &obb, &confidence, &originator)) {
    //         SplitSelectedObjects();
    //         if (NObjectSelections() == 1) {
    //           R3SurfelObject *object = ObjectSelection(0);
    //           SetObjectOBBProperty(object, obb, confidence, originator);
    //         }
    //         redraw = 1;
    //       }
    //     }
    //   }
    //   break;

    case 'P':
    case 'p':
      // classifier.PredictLabelAssignments();
      // redraw = 1;
      break;

    case 'R':
    case 'r':
      ResetCamera();
      redraw = 1;
      break;

    case 'S':
    case 's':
      Snapshot();
      SetMessage("Saved scene file");
      redraw = 1;
      break;

    // case 'v':
      // Save for attribute "Moving"
      // break;
      
    case 'Y':
    case 'y':
      Redo();
      redraw = 1;
      break;

    case 'Z':
    case 'z':
      Undo();
      redraw = 1;
      break;

    default:
      for (unsigned int i = 0; i < attribute_menu_keystrokes.size(); i++) {
        if (!isalpha(key)) continue;
        if (i >= attribute_menu_flags.size()) continue;
        if (i >= attribute_menu_names.size()) continue;
        if (key == attribute_menu_keystrokes[i]) {
          // if (NObjectSelections() > 0) AssignAttributeToSelectedObjects(attribute_menu_flags[i], attribute_menu_names[i], -1);
          // else AssignAttributeToPickedObject(x, y, attribute_menu_flags[i], attribute_menu_names[i], -1);
          AssignAttributeToSelectedObjects(attribute_menu_flags[i], attribute_menu_names[i], -1);
          redraw = 1;
          break;
        }
      } break;
    }
  }
  else if (tab) {
    // Make only one label visible
    if (scene) {
      R3SurfelLabel *label = scene->FindLabelByAssignmentKeystroke(key);
      if (label) {
        SetLabelVisibility(-1, 0);
        SetLabelVisibility(label->SceneIndex(), 1);
        redraw = 1;
      }
    }
  }
  else {
    // Process other keyboard events
    switch (key) {
    case R3_SURFEL_VIEWER_F1_KEY:
    case R3_SURFEL_VIEWER_F2_KEY:
    case R3_SURFEL_VIEWER_F3_KEY:
    case R3_SURFEL_VIEWER_F4_KEY: {
      // Copied from R3SurfelViewer
      int scale = key - R3_SURFEL_VIEWER_F1_KEY + 1;
      ZoomCamera(0.05 + 0.25*scale*scale);
      redraw = 1;
      break; }

    case R3_SURFEL_VIEWER_F7_KEY:
      // Copied from R3SurfelViewer
      if (selected_image) SelectImage(selected_image, TRUE, TRUE);
      redraw = 1;
      break;

    case R3_SURFEL_VIEWER_F8_KEY:
      // Copied from R3SurfelViewer
      ResetCamera();
      redraw = 1;
      break;
      
    case R3_SURFEL_VIEWER_F11_KEY: 
      // R3SurfelGraphCut(scene);
      redraw = 1;
      break; 

    case R3_SURFEL_VIEWER_DOWN_KEY:
      // Copied from R3SurfelViewer
      SetImageInsetSize(0.8 * ImageInsetSize());
      redraw = 1;
      break;

    case R3_SURFEL_VIEWER_UP_KEY:
      // Copied from R3SurfelViewer
      SetImageInsetSize(1.25 * ImageInsetSize());
      redraw = 1;
      break;

    case R3_SURFEL_VIEWER_LEFT_KEY:
    case R3_SURFEL_VIEWER_RIGHT_KEY:
      if (shift) {
        // Select next/prev object
        if (key == R3_SURFEL_VIEWER_LEFT_KEY) {
          // Select previous object (kinda)
          UndoCommandOfType(R3_SURFEL_LABELER_SELECT_SUGGESTED_COMMAND);
          redraw = 1;
        }
        else {
          // Select next object
          SelectSuggestedObject();
          redraw = 1;
        }  
      }
      else {
        // Rotate obb manipulator
        if (IsOBBManipulatorVisible()) {
          static const double delta_angle = RN_PI / 8.0;
          double sign = (key == R3_SURFEL_VIEWER_LEFT_KEY) ? 1 : -1;
          obb_manipulator.RotateOrientedBox(sign * delta_angle);
          UpdateOBBManipulator(TRUE, TRUE, TRUE);
          AssignOBBToSelectedObject(obb_manipulator.OrientedBox(), 1.0, R3_SURFEL_HUMAN_ORIGINATOR);
          redraw = 1;
        }
      }
      break;

    case R3_SURFEL_VIEWER_PAGE_UP_KEY: 
      // Copied from R3SurfelViewer
      if (viewing_extent.IsEmpty()) viewing_extent = scene->BBox();
      if (shift) viewing_extent[RN_LO][RN_Z] += 0.01 * scene->BBox().ZLength();
      else viewing_extent[RN_HI][RN_Z] += 0.01 * scene->BBox().ZLength();
      if (R3Contains(viewing_extent, scene->BBox())) viewing_extent = R3null_box;
      redraw = 1;
      break;

    case R3_SURFEL_VIEWER_PAGE_DOWN_KEY: 
      // Copied from R3SurfelViewer
      if (viewing_extent.IsEmpty()) viewing_extent = scene->BBox();
      if (shift) viewing_extent[RN_LO][RN_Z] -= 0.01 * scene->BBox().ZLength();
      else viewing_extent[RN_HI][RN_Z] -= 0.01 * scene->BBox().ZLength();
      if (R3Contains(viewing_extent, scene->BBox())) viewing_extent = R3null_box;
      redraw = 1;
      break;

    case 27: { // ESC
      SelectPickedObject(-1, -1, 0, 0);
      SetLabelVisibility(-1, 1);
      SetAttributeVisibility(0, 1);
      SetElevationRange(RNnull_interval);
      SetViewingExtent(R3null_box);
      click_polygon_active = FALSE;
      select_polygon_active = FALSE;
      split_polygon_active = FALSE;
      split_line_active = FALSE;
      select_box_active = FALSE;
      redraw = 1;
      break; }

#if 0
    case ' ':
      // Confirm predicted label
      if (NObjectSelections() > 0) ConfirmLabelsOnSelectedObjects();
      else if (!ConfirmLabelOnPickedObject(x, y)) SelectPickedObject(x, y);
      redraw = 1;
      break;
#endif
      
    default: 
      // Assign label by key
      if (scene) {
        R3SurfelLabel *label = scene->FindLabelByAssignmentKeystroke(key);
        if (label) {
          if (NObjectSelections() > 0) AssignLabelToSelectedObjects(label);
          else AssignLabelToPickedObject(label, x, y);
          redraw = 1;
        }
      }
      break;
    }
  }

  // Return whether need redraw
  return redraw;
}



////////////////////////////////////////////////////////////////////////
// COMMANDS
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
Initialize(void)
{
  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_INITIALIZE_COMMAND);

  // Send event to viewer
  R3SurfelViewer::Initialize();

  // End logging command 
  EndCommand();
}



void R3SurfelLabeler::
Terminate(void)
{
  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_TERMINATE_COMMAND);

  // Send event to viewer
  R3SurfelViewer::Terminate();

  // End logging command
  EndCommand();
}



int R3SurfelLabeler::
Sync(void) 
{
  // Check scene
  if (!scene) return 0;

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_SYNC_COMMAND);

  // Sync file
  scene->SyncFile();

  // End logging command 
  EndCommand();

  // Return success
  return 1;
}




int R3SurfelLabeler::
Snapshot(void)
{
  // Sync file
  Sync();
  
  // Check snapshot directory
  if (!snapshot_directory) return 1;

  // Get convenient variables
  if (!scene) return 0;
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;
  char cmd[1024];

  // Create snapshot directory
  sprintf(cmd, "mkdir -p %s", snapshot_directory);
  system(cmd);
  
  // Copy current scene files to snapshot directory
  sprintf(cmd, "cp -f %s %s/snapshot.ssa", scene->Filename(), snapshot_directory);
  system(cmd);
  sprintf(cmd, "cp -f %s %s/snapshot.ssb", database->Filename(), snapshot_directory);
  system(cmd);

  // Return success
  return 1;
}



void R3SurfelLabeler::
ZoomCamera(RNScalar scale)
{
  // Set message
  SetMessage("Zoomed to closeup view");

  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_ZOOM_COMMAND, scale);
  
  // Set the camera viewpoint
  R3SurfelViewer::ZoomCamera(scale);

  // End logging command
  EndCommand();
}



int R3SurfelLabeler::
SelectPickedObject(int x, int y, RNBoolean shift, RNBoolean ctrl, RNBoolean alt) 
{
  // Check scene
  if (!scene) return 0;

  // Set message
  SetMessage(NULL);

  // Pick object
  R3Point pick_point;
  R3SurfelNode *node = PickNode(x, y, &pick_point, NULL, NULL, TRUE);
  R3SurfelObject *object = (node) ? node->Object(TRUE) : NULL;
  if (!object && (shift || selection_objects.IsEmpty())) return 0;
  
  // Find top level object
  while (object && object->Parent() && (object->Parent() != scene->RootObject())) {
    object = object->Parent();
  }

  // Set message
  if (object) {
    R3SurfelLabel *label = object->CurrentLabel();
    const char *label_name = (label) ? label->Name() : "NoLabel";
    const char *object_name = (object->Name()) ? object->Name() : "without name";
    if (ctrl) SetMessage("Unselected object %s / %s", object_name, label_name);
    else SetMessage("Selected 1 object %s / %s", object_name, label_name);
  }

  // Note: center_point got updated in R3SurfelViewer::PickNode
  // So, center_point stored with this command is new one
  // Old center_point will not be restored with undo
  
  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_SELECT_PICKED_COMMAND);

  // Remove previous selections
  if (!ctrl && !shift) EmptyObjectSelections();
 
  // Update object selections
  if (object) {
    // Check if object is already selected
    if (selection_objects.FindEntry(object)) {
      // Remove object from selected objects
      if (ctrl) RemoveObjectSelection(object);
    }
    else {
      // Insert object into selected objects
      if (!ctrl) InsertObjectSelection(object);
    }
  }

  // End logging command
  EndCommand();

  // Update obb manipulator
  UpdateOBBManipulator(TRUE, TRUE);

  // Return success
  return 1;
}



int R3SurfelLabeler::
SelectObjects(const RNArray<R3SurfelObject *>& objects, int command_type,
  RNBoolean shift, RNBoolean ctrl, RNBoolean alt)
{
  // Set message
  if (ctrl) SetMessage("Unselected %d objects", objects.NEntries());
  else SetMessage("Selected %d objects", objects.NEntries());

  // Check objects
  if (objects.IsEmpty()) return 1;

  // Begin logging command 
  BeginCommand(command_type);
  
  // Remove previous selections
  if (!ctrl && !shift) EmptyObjectSelections();
 
  // Update object selections
  for (int i = 0; i < objects.NEntries(); i++) {
    R3SurfelObject *object = objects.Kth(i);

    // Check if object is already selected
    if (selection_objects.FindEntry(object)) {
      // Remove object from selected objects
      if (ctrl) RemoveObjectSelection(object);
    }
    else {
      // Insert object into selected objects
      if (!ctrl) InsertObjectSelection(object);
    }
  }

  // Set viewing center point
  R3Box selection_bbox = ObjectSelectionBBox();
  if (!selection_bbox.IsEmpty()) {
    SetCenterPoint(selection_bbox.Centroid());
  }
  
  // End logging command
  EndCommand();

  // Update obb manipulator
  UpdateOBBManipulator(TRUE, TRUE);

  // Return success
  return 1;
}



int R3SurfelLabeler::
SelectEnclosedObjects(const R2Box& box, RNBoolean shift, RNBoolean ctrl, RNBoolean alt, RNBoolean unlabeled_only) 
{
  // Check scene
  if (!scene) return 0;
  if (box.IsEmpty()) return 0;
  if (!SurfelVisibility()) return 0;
  
  // Get projection matrices
  GLdouble p[3];
  GLdouble modelview_matrix[16];
  GLdouble projection_matrix[16];
  GLint viewport[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  // Make array of picked objects
  RNArray<R3SurfelObject *> picked_objects;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    if (picked_objects.FindEntry(object)) continue;

    // Check if object's projected bounding box is inside selection box
    int corner_count = 0;
    for (int octant = 0; octant < 8; octant++) {
      R3Point corner = object->BBox().Corner(octant);
      if (!R3Contains(viewer.Camera().Halfspace(RN_LO, RN_Z), corner)) continue;
      gluProject(corner[0], corner[1], corner[2], modelview_matrix, projection_matrix, viewport, &(p[0]), &(p[1]), &(p[2]));
      R2Point corner_projection(p[0], p[1]);
      if (!R2Contains(viewer.Viewport().BBox(), corner_projection)) continue;
      if (!R2Contains(box, corner_projection)) continue;
      corner_count++;
    }
    
    // Add object if inside selection box
    if (corner_count >= 5) picked_objects.Insert(object);
  }

  // Select objects
  return SelectObjects(picked_objects, R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND, shift, ctrl, alt);

  // Return success
  return 1;
}



int R3SurfelLabeler::
SelectEnclosedObjects(const R2Polygon& polygon, RNBoolean shift, RNBoolean ctrl, RNBoolean alt, RNBoolean unlabeled_only) 
{
  // Check scene
  if (!scene) return 0;
  if (polygon.IsLinear()) return 0;
  if (polygon.NPoints() < 3) return 0;
  if (!SurfelVisibility()) return 0;

  // Get projection matrices
  GLdouble p[3];
  GLdouble modelview_matrix[16];
  GLdouble projection_matrix[16];
  GLint viewport[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  // Make array of picked objects
  RNArray<R3SurfelObject *> picked_objects;
  // for (int i = 0; i < resident_nodes.NNodes(); i++) {
  //   R3SurfelNode *node = resident_nodes.Node(i);
  //   R3SurfelObject *object = node->Object(TRUE);
  //   if (!object) continue;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    if (picked_objects.FindEntry(object)) continue;
    
    // Check if object's projected bounding box is inside selection polygon
    int corner_count = 0;
    for (int octant = 0; octant < 8; octant++) {
      R3Point corner = object->BBox().Corner(octant);
      if (!R3Contains(viewer.Camera().Halfspace(RN_LO, RN_Z), corner)) continue;
      gluProject(corner[0], corner[1], corner[2], modelview_matrix, projection_matrix, viewport, &(p[0]), &(p[1]), &(p[2]));
      R2Point corner_projection(p[0], p[1]);
      if (!R2Contains(viewer.Viewport().BBox(), corner_projection)) continue;
      if (!R2Contains(polygon.BBox(), corner_projection)) continue;
      if (!R2Contains(polygon, corner_projection)) continue;
      corner_count++;
    }

    // Add object if inside selection box
    if (corner_count >= 5) picked_objects.Insert(object);
  }

  // Select objects
  return SelectObjects(picked_objects, R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND, shift, ctrl, alt);
}



int R3SurfelLabeler::
SelectEnclosedObjects(const R3OrientedBox& box, RNBoolean shift, RNBoolean ctrl, RNBoolean alt, RNBoolean unlabeled_only) 
{
  // Check scene
  if (!scene) return 0;
  if (box.IsEmpty()) return 0;
  if (!SurfelVisibility()) return 0;

  // Make array of picked objects
  RNArray<R3SurfelObject *> picked_objects;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    if (picked_objects.FindEntry(object)) continue;

    // Check if object's bounding box is inside selection box
    int corner_count = 0;
    for (int octant = 0; octant < 8; octant++) {
      R3Point corner = object->BBox().Corner(octant);
      if (!R3Contains(box, corner)) continue;
      corner_count++;
    }
    
    // Add object if inside selection box
    if (corner_count >= 5) picked_objects.Insert(object);
  }

  // Select objects
  return SelectObjects(picked_objects, R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND, shift, ctrl, alt);
}



int R3SurfelLabeler::
SelectIntersectedObjects(const R2Polygon& polygon, RNBoolean shift, RNBoolean ctrl, RNBoolean alt, RNBoolean unlabeled_only) 
{
  // Check stuff
  if (!scene) return 0;
  if (scene->NObjects() == 0) return 0;
  if (polygon.IsLinear()) return 0;
  if (polygon.NPoints() < 3) return 0;
  if (!SurfelVisibility()) return 0;
  int width = viewer.Viewport().Width();
  if (width <= 0) return 0;
  int height = viewer.Viewport().Height();
  if (height <= 0) return 0;

  // Rasterize polygon mask
  R2Grid polygon_mask(width, height);
  polygon_mask.RasterizeGridPolygon(polygon, 1);

  // Rasterize object mask
  unsigned int *object_mask = new unsigned int [ width * height ];
  RasterizeObjectMask(object_mask);

  // Allocate object marks
  unsigned char *object_marks = new unsigned char [ scene->NObjects() ];
  for (int i = 0; i < scene->NObjects(); i++) object_marks[i] = 0;

  // Find objects intersecting polygon mask
  RNArray<R3SurfelObject *> picked_objects;
  for (int i = 0; i < width*height; i++) {
    if (object_mask[i] <= 0) continue;
    if (object_mask[i] >= (unsigned int) scene->NObjects()) continue;
    if (polygon_mask.GridValue(i) <= 0) continue;
    int object_index = object_mask[i];
    if (object_marks[object_index] > 0) continue;
    R3SurfelObject *object = scene->Object(object_index);
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    picked_objects.Insert(object);
    object_marks[object_index] = 1;
  }

  // Delete object marks
  delete [] object_marks;

  // Select objects
  return SelectObjects(picked_objects, R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND, shift, ctrl, alt);
}



static void
FindLeafObjects(R3SurfelObject *object, RNArray<R3SurfelObject *>& leaf_objects)
{
  RNArray<R3SurfelObject *> stack;
  stack.Insert(object);
  while (!stack.IsEmpty()) {
    R3SurfelObject *object = stack.Tail();
    stack.RemoveTail();
    if (object->NParts() == 0) {
      leaf_objects.Insert(object);
    }
    else {
      for (int k = 0; k < object->NParts(); k++) 
        stack.Insert(object->Part(k));
    }
  }
}



static RNScalar
EstimateOverlapFraction(const R3OrientedBox& obb, R3SurfelObject *object)
{
  // Get convenient variables
  R3SurfelScene *scene = object->Scene();
  if (!scene) return 0;
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Make array of leaf objects
  RNArray<R3SurfelObject *> leaf_objects;
  FindLeafObjects(object, leaf_objects);

  // Count surfels overlapping obb
  unsigned int total_count = 0;
  unsigned int overlap_count = 0;
  for (int i = 0; i < leaf_objects.NEntries(); i++) {
    R3SurfelObject *leaf_object = leaf_objects.Kth(i);
    for (int j = 0; j < leaf_object->NNodes(); j++) {
      R3SurfelNode *node = leaf_object->Node(j);
      for (int k = 0; k < node->NBlocks(); k++) {
        R3SurfelBlock *block = node->Block(k);
        database->ReadBlock(block);
        for (int s = 0; s < block->NSurfels(); s++) {
          R3Point position = block->SurfelPosition(s);
          if (R3Contains(obb, position)) overlap_count++;
          total_count++;
        }
        database->ReleaseBlock(block);
      }
    }
  }

  // Return overlap fraction
  if (total_count == 0) return 0;
  return (RNScalar) overlap_count / (RNScalar) total_count;
}



int R3SurfelLabeler::
SelectOverlappedObjects(const R3OrientedBox& box,
  RNScalar min_overlap_fraction, RNLength overlap_tolerance, RNBoolean unlabeled_only)
{
  // Check stuff
  if (!scene) return 0;
  if (scene->NObjects() == 0) return 0;
  if (!SurfelVisibility()) return 0;

  // Inflate input box by overlap tolerance
  R3OrientedBox query_box(box.Center(), box.Axis(0), box.Axis(1),
    box.Radius(0) + overlap_tolerance, box.Radius(1) + overlap_tolerance, box.Radius(2) + overlap_tolerance);

  // Mark which objects are selected
  std::vector<unsigned char> object_is_selected;
  object_is_selected.resize(scene->NObjects());
  for (int i = 0; i < scene->NObjects(); i++) object_is_selected[i] = 0;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *selected_object = ObjectSelection(i);
    object_is_selected[selected_object->SceneIndex()] = 1;
  }
  
  // Find objects overlapping query box
  RNArray<R3SurfelObject *> picked_objects;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (!object->Name()) continue;
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    if (object_is_selected[object->SceneIndex()]) continue;
    if (!R3Intersects(query_box, object->BBox())) continue;
    RNScalar overlap_fraction = EstimateOverlapFraction(query_box, object);
    if (overlap_fraction < min_overlap_fraction) continue;
    picked_objects.Insert(object);
  }

  // Select objects
  return SelectObjects(picked_objects, R3_SURFEL_LABELER_SELECT_OVERLAPPING_COMMAND, TRUE, FALSE, FALSE);
}



int R3SurfelLabeler::
SelectOverlappedObjects(RNScalar min_overlap_fraction, RNLength overlap_tolerance, RNBoolean unlabeled_only) 
{
  // Get/check scene stuff
  if (!scene) return 0;
  if (!SurfelVisibility()) return 0;
  if (NObjectSelections() == 0) return 0;
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Compute bbox of selected objects
  R3Box grid_bbox = R3null_box;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *selected_object = ObjectSelection(i);
    grid_bbox.Union(selected_object->BBox());
  }

  // Check volume of selected objects
  if (grid_bbox.Volume() == 0) return 0;

  // Make array of selected leaf objects
  RNArray<R3SurfelObject *> selected_leaf_objects;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *selected_object = ObjectSelection(i);
    FindLeafObjects(selected_object, selected_leaf_objects);
  }

  // Create mask from selected leaf objects
  R3Grid mask(grid_bbox, overlap_tolerance, 5, 256, 3);
  for (int i = 0; i < selected_leaf_objects.NEntries(); i++) {
    R3SurfelObject *object = selected_leaf_objects.Kth(i);
    for (int j = 0; j < object->NNodes(); j++) {
      R3SurfelNode *node = object->Node(j);
      for (int k = 0; k < node->NBlocks(); k++) {
        R3SurfelBlock *block = node->Block(k);
        if (!database->IsBlockResident(block)) continue;
        for (int s = 0; s < block->NSurfels(); s++) {
          R3Point position = block->SurfelPosition(s);
          mask.RasterizeWorldPoint(position, 1);
        }
      }
    }
  }

  // Mark which objects are selected
  std::vector<unsigned char> object_is_selected;
  object_is_selected.resize(scene->NObjects());
  for (int i = 0; i < scene->NObjects(); i++) object_is_selected[i] = 0;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *selected_object = ObjectSelection(i);
    object_is_selected[selected_object->SceneIndex()] = 1;
  }
  
  // Find other top-level objects overlapping selected objects
  RNArray<R3SurfelObject *> picked_objects;
  R3SurfelObject *root_object = scene->RootObject();
  for (int r = 0; r < root_object->NParts(); r++) {
    R3SurfelObject *top_level_object = root_object->Part(r);

    // Check if should select if overlapped
    if (object_is_selected[top_level_object->SceneIndex()]) continue;
    if (unlabeled_only && top_level_object->HumanLabel()) continue;

    // Count overlaps
    int overlap_count = 0, total_count = 0;
    RNArray<R3SurfelObject *> leaf_objects;
    FindLeafObjects(top_level_object, leaf_objects);
    for (int i = 0; i < leaf_objects.NEntries(); i++) {
      R3SurfelObject *object = leaf_objects.Kth(i);
      if (!ObjectVisibility(object)) continue;
      for (int j = 0; j < object->NNodes(); j++) {
        R3SurfelNode *node = object->Node(j);
        for (int k = 0; k < node->NBlocks(); k++) {
          R3SurfelBlock *block = node->Block(k);
          if (!database->IsBlockResident(block)) continue;
          for (int s = 0; s < block->NSurfels(); s++) {
            R3Point position = block->SurfelPosition(s);
            RNScalar mask_value = mask.WorldValue(position);
            if (mask_value > 0.5) overlap_count++;
            total_count++;
          }
        }
      }
    }

    // Check overlap fraction
    if (total_count == 0) continue;
    RNScalar overlap_fraction = (RNScalar) overlap_count / (RNScalar) total_count;
    if (overlap_fraction < min_overlap_fraction) continue;
    picked_objects.Insert(top_level_object);
  }

  // Select objects
  return SelectObjects(picked_objects, R3_SURFEL_LABELER_SELECT_OVERLAPPING_COMMAND, TRUE, FALSE, FALSE);
}



int R3SurfelLabeler::
SelectAllObjects(RNBoolean unlabeled_only)
{
  // Check scene
  if (!scene) return 0;
  if (!SurfelVisibility()) return 0;

  // Create array of picked objects
  RNArray<R3SurfelObject *> picked_objects;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (!object->Name()) continue;
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    picked_objects.Insert(object);
  }

  // Select objects
  return SelectObjects(picked_objects, R3_SURFEL_LABELER_SELECT_ALL_COMMAND, FALSE, FALSE, FALSE);
}



int R3SurfelLabeler::
SelectSuggestedObject(RNBoolean unlabeled_only) 
{
  // Check scene
  if (!scene) return 0;

  // Set message
  SetMessage(NULL);

  // Pick object
  R3SurfelObject *best_object = NULL;
  RNScalar least_time = FLT_MAX;
  RNScalar least_confidence = FLT_MAX;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (!object->Name()) continue;
    if (object->Parent() != scene->RootObject()) continue;
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    R3SurfelLabelAssignment *assignment = object->CurrentLabelAssignment();
    RNScalar confidence = (assignment) ? assignment->Confidence() : 0;
    RNScalar select_time = -1;
    if (object_selection_times.size() > (unsigned int) object->SceneIndex()) {
      select_time = object_selection_times[object->SceneIndex()] + 0.25 * RNRandomScalar();
    }
    if (confidence < least_confidence) {
      least_confidence = confidence;
      least_time = select_time;
      best_object = object;
    }
    else if ((confidence == least_confidence) && (select_time > 0)) {
      if (select_time < least_time) {
        least_confidence = confidence;
        least_time = select_time;
        best_object = object;
      }
    }
  }

  // Set message
  if (best_object) {
    R3SurfelLabel *label = best_object->CurrentLabel();
    const char *label_name = (label) ? label->Name() : "NoLabel";
    const char *object_name = (best_object->Name()) ? best_object->Name() : "without name";
    SetMessage("Selected object %s / %s", object_name, label_name);
  }

  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_SELECT_SUGGESTED_COMMAND);
  
  // Empty object selections
  EmptyObjectSelections();

  // Insert best object into selected objects
  if (best_object) {
    // Select object
    InsertObjectSelection(best_object);

    // Select image
    R3Point centroid = best_object->Centroid();
    R3SurfelImage *image = scene->FindImageByBestView(centroid, R3posz_vector);
    SelectImage(image, FALSE, FALSE);

    // Select point
    SelectPoint(best_object);

    // Set center point
    SetCenterPoint(centroid);

    // Set viewer's camera viewpoint
    R3Camera camera = viewer.Camera();
    R3Box bbox = best_object->BBox();
    RNScalar radius = bbox.DiagonalRadius();
    if (radius < 0.1) radius = 0.1;
    R3Vector towards = (image)? image->Towards(): R3Vector(0, 0.25 * RN_PI, 0);
    towards[2] = -0.25 * RN_PI; // looking down a bit
    towards.Normalize();
    R3Vector right = towards % R3posz_vector;
    right.Normalize();
    R3Vector up = right % towards;
    up.Normalize();
    camera.Reorient(towards, up);
    camera.SetOrigin(centroid - towards * (16 * radius));
    viewer.SetCamera(camera);
  }

  // End logging command
  EndCommand();

  // Update obb manipulator
  UpdateOBBManipulator(TRUE, TRUE);

  // Return success
  return 1;
}



int R3SurfelLabeler::
AssignLabelToObject(R3SurfelObject *object, R3SurfelLabel *label,
  RNScalar confidence, int originator, int command)
{
  // Set message
  SetMessage(NULL);

  // Set center point 
  if (object) {
    SetCenterPoint(object->Centroid());
  }

  // Set message
  const char *label_name = (label->Name()) ? label->Name() : "without name";
  const char *object_name = (object->Name()) ? object->Name() : "without name";
  SetMessage("Assigned label %s to object %s", label_name, object_name);

  // Begin logging command
  BeginCommand(command);
  
  // Create/insert label assignment
  R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, label, confidence, originator);
  InsertLabelAssignment(assignment);

  // Predict label assignments for all other objects
  if (classify_after_change_label) classifier.PredictLabelAssignments();

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // Empty object selections
  EmptyObjectSelections();

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
AssignLabelToSelectedObjects(R3SurfelLabel *label)
{
  // Check everything
  if (!scene) return 0;
  if (!label) return 0;
  if (NObjectSelections() == 0) return 0;

  // Set message
  const char *label_name = (label->Name()) ? label->Name() : "without name";
  SetMessage("Assigned label %s to %d selected objects", label_name, NObjectSelections());
  if (NObjectSelections() == 1) {
    R3SurfelObject *object = ObjectSelection(0);   
    const char *object_name = (object->Name()) ? object->Name() : "without name";
    SetMessage("Assigned label %s to object %s", label_name, object_name);
  }

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_LABEL_SELECTION_COMMAND);
  
  // Create assignments
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, label, 1, R3_SURFEL_HUMAN_ORIGINATOR);
    InsertLabelAssignment(assignment);
  }

  // Predict label assignments for all other objects
  if (classify_after_change_label) classifier.PredictLabelAssignments();

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // Empty object selections
  EmptyObjectSelections();

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
AssignLabelToPickedObject(R3SurfelLabel *label, int x, int y)
{
  // Pick object
  R3SurfelNode *node = PickNode(x, y, NULL, NULL, NULL, TRUE);
  if (!node) return 0;
  R3SurfelObject *object = node->Object(TRUE);
  if (!object) return 0;

  // Find top level object
  while (object && object->Parent() && (object->Parent() != scene->RootObject())) {
    object = object->Parent();
  }

  // Assign label to object
  return AssignLabelToObject(object, label, 1,
    R3_SURFEL_HUMAN_ORIGINATOR,
    R3_SURFEL_LABELER_LABEL_PICKED_COMMAND);
}



static R3SurfelLabel *
BestNewLabel(R3SurfelScene *scene, R3SurfelObject *object)
{
  // Find best new label
  RNLength best_distance = FLT_MAX;
  R3SurfelLabel *best_label = NULL;
  R3SurfelLabel *current_label = object->CurrentLabel();
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *other_object = scene->Object(i);

    // Get/check label
    R3SurfelLabel *other_label = other_object->CurrentLabel();
    if (!other_label) continue;
    if (!other_label->Name()) continue;
    if (!strcmp(other_label->Name(), "Unknown")) continue;
    if (other_label == current_label) continue;

    // Estimate "distance"
    double distance = 0;

    // Consider bbox-bbox distance
    distance += R3Distance(object->BBox(), other_object->BBox());
    
    // Consider bbox-centroid distance
    distance += R3Distance(object->Centroid(), other_object->BBox());
    distance += R3Distance(object->BBox(), other_object->Centroid());
                           
    // Consider centroid-centroid distance
    distance += R3Distance(object->Centroid(), other_object->Centroid());

    // Check if found best label so far
    if (distance < best_distance) {
      best_label = other_label;
      best_distance = distance;
    }
  }

  // Return best label
  return best_label;
}



int R3SurfelLabeler::
AssignNewLabelToPickedObject(int x, int y)
{
  // Pick object
  R3SurfelNode *node = PickNode(x, y, NULL, NULL, NULL, TRUE);
  if (!node) return 0;
  R3SurfelObject *object = node->Object(TRUE);
  if (!object) return 0;

  // Find top level object
  while (object && object->Parent() && (object->Parent() != scene->RootObject())) {
    object = object->Parent();
  }

  // Find best new label
  R3SurfelLabel *best_label = BestNewLabel(scene, object);
  if (!best_label) return 0;

  // Assign best new label
  return AssignLabelToObject(object, best_label, 0.01,
    R3_SURFEL_MACHINE_ORIGINATOR,
    R3_SURFEL_LABELER_RELABEL_PICKED_COMMAND);
}



int R3SurfelLabeler::
AssignNewLabelToSelectedObjects(void)
{
  // Check everything
  if (!scene) return 0;
  if (NObjectSelections() == 0) return 0;

  // Set message
  SetMessage("Assigned new label(s) to %d selected objects", NObjectSelections());

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_RELABEL_SELECTION_COMMAND);
  
  // Create assignments
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    R3SurfelLabel *label = BestNewLabel(scene, object);
    if (label) {
      R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, label,
        0.01, R3_SURFEL_MACHINE_ORIGINATOR);
      InsertLabelAssignment(assignment);
    }
  }

  // Predict label assignments for all other objects
  if (classify_after_change_label) classifier.PredictLabelAssignments();

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // Empty object selections
  EmptyObjectSelections();

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
ConfirmLabelOnPickedObject(int x, int y)
{
  // Set message
  SetMessage(NULL);

  // Pick object
  R3SurfelNode *node = PickNode(x, y, NULL, NULL, NULL, TRUE);
  R3SurfelObject *object = (node) ? node->Object(TRUE) : NULL;
  if (!object) {
    SetMessage("Nothing selected");
    return 0;
  }

  // Set center point 
  if (object) {
    SetCenterPoint(object->Centroid());
  }

  // Get/check ground truth label
  R3SurfelLabel *label = object->HumanLabel();
  if (label) {
    const char *label_name = (label->Name()) ? label->Name() : "without name";
    const char *object_name = (object->Name()) ? object->Name() : "without name";
    SetMessage("Reconfirmed label %s on object %s", label_name, object_name);
    return 1;
  }

  // Get/check predicted label
  label = object->PredictedLabel();
  if (!label) {
    const char *object_name = (object->Name()) ? object->Name() : "without name";
    SetMessage("Object %s has no predicted label", object_name);
    return 0;
  }

  // Set message
  const char *label_name = (label->Name()) ? label->Name() : "without name";
  const char *object_name = (object->Name()) ? object->Name() : "without name";
  SetMessage("Confirmed label %s on object %s", label_name, object_name);

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_CONFIRM_PICKED_COMMAND);
  
  // Create/insert label assignment
  R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, label, 1);
  InsertLabelAssignment(assignment);

  // Predict label assignments for all other objects
  if (classify_after_change_label) classifier.PredictLabelAssignments();

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // Empty object selections
  EmptyObjectSelections();

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
ConfirmLabelsOnSelectedObjects(void)
{
  // Set message
  SetMessage(NULL);

  // Check number of selections
  if (NObjectSelections() == 0) {
    SetMessage("No objects selected");
    return 0;
  }

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_CONFIRM_SELECTION_COMMAND);
  
  // Confirm label for all selected objects
  int count = 0;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);

    // Get/check label
    R3SurfelLabel *label = object->PredictedLabel();
    if (!label) continue;

    // Create/insert label assignment
    R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, label, 1);
    InsertLabelAssignment(assignment);
    count++;
  }

  // Check if confirmed any labels
  if (count > 0) {
    // Predict label assignments for all other objects
    if (classify_after_change_label) classifier.PredictLabelAssignments();

    // Predict object segmentations for all other objects
    if (segment_after_change_label) segmenter.PredictObjectSegmentations();

    // Empty object selections
    EmptyObjectSelections();

    // Set message
    SetMessage("Confirmed labels for %d objects", count);
  }
  else {
    // Set message
    SetMessage("None of the selected objects have predicted labels to confirm");
  }

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
ConfirmLabelsOnAllObjects(void)
{
  // Set message
  SetMessage(NULL);

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_CONFIRM_ALL_COMMAND);
  
  // Confirm label for all objects
  int count = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);

    // Get/check label
    if (object->GroundTruthLabel()) continue;
    R3SurfelLabel *label = object->PredictedLabel();
    if (!label) continue;

    // Create/insert label assignment
    R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, label, 1);
    InsertLabelAssignment(assignment);
    count++;
  }

  // Empty object selections
  EmptyObjectSelections();

  // Set message
  SetMessage("Confirmed label for %d objects", count);

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
MergeSelectedObjects(void)
{
  // Just checking
  assert(IsValid());

  // Set message
  SetMessage(NULL);

  // Check number of selections
  if (NObjectSelections() < 2) {
    SetMessage("At least two objects must be selected to merge");
    return 0;
  }

  // Get/check grandparent
  R3SurfelObject *grandparent = ObjectSelection(0)->Parent();
  if (!grandparent) {
    SetMessage("Cannot merge root object");
    return 0;
  }

  // Get/check label and attribute flags
  R3SurfelLabelAssignment *assignment = ObjectSelection(0)->CurrentLabelAssignment();
  R3SurfelLabel *label = (assignment) ? assignment->Label() : scene->FindLabelByName("Unknown");
  int originator = (assignment) ? assignment->Originator() : R3_SURFEL_MACHINE_ORIGINATOR;
  RNScalar confidence = (assignment) ? assignment->Confidence() : 0;
  RNFlags attribute_flags = ObjectSelection(0)->Flags() & R3_SURFEL_ALL_ATTRIBUTES;
  
  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_MERGE_SELECTION_COMMAND);
  
  // Create new parent object
  char parent_name[4096];
  static int counter = 1;
  sprintf(parent_name, "MERGE_%d", counter++);
  R3SurfelObject *parent = CreateObject(grandparent, parent_name);
  if (!parent) { EndCommand(); return 0; }

  // Assign label to parent object
  if (label) {
    R3SurfelLabelAssignment *a = new R3SurfelLabelAssignment(parent, label, confidence, originator);
    scene->InsertLabelAssignment(a);
  }

  // Assign attribute flags to parent object
  parent->SetFlags(parent->Flags() | attribute_flags);

  // Compute feature vector
  RNScalar weight = 0;
  R3SurfelFeatureVector vector(scene->NFeatures());
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    vector += object->Complexity() * object->FeatureVector();
    weight += object->Complexity();
  }

  // Assign feature vector to parent object
  if (weight > 0) vector /= weight;
  parent->SetFeatureVector(vector);

  // Set parent for all selected objects
  int count = 0;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    SetObjectParent(object, parent);
    count++;
  }

  // Update OBB for parent object
  UpdateObjectOrientedBBox(parent);

  // Empty object selections
  EmptyObjectSelections();

  // Insert object selection
  InsertObjectSelection(parent);

  // Set message
  SetMessage("Merged %d objects", count);

  // End logging command
  EndCommand();

  // Update OBB manipulator
  UpdateOBBManipulator(TRUE, TRUE);
  
  // Invalidate VBO colors
  InvalidateVBO();

  // Just checking
  assert(IsValid());

  // Return success
  return 1;
}



int R3SurfelLabeler::
UnmergeSelectedObjects(void)
{
  // Just checking
  assert(IsValid());

  // Set message
  SetMessage(NULL);

  // Check number of selections
  if (NObjectSelections() < 1) {
    SetMessage("At least one object must be selected to unmerge");
    return 0;
  }

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_UNMERGE_SELECTION_COMMAND);
  
  // Empty object selections
  RNArray<R3SurfelObject *> selection_objects_copy = selection_objects;
  EmptyObjectSelections();

  // Unmerge all selected objects
  int count = 0;
  for (int i = 0; i < selection_objects_copy.NEntries(); i++) {
    R3SurfelObject *object = selection_objects_copy.Kth(i);

    // Get/check parent
    R3SurfelObject *parent = object->Parent();
    if (!parent) continue;
  
    // Get/check assignment info
    R3SurfelLabelAssignment *assignment = object->CurrentLabelAssignment();
    R3SurfelLabel *label = (assignment) ? assignment->Label() : scene->FindLabelByName("Unknown");
    int originator = (assignment) ? assignment->Originator() : R3_SURFEL_MACHINE_ORIGINATOR;
    RNScalar confidence = (assignment) ? assignment->Confidence() : 0;
  
    // Find previous assignments
    RNArray<R3SurfelLabelAssignment *> previous_assignments;
    for (int i = 0; i < object->NLabelAssignments(); i++) {
      R3SurfelLabelAssignment *a = object->LabelAssignment(i);
      if (a->Originator() == R3_SURFEL_GROUND_TRUTH_ORIGINATOR) continue;
      previous_assignments.Insert(a);
    }

    // Remove previous assignments
    for (int i = 0; i < previous_assignments.NEntries(); i++) {
      R3SurfelLabelAssignment *a = previous_assignments.Kth(i);
      if (!RemoveLabelAssignment(a)) continue;
    }

    // Elevate children in hierarchy
    while (object->NParts() > 0) {
      R3SurfelObject *part = object->Part(0);

      // Assign part's label
      if (label) {
        R3SurfelLabelAssignment *a = new R3SurfelLabelAssignment(part, label, confidence, originator);
        InsertLabelAssignment(a);
      }

      // Set part's parent
      SetObjectParent(part, parent);

      // Insert object selection
      InsertObjectSelection(part);

      // Update part OBB
      UpdateObjectOrientedBBox(part);

      // Increment counter
      count++;
    }

    // Remove object from hierarchy
    // SetObjectParent(object, NULL);
  }

  // Set message
  SetMessage("Unmerged %d objects", count);

  // End logging command
  EndCommand();

  // Update OBB manipulator
  UpdateOBBManipulator(TRUE, TRUE);
  
  // Invalidate VBO colors
  InvalidateVBO();

  // Just checking
  assert(IsValid());

  // Return success
  return 1;
}



int R3SurfelLabeler::
SplitObject(R3SurfelObject *object, R3SurfelObject *parent, const R3SurfelConstraint& constraint,
  RNArray<R3SurfelObject *> *resultA, RNArray<R3SurfelObject *> *resultB)
{
  // Just checking
  assert(object);
  assert(strcmp(object->Name(), "Root"));

  // Get assignment and attribute info
  R3SurfelObject *ancestor = object;
  while (ancestor && ancestor->Parent() && (ancestor->Parent() != scene->RootObject())) ancestor = ancestor->Parent();
  R3SurfelLabelAssignment *assignment = ancestor->CurrentLabelAssignment();
  R3SurfelLabel *label = (assignment) ? assignment->Label() : scene->FindLabelByName("Unknown");
  int originator = (assignment) ? assignment->Originator() : R3_SURFEL_MACHINE_ORIGINATOR;
  double confidence = (assignment) ? assignment->Confidence() : 0;
  RNFlags attribute_flags = object->Flags() & R3_SURFEL_ALL_ATTRIBUTES;

  // Create constraint
  R3SurfelMultiConstraint multi_constraint;
  R3SurfelObjectConstraint object_constraint(object);
  multi_constraint.InsertConstraint(&constraint);
  multi_constraint.InsertConstraint(&object_constraint);

  // Split Nodes
  RNArray<R3SurfelNode *> nodesA, nodesB;
  if (object->NNodes() > 0) {
    // Create array of nodes
    RNArray<R3SurfelNode *> nodes;
    for (int i = 0; i < object->NNodes(); i++) {
      R3SurfelNode *node = object->Node(i);
      nodes.Insert(node);
    }

    // Split all nodes
    for (int i = 0; i < nodes.NEntries(); i++) {
      R3SurfelNode *node = nodes.Kth(i);
      SplitLeafNodes(node, constraint, &nodesA, &nodesB);
    }
  }

  // Split parts
  RNArray<R3SurfelObject *> partsA, partsB;
  if (object->NParts() > 0) {
    // Create array of parts
    RNArray<R3SurfelObject *> parts;
    for (int i = 0; i < object->NParts(); i++) {
      R3SurfelObject *part = object->Part(i);
      parts.Insert(part);
    }

    // Split parts  
    for (int i = 0; i < parts.NEntries(); i++) {
      R3SurfelObject *part = parts.Kth(i);
      SplitObject(part, object, constraint, &partsA, &partsB);
    }
  }

  // Split object
  if (nodesB.IsEmpty() && partsB.IsEmpty()) {
    SetObjectParent(object, parent);
    if (resultA) resultA->Insert(object);
  }
  else if (nodesA.IsEmpty() && partsA.IsEmpty()) {
    SetObjectParent(object, parent);
    if (resultB) resultB->Insert(object);
  }
  else {
    // Create new objects
    R3SurfelObject *objectA = new R3SurfelObject();
    R3SurfelObject *objectB = new R3SurfelObject();
    if (!objectA || !objectB) return 0;
      
    // Insert objects into scene
    scene->InsertObject(objectA, object);
    scene->InsertObject(objectB, object);
    
    // Set names
    const char *name = object->Name();
    if (!name) name = "SPLIT";
    char nameA[4096], nameB[4096];
    sprintf(nameA, "%s_A", name);
    sprintf(nameB, "%s_B", name);
    objectA->SetName(nameA);
    objectB->SetName(nameB);
    
    // Set feature vectors ???
    objectA->SetFeatureVector(object->FeatureVector());
    objectB->SetFeatureVector(object->FeatureVector());

    // Set attribute flags
    objectA->SetFlags(objectA->Flags() | attribute_flags);
    objectB->SetFlags(objectB->Flags() | attribute_flags);

    // Create assignments
    R3SurfelLabelAssignment *assignmentA = new R3SurfelLabelAssignment(objectA, label, confidence, originator);
    R3SurfelLabelAssignment *assignmentB = new R3SurfelLabelAssignment(objectB, label, confidence, originator);
    scene->InsertLabelAssignment(assignmentA);
    scene->InsertLabelAssignment(assignmentB);

    // Remove nodes from object
    while (object->NNodes() > 0) {
      R3SurfelNode *node = object->Node(0);
      object->RemoveNode(node);
    }

    // Insert nodes into objectA
    for (int j = 0; j < nodesA.NEntries(); j++) {
      R3SurfelNode *nodeA = nodesA.Kth(j);
      objectA->InsertNode(nodeA);
    }
  
    // Insert nodes into objectB
    for (int j = 0; j < nodesB.NEntries(); j++) {
      R3SurfelNode *nodeB = nodesB.Kth(j);
      objectB->InsertNode(nodeB);
    }

    // Move parts into objectA
    for (int j = 0; j < partsA.NEntries(); j++) {
      R3SurfelObject *partA = partsA.Kth(j);
      partA->SetParent(objectA);
    }
      
    // Move parts into objectB
    for (int j = 0; j < partsB.NEntries(); j++) {
      R3SurfelObject *partB = partsB.Kth(j);
      partB->SetParent(objectB);
    }
      
    // Set parents (use this function so can undo)
    SetObjectParent(objectA, parent);
    SetObjectParent(objectB, parent);
    // SetObjectParent(object, NULL);

    // Assign labels (use this function so can undo)
    InsertLabelAssignment(assignmentA);
    InsertLabelAssignment(assignmentB);
    
    // Insert objects into result
    if (resultA) resultA->Insert(objectA);
    if (resultB) resultB->Insert(objectB);
  }
  
  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
SplitSelectedObjects(void)
{
  // Just checking
  assert(IsValid());
  
  // Set message
  SetMessage(NULL);

  // Check selected objects
  if (NObjectSelections() == 0) {
    SetMessage("Must select object(s) if you want to split them");
    return 0;
  }
  
  // Check selected object parents 
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    if (!object->Parent()) {
      const char *object_name = (object->Name()) ? object->Name() : "without name";
      SetMessage("Cannot split object %s because it has no parent", object_name);
      return 0;
    }
  }

  // Create split constraint
  R3SurfelConstraint *constraint = NULL;
  RNBoolean update_obb_manipulator = TRUE;
  static const int keep_obb_orientation = 0;
  if (!R2Contains(rubber_line_points[0], rubber_line_points[1])) {
    // Create plane constraint based on split line
    R3Ray ray0 = viewer.WorldRay(rubber_line_points[0].X(), rubber_line_points[0].Y());
    R3Ray ray1 = viewer.WorldRay(rubber_line_points[1].X(), rubber_line_points[1].Y());
    R3Plane plane(ray0.Start(), ray0.Vector(), ray1.Vector());
    static R3SurfelPlaneConstraint plane_constraint(R3null_plane, TRUE, FALSE, TRUE);
    plane_constraint = R3SurfelPlaneConstraint(plane, TRUE, FALSE, FALSE);
    constraint = &plane_constraint;
  }
  else if (num_rubber_polygon_points > 1) {
    // Create view constraint based on split polygon
    static R2Grid split_image;
    R2Polygon polygon(rubber_polygon_points, num_rubber_polygon_points);
    R2Viewport viewport = viewer.Viewport();
     split_image.Resample(viewport.Width(), viewport.Height());
    int end_condition = (click_polygon_active) ? 1 : 0;
    RasterizeSDF(split_image, polygon, end_condition);
    split_image.Threshold(0, 0, 1);
    static R3SurfelViewConstraint view_constraint(viewer, &split_image, FALSE);
    view_constraint = R3SurfelViewConstraint(viewer, &split_image, FALSE);
    constraint = &view_constraint;
  }
  else if (IsOBBManipulatorVisible()) {
    // Create obb constraint based on obb manipulator
    static R3SurfelOrientedBoxConstraint obb_constraint(R3null_oriented_box);
    obb_constraint = R3SurfelOrientedBoxConstraint(obb_manipulator.OrientedBox());
    constraint = &obb_constraint;
    update_obb_manipulator = FALSE;
  }
  else {
    // Print error message
    SetMessage("Must specify a region if you want to split selected object(s)");
    return 0;
  }

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_SPLIT_SELECTION_COMMAND);

  // Empty object selections
  RNArray<R3SurfelObject *> copy_selection_objects = selection_objects;
  EmptyObjectSelections();

  // Split all selected objects
  RNArray<R3SurfelObject *> objectsA;
  for (int i = 0; i < copy_selection_objects.NEntries(); i++) {
    R3SurfelObject *object = copy_selection_objects.Kth(i);

    // Get info
    R3SurfelObject *parent = object->Parent();
    assert(parent == scene->RootObject());
    R3OrientedBox obb = object->CurrentOrientedBBox();
    if (obb.IsEmpty()) obb = R3unit_oriented_box;
      
    // Split objects
    RNArray<R3SurfelObject *> objsA, objsB;
    SplitObject(object, parent, *constraint, &objsA, &objsB);
    objectsA.Append(objsA);

    // Update oriented bounding boxes of new objects
    for (int j = 0; j < objsA.NEntries(); j++) {
      R3SurfelObject *objA = objsA.Kth(j);
      if (!keep_obb_orientation) UpdateObjectOrientedBBox(objA);
      else UpdateObjectOrientedBBox(objA, objA->Centroid(), obb.Axes());
    }
    for (int j = 0; j < objsB.NEntries(); j++) {
      R3SurfelObject *objB = objsB.Kth(j);
      if (!keep_obb_orientation) UpdateObjectOrientedBBox(objB);
      else UpdateObjectOrientedBBox(objB, objB->Centroid(), obb.Axes());
    }
  }
  
  // Select objects on one side of split polygon
  for (int i = 0; i < objectsA.NEntries(); i++) {
    R3SurfelObject *object = objectsA.Kth(i);
    assert(!object->Parent()->Parent());
    InsertObjectSelection(object);
  }
    
  // Set message
  SetMessage("Split %d objects", objectsA.NEntries());
  
  // End logging command
  EndCommand();

  // Update obb manipulator
  if (update_obb_manipulator) UpdateOBBManipulator(TRUE, TRUE, keep_obb_orientation);

  // Invalidate VBO colors
  InvalidateVBO();

  // Just checking
  assert(IsValid());

  // Empty undo stack ???
  // undo_stack.Empty();
  // undo_index = -1;

  // Return success
  return 1;
}



static RNBoolean 
DoAllSelectedObjectsHaveAttribute(const R3SurfelLabeler *labeler, RNFlags attribute)
{
  // Check if there are any selected objects
  if (labeler->NObjectSelections() == 0) return FALSE;
  
  // Check all selected objects
  for (int i = 0; i < labeler->NObjectSelections(); i++) {
    R3SurfelObject *object = labeler->ObjectSelection(i);
    if (!object->Flags()[attribute]) return FALSE;
  }

  // Passed all tests
  return TRUE;
}



int R3SurfelLabeler::
AssignAttributeToPickedObject(int x, int y,
  RNFlags attribute, const char *attribute_name, int value)
{
  // Check everything
  if (!scene) return 0;

  // Pick object
  R3SurfelNode *node = PickNode(x, y, NULL, NULL, NULL, TRUE);
  if (!node) return 0;
  R3SurfelObject *object = node->Object(TRUE);
  if (!object) return 0;

  // Find top level object
  while (object && object->Parent() && (object->Parent() != scene->RootObject())) {
    object = object->Parent();
  }

  // Resolve value -1
  if (value < 0) {
    if (object->Flags() & attribute) value = FALSE;
    else value = TRUE;
  }
  
  // Set center point 
  SetCenterPoint(object->Centroid());

  // Set message
  const char *action = (value) ? "Assigned" : "Removed";
  const char *preposition = (value) ? "to" : "from";
  const char *object_name = (object->Name()) ? object->Name() : "without name";
  SetMessage("%s attribute %s %s object %s", action, attribute_name, preposition, object_name);

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_ASSIGN_ATTRIBUTE_COMMAND);
  
  // Assign attributes
  AssignAttribute(object, attribute, value);

  // Empty object selections
  EmptyObjectSelections();

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
AssignAttributeToSelectedObjects(RNFlags attribute,
  const char *attribute_name, int value)
{
  // Check everything
  if (!scene) return 0;
  if (NObjectSelections() == 0) return 0;

  // Resolve value -1
  if (value < 0) {
    if (DoAllSelectedObjectsHaveAttribute(this, attribute)) value = FALSE;
    else value = TRUE;
  }
  
  // Set message
  const char *action = (value) ? "Assigned" : "Removed";
  const char *preposition = (value) ? "to" : "from";
  SetMessage("%s attribute %s %s %d selected objects", action, attribute_name, preposition, NObjectSelections());
  if (NObjectSelections() == 1) {
    R3SurfelObject *object = ObjectSelection(0);   
    const char *object_name = (object->Name()) ? object->Name() : "without name";
    SetMessage("%s attribute %s %s object %s", action, attribute_name, preposition, object_name);
  }

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_ASSIGN_ATTRIBUTE_COMMAND);
  
  // Assign attributes
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    AssignAttribute(object, attribute, value);
  }

  // Empty object selections
  EmptyObjectSelections();

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
AssignOBBToSelectedObject(const R3OrientedBox& obb, RNScalar confidence, int originator)
{
  // Check everything
  if (!scene) return 0;

  // Check object selections
  if (NObjectSelections() != 1) {
    SetMessage("You must select exactly one object before setting its bounding box");
    return 0;
  }

  // Get selected object
  R3SurfelObject *object = ObjectSelection(0);

  // Set message
  const char *object_name = (object->Name()) ? object->Name() : "without name";
  SetMessage("Adjusted bounding box for object %s", object_name);


  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_ASSIGN_OBB_COMMAND);

  // Set oriented box
  AssignOBB(object, obb, confidence, originator);
  
  // End logging command
  EndCommand();

  // Return success
  return 1;
}



int R3SurfelLabeler::
UndoCommandOfType(int command_type)
{
  // Check scene
  if (!scene) return 0;
  assert(IsValid());


  // Reset message
  SetMessage(NULL);

  // Check undo stack
  if (undo_index < 0) {
    // SetMessage("Nothing to undo");
    return 0;
  }

  // Check command type
  if ((command_type < 0) || (command_type >= R3_SURFEL_LABELER_NUM_COMMANDS)) {
    // SetMessage("Can't undo unrecognized command type: %d", command_type);
    return 0;
  }

  // Check command at top of undo stack
  R3SurfelLabelerCommand *command = undo_stack.Kth(undo_index);
  if (command->type != command_type) {
    // SetMessage("Last command is not of type: %s", command_names[command_type]);
    return 0;
  }

  // Undo last command
  return Undo();
}



int R3SurfelLabeler::
Undo(void)
{
  // Check scene
  if (!scene) return 0;
  assert(IsValid());

  // Check undo stack
  if (undo_index < 0) {
    SetMessage("Nothing to undo");
    return 0;
  }

  // Pop command from undo stack
  R3SurfelLabelerCommand *command = undo_stack.Kth(undo_index--);

  // Set camera
  viewer.SetCamera(command->camera);

  // Set center point
  SetCenterPoint(command->center_point);

  // Set message
  SetMessage("Undid last %s command", command_names[command->type]);

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_UNDO_COMMAND);

  // Remove inserted label assignments
  for (int i = 0; i < command->inserted_label_assignments.NEntries(); i++) {
    R3SurfelLabelAssignment *assignment = command->inserted_label_assignments.Kth(i);
    RemoveLabelAssignment(assignment);
  }

  // Insert removed label assignments
  for (int i = 0; i < command->removed_label_assignments.NEntries(); i++) {
    R3SurfelLabelAssignment *assignment = command->removed_label_assignments.Kth(i);
    InsertLabelAssignment(assignment);
  }

  // Remove inserted object selections
  for (int i = 0; i < command->inserted_object_selections.NEntries(); i++) {
    R3SurfelObject *object = command->inserted_object_selections.Kth(i);
    RemoveObjectSelection(object);
  }

  // Insert removed object selections
  for (int i = 0; i < command->removed_object_selections.NEntries(); i++) {
    R3SurfelObject *object = command->removed_object_selections.Kth(i);
    InsertObjectSelection(object);
  }

  // Unperform parent assignments
  for (int i = 0; i < command->part_parent_assignments.NEntries(); i++) {
    R3SurfelObject *part = command->part_parent_assignments.Kth(i);
    R3SurfelObject *removed_parent = command->removed_parent_assignments.Kth(i);
    part->SetParent(removed_parent);
  }

  // Unperform attribute assignments
  for (unsigned int i = 0; i < command->attribute_assignments.size(); i++) {
    R3SurfelAttributeAssignment& assignment = command->attribute_assignments[i];
    R3SurfelObject *object = assignment.object;
    RNFlags flags = object->Flags();
    if (!assignment.previous_value) flags.Remove(assignment.attribute);
    else flags.Add(assignment.attribute);
    object->SetFlags(flags);
  }

  // Unperform obb assignments
  for (unsigned int i = 0; i < command->obb_assignments.size(); i++) {
    R3SurfelOBBAssignment& assignment = command->obb_assignments[i];
    if (assignment.object) SetObjectOBBProperty(assignment.object,
      assignment.previous_obb, assignment.previous_confidence, assignment.previous_originator);
  }

  // Predict label assignments for all objects
  if (classify_after_change_label) classifier.PredictLabelAssignments();

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // End command
  EndCommand();

  // Update OBB manipulator
  UpdateOBBManipulator(TRUE, TRUE);
  
  // Invalidate VBO colors
  InvalidateVBO();

  // Just checking
  assert(IsValid());

  // Return success
  return 1;
}



int R3SurfelLabeler::
Redo(void) 
{
  // Not supported, for now
  return 1;

  // Check everything
  if (!scene) return 0;
  if (undo_stack.IsEmpty() || (undo_index >= undo_stack.NEntries()-1)) {
    SetMessage("Nothing to redo");
    return 0;
  }

  // Set message
  SetMessage("Redid previous command");

  // Get last command
  R3SurfelLabelerCommand *command = undo_stack.Kth(++undo_index);

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_REDO_COMMAND);

  // Set camera
  viewer.SetCamera(command->camera);

  // Remove label assignments
  for (int i = 0; i < command->removed_label_assignments.NEntries(); i++) {
    R3SurfelLabelAssignment *assignment = command->removed_label_assignments.Kth(i);
    RemoveLabelAssignment(assignment);
  }

  // Insert label assignments
  for (int i = 0; i < command->inserted_label_assignments.NEntries(); i++) {
    R3SurfelLabelAssignment *assignment = command->inserted_label_assignments.Kth(i);
    InsertLabelAssignment(assignment);
  }

  // Predict label assignments for all other objects
  if (classify_after_change_label) classifier.PredictLabelAssignments();

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // Remove object selections
  for (int i = 0; i < command->removed_object_selections.NEntries(); i++) {
    R3SurfelObject *object = command->removed_object_selections.Kth(i);
    RemoveObjectSelection(object);
  }

  // Insert object selections
  for (int i = 0; i < command->inserted_object_selections.NEntries(); i++) {
    R3SurfelObject *object = command->inserted_object_selections.Kth(i);
    InsertObjectSelection(object);
  }

  // Perform parent assignments
  for (int i = 0; i < command->part_parent_assignments.NEntries(); i++) {
    R3SurfelObject *part = command->part_parent_assignments.Kth(i);
    R3SurfelObject *inserted_parent = command->inserted_parent_assignments.Kth(i);
    part->SetParent(inserted_parent);
  }

  // Perform attribute assignments
  for (unsigned int i = 0; i < command->attribute_assignments.size(); i++) {
    R3SurfelAttributeAssignment& assignment = command->attribute_assignments[i];
    R3SurfelObject *object = assignment.object;
    RNFlags flags = object->Flags();
    if (!assignment.new_value) flags.Remove(assignment.attribute);
    else flags.Add(assignment.attribute);
    object->SetFlags(flags);
  }

  // Perform obb assignments
  for (unsigned int i = 0; i < command->obb_assignments.size(); i++) {
    R3SurfelOBBAssignment& assignment = command->obb_assignments[i];
    if (assignment.object) SetObjectOBBProperty(assignment.object,
      assignment.obb, assignment.confidence, assignment.originator);
  }

  // End logging command
  EndCommand();

  // Update OBB manipulator
  UpdateOBBManipulator(TRUE, TRUE);

  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}



int R3SurfelLabeler::
Reset(void)
{
  // Check everything
  if (!scene) return 0;
  if (current_command) return 0;

  // Set message
  SetMessage("Reset");

  // Remove all object selections
  EmptyObjectSelections();

  // Remove all label assignments
  EmptyLabelAssignments();

  // Log command
  BeginCommand(R3_SURFEL_LABELER_RESET_COMMAND);
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Empty undo stack
  undo_stack.Empty();
  undo_index = -1;

  // Return success
  return 1;
}


////////////////////////////////////////////////////////////////////////
// LABELING STATISTICS
////////////////////////////////////////////////////////////////////////

RNScalar R3SurfelLabeler::
CurrentLabelPrecision(void) const
{
  // Count predictions
  int correct_predictions = 0;
  int total_predictions = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelLabel *ground_truth_label = object->GroundTruthLabel();
    if (!ground_truth_label) continue;
    R3SurfelLabel *current_label = object->CurrentLabel();
    if (current_label && strcmp(current_label->Name(), "Unknown")) {
      if (current_label == ground_truth_label) correct_predictions++;
      total_predictions++;
    }
  }

  // Return precision
  if (total_predictions > 0) return (RNScalar) correct_predictions / (RNScalar) total_predictions;
  else return 1;
}



RNScalar R3SurfelLabeler::
CurrentLabelRecall(void) const
{
  // Count predictions
  int correct_predictions = 0;
  int total_objects = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelLabel *ground_truth_label = object->GroundTruthLabel();
    if (!ground_truth_label) continue;
    R3SurfelLabel *current_label = object->CurrentLabel();
    if (current_label && strcmp(current_label->Name(), "Unknown")) {
      if (current_label == ground_truth_label) correct_predictions++;
    }
    total_objects++;
  }

  // Return precision
  if (total_objects > 0) return (RNScalar) correct_predictions / (RNScalar) total_objects;
  else return 0;
}



RNScalar R3SurfelLabeler::
CurrentLabelFMeasure(void) const
{
  // Return F-measure
  RNScalar precision = CurrentLabelPrecision();
  RNScalar recall = CurrentLabelRecall();
  RNScalar sum = precision + recall;
  if (sum > 0) return 2 * precision * recall / sum;
  else return 0;
}



RNScalar R3SurfelLabeler::
HumanLabelPrecision(void) const
{
  // Count predictions
  int correct_predictions = 0;
  int total_predictions = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelLabel *ground_truth_label = object->GroundTruthLabel();
    if (!ground_truth_label) continue;
    R3SurfelLabel *human_label = object->HumanLabel();
    if (human_label && strcmp(human_label->Name(), "Unknown")) {
      if (human_label == ground_truth_label) correct_predictions++;
      total_predictions++;
    }
  }

  // Return precision
  if (total_predictions > 0) return (RNScalar) correct_predictions / (RNScalar) total_predictions;
  else return 1;
}



RNScalar R3SurfelLabeler::
HumanLabelRecall(void) const
{
  // Count predictions
  int correct_predictions = 0;
  int total_objects = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelLabel *ground_truth_label = object->GroundTruthLabel();
    if (!ground_truth_label) continue;
    R3SurfelLabel *human_label = object->HumanLabel();
    if (human_label && strcmp(human_label->Name(), "Unknown")) {
      if (human_label == ground_truth_label) correct_predictions++;
    }
    total_objects++;
  }

  // Return precision
  if (total_objects > 0) return (RNScalar) correct_predictions / (RNScalar) total_objects;
  else return 0;
}



RNScalar R3SurfelLabeler::
HumanLabelFMeasure(void) const
{
  // Return F-measure
  RNScalar precision = HumanLabelPrecision();
  RNScalar recall = HumanLabelRecall();
  RNScalar sum = precision + recall;
  if (sum > 0) return 2 * precision * recall / sum;
  else return 0;
}



RNScalar R3SurfelLabeler::
PredictedLabelPrecision(void) const
{
  // Count predictions
  int correct_predictions = 0;
  int total_predictions = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelLabel *ground_truth_label = object->GroundTruthLabel();
    if (!ground_truth_label) continue;
    R3SurfelLabel *human_label = object->HumanLabel();
    if (human_label) continue;
    R3SurfelLabel *predicted_label = object->PredictedLabel();
    if (predicted_label && strcmp(predicted_label->Name(), "Unknown")) {
      if (predicted_label == ground_truth_label) correct_predictions++;
      total_predictions++;
    }
  }

  // Return precision
  if (total_predictions > 0) return (RNScalar) correct_predictions / (RNScalar) total_predictions;
  else return 1;
}



RNScalar R3SurfelLabeler::
PredictedLabelRecall(void) const
{
  // Count predictions
  int correct_predictions = 0;
  int total_objects = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelLabel *ground_truth_label = object->GroundTruthLabel();
    if (!ground_truth_label) continue;
    R3SurfelLabel *human_label = object->HumanLabel();
    if (human_label) continue;
    R3SurfelLabel *predicted_label = object->PredictedLabel();
    if (predicted_label && strcmp(predicted_label->Name(), "Unknown")) {
      if (predicted_label == ground_truth_label) correct_predictions++;
    }
    total_objects++;
  }

  // Return precision
  if (total_objects > 0) return (RNScalar) correct_predictions / (RNScalar) total_objects;
  else return 0;
}



RNScalar R3SurfelLabeler::
PredictedLabelFMeasure(void) const
{
  // Return F-measure
  RNScalar precision = PredictedLabelPrecision();
  RNScalar recall = PredictedLabelRecall();
  RNScalar sum = precision + recall;
  if (sum > 0) return 2 * precision * recall / sum;
  else return 0;
}



////////////////////////////////////////////////////////////////////////
// OBJECT SELECTION UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////

R3Box R3SurfelLabeler::
ObjectSelectionBBox(void) const
{
  // Compute bbox of object selections
  R3Box selection_bbox = R3null_box;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    selection_bbox.Union(object->BBox());
  }

  // Return bbox of object selections
  return selection_bbox;
}
  


void R3SurfelLabeler::
InsertObjectSelection(R3SurfelObject *object) 
{
  // Check scene
  if (!scene) return;

  // Update current command 
  if (current_command) current_command->inserted_object_selections.Insert(object);

  // Insert into selected objects
  selection_objects.Insert(object);

  // Update last selection time
  while (object_selection_times.size() <= (unsigned int) object->SceneIndex())
    object_selection_times.push_back(-1.0);
  object_selection_times[object->SceneIndex()] = start_timer.Elapsed();

  // Read blocks
  object->ReadBlocks(TRUE);
}



void R3SurfelLabeler::
RemoveObjectSelection(R3SurfelObject *object) 
{
  // Check scene
  if (!scene) return;

  // Update current command 
  if (current_command) current_command->removed_object_selections.Insert(object);

  // Insert into selected objects
  selection_objects.Remove(object);

  // Release blocks
  object->ReleaseBlocks(TRUE);
}



int R3SurfelLabeler::
EmptyObjectSelections(void) 
{
  // Check scene
  if (!scene) return 0;

  // Update current command 
  if (current_command) current_command->removed_object_selections.Append(selection_objects);

  // Release blocks of selected objects
  for (int i = 0; i < selection_objects.NEntries(); i++) {
    R3SurfelObject *object = selection_objects.Kth(i);
    object->ReleaseBlocks(TRUE);
  }

  // Empty object selection
  selection_objects.Empty();

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// LABEL ASSIGNMENT UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3SurfelLabeler::
InsertLabelAssignment(R3SurfelLabelAssignment *assignment)
{
  // Check everything
  if (!scene) return 0;
  if (!assignment) return 0;

  // Get convenient variables
  R3SurfelObject *object = assignment->Object();
  R3SurfelLabel *label = assignment->Label();

  // Check if label is already assigned to an equivalent object 
  for (int i = 0; i < object->NLabelAssignments(); i++) {
    R3SurfelLabelAssignment *a = object->LabelAssignment(i);
    if ((a->Label() == label) && 
        (a->Confidence() == assignment->Confidence()) && 
        (a->Originator() == assignment->Originator())) {
      return 0;
    }
  }

  // Find previous assignments
  RNArray<R3SurfelLabelAssignment *> previous_assignments;
  for (int i = 0; i < object->NLabelAssignments(); i++) {
    R3SurfelLabelAssignment *a = object->LabelAssignment(i);
    if (a->Originator() == R3_SURFEL_GROUND_TRUTH_ORIGINATOR) continue;
    previous_assignments.Insert(a);
  }

  // Remove previous assignments
  for (int i = 0; i < previous_assignments.NEntries(); i++) {
    R3SurfelLabelAssignment *a = previous_assignments.Kth(i);
    if (!RemoveLabelAssignment(a)) return 0;
  }

  // Update current command 
  if (current_command) current_command->inserted_label_assignments.Insert(assignment);

  // Insert assignment into scene
  scene->InsertLabelAssignment(assignment);

  // Update classifier
  classifier.UpdateAfterInsertLabelAssignment(assignment);

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // Return success
  return 1;
}



int R3SurfelLabeler::
RemoveLabelAssignment(R3SurfelLabelAssignment *assignment)
{
  // Check everything
  if (!scene) return 0;
  if (!assignment) return 0;

  // Update classifier
  classifier.UpdateBeforeRemoveLabelAssignment(assignment);

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // Update current command 
  if (current_command) current_command->removed_label_assignments.Insert(assignment);

  // Remove assignment from scene
  scene->RemoveLabelAssignment(assignment);

  // Return success
  return 1;
}



int R3SurfelLabeler::
EmptyLabelAssignments(void)
{
  // Check everything
  if (!scene) return 0;

  // Remove all label assignments
  RNArray<R3SurfelLabelAssignment *> tmp_assignments;
  for (int i = 0; i < scene->NLabelAssignments(); i++) 
    tmp_assignments.Insert(scene->LabelAssignment(i));
  for (int i = 0; i < tmp_assignments.NEntries(); i++) {
    R3SurfelLabelAssignment *assignment = tmp_assignments.Kth(i);
    RemoveLabelAssignment(assignment);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// OBJECT HIERARCHY UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////

R3SurfelLabel *R3SurfelLabeler::
CreateLabel(R3SurfelLabel *parent, const char *name)
{
  // Allocate label
  R3SurfelLabel *label = new R3SurfelLabel(name);
  if (!label) {
    RNFail("Unable to allocate label\n");
    return NULL;
  }

  // Insert label into scene
  if (!parent) parent = scene->RootLabel();
  scene->InsertLabel(label, parent);

  // Return label
  return label;
}



R3SurfelObject *R3SurfelLabeler::
CreateObject(R3SurfelObject *parent, const char *name)
{
  // Allocate object
  R3SurfelObject *object = new R3SurfelObject(name);
  if (!object) {
    RNFail("Unable to allocate object\n");
    return NULL;
  }

  // Insert object into scene
  if (!parent) parent = scene->RootObject();
  scene->InsertObject(object, NULL);
  if (parent) SetObjectParent(object, parent);

  // Initialize object info
  int object_index = object->SceneIndex();
  assert(object_index == scene->NObjects()-1);
  while (object_selection_times.size() <= (unsigned int) object_index) {
    object_selection_times.push_back(-1.0);
  }

  // Return object
  return object;
}



int R3SurfelLabeler::
SetObjectParent(R3SurfelObject *object, R3SurfelObject *parent)
{
  // Check everything
  if (!scene) return 0;
  if (!object) return 0;
  R3SurfelObject *old_parent = object->Parent();
  if (old_parent == parent) return 1;
  
  // Update current command 
  if (current_command) {
    current_command->part_parent_assignments.Insert(object);
    current_command->inserted_parent_assignments.Insert(parent);
    current_command->removed_parent_assignments.Insert(old_parent);
  }  

  // Set object parent
  object->SetParent(parent);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// OBJECT ATTRIBUTE ASSIGNMENT UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3SurfelLabeler::
AssignAttribute(R3SurfelObject *object, RNFlags attribute, RNBoolean value)
{
  // Check everything
  if (!scene) return 0;
  if (!object) return 0;

  // Check if attribute is already set
  RNBoolean previous_value = object->Flags()[attribute];
  if (previous_value == value) return 0;

  // Assign attribute
  RNFlags flags = object->Flags();
  if (!value) flags.Remove(attribute);
  else flags.Add(attribute);
  object->SetFlags(flags);

  // Add attribute assignment to command
  if (current_command) {
    R3SurfelAttributeAssignment assignment;
    assignment.object = object;
    assignment.attribute = attribute;
    assignment.previous_value = previous_value;
    assignment.new_value = value;
    current_command->attribute_assignments.push_back(assignment);
  }
  
  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// OBJECT OBB ASSIGNMENT UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3SurfelLabeler::
AssignOBB(R3SurfelObject *object, const R3OrientedBox& obb, RNScalar confidence, int originator)
{
  // Check everything
  if (!scene) return 0;
  if (!object) return 0;
  
  // Get previous obb
  R3OrientedBox previous_obb = R3null_oriented_box;
  RNScalar previous_confidence = 0;
  int previous_originator = R3_SURFEL_MACHINE_ORIGINATOR;
  if (object) {
    if (!GetObjectOBBProperty(object, &previous_obb,
      &previous_confidence, &previous_originator)) return 0;
  }

  // Set new obb
  SetObjectOBBProperty(object, obb, confidence, originator);

  // Add obb assignment to command
  if (current_command) {
    R3SurfelOBBAssignment assignment;
    assignment.object = object;
    assignment.obb = obb;
    assignment.confidence = confidence;
    assignment.originator = originator;
    assignment.previous_obb = previous_obb;
    assignment.previous_confidence = previous_confidence;
    assignment.previous_originator = previous_originator;
    current_command->obb_assignments.push_back(assignment);
  }
  
  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// INTERACTION
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
DrawRubberBox(RNBoolean front_buffer, RNBoolean xor_op) const
{
  // Check if rubber box is active
  if (!select_box_active) return;
    
  // Set front buffer
  if (front_buffer) {
    glDrawBuffer(GL_FRONT);
  }

  // Set logic op
  if (xor_op) {
    glLogicOp(GL_XOR);
    glEnable(GL_COLOR_LOGIC_OP);
  }
  
  // Set rendering modes
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(0);
  // glLineStipple(1, 0xFF00);
  // glEnable(GL_LINE_STIPPLE);

  // Set projection matrix
  glMatrixMode(GL_PROJECTION);  
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, window_width, 0, window_height);

  // Set model view matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Draw box
  R2BeginLine();
  RNLoadRgb(1.0, 1.0, 1.0);
  R2LoadPoint(rubber_box_corners[0][0], rubber_box_corners[0][1]);
  R2LoadPoint(rubber_box_corners[0][0], rubber_box_corners[1][1]);
  R2LoadPoint(rubber_box_corners[1][0], rubber_box_corners[1][1]);
  R2LoadPoint(rubber_box_corners[1][0], rubber_box_corners[0][1]);
  R2EndLine();

  // Reset projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Reset model view matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  // Reset rendering modes
  glDrawBuffer(GL_BACK);
  glDisable(GL_COLOR_LOGIC_OP);
  glDisable(GL_LINE_STIPPLE);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(1);
  glFlush();
}




void R3SurfelLabeler::
DrawRubberPolygon(RNBoolean front_buffer, RNBoolean xor_op) const
{
  // Check if rubber polygon is active
  if (!select_polygon_active && !split_polygon_active && !click_polygon_active) return;

  // Set front buffer
  if (front_buffer) {
    glDrawBuffer(GL_FRONT);
  }

  // Set logic op
  if (xor_op) {
    glLogicOp(GL_XOR);
    glEnable(GL_COLOR_LOGIC_OP);
  }
  
  // Set rendering modes
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(0);
  glLineWidth(3);
  // glLineStipple(1, 0xFF00);
  // glEnable(GL_LINE_STIPPLE);

  // Set projection matrix
  glMatrixMode(GL_PROJECTION);  
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, window_width, 0, window_height);

  // Set model view matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Draw polygon
  GLenum primitive = RN_GRFX_LINE_LOOP;
  if (num_rubber_polygon_points < 3) primitive = RN_GRFX_LINE_STRIP;
  if (split_polygon_active) primitive = RN_GRFX_LINE_STRIP;
  RNGrfxBegin(primitive);
  RNLoadRgb(1.0, 1.0, 1.0);
  for (int i = 0; i < num_rubber_polygon_points; i++) {
    R2LoadPoint(rubber_polygon_points[i][0], rubber_polygon_points[i][1]);
  }
  RNGrfxEnd();

  // Reset projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Reset model view matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  // Reset rendering modes
  glDrawBuffer(GL_BACK);
  glDisable(GL_COLOR_LOGIC_OP);
  glDisable(GL_LINE_STIPPLE);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(1);
  glDepthMask(1);
  glFlush();
}



void R3SurfelLabeler::
DrawRubberLine(RNBoolean front_buffer, RNBoolean xor_op) const
{
  // Check if rubber box is active
  if (!split_line_active) return;
  
  // Set front buffer
  if (front_buffer) {
    glDrawBuffer(GL_FRONT);
  }

  // Set logic op
  if (xor_op) {
    glLogicOp(GL_XOR);
    glEnable(GL_COLOR_LOGIC_OP);
  }
  
  // Set rendering modes
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(0);
  glPointSize(3);

  // Set projection matrix
  glMatrixMode(GL_PROJECTION);  
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, window_width, 0, window_height);

  // Set model view matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Draw line
  R2BeginLine();
  RNLoadRgb(1.0, 1.0, 1.0);
  R2LoadPoint(rubber_line_points[0][0], rubber_line_points[0][1]);
  R2LoadPoint(rubber_line_points[1][0], rubber_line_points[1][1]);
  R2EndLine();
  
  // Reset projection matrix
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Reset model view matrix
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  // Reset rendering modes
  glDrawBuffer(GL_BACK);
  glPointSize(1);
  glDisable(GL_COLOR_LOGIC_OP);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(1);
  glFlush();
}




////////////////////////////////////////////////////////////////////////
// MESSAGE
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
DrawMessage(void) const
{
  // Check stuff
  if (!message) return;
  if (!message_visibility) return;

  // Get convenient variables
  int width = viewer.Viewport().Width();
  int height = viewer.Viewport().Height();

  // Get position
  R2Box label_menu_bbox = LabelMenuBBox();
  double x = label_menu_bbox[1][0] + label_menu_item_height;
  double y = label_menu_item_height;

  // Get font
  void *font = RN_GRFX_BITMAP_HELVETICA_10;
  if (height > 300) font = RN_GRFX_BITMAP_HELVETICA_12;
  if (height > 600) font = RN_GRFX_BITMAP_HELVETICA_18;
  if (height > 1200) font = RN_GRFX_BITMAP_TIMES_ROMAN_24;
  
  // Set OpenGL modes
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(FALSE);

  // Draw message (haloed in background color)
  RNLoadRgb(background_color);
  R2DrawText(x-1, y-1, message, font);
  R2DrawText(x-1, y+1, message, font);
  R2DrawText(x+1, y-1, message, font);
  R2DrawText(x+1, y+1, message, font);
  RNLoadRgb(RNwhite_rgb - background_color);
  R2DrawText(x, y, message, font);

  // Reset OpenGL modes
  glDepthMask(TRUE);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}



////////////////////////////////////////////////////////////////////////
// STATUS
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
DrawStatus(void) const
{
  // Check stuff
  if (!status_visibility) return;

  // Get convenient variables
  int width = viewer.Viewport().Width();
  int height = viewer.Viewport().Height();

  // Get position
  R2Box label_menu_bbox = LabelMenuBBox();
  R2Point origin = label_menu_bbox[1];
  origin[0] += label_menu_item_height;
  origin[1] -= label_menu_item_height;

  // Get font
  void *font = RN_GRFX_BITMAP_HELVETICA_10;
  if (height > 600) font = RN_GRFX_BITMAP_HELVETICA_12;
  if (height > 1200) font = RN_GRFX_BITMAP_HELVETICA_18;
  
  // Set OpenGL modes
  glDisable(GL_LIGHTING);
  RNLoadRgb(1,1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(FALSE);

  // Draw status
  char buffer[1024];
  const char *surfel_shape = (shape_draw_flags & R3_SURFEL_DISC_DRAW_FLAG) ? "Ellipse" : "Point";
  sprintf(buffer, "ColorScheme = %s,  SurfelShape = %s,  PointSize = %.1f, Subsampling = %d, DrawLabeled = %s, Frame rate = %.1f",
    SurfelColorSchemeName(), surfel_shape, SurfelSize(), SubsamplingFactor(),
    (HumanLabeledObjectVisibility()) ? "Yes" : "No", FrameRate());
  R2DrawText(origin, buffer, font);

  // Reset OpenGL modes
  glDepthMask(TRUE);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}



////////////////////////////////////////////////////////////////////////
// COMMAND MENU
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
DrawCommandMenu(void) const
{
  // Only draw menu if it is visible
  if (!command_menu_visibility) return;

  // Get convenient variables
  int width = viewer.Viewport().Width();
  int height = viewer.Viewport().Height();
  int x = width - 164 - 20;
  int y = height - 42;

  // Set OpenGL modes
  glDisable(GL_LIGHTING);
  RNLoadRgb(1,1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  // Draw "Suggest an Object" command
  if (1) {
    R2Box box(x-4, y-4, x + 164, y + 26);
    RNLoadRgb(0.0, 0.2, 0.0);
    box.Draw();
    RNLoadRgb(1, 1, 1);
    box.Outline();
    R2DrawText(x, y, "Suggest an Object", RN_GRFX_BITMAP_HELVETICA_18); 
    y -= 32;
  }

  // Draw "Predict All" command
  if (0) {
    R2Box box(x-4, y-4, x + 164, y + 26);
    RNLoadRgb(0.0, 0.2, 0.0);
    box.Draw();
    RNLoadRgb(1, 1, 1);
    box.Outline();
    R2DrawText(x, y, "Predict All", RN_GRFX_BITMAP_HELVETICA_18); 
    y -= 32;
  }

  // Draw box around commands
  y += 32;
  RNLoadRgb(1, 1 , 1);
  R2Box(x - 4 - 8, y - 4 - 8, x + 164 + 8, height - 42 + 26 + 8).Outline();

  // Reset OpenGL modes
  glDisable(GL_BLEND);
  glDepthMask(TRUE);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}



int R3SurfelLabeler::
PickCommandMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt)
{
  // Only pick from menu if it is visible
  if (!command_menu_visibility) return 0;

  // Only process left-button down clicks
  if (button != 0) return 0;
  if (state != 0) return 0;

  // Get convenient variables
  // Note: this must match DrawCommandMenu
  int width = viewer.Viewport().Width();
  int height = viewer.Viewport().Height();
  int x = width - 164 - 20;
  int y = height - 42;  

  // Check "Suggest an Object" command
  if (1) {
    R2Box box(x-4, y-4, x + 164, y + 26);
    if (R2Contains(box, R2Point(xcursor, ycursor))) {
      // Process predict command
      SelectSuggestedObject();
      return 1;
    }
    y -= 32;
  }


  // Check "Predict Selected" command
  if (0) {
    R2Box box(x-4, y-4, x + 164, y + 26);
    if (R2Contains(box, R2Point(xcursor, ycursor))) {
      // Process predict command
      classifier.PredictLabelAssignments();
      return 1;
    }
    y -= 32;
  }

  // No command picked
  return 0;
}



////////////////////////////////////////////////////////////////////////
// LABEL MENU
////////////////////////////////////////////////////////////////////////

static int
CompareSurfelLabelNames(const void *data1, const void *data2)
{
  R3SurfelLabel *label1 = *((R3SurfelLabel **) data1);
  R3SurfelLabel *label2 = *((R3SurfelLabel **) data2);
  if (!label1->Name()) return 1;
  if (!label2->Name()) return -1;
  return strcmp(label1->Name(), label2->Name());
}



R2Box R3SurfelLabeler::
LabelMenuBBox(void) const
{
  // Update dimension parameters of label menu
  ((R3SurfelLabeler *) this)->UpdateLabelMenu();
  
  // Determine bbox of entire label menu
  int nitems = label_menu_list.NEntries() + 1;
  int height = viewer.Viewport().Height();
  double x1 = label_menu_item_height / 2.0;
  double x2 = x1 + label_menu_item_width;
  double y2 = height - label_menu_item_height / 2.0;
  double y1 = y2 - nitems * label_menu_item_height;
  return R2Box(x1, y1, x2, y2);
}



void R3SurfelLabeler::
UpdateLabelMenu(void)
{
  // Update label menu list
  if (label_menu_list.IsEmpty()) {
    // Add labels to list
    for (int i = 0; i < scene->NLabels(); i++) {
      R3SurfelLabel *label = scene->Label(i);
      if (label->NParts() > 0) continue;
      if (!strcmp(label->Name(), "Root")) continue;
      label_menu_list.Insert(label);
    }

    // Sort list
    label_menu_list.Sort(CompareSurfelLabelNames);
  }

  // Set size variables based on window size
  int nitems = label_menu_list.NEntries();
  nitems += 1; // for All labels
  nitems += attribute_menu_names.size(); // for Attribute Menu
  int height = viewer.Viewport().Height();
  label_menu_font = RN_GRFX_BITMAP_TIMES_ROMAN_24;
  label_menu_item_height = 28;
  label_menu_item_width = 260;
  if (height < label_menu_item_height * nitems) {
    label_menu_font = RN_GRFX_BITMAP_HELVETICA_18;
    label_menu_item_height = 24;
    label_menu_item_width = 240;
  }
  if ((nitems <= 2) || (height < label_menu_item_height * nitems)) {
    label_menu_font = RN_GRFX_BITMAP_HELVETICA_12;
    label_menu_item_height = 18;
    label_menu_item_width = 190;
  }
  if (height < label_menu_item_height * nitems) {
    label_menu_font = RN_GRFX_BITMAP_HELVETICA_10;
    label_menu_item_height = 14;
    label_menu_item_width = 140;
  }
}



void R3SurfelLabeler::
DrawLabelMenu(void) const
{
  // Only draw menu if it is visible
  if (!label_menu_visibility) return;

  // Get convenient variables
  int width = viewer.Viewport().Width();
  int height = viewer.Viewport().Height();

  // Get label menu dimension parameters
  // Note: this must match PickLabelMenu
  R2Box menu_bbox = LabelMenuBBox();
  int small_gap = label_menu_item_height / 8;
  int x = menu_bbox.XMin();
  int y = menu_bbox.YMax() - label_menu_item_height;
  
  // Set OpenGL modes
  glDisable(GL_LIGHTING);
  RNLoadRgb(0,0,0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  // Check if all labels are visible
  int all_visible = 1;
  for (int i = 0; i < scene->NLabels(); i++) {
    if (!LabelVisibility(i)) { all_visible = 0; break; }
  }

  // Clear area behind menu
  RNLoadRgb(0, 0, 0);
  menu_bbox.Draw();

  // Draw "All" visibility box and "Labels" header
  // Note: this must match PickLabelMenu
  R2Box visibility_box(x + 2*small_gap, y + 2*small_gap, x + label_menu_item_height - 2*small_gap, y + label_menu_item_height - 2*small_gap);
  R2Box name_box(x + label_menu_item_height, y + small_gap, x + label_menu_item_width - small_gap, y + label_menu_item_height - small_gap);
  R2Point visibility_origin(visibility_box[0][0] + small_gap, visibility_box[0][1] + small_gap);
  R2Point name_origin(name_box[0][0]+small_gap, name_box[0][1]+small_gap);
  if (all_visible) RNLoadRgba(1.0, 1.0, 1.0, 0.5);
  else RNLoadRgba(0.0, 0.0, 0.0, 0.5);
  visibility_box.Draw();
  RNLoadRgb(1, 1, 1);
  visibility_box.Outline();
  R2DrawText(visibility_origin, "v", label_menu_font); 
  R2DrawText(name_origin, "Labels", label_menu_font); 
  y -= label_menu_item_height;

  // Draw labels
  for (int i = 0; i < label_menu_list.NEntries(); i++) {
    R3SurfelLabel *label = label_menu_list.Kth(i);

    // Get label placement info
    // Note: this must match PickLabelMenu
    R2Box visibility_box(x + 2*small_gap, y + 2*small_gap, x + label_menu_item_height - 2*small_gap, y + label_menu_item_height - 2*small_gap);
    R2Box name_box(x + label_menu_item_height, y + small_gap, x + label_menu_item_width - small_gap, y + label_menu_item_height - small_gap);
    R2Point visibility_origin(visibility_box[0][0], visibility_box[0][1]);
    R2Point name_origin(name_box[0][0]+small_gap, name_box[0][1]+small_gap);
    RNRgb color = label->Color();

    // Get label hierarchy level
    int level = 0;
    // R3SurfelLabel *ancestor = label->Parent();
    // while (ancestor) { 
    //   ancestor = ancestor->Parent(); 
    //   level++; 
    // }

    // Draw visibility box
    if (LabelVisibility(label)) RNLoadRgba(1.0, 1.0, 1.0, 0.5);
    else RNLoadRgba(0.0, 0.0, 0.0, 0.5);
    visibility_box.Draw();
    RNLoadRgb(color[0], color[1], color[2]);
    visibility_box.Outline();
    RNLoadRgb(color[0] + 0.5, color[1] + 0.5, color[2] + 0.5);
    R2DrawText(visibility_origin, "v", label_menu_font); 

    // Draw label box
    RNLoadRgb(0.0, 0.0, 0.0);
    name_box.Draw();
    RNLoadRgb(color[0], color[1], color[2]);
    name_box.Outline();

    // Draw label name 
    char buffer[2048];
    int key = label->AssignmentKeystroke();
    char prefix[1024] = { '\0' };
    for (int j = 0; j < level; j++) strcat(prefix, " ");
    if (isalpha(key)) sprintf(buffer, "%s%s (%c)", prefix, label->Name(), key);
    else sprintf(buffer, "%s%s", prefix, label->Name());
    static const RNRgb base_text_color(0.25, 0.25, 0.25);
    RNLoadRgb(base_text_color + 0.75*color);
    R2DrawText(name_origin, buffer, label_menu_font); 

    // Update location
    // Note: this must match PickLabelMenu
    y -= label_menu_item_height;
    if (y < label_menu_item_height) {
      break; // temporary -- so doesn't wrap
      y = menu_bbox.YMax() - label_menu_item_height;
      x += 2*small_gap + label_menu_item_width;
    }
  }

  // Draw box around menu
  RNLoadRgb(1, 0, 0);
  menu_bbox.Outline();

  // Reset OpenGL modes
  glDisable(GL_BLEND);
  glDepthMask(TRUE);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}



int R3SurfelLabeler::
PickLabelMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt)
{
  // Only pick from menu if it is visible
  if (!label_menu_visibility) return 0;

  // Only process left-button down clicks
  if (button != 0) return 0;
  if (state != 0) return 0;
  R2Point cursor(xcursor, ycursor);

  // Get label menu dimension parameters
  // Note: this must match DrawLabelMenu
  R2Box menu_bbox = LabelMenuBBox();
  int small_gap = label_menu_item_height / 8;
  int x = menu_bbox.XMin();
  int y = menu_bbox.YMax() - label_menu_item_height;
  
  // Pick "All" visibility box 
  R2Box visibility_box(x + 2*small_gap, y + 2*small_gap,
    x + label_menu_item_height - 2*small_gap, y + label_menu_item_height - 2*small_gap);
  if (R2Contains(visibility_box, cursor)) {
    // Toggle all
    SetLabelVisibility(-1, -1);
    return 1;
  }
  y -= label_menu_item_height;

  // Pick label
  for (int i = 0; i < label_menu_list.NEntries(); i++) {
    R3SurfelLabel *label = label_menu_list.Kth(i);

    // Get label info
    // Note: this must match DrawLabelMenu
    R2Box visibility_box(x + 2*small_gap, y + 2*small_gap,
      x + label_menu_item_height - 2*small_gap, y + label_menu_item_height - 2*small_gap);
    R2Box name_box(x + label_menu_item_height, y + small_gap,
      x + label_menu_item_width - small_gap, y + label_menu_item_height - small_gap);

    // Process command
    if (R2Contains(visibility_box, cursor)) {
      if (ctrl) {
        // Toggle all except selected label
        SetLabelVisibility(-1, -1);
        SetLabelVisibility(label->SceneIndex(), -1);
      }
      else {
        // Toggle selected label
        SetLabelVisibility(label->SceneIndex(), -1);
      }
      return 1;
    }
    else if (R2Contains(name_box, cursor)) {
      AssignLabelToSelectedObjects(label);
      return 1;
    }

    // Update location
    // Note: this must match DrawLabelMenu
    y -= label_menu_item_height;
    if (y < label_menu_item_height) {
      break; // temporary -- so doesn't wrap
      y = menu_bbox.YMax() - label_menu_item_height;
      x += 2*small_gap + label_menu_item_width;
    }
  }

  // No label picked
  return 0;
}



////////////////////////////////////////////////////////////////////////
// ATTRIBUTE MENU
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
UpdateAttributeMenu(void)
{
  // Update label menu first
  UpdateLabelMenu();
  
  // Set parameters to match label menu
  attribute_menu_item_width = label_menu_item_width;
  attribute_menu_item_height = label_menu_item_height;
  attribute_menu_font = label_menu_font;
}



R2Box R3SurfelLabeler::
AttributeMenuBBox(void) const
{
  // Update dimension parameters of attribute menu
  ((R3SurfelLabeler *) this)->UpdateAttributeMenu();
  
  // Get bbox of label menu
  R2Box label_menu_bbox = LabelMenuBBox();

  // Determine bbox of entire attribute menu
  unsigned int nitems = attribute_menu_names.size();
  double x1 = label_menu_bbox.XMin();
  double x2 = label_menu_bbox.XMax();
  double y1 = attribute_menu_item_height / 2.0;
  double y2 = y1 + nitems * attribute_menu_item_height;
  return R2Box(x1, y1, x2, y2);
}



void R3SurfelLabeler::
DrawAttributeMenu(void) const
{
  // Only draw menu if it is visible
  if (!attribute_menu_visibility) return;

  // Get convenient variables
  int width = viewer.Viewport().Width();
  int height = viewer.Viewport().Height();

  // Get attribute menu dimension parameters
  // Note: this must match PickAttributeMenu
  R2Box menu_bbox = AttributeMenuBBox();
  int small_gap = attribute_menu_item_height / 8;
  int x = menu_bbox.XMin();
  int y = menu_bbox.YMax() - attribute_menu_item_height;
  
  // Set OpenGL modes
  glDisable(GL_LIGHTING);
  RNLoadRgb(0,0,0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  // Clear area behind menu
  RNLoadRgb(0, 0, 0);
  menu_bbox.Draw();

  // Set some colors
  static const RNRgb off_color(1.0, 0.7, 0.7);
  static const RNRgb on_color(0.7, 1.0, 0.7);
  
  // Draw attributes
  for (unsigned int i = 0; i < attribute_menu_names.size(); i++) {
    const char *attribute_name = attribute_menu_names[i];
    unsigned char attribute_keystroke = attribute_menu_keystrokes[i];
    RNFlags attribute_flags = attribute_menu_flags[i];

    // Get attribute placement info
    // Note: this must match PickAttributeMenu
    R2Box visibility_box(x + 2*small_gap, y + 2*small_gap,
      x + attribute_menu_item_height - 2*small_gap, y + attribute_menu_item_height - 2*small_gap);
    R2Box name_box(x + attribute_menu_item_height, y + small_gap,
      x + attribute_menu_item_width - small_gap, y + attribute_menu_item_height - small_gap);
    R2Point visibility_origin(visibility_box[0][0], visibility_box[0][1]);
    R2Point name_origin(name_box[0][0]+small_gap, name_box[0][1]+small_gap);

    // Draw visibility box
    RNBoolean vis = FALSE;
    if (attribute_visibility_flags != 0)
      vis = attribute_visibility_flags.Intersects(attribute_flags);
    if (vis) RNLoadRgba(1.0, 1.0, 1.0, 0.5);
    else RNLoadRgba(0.0, 0.0, 0.0, 0.5);
    visibility_box.Draw();
    RNLoadRgb(0.5, 0.5, 0.5);
    visibility_box.Outline();
    RNLoadRgb(1, 1, 1);
    R2DrawText(visibility_origin, "v", attribute_menu_font); 

    // Determine if attribute is "on"
    RNBoolean on = DoAllSelectedObjectsHaveAttribute(this, attribute_flags);
    RNRgb color = (on) ? on_color : off_color;
    
    // Draw name box
    RNLoadRgb(0.0, 0.0, 0.0);
    name_box.Draw();
    if (isalpha(attribute_keystroke)) {
      RNLoadRgb(color);
      name_box.Outline();
    }

    // Draw attribute name 
    char buffer[2048];
    if (isalpha(attribute_keystroke)) {
      RNLoadRgb(color);
      sprintf(buffer, "%s (ctrl-%c)", attribute_name, attribute_keystroke);
      R2DrawText(name_origin, buffer, attribute_menu_font); 
    }
    else {
      RNLoadRgb(1, 1, 1);
      sprintf(buffer, "%s", attribute_name);
      R2DrawText(name_origin, buffer, attribute_menu_font); 
    }

    // Update location
    // Note: this must match PickAttributeMenu
    y -= attribute_menu_item_height;
  }

  // Draw box around menu
  RNLoadRgb(1, 0, 0);
  menu_bbox.Outline();

  // Reset OpenGL modes
  glDisable(GL_BLEND);
  glDepthMask(TRUE);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}



int R3SurfelLabeler::
PickAttributeMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt)
{
  // Only pick from menu if it is visible
  if (!attribute_menu_visibility) return 0;

  // Only process left-button down clicks
  if (button != 0) return 0;
  if (state != 0) return 0;
  R2Point cursor(xcursor, ycursor);

  // Get attribute menu dimension parameters
  // Note: this must match DrawAttributeMenu
  R2Box menu_bbox = AttributeMenuBBox();
  int small_gap = attribute_menu_item_height / 8;
  int x = menu_bbox.XMin();
  int y = menu_bbox.YMax() - attribute_menu_item_height;
  
  // Pick attribute
  for (unsigned int i = 0; i < attribute_menu_names.size(); i++) {
    const char *attribute_name = attribute_menu_names[i];
    unsigned char attribute_keystroke = attribute_menu_keystrokes[i];
    RNFlags attribute_flags = attribute_menu_flags[i];

    // Get attribute info
    // Note: this must match DrawAttributeMenu
    R2Box visibility_box(x + 2*small_gap, y + 2*small_gap,
      x + attribute_menu_item_height - 2*small_gap, y + attribute_menu_item_height - 2*small_gap);
    R2Box name_box(x + attribute_menu_item_height, y + small_gap,
      x + attribute_menu_item_width - small_gap, y + attribute_menu_item_height - small_gap);

    // Process command
    if (R2Contains(visibility_box, cursor)) {
      if (ctrl) SetAttributeVisibility(0, -1);
      SetAttributeVisibility(attribute_flags, -1);
      return 1;
    }
    else if (R2Contains(name_box, cursor)) {
      if (isalpha(attribute_keystroke)) {
        AssignAttributeToSelectedObjects(attribute_flags, attribute_name, -1);
        return 1;
      }
    }

    // Update location
    // Note: this must match DrawLabelMenu
    y -= label_menu_item_height;
  }

  // No label picked
  return 0;
}



////////////////////////////////////////////////////////////////////////
// Pointset functions
////////////////////////////////////////////////////////////////////////

R3SurfelPointSet *R3SurfelLabeler::
ObjectSelectionPointSet(void) const
{
  // Check selections
  if (NObjectSelections() == 0) return NULL;
  
  // Allocate pointset
  R3SurfelPointSet *pointset = new R3SurfelPointSet();
  if (!pointset) {
    RNFail("Unable to allocate pointset\n");
    return NULL;
  }

  // Load leaf nodes descendents of all selected objects
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *seed_object = ObjectSelection(i);
    RNArray<R3SurfelObject *> object_stack;
    object_stack.Insert(seed_object);
    while (!object_stack.IsEmpty()) {
      R3SurfelObject *object = object_stack.Tail();
      object_stack.RemoveTail();
      if (object->NParts() > 0) {
        // Visit children objects
        for (int k = 0; k < object->NParts(); k++) {
          R3SurfelObject *part = object->Part(k);
          object_stack.InsertTail(part);
        }
      }
      else {
        // Load nodes of leaf object
        for (int j = 0; j < object->NNodes(); j++) {
          R3SurfelNode *object_node = object->Node(j);
          RNArray<R3SurfelNode *> node_stack;
          node_stack.Insert(object_node);
          while (!node_stack.IsEmpty()) {
            R3SurfelNode *node = node_stack.Tail();
            node_stack.RemoveTail();
            if (node->NParts() > 0) {
              // Visit children nodes
              for (int k = 0; k < node->NParts(); k++) {
                R3SurfelNode *part = node->Part(k);
                node_stack.InsertTail(part);
              }
            }
            else {
              // Insert points from leaf node
              for (int k = 0; k < node->NBlocks(); k++) {
                R3SurfelBlock *block = node->Block(k);
                pointset->InsertPoints(block);
              }
            }
          }
        }
      }
    }
  }

  // Check pointset
  if (pointset->NPoints() == 0) {
    delete pointset;
    return NULL;
  }

  // Return pointset
  return pointset;
}



////////////////////////////////////////////////////////////////////////
// Working set management
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
UpdateWorkingSet(const R3Point& center, RNScalar resolution, RNScalar radius)
{
  // Update working set
  R3SurfelViewer::UpdateWorkingSet(center, resolution, radius);

  // Objectify working set
  ObjectifyWorkingSet();
}



void R3SurfelLabeler::
UpdateWorkingSet(const R3Viewer& view)
{
  // Update working set
  R3SurfelViewer::UpdateWorkingSet(view);

  // Objectify working set
  ObjectifyWorkingSet();
}



void R3SurfelLabeler::
UpdateWorkingSet(void)
{
  // Update working set
  R3SurfelViewer::UpdateWorkingSet();

  // Objectify working set
  ObjectifyWorkingSet();
}



void R3SurfelLabeler::
ObjectifyWorkingSet(void)
{
#if 0
  // Just checking
  if (!scene) return;

  // Make sure that every resident node has an object associated with it
  R3SurfelNodeSet new_resident_nodes;
  R3SurfelNodeSet old_resident_nodes;
  for (int i = 0; i < resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = resident_nodes.Node(i);
    if (node->NParts() == 0) continue;
    if (node->Object(TRUE)) continue;
    old_resident_nodes.InsertNode(node);
    RNArray<R3SurfelNode *> stack;
    stack.Insert(node);
    while (!stack.IsEmpty()) {
      R3SurfelNode *node = stack.Tail();
      stack.RemoveTail();
      for (int j = 0; j < node->NParts(); j++) {
        R3SurfelNode *part = node->Part(j);
        if ((part->NParts() == 0) || (part->Object(TRUE))) {
          new_resident_nodes.InsertNode(part);
        }
        else {
          stack.Insert(part);
        }
      }
    }
  }

  // Add new resident nodes
  for (int i = 0; i < new_resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = new_resident_nodes.Node(i);
    InsertIntoWorkingSet(node);
  }
  
  // Remove old resident nodes
  for (int i = 0; i < old_resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = old_resident_nodes.Node(i);
    RemoveFromWorkingSet(node);
  }
#endif
}



////////////////////////////////////////////////////////////////////////
// Command functions
////////////////////////////////////////////////////////////////////////

void R3SurfelLabeler::
BeginCommand(int type, double operand)
{
  // Check current command
  assert(!current_command);

  // Start current command
  current_command = new R3SurfelLabelerCommand(this, type, operand);
}



void R3SurfelLabeler::
EndCommand(void)
{
  // Check current command
  assert(current_command);

  // Insert command into undo stack, if it can be undone
  switch (current_command->type) {
  case R3_SURFEL_LABELER_ZOOM_COMMAND:
  case R3_SURFEL_LABELER_SELECT_NONE_COMMAND:
  case R3_SURFEL_LABELER_SELECT_PICKED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_OVERLAPPING_COMMAND:
  case R3_SURFEL_LABELER_SELECT_UNLABELED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_SUGGESTED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_ALL_COMMAND:
  case R3_SURFEL_LABELER_LABEL_PICKED_COMMAND:
  case R3_SURFEL_LABELER_LABEL_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_RELABEL_PICKED_COMMAND:
  case R3_SURFEL_LABELER_RELABEL_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_CONFIRM_PICKED_COMMAND:  
  case R3_SURFEL_LABELER_CONFIRM_SELECTION_COMMAND:  
  case R3_SURFEL_LABELER_CONFIRM_ALL_COMMAND:  
  case R3_SURFEL_LABELER_REFUTE_PICKED_COMMAND:  
  case R3_SURFEL_LABELER_REFUTE_SELECTION_COMMAND:  
  case R3_SURFEL_LABELER_REFUTE_ALL_COMMAND: 
  case R3_SURFEL_LABELER_MERGE_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_UNMERGE_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_SPLIT_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_ASSIGN_ATTRIBUTE_COMMAND:
  case R3_SURFEL_LABELER_ASSIGN_OBB_COMMAND:
    // Delete commands that were undone
    for (int i = undo_index+1; i < undo_stack.NEntries(); i++) 
      delete undo_stack.Kth(i);
    undo_stack.Truncate(undo_index+1);

    // Insert command into undo stack
    undo_stack.Insert(current_command);
    undo_index = undo_stack.NEntries() - 1;
    break;
  }

  // Write command to log
  if (logging_fp) {
    PrintCommand(current_command, logging_fp);
  }

  // Write checkpoint to log
  if (logging_fp) {
    const RNScalar checkpoint_interval = 3600;
    static RNScalar last_checkpoint_time = -FLT_MAX;
    if ((current_command->type == R3_SURFEL_LABELER_INITIALIZE_COMMAND) || 
        (current_command->type == R3_SURFEL_LABELER_TERMINATE_COMMAND) || 
        (current_command->type == R3_SURFEL_LABELER_SYNC_COMMAND) || 
        (current_command->timestamp >= last_checkpoint_time + checkpoint_interval)) {
      last_checkpoint_time = current_command->timestamp;
      // PrintCheckpoint(logging_fp);
    }
  }

  // End current command
  current_command = NULL;
}



void R3SurfelLabeler::
PrintCommand(R3SurfelLabelerCommand *command, FILE *fp) const
{
  // Just checking
  if (!command) return;
  if (!fp) fp = stdout;

  // Print 
  const R3Point& eye = command->camera.Origin();
  const R3Vector& towards = command->camera.Towards();
  const R3Vector& up = command->camera.Up();

  // Count label assignments inserted by human
  int ninserted_label_assignments = 0;
  for (int j = 0; j < command->inserted_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->inserted_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_HUMAN_ORIGINATOR) continue;
    ninserted_label_assignments++;
  }

  // Count label assignments removed by human
  int nremoved_label_assignments = 0;
  for (int j = 0; j < command->removed_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->removed_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_HUMAN_ORIGINATOR) continue;
    nremoved_label_assignments++;
  }

  fprintf(fp, "CMD %d %g %g    ", command->type, command->operand, command->timestamp);
  fprintf(fp, "%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f ", eye[0], eye[1], eye[2], towards[0], towards[1], towards[2], up[0], up[1], up[2]);
  fprintf(fp, "    ");

  fprintf(fp, "%d", command->inserted_object_selections.NEntries());
  for (int j = 0; j < command->inserted_object_selections.NEntries(); j++) 
    fprintf(fp, " %d", command->inserted_object_selections[j]->SceneIndex());
  fprintf(fp, "    ");

  fprintf(fp, "%d", command->removed_object_selections.NEntries());
  for (int j = 0; j < command->removed_object_selections.NEntries(); j++) 
    fprintf(fp, " %d", command->removed_object_selections[j]->SceneIndex());
  fprintf(fp, "    ");

  fprintf(fp, "%d", ninserted_label_assignments);
  for (int j = 0; j < command->inserted_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->inserted_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_HUMAN_ORIGINATOR) continue;
    fprintf(fp, " ( %d %d %g %d )", assignment->Object()->SceneIndex(), assignment->Label()->Identifier(), assignment->Confidence(), assignment->Originator());
  }
  fprintf(fp, "    ");

  fprintf(fp, "%d", nremoved_label_assignments);
  for (int j = 0; j < command->removed_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->removed_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_HUMAN_ORIGINATOR) continue;
    fprintf(fp, " ( %d %d %g %d )", assignment->Object()->SceneIndex(), assignment->Label()->Identifier(), assignment->Confidence(), assignment->Originator());
  }
  fprintf(fp, "    ");

  fprintf(fp, "%d", command->part_parent_assignments.NEntries());
  for (int j = 0; j < command->part_parent_assignments.NEntries(); j++) {
    R3SurfelObject *part = command->part_parent_assignments[j];
    R3SurfelObject *removed_parent = command->removed_parent_assignments[j];
    R3SurfelObject *inserted_parent = command->inserted_parent_assignments[j];
    int part_index = (part) ? part->SceneIndex() : -1;
    int removed_parent_index = (removed_parent) ? removed_parent->SceneIndex() : -1;
    int inserted_parent_index = (inserted_parent) ? inserted_parent->SceneIndex() : -1;
    fprintf(fp, " ( %d %d %d )", part_index, removed_parent_index, inserted_parent_index);
  }
  fprintf(fp, "    ");

  fprintf(fp, "%lu", command->attribute_assignments.size());
  for (unsigned int j = 0; j < command->attribute_assignments.size(); j++) {
    R3SurfelAttributeAssignment& assignment = command->attribute_assignments[j];
    fprintf(fp, " ( %d %u %d %d )", assignment.object->SceneIndex(), (unsigned int) assignment.attribute,
      assignment.previous_value, assignment.new_value);
  }
  fprintf(fp, "    ");

  fprintf(fp, "\n");
  fflush(fp);
}



void R3SurfelLabeler::
PrintCheckpoint(FILE *fp) const
{
  // Just checking
  if (!fp) fp = stdout;

  // Count assignments
  int ntotal = 0;
  int npredicted = 0;
  int nconfirmed = 0;
  int nunlabeled = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (object->HumanLabel()) nconfirmed++;
    else if (object->PredictedLabel()) npredicted++;
    else nunlabeled++;
    ntotal++;
  }

  // Print status
  fprintf(fp, "CHECKPOINT  %g  %d   %d %d %d %d", 
          CurrentTime(), scene->NLabels(),
          ntotal, nconfirmed, npredicted, nunlabeled);

#if 0
  // Print labels
  fprintf(fp, "    ");
  for (int i = 0; i < scene->NLabels(); i++) {
    R3SurfelLabel *label = scene->Label(i);
    fprintf(fp, " ( L %d %s )", label->Identifier(), label->Name());
  }

  // Print object assignments
  fprintf(fp, "    ");
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    R3SurfelLabelAssignment *current_assignment = object->CurrentLabelAssignment();
    R3SurfelLabel *current_label = current_assignment->Label();
    fprintf(fp, " ( O %d %s %d %d )", object->SceneIndex(), object->Name(), 
      (current_label) ? current_label->Identifier() : -1, 
      (current_assignment) ? current_assignment->Originator() : -1);
  }
#endif

  // End checkpoint
  fprintf(fp, "\n");
  fflush(fp);
}



////////////////////////////////////////////////////////////////////////
// Debugging functions
////////////////////////////////////////////////////////////////////////

int R3SurfelLabeler::
IsValid(void) const
{
  // Check object selections
  for (int i = 0; i < selection_objects.NEntries(); i++) {
    R3SurfelObject *object = selection_objects.Kth(i);
    if (!object)  { printf("A\n"); return 0; }
    if (object->Parent() != scene->RootObject()) { printf("B\n"); return 0; }
    for (int j = 0; j < selection_objects.NEntries(); j++) {
      if ((i != j) && (object == selection_objects.Kth(j))) { printf("C\n"); return 0; }
    }
  }

  // Check resident nodes
  for (int i = 0; i < resident_nodes.NNodes(); i++) {
    R3SurfelNode *node = resident_nodes.Node(i);
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      if (block->ReadCount() <= 0) return 0;
    }
  }
  

  // Passed all tests
  return 1;
}



}; // end namespace
