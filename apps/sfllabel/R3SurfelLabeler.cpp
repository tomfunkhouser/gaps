/* Source file for the surfel scene labeler class */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Graphics/R3Graphics.h"
#include "R3Surfels/R3Surfels.h"
#include "R3SurfelClassifier.h"
#include "R3SurfelSegmenter.h"
#include "R3SurfelLabeler.h"
#include "R3SurfelLabelerCommand.h"
#include "DrawText.h"
#include "split.h"



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
    label_menu_visibility(1),
    label_menu_item_width(190),
    label_menu_item_height(14),
    label_menu_font(GLUT_BITMAP_HELVETICA_12),
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

  // Initialize labels
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->NParts() > 0) continue;
    if (object->HumanLabel()) continue;
    if (object->PredictedLabel()) continue;
    R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, unknown_label, 0, R3_SURFEL_LABEL_ASSIGNMENT_MACHINE_ORIGINATOR);
    scene->InsertLabelAssignment(assignment);
  }

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
  glColor3d(1.0, 1.0, 0.0);
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
      if (assignment->Originator() == R3_SURFEL_LABEL_ASSIGNMENT_GROUND_TRUTH_ORIGINATOR) continue;
      RNBoolean confirmed = (assignment->Originator() == R3_SURFEL_LABEL_ASSIGNMENT_HUMAN_ORIGINATOR) ? 1 : 0;
      R3SurfelLabel *label = assignment->Label();
      if (!strcmp(label->Name(), "Unknown")) continue;
      if (!LabelVisibility(label)) continue;
      R3Point position = object->Centroid();
      position[2] = object->BBox().ZMax() + 0.01;
      R2Point p = viewer.ViewportPoint(position);
      void *font = (confirmed) ? GLUT_BITMAP_HELVETICA_18 : GLUT_BITMAP_HELVETICA_12;
      int width = glutBitmapLength(font, (const unsigned char *) label->Name());
      p[0] -= width / 2;
      RNLoadRgb(label->Color());
      DrawText(p, label->Name(), font);
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
  DrawStatus();
  DrawMessage();
  DrawCommandMenu();
  DrawLabelMenu();

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

  // Remember mouse position 
  mouse_position[0] = x;
  mouse_position[1] = y;

  // Update mouse drag movement
  mouse_drag_distance_squared += dx*dx + dy*dy;

  // Return whether need redraw
  return redraw;
}



