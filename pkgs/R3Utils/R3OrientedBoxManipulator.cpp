// Source file for interactive obb manipulation



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils/R3Utils.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {


  
////////////////////////////////////////////////////////////////////////
// Constructor/destructor functions
////////////////////////////////////////////////////////////////////////

R3OrientedBoxManipulator::
R3OrientedBoxManipulator(void)
  : oriented_box(R3Point(0.0, 0.0, 0.0),
      R3Vector(1.0, 0.0, 0.0), R3Vector(0.0, 1.0, 0.0),
      -1.0, -1.0, -1.0),
    rotating_allowed(TRUE),
    scaling_allowed(TRUE),
    translating_allowed(TRUE),
    selection_corner0(-1),
    selection_corner1(-1),
    selection_t(-1),
    manipulation_type(R3_NO_MANIPULATION)
{
}



R3OrientedBoxManipulator::
R3OrientedBoxManipulator(const R3OrientedBox& oriented_box)
  : oriented_box(oriented_box),
    selection_corner0(-1),
    selection_corner1(-1),
    selection_t(-1),
    manipulation_type(R3_NO_MANIPULATION),
    manipulation_counter(0)
{
}



////////////////////////////////////////////////////////////////////////
// Top-level interactive manipulation functions
////////////////////////////////////////////////////////////////////////

int R3OrientedBoxManipulator::
BeginManipulation(const R3Viewer& viewer, int x, int y)
{
#if 0
  // Useful constants defining pairs of corners for all edges
  static const int nedges = 12;
  static const int edges[nedges][2] =
    { { RN_NNN_OCTANT, RN_PNN_OCTANT }, { RN_NNN_OCTANT, RN_NPN_OCTANT }, { RN_NNN_OCTANT, RN_NNP_OCTANT },
      { RN_PNN_OCTANT, RN_PPN_OCTANT }, { RN_PNN_OCTANT, RN_PNP_OCTANT }, 
      { RN_NPN_OCTANT, RN_NPP_OCTANT }, { RN_NPN_OCTANT, RN_PPN_OCTANT },
      { RN_NNP_OCTANT, RN_NPP_OCTANT }, { RN_NNP_OCTANT, RN_PNP_OCTANT },
      { RN_NPP_OCTANT, RN_PPP_OCTANT }, { RN_PNP_OCTANT, RN_PPP_OCTANT }, { RN_PPN_OCTANT, RN_PPP_OCTANT } };
#endif
  
  // Reset manipulation
  ResetManipulation();
  ResetSelection();
  
  // Check oriented_box
  if (oriented_box.IsEmpty()) return 0;

  // Get convenient variables
  double max_distance = 10;
  double max_squared_distance = max_distance * max_distance;
  R2Box viewport_bbox = viewer.Viewport().BBox();
  R2Point cursor_position(x, y);

  // Check corners
  if (manipulation_type == R3_NO_MANIPULATION) {
    if (scaling_allowed) {
      for (int octant = 0; octant < RN_NUM_OCTANTS; octant++) {
        R3Point position = oriented_box.Corner(octant);
        R2Point projection = viewer.ViewportPoint(position);
        if (!R2Contains(viewport_bbox, projection)) continue;
        RNScalar dd = R2SquaredDistance(projection, cursor_position);
        if (dd > max_squared_distance) continue;
        selection_corner0 = octant;
        selection_corner1 = octant;
        selection_t = 0.0;
        manipulation_type = R3_SCALE_MANIPULATION;
      }
    }
  }

#if 0
  // Check edges
  if (manipulation_type == R3_NO_MANIPULATION) {
    if (scaling_allowed) {
      for (int i = 0; i < nedges; i++) {
        int octant0 = edges[i][0];
        int octant1 = edges[i][1];
        R3Point position0 = oriented_box.Corner(octant0);
        R3Point position1 = oriented_box.Corner(octant1);
        R2Point projection0 = viewer.ViewportPoint(position0);
        R2Point projection1 = viewer.ViewportPoint(position1);
        if (R2Contains(R2infinite_point, projection0)) continue;
        if (R2Contains(R2infinite_point, projection1)) continue;
        R2Span edge(projection0, projection1);
        if (RNIsZero(edge.Length())) continue;
        RNScalar d = R2Distance(edge, cursor_position);
        if (d < max_distance) {
          double t = edge.T(cursor_position) / edge.Length();
          t = (t > 1) ? 1 : ((t < 0) ? 0 : t);
          selection_corner0 = octant0;
          selection_corner1 = octant1;
          selection_t = t;
          manipulation_type = R3_SCALE_MANIPULATION;
        }
      }
    }
  }
#endif
  
  // Check nose
  if (manipulation_type == R3_NO_MANIPULATION) {
    if (rotating_allowed) {
      double min_nose_vector_length = 1;
      double nose_vector_length = 0.5 * oriented_box.Radius(0);
      if (nose_vector_length < min_nose_vector_length) 
        nose_vector_length = min_nose_vector_length;
      R3Point position0 = oriented_box.Center() + oriented_box.Radius(0) * oriented_box.Axis(0);
      R3Point position1 = position0 + nose_vector_length * oriented_box.Axis(0);
      R2Point projection0 = viewer.ViewportPoint(position0);
      R2Point projection1 = viewer.ViewportPoint(position1);
      R2Span nose(projection0, projection1);
      if (RNIsPositive(nose.Length())) {
        RNScalar d = R2Distance(nose, cursor_position);
        if (d < max_distance) {
          selection_corner0 = RN_PNP_OCTANT;
          selection_corner1 = RN_PPP_OCTANT;
          selection_t = 0.5;
          manipulation_type = R3_ROTATION_MANIPULATION;
        }
      }
    }
  }

  // Reset manipulation counter
  manipulation_counter = 0;
  
  // Return manipulation_type
  return manipulation_type;
}


  
int R3OrientedBoxManipulator::
UpdateManipulation(const R3Viewer& viewer, int x, int y)
{
  // Increment manipulation counter
  if (manipulation_type != R3_NO_MANIPULATION)
    manipulation_counter++;
  
  // Update the active manipulation
  switch (manipulation_type) {
  case R3_ROTATION_MANIPULATION:
    return UpdateRotation(viewer, x, y);
    
  case R3_SCALE_MANIPULATION:
    return UpdateScale(viewer, x, y);

  case R3_TRANSLATION_MANIPULATION:
    return UpdateTranslation(viewer, x, y);
  }
  
  // Return zero indicating that did nothing
  return 0;
}



void R3OrientedBoxManipulator::
EndManipulation(void)
{
  // Reset manipulation and selection variables
  ResetManipulation();
  ResetSelection();
}


  
void R3OrientedBoxManipulator::
ResetOrientedBox(void)
{
  // Reset oriented box
  oriented_box = R3null_oriented_box;
}



void R3OrientedBoxManipulator::
ResetManipulation(void)
{
  // Reset manipulation
  manipulation_type =  R3_NO_MANIPULATION;
}

  

void R3OrientedBoxManipulator::
ResetSelection(void)
{
  // Reset selection variables
  selection_corner0 = -1;
  selection_corner1 = -1;
  selection_t = -1;
}

  

////////////////////////////////////////////////////////////////////////
// Property manipulation functions
////////////////////////////////////////////////////////////////////////
  
void R3OrientedBoxManipulator::
SetRotatingAllowed(RNBoolean allowed)
{
  // Set whether rotating is allowed
  rotating_allowed = allowed;
}


  
void R3OrientedBoxManipulator::
SetScalingAllowed(RNBoolean allowed)
{
  // Set whether scaling is allowed
  scaling_allowed = allowed;
}


  
void R3OrientedBoxManipulator::
SetTranslatingAllowed(RNBoolean allowed)
{
  // Set whether translating is allowed
  translating_allowed = allowed;
}


  
void R3OrientedBoxManipulator::
SetOrientedBox(const R3OrientedBox& oriented_box)
{
  // Set oriented box
  this->oriented_box = oriented_box;

  // Reset manipulation
  // ResetManipulation();
}

  
void R3OrientedBoxManipulator::
RotateOrientedBox(RNAngle theta)
{
  // Rotate oriented box around Z axis
  oriented_box.Rotate(RN_Z, theta);

  // Reset manipulation
  ResetManipulation();
}


  
void R3OrientedBoxManipulator::
SwapOrientedBoxAxes(void)
{
  // Swap axis0 and axis1
  oriented_box.Reset(oriented_box.Center(), oriented_box.Axis(1), -oriented_box.Axis(0),
    oriented_box.Radius(1), oriented_box.Radius(0), oriented_box.Radius(2));

  // Reset manipulation
  ResetManipulation();
}


  
////////////////////////////////////////////////////////////////////////
// Lower-level Interactive manipulation functions
////////////////////////////////////////////////////////////////////////

int R3OrientedBoxManipulator::
UpdateRotation(const R3Viewer& viewer, int x, int y)
{
  // Check oriented_box
  if (oriented_box.IsEmpty()) return 0;

  // Check manipulation
  if (manipulation_type != R3_ROTATION_MANIPULATION) return 0;

  // Find ray through pixel
  R3Ray ray = viewer.WorldRay(x, y);

  // Find target position
  R3Point target_position;
  R3Plane plane(oriented_box.Center(), R3posz_vector);
  if (!R3Intersects(ray, plane, &target_position)) return 0;

  // Compute new axes
  R3Vector axis0 = target_position - oriented_box.Centroid();
  axis0[2] = 0;
  axis0.Normalize();
  R3Vector axis1 = R3posz_vector % axis0;
  axis1.Normalize();

  // Reorient oriented_box
  oriented_box.Reorient(axis0, axis1);

  // Return success
  return 1;
}



int R3OrientedBoxManipulator::
UpdateScale(const R3Viewer& viewer, int x, int y)
{
  // Check oriented_box
  if (oriented_box.IsEmpty()) return 0;

  // Check manipulation
  if (manipulation_type != R3_SCALE_MANIPULATION) return 0;

  // Check if anchor position is set
  if (selection_corner0 < 0) return 0;
  if (selection_corner1 < 0) return 0;
  if (selection_t < 0) return 0;
  if (selection_t > 1) return 0;

  // Find ray through pixel
  R3Ray ray = viewer.WorldRay(x, y);

  // Find dimension of oriented_box ray is most aligned with
  int dim = 0;
  double dot0 = (fabs(ray.Vector().Dot(oriented_box.Axis(0))));
  double dot1 = (fabs(ray.Vector().Dot(oriented_box.Axis(1))));
  double dot2 = (fabs(ray.Vector().Dot(oriented_box.Axis(2))));
  if (dot0 > dot1) {
    if (dot0 > dot2) dim = 0;
    else dim = 2;
  }
  else {
    if (dot1 > dot2) dim = 1;
    else dim = 2;
  }

  
  // Determine anchor position
  double t = selection_t;
  R3Point p0 = oriented_box.Corner(selection_corner0);
  R3Point p1 = oriented_box.Corner(selection_corner1);
  R3Point anchor_position = (1-t)*p0 + t*p1;

  // Find target position
  R3Point target_position;
  R3Plane target_plane(anchor_position, oriented_box.Axis(dim));
  if (!R3Intersects(ray, target_plane, &target_position)) return 0;
  
  // Initialize which dimensions are locked
  int lock[2][3] = { { 0, 0, 0 }, { 0, 0, 0 } };

  // Lock some dimensions based on corner selection
  if (selection_corner0 & RN_PNN_OCTANT) lock[0][0]= 1;
  else lock[1][0]= 1;
  if (selection_corner0 & RN_NPN_OCTANT) lock[0][1]= 1;
  else lock[1][1]= 1;
  if (selection_corner0 & RN_NNP_OCTANT) lock[0][2]= 1;
  else lock[1][2]= 1;
  if (selection_corner1 & RN_PNN_OCTANT) lock[0][0]= 1;
  else lock[1][0]= 1;
  if (selection_corner1 & RN_NPN_OCTANT) lock[0][1]= 1;
  else lock[1][1]= 1;
  if (selection_corner1 & RN_NNP_OCTANT) lock[0][2]= 1;
  else lock[1][2]= 1;

  // Lock dimension best aligned with ray
  lock[0][dim] = lock[1][dim] = 1;

  // Resize box
  R3Point center = oriented_box.Center();
  R3Vector radii(oriented_box.Radius(0), oriented_box.Radius(1), oriented_box.Radius(2));
  R3Vector vector = 0.5*(target_position - anchor_position);
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 3; j++) {
      if (lock[i][j]) continue;
      double dradius = vector.Dot(oriented_box.Axis(j));
      if (i == 0) dradius = -dradius;
      double min_dradius = -(radii[j] - 0.1);
      if (dradius < min_dradius) dradius = min_dradius;
      if (i == 0) center -= dradius * oriented_box.Axis(j);
      else center += dradius * oriented_box.Axis(j);
      radii[j] += dradius;
    }
  }
  oriented_box.Reposition(center);
  oriented_box.Resize(radii[0], radii[1], radii[2]);
  
  // Return success
  return 1;
}