int R3SurfelLabeler::
MouseButton(int x, int y, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt)
{
  // Send event to viewer
  int redraw = R3SurfelViewer::MouseButton(x, y, button, state, shift, ctrl, alt);

  // Process mouse button event
  if (state == 1) { // Down
    if (!click_polygon_active) {
      if ((button == 0) && (shift || ctrl)) {
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
    if (select_box_active) {
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
      if (!drag && !double_click && !click_polygon_active) {
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
          else if (PickImage(x, y)) redraw = 1;
          else if (SelectPickedObject(x, y, shift, ctrl, alt)) redraw = 1;
        }
      }
    }
  }

  // Return whether need redraw
  return redraw;
}



int R3SurfelLabeler::
Keyboard(int x, int y, int key, RNBoolean shift, RNBoolean ctrl, RNBoolean alt)
{
  // Do not redraw by default
  int redraw = 0;
  
  // Process mouse button event
  if (alt) {
    // Make sure does not conflict with keys used by R3SurfelViewer
    switch(key) {
    case 'B':
    case 'b':
      // Copied from R3SurfelViewer
      SetImagePlaneVisibility(-1);
      SelectImage(selected_image, FALSE, FALSE);
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
      
    case '!': 
      SetStatusVisibility(-1);
      redraw = 1;
      break; 

    case '@': 
      SetLabelMenuVisibility(-1);
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
      
    case 'E':
    case 'e': {
      SelectOverlappedObjects();
      redraw = 1;
      break; }
      
    case 'F':
    case 'f':
      // segmenter.PredictObjectSegmentations(selection_objects);
      // redraw = 1;
      break;

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

    case 'N':
    case 'n':
      // Assign new label (copy label from closest other object)
      // if (NObjectSelections() > 0) AssignNewLabelToSelectedObjects();
      // else if (!AssignNewLabelToPickedObject(x, y)) SelectPickedObject(x, y);
      break;
      
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
    }
  }
  else {
    // Send event to viewer
    redraw |= R3SurfelViewer::Keyboard(x, y, key, shift, ctrl, alt);

    // Process other keyboard events
    switch (key) {
    case R3_SURFEL_VIEWER_F11_KEY: 
      // R3SurfelGraphCut(scene);
      redraw = 1;
      break; 

    case 27: { // ESC
      SelectPickedObject(-1, -1, 0, 0);
      SetLabelVisibility(-1, 1);
      SetElevationRange(RNnull_interval);
      SetViewingExtent(R3null_box);
      click_polygon_active = FALSE;
      select_polygon_active = FALSE;
      split_polygon_active = FALSE;
      split_line_active = FALSE;
      select_box_active = FALSE;
      redraw = 1;
      break; }

    case ' ':
      // Confirm predicted label
      if (NObjectSelections() > 0) ConfirmLabelsOnSelectedObjects();
      else if (!ConfirmLabelOnPickedObject(x, y)) SelectPickedObject(x, y);
      redraw = 1;
      break;

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



int R3SurfelLabeler::
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

  // Return success
  return 1;
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
    const char *object_name = object->Name();
    if (!object_name) return 0;
    R3SurfelLabel *label = object->CurrentLabel();
    const char *label_name = (label) ? label->Name() : "NoLabel";
    if (ctrl) SetMessage("Unselected object %s / %s", object_name, label_name);
    else SetMessage("Selected object %s / %s", object_name, label_name);
  }

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

  // Set message
  if (ctrl) SetMessage("Unselected %d objects", picked_objects.NEntries());
  else SetMessage("Selected %d objects", picked_objects.NEntries());

  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND);
  
  // Remove previous selections
  if (!ctrl && !shift) EmptyObjectSelections();
 
  // Update object selections
  for (int i = 0; i < picked_objects.NEntries(); i++) {
    R3SurfelObject *object = picked_objects.Kth(i);

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

  // Set message
  if (ctrl) SetMessage("Unselected %d objects", picked_objects.NEntries());
  else SetMessage("Selected %d objects", picked_objects.NEntries());

  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND);
  
  // Remove previous selections
  if (!ctrl && !shift) EmptyObjectSelections();
 
  // Update object selections
  for (int i = 0; i < picked_objects.NEntries(); i++) {
    R3SurfelObject *object = picked_objects.Kth(i);

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

  // Set message
  if (ctrl) SetMessage("Unselected %d objects", picked_objects.NEntries());
  else SetMessage("Selected %d objects", picked_objects.NEntries());

  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND);
  
  // Remove previous selections
  if (!ctrl && !shift) EmptyObjectSelections();
 
  // Update object selections
  for (int i = 0; i < picked_objects.NEntries(); i++) {
    R3SurfelObject *object = picked_objects.Kth(i);

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

  // Return success
  return 1;
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

  // Set message
  SetMessage("Selected %d more objects", picked_objects.NEntries());

  // Check picked objects
  if (picked_objects.IsEmpty()) return 0;
  
  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_SELECT_OVERLAPPING_COMMAND);
  
  // Update object selections
  for (int i = 0; i < picked_objects.NEntries(); i++) {
    R3SurfelObject *object = picked_objects.Kth(i);
    InsertObjectSelection(object);
  }

  // End logging command
  EndCommand();

  // Return success
  return 1;
}



int R3SurfelLabeler::
SelectAllObjects(RNBoolean unlabeled_only)
{
  // Check scene
  if (!scene) return 0;
  if (!SurfelVisibility()) return 0;

  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_SELECT_UNLABELED_COMMAND);
  
  // Remove previous selections
  EmptyObjectSelections();

  // Update object selections
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (!object->Name()) continue;
    if (unlabeled_only && object->HumanLabel()) continue;
    if (!ObjectVisibility(object)) continue;
    InsertObjectSelection(object);
  }

  // End logging command
  EndCommand();

  // Set message
  SetMessage("Selected %d unlabeled objects", NObjectSelections());

  // Return success
  return 1;
}



int R3SurfelLabeler::
SelectSuggestedObject(void) 
{
  // Check scene
  if (!scene) return 0;

  // Set message
  SetMessage(NULL);

  // Pick object
  R3SurfelObject *object = NULL;
  RNScalar least_time = FLT_MAX;
  RNScalar least_confidence = FLT_MAX;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *o = scene->Object(i);
    if (o->NParts() > 0) continue;
    if (!o->Name()) continue;
    if (o->HumanLabel()) continue;
    if (selection_objects.FindEntry(object)) continue;
    R3SurfelLabelAssignment *assignment = o->CurrentLabelAssignment();
    RNScalar confidence = (assignment) ? assignment->Confidence() : 0;
    RNScalar select_time = -1;
    if (object_selection_times.size() > (unsigned int) o->SceneIndex()) {
      select_time = object_selection_times[o->SceneIndex()] + 0.25 * RNRandomScalar();
    }
    if (confidence < least_confidence) {
      least_confidence = confidence;
      least_time = select_time;
      object = o;
    }
    else if ((confidence == least_confidence) && (select_time > 0)) {
      if (select_time < least_time) {
        least_confidence = confidence;
        least_time = select_time;
        object = o;
      }
    }
  }

  // Set message
  if (object) {
    const char *object_name = object->Name();
    if (!object_name) return 0;
    const char *object_namep = strchr(object_name, '_');
    if (object_namep) object_namep++;
    else object_namep = object_name;
    SetMessage("Selected suggested object %s", object_namep);
  }

  // Begin logging command 
  BeginCommand(R3_SURFEL_LABELER_SELECT_SUGGESTED_COMMAND);
  
  // Empty object selections
  EmptyObjectSelections();

  // Insert object into selected objects
  if (object) {
    // Select object
    InsertObjectSelection(object);

    // Set center point
    SetCenterPoint(object->Centroid());

    // Set the camera viewpoint
    R3Box bbox = object->BBox();
    RNLength r = bbox.DiagonalRadius();
    R3Vector towards(0, 0.25 * RN_PI, -0.25 * RN_PI); towards.Normalize();
    R3Vector right = towards % R3posz_vector; right.Normalize();
    R3Vector up = right % towards; up.Normalize();
    R3Point eye = bbox.Centroid() - towards * (16 * r);
    R3Camera camera(eye, towards, up, 0.4, 0.4, 0.01 * r, 10000.0 * r);
    viewer.SetCamera(camera);
  }

  // End logging command
  EndCommand();

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
    R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, label, 1, R3_SURFEL_LABEL_ASSIGNMENT_HUMAN_ORIGINATOR);
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
    R3_SURFEL_LABEL_ASSIGNMENT_HUMAN_ORIGINATOR,
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
    R3_SURFEL_LABEL_ASSIGNMENT_MACHINE_ORIGINATOR,
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
        0.01, R3_SURFEL_LABEL_ASSIGNMENT_MACHINE_ORIGINATOR);
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

  // Get/check label
  R3SurfelLabelAssignment *assignment = ObjectSelection(0)->CurrentLabelAssignment();
  R3SurfelLabel *label = (assignment) ? assignment->Label() : scene->FindLabelByName("Unknown");
  int originator = (assignment) ? assignment->Originator() : R3_SURFEL_LABEL_ASSIGNMENT_MACHINE_ORIGINATOR;
  RNScalar confidence = (assignment) ? assignment->Confidence() : 0;
  
  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_MERGE_SELECTION_COMMAND);
  
  // Create new parent object
  char parent_name[4096];
  static int counter = 1;
  sprintf(parent_name, "MERGE_%d", counter++);
  R3SurfelObject *parent = CreateObject(grandparent, parent_name);
  if (!parent) { EndCommand(); return 0; }

  // Create assignment to label
  if (label) {
    R3SurfelLabelAssignment *a = new R3SurfelLabelAssignment(parent, label, confidence, originator);
    scene->InsertLabelAssignment(a);
  }

  // Compute feature vector
  RNScalar weight = 0;
  R3SurfelFeatureVector vector(scene->NFeatures());
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    vector += object->Complexity() * object->FeatureVector();
    weight += object->Complexity();
  }

  // Assign feature vector
  if (weight > 0) vector /= weight;
  parent->SetFeatureVector(vector);

  // Set parent for all selected objects
  int count = 0;
  for (int i = 0; i < NObjectSelections(); i++) {
    R3SurfelObject *object = ObjectSelection(i);
    SetObjectParent(object, parent);
    count++;
  }

  // Empty object selections
  EmptyObjectSelections();

  // Insert object selection
  InsertObjectSelection(parent);

  // Set message
  SetMessage("Merged %d objects", count);

  // End logging command
  EndCommand();

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
    int originator = (assignment) ? assignment->Originator() : R3_SURFEL_LABEL_ASSIGNMENT_MACHINE_ORIGINATOR;
    RNScalar confidence = (assignment) ? assignment->Confidence() : 0;
  
     // Find previous assignments
    RNArray<R3SurfelLabelAssignment *> previous_assignments;
    for (int i = 0; i < object->NLabelAssignments(); i++) {
      R3SurfelLabelAssignment *a = object->LabelAssignment(i);
      if (a->Originator() == R3_SURFEL_LABEL_ASSIGNMENT_GROUND_TRUTH_ORIGINATOR) continue;
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

      // Increment counter
      count++;
    }
  }

  // Set message
  SetMessage("Unmerged %d objects", count);

  // End logging command
  EndCommand();

  // Invalidate VBO colors
  InvalidateVBO();

  // Just checking
  assert(IsValid());

  // Return success
  return 1;
}