int R3OrientedBoxManipulator::
UpdateTranslation(const R3Viewer& viewer, int x, int y)
{
  // Check oriented_box
  if (oriented_box.IsEmpty()) return 0;

  // Check manipulation
  if (manipulation_type != R3_TRANSLATION_MANIPULATION) return 0;

  // Check if anchor position is set
  if (selection_corner0 < 0) return 0;
  if (selection_corner1 < 0) return 0;
  if (selection_t < 0) return 0;
  if (selection_t > 1) return 0;

  // Find ray through pixel
  R3Ray ray = viewer.WorldRay(x, y);

  // Determine anchor position
  double t = selection_t;
  R3Point p0 = oriented_box.Corner(selection_corner0);
  R3Point p1 = oriented_box.Corner(selection_corner1);
  R3Point anchor_position = (1-t)*p0 + t*p1;
  
  // Determine plane of translation
  unsigned int a = selection_corner0 ^ selection_corner1;
  int axis = (a == 1) ? 0 : ((a == 2) ? 1 : 2);
  R3Plane plane(anchor_position, oriented_box.Axis(axis));

  // Find target position
  R3Point target_position;
  if (!R3Intersects(ray, plane, &target_position)) return 0;

  // Translate oriented_box
  R3Vector translation = target_position - anchor_position;
  oriented_box.Reposition(oriented_box.Center() + translation);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Draw functions
////////////////////////////////////////////////////////////////////////

void R3OrientedBoxManipulator::
DrawOrientedBox(void) const
{
  // Check oriented box
  if (oriented_box.IsEmpty()) return;

  // Draw oriented box
  oriented_box.Outline();
}



void R3OrientedBoxManipulator::
DrawNose(void) const
{
  // Check oriented box
  if (oriented_box.IsEmpty()) return;

  // Get nose vector length
  double min_nose_vector_length = 1;
  double nose_vector_length = 0.5 * oriented_box.Radius(0);
  if (nose_vector_length < min_nose_vector_length) {
    nose_vector_length = min_nose_vector_length;
  }

  // Draw nose vector
  R3Point start = oriented_box.Center() + oriented_box.Radius(0) * oriented_box.Axis(0);
  R3Point end = start + nose_vector_length * oriented_box.Axis(0);
  R3Span(start, end).Draw();
}


  
void R3OrientedBoxManipulator::
DrawAnchor(void) const
{
  // Check manipulation type
  if (manipulation_type == R3_NO_MANIPULATION) return;

  // Check oriented box
  if (oriented_box.IsEmpty()) return;

  // Compute anchor radius
  RNLength radius = 0.05 * oriented_box.DiagonalRadius();
  
  // Draw anchor position
  if (manipulation_type == R3_ROTATION_MANIPULATION) {
    R3Point anchor_position = oriented_box.Center() + oriented_box.Radius(0) * oriented_box.Axis(0);
    R3Sphere(anchor_position, radius).Draw();
  }
  else if (manipulation_type == R3_SCALE_MANIPULATION) {
    if (selection_corner0 < 0) return;
    if (selection_corner1 < 0) return;
    if (selection_t < 0) return;
    if (selection_t > 1) return;
    R3Point p0 = oriented_box.Corner(selection_corner0);
    R3Point p1 = oriented_box.Corner(selection_corner1);
    R3Point anchor_position = (1-selection_t)*p0 + selection_t*p1;
    R3Sphere(anchor_position, radius).Draw();
  }
}



void R3OrientedBoxManipulator::
Draw(void) const
{
  // Draw obb
  DrawOrientedBox();

  // Draw nose
  if (rotating_allowed) {
    DrawNose();
  }

  // Draw anchor
  if (IsManipulating()) {
    DrawAnchor();
  }
}


  
}; // end namespace