#if 0

int R3SurfelLabeler::
SplitObject(R3SurfelObject *object, R3SurfelObject *parent, const R3SurfelConstraint& constraint,
  RNArray<R3SurfelObject *> *resultA, RNArray<R3SurfelObject *> *resultB)
{
  // Just checking
  assert(object);
  assert(strcmp(object->Name(), "Root"));
  assert(parent == scene->RootObject());

  // Get label assignment info
  R3SurfelObject *ancestor = object;
  while (ancestor && ancestor->Parent() && (ancestor->Parent() != scene->RootObject())) ancestor = ancestor->Parent();
  R3SurfelLabelAssignment *assignment = ancestor->CurrentLabelAssignment();
  R3SurfelLabel *label = (assignment) ? assignment->Label() : scene->FindLabelByName("Unknown");
  int originator = (assignment) ? assignment->Originator() : R3_SURFEL_LABEL_ASSIGNMENT_MACHINE_ORIGINATOR;
  double confidence = (assignment) ? assignment->Confidence() : 0;

  // Check if leaf object
  if (object->NParts() == 0) {
    // Check object
    if (object->NNodes() == 0) return 0;

    // Create constraint
    R3SurfelMultiConstraint multi_constraint;
    R3SurfelObjectConstraint object_constraint(object);
    multi_constraint.InsertConstraint(&constraint);
    multi_constraint.InsertConstraint(&object_constraint);

    // Create array of nodes
    RNArray<R3SurfelNode *> nodes;
    for (int i = 0; i < object->NNodes(); i++) {
      R3SurfelNode *node = object->Node(i);
      nodes.Insert(node);
    }

    // Split all nodes
    RNArray<R3SurfelNode *> nodesA, nodesB;
    for (int i = 0; i < nodes.NEntries(); i++) {
      R3SurfelNode *node = nodes.Kth(i);
      SplitLeafNodes(node, constraint, &nodesA, &nodesB);
    }

    // Create objects
    R3SurfelObject *objectA = NULL;
    R3SurfelObject *objectB = NULL;
    if (nodesB.NEntries() == 0) {
      objectA = object;
    }
    else if (nodesA.NEntries() == 0) {
      objectB = object;
    }
    else {
      // Create new objects
      objectA = new R3SurfelObject();
      objectB = new R3SurfelObject();
      if (!objectA || !objectB) return 0;
      
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

      // Insert objects into scene
      scene->InsertObject(objectA, object);
      scene->InsertObject(objectB, object);
    }
    
    // Set parents
    if (objectA) SetObjectParent(objectA, parent);
    if (objectB) SetObjectParent(objectB, parent);
    
    // Create assignments to label
    if (objectA) {
      R3SurfelLabelAssignment *assignmentA = new R3SurfelLabelAssignment(objectA, label, confidence, originator);
      InsertLabelAssignment(assignmentA);
    }
    if (objectB) {
      R3SurfelLabelAssignment *assignmentB = new R3SurfelLabelAssignment(objectB, label, confidence, originator);
      InsertLabelAssignment(assignmentB);
    }

    // Insert object into result
    if (resultA && objectA) resultA->Insert(objectA);
    if (resultB && objectB) resultB->Insert(objectB);
  }
  else {
    // Split leaves of parts
    RNArray<R3SurfelObject *> parts;
    for (int i = 0; i < object->NParts(); i++)
      parts.Insert(object->Part(i));
    for (int i = 0; i < parts.NEntries(); i++) {
      R3SurfelObject *part = parts.Kth(i);
      SplitObject(part, parent, constraint, resultA, resultB);
    }
  }
  
  // Invalidate VBO colors
  InvalidateVBO();

  // Return success
  return 1;
}

#else

int R3SurfelLabeler::
SplitObject(R3SurfelObject *object, R3SurfelObject *parent, const R3SurfelConstraint& constraint,
  RNArray<R3SurfelObject *> *resultA, RNArray<R3SurfelObject *> *resultB)
{
  // Just checking
  assert(object);
  assert(strcmp(object->Name(), "Root"));
  assert(parent == scene->RootObject());

  // Get label assignment info
  R3SurfelObject *ancestor = object;
  while (ancestor && ancestor->Parent() && (ancestor->Parent() != scene->RootObject())) ancestor = ancestor->Parent();
  R3SurfelLabelAssignment *assignment = ancestor->CurrentLabelAssignment();
  R3SurfelLabel *label = (assignment) ? assignment->Label() : scene->FindLabelByName("Unknown");
  int originator = (assignment) ? assignment->Originator() : R3_SURFEL_LABEL_ASSIGNMENT_MACHINE_ORIGINATOR;
  double confidence = (assignment) ? assignment->Confidence() : 0;

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

#endif



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
      SetMessage("Cannot split object %s because it has no parent", object->Name());
      return 0;
    }
  }

  // Create split constraint
  R3SurfelConstraint *constraint = NULL;
  if (!R2Contains(rubber_line_points[0], rubber_line_points[1])) {
    // Create plane constraint based on split line
    int ix0 = (int) (rubber_line_points[0].X() + 0.5);
    int iy0 = (int) (rubber_line_points[0].Y() + 0.5);
    int ix1 = (int) (rubber_line_points[1].X() + 0.5);
    int iy1 = (int) (rubber_line_points[1].Y() + 0.5);
    R3Ray ray0 = viewer.WorldRay(ix0, iy0);
    R3Ray ray1 = viewer.WorldRay(ix1, iy1);
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
  else {
    // Print error message
    SetMessage("Must drag out a curve if you want to split selected object(s)");
    return 0;
  }

  // Begin logging command
  BeginCommand(R3_SURFEL_LABELER_SPLIT_SELECTION_COMMAND);

  // Empty object selections
  RNArray<R3SurfelObject *> copy_selection_objects = selection_objects;
  EmptyObjectSelections();

  // Split all selected objects
  int split_count = 0;
  RNArray<R3SurfelObject *> objectsA;
  for (int i = 0; i < copy_selection_objects.NEntries(); i++) {
    R3SurfelObject *object = copy_selection_objects.Kth(i);
    R3SurfelObject *parent = object->Parent();
    assert(parent == scene->RootObject());
    if (SplitObject(object, parent, *constraint, &objectsA, NULL)) {
      split_count += objectsA.NEntries();
    }
  }

  // Select objects on one side of split polygon
  for (int i = 0; i < objectsA.NEntries(); i++) {
    R3SurfelObject *object = objectsA.Kth(i);
    assert(!object->Parent()->Parent());
    InsertObjectSelection(object);
  }
    
  // Set message
  SetMessage("Split %d objects", split_count);
  
  // End logging command
  EndCommand();

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

  // Predict label assignments for all objects
  if (classify_after_change_label) classifier.PredictLabelAssignments();

  // Predict object segmentations for all other objects
  if (segment_after_change_label) segmenter.PredictObjectSegmentations();

  // End command
  EndCommand();

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

  // End logging command
  EndCommand();

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

  // Check if label is already assigned to object with same attributes
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
    if (a->Originator() == R3_SURFEL_LABEL_ASSIGNMENT_GROUND_TRUTH_ORIGINATOR) continue;
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
  scene->InsertObject(object, parent);

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
  if (!parent) return 0;
  
  // Update current command 
  if (current_command) {
    R3SurfelObject *old_parent = object->Parent();
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
  glBegin(GL_LINE_LOOP);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2f(rubber_box_corners[0][0], rubber_box_corners[0][1]);
  glVertex2f(rubber_box_corners[0][0], rubber_box_corners[1][1]);
  glVertex2f(rubber_box_corners[1][0], rubber_box_corners[1][1]);
  glVertex2f(rubber_box_corners[1][0], rubber_box_corners[0][1]);
  glEnd();

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
  GLenum primitive = GL_LINE_LOOP;
  if (num_rubber_polygon_points < 3) primitive = GL_LINE_STRIP;
  if (split_polygon_active) primitive = GL_LINE_STRIP;
  glBegin(primitive);
  glColor3f(1.0, 1.0, 1.0);
  for (int i = 0; i < num_rubber_polygon_points; i++) {
    glVertex2f(rubber_polygon_points[i][0], rubber_polygon_points[i][1]);
  }
  glEnd();

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
  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2f(rubber_line_points[0][0], rubber_line_points[0][1]);
  glVertex2f(rubber_line_points[1][0], rubber_line_points[1][1]);
  glEnd();

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
  void *font = GLUT_BITMAP_HELVETICA_10;
  if (height > 300) font = GLUT_BITMAP_HELVETICA_12;
  if (height > 600) font = GLUT_BITMAP_HELVETICA_18;
  if (height > 1200) font = GLUT_BITMAP_TIMES_ROMAN_24;
  
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
  DrawText(R2Point(x-1, y-1), message, font);
  DrawText(R2Point(x-1, y+1), message, font);
  DrawText(R2Point(x+1, y-1), message, font);
  DrawText(R2Point(x+1, y+1), message, font);
  RNLoadRgb(RNwhite_rgb - background_color);
  DrawText(R2Point(x, y), message, font);

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
  void *font = GLUT_BITMAP_HELVETICA_10;
  if (height > 600) font = GLUT_BITMAP_HELVETICA_12;
  if (height > 1200) font = GLUT_BITMAP_HELVETICA_18;
  
  // Set OpenGL modes
  glDisable(GL_LIGHTING);
  glColor3f(1,1,1);
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
  DrawText(origin, buffer, font);

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
  glColor3f(1,1,1);
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
    glColor4d(0, 0.2, 0, 1);
    box.Draw();
    glColor4d(1, 1, 1, 1);
    box.Outline();
    DrawText(R2Point(x, y), "Suggest an Object", GLUT_BITMAP_HELVETICA_18); 
    y -= 32;
  }

  // Draw "Predict All" command
  if (0) {
    R2Box box(x-4, y-4, x + 164, y + 26);
    glColor4d(0, 0.2, 0, 1);
    box.Draw();
    glColor4d(1, 1, 1, 1);
    box.Outline();
    DrawText(R2Point(x, y), "Predict All", GLUT_BITMAP_HELVETICA_18); 
    y -= 32;
  }

  // Draw box around commands
  y += 32;
  glColor4d(1, 1, 1 , 1);
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
NLabelMenuItems(R3SurfelScene *scene)
{
  // Count leaf labels
  int count = 0;
  for (int i = 0; i < scene->NLabels(); i++) {
    R3SurfelLabel *label = scene->Label(i);
    if (label->NParts() > 0) continue;
    if (!strcmp(label->Name(), "Root")) continue;
    count++;
  }

  // Return number of leaf labels, plus one for "All"
  return count + 1;
}



R2Box R3SurfelLabeler::
LabelMenuBBox(void) const
{
  // Update dimension parameters of label menu
  ((R3SurfelLabeler *) this)->UpdateLabelMenu();
  
  // Determine bbox of entire label menu
  int nitems = NLabelMenuItems(scene);
  int height = viewer.Viewport().Height();
  double x1 = label_menu_item_height / 2;
  double x2 = x1 + label_menu_item_width;
  double y2 = height - label_menu_item_height / 2;
  double y1 = y2 - nitems * label_menu_item_height;
  return R2Box(x1, y1, x2, y2);
}



void R3SurfelLabeler::
UpdateLabelMenu(void)
{
  // Set size variables based on window size
  int nitems = NLabelMenuItems(scene);
  int height = viewer.Viewport().Height();
  label_menu_font = GLUT_BITMAP_TIMES_ROMAN_24;
  label_menu_item_height = 28;
  label_menu_item_width = 260;
  if (height < label_menu_item_height * nitems) {
    label_menu_font = GLUT_BITMAP_HELVETICA_18;
    label_menu_item_height = 24;
    label_menu_item_width = 240;
  }
  if ((nitems <= 2) || (height < label_menu_item_height * nitems)) {
    label_menu_font = GLUT_BITMAP_HELVETICA_12;
    label_menu_item_height = 18;
    label_menu_item_width = 190;
  }
  if (height < label_menu_item_height * nitems) {
    label_menu_font = GLUT_BITMAP_HELVETICA_10;
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
  glColor3f(0,0,0);
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

  // Draw "All" visibility box and "All Labels" header
  // Note: this must match PickLabelMenu
  R2Box visibility_box(x + 2*small_gap, y + 2*small_gap, x + label_menu_item_height - 2*small_gap, y + label_menu_item_height - 2*small_gap);
  R2Box name_box(x + label_menu_item_height, y + small_gap, x + label_menu_item_width - small_gap, y + label_menu_item_height - small_gap);
  R2Point visibility_origin(visibility_box[0][0] + small_gap, visibility_box[0][1] + small_gap);
  R2Point name_origin(name_box[0][0]+small_gap, name_box[0][1]+small_gap);
  if (all_visible) glColor4d(1, 1, 1, 0.5);
  else glColor4d(0, 0, 0, 0.5);
  visibility_box.Draw();
  glColor4d(1, 1, 1, 1);
  visibility_box.Outline();
  DrawText(visibility_origin, "v", label_menu_font); 
  DrawText(name_origin, "All Labels", label_menu_font); 
  y -= label_menu_item_height;

  // Draw labels
  for (int i = 0; i < scene->NLabels(); i++) {
    R3SurfelLabel *label = scene->Label(i);
    if (label->NParts() > 0) continue;
    if (!strcmp(label->Name(), "Root")) continue;

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
    if (LabelVisibility(label)) glColor4d(1, 1, 1, 0.5);
    else glColor4d(0, 0, 0, 0.5);
    visibility_box.Draw();
    glColor4d(color[0], color[1], color[2], 1);
    visibility_box.Outline();
    glColor4d(color[0] + 0.5, color[1] + 0.5, color[2] + 0.5, 1);
    DrawText(visibility_origin, "v", label_menu_font); 

    // Draw label box
    glColor3d(0.0, 0.0, 0.0);
    name_box.Draw();
    glColor3d(color[0], color[1], color[2]);
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
    DrawText(name_origin, buffer, label_menu_font); 

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
  glColor4d(1, 0, 0, 1);
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
  R2Box visibility_box(x + 2*small_gap, y + 2*small_gap, x + label_menu_item_height - 2*small_gap, y + label_menu_item_height - 2*small_gap);
  if (R2Contains(visibility_box, cursor)) {
    // Toggle all
    SetLabelVisibility(-1, -1);
  }
  y -= label_menu_item_height;

  // Pick label
  for (int i = 0; i < scene->NLabels(); i++) {
    R3SurfelLabel *label = scene->Label(i);
    if (label->NParts() > 0) continue;
    if (!strcmp(label->Name(), "Root")) continue;

    // Get label info
    // Note: this must match DrawLabelMenu
    R2Box visibility_box(x + 2*small_gap, y + 2*small_gap, x + label_menu_item_height - 2*small_gap, y + label_menu_item_height - 2*small_gap);
    R2Box name_box(x + label_menu_item_height, y + small_gap, x + label_menu_item_width - small_gap, y + label_menu_item_height - small_gap);

    // Process command
    if (R2Contains(visibility_box, cursor)) {
      BeginCommand(R3_SURFEL_LABELER_SET_LABEL_VISIBILITY_COMMAND, (ctrl) ? 1 : 0);

      if (ctrl) {
        // Toggle all except selected label
        SetLabelVisibility(-1, -1);
        SetLabelVisibility(i, -1);
      }
      else {
        // Toggle selected label
        SetLabelVisibility(i, -1);
      }

      EndCommand();
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



