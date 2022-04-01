/* Source file for utility that creates a signed distance field */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Graphics/R3Graphics.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////

int RasterizeSDF(R2Grid& grid, const R2Polygon& polygon, int end_condition)
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
    glColor3ub(0, 255, 0);
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
    glColor3ub(255, 0, 0);
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
      glColor3ub(0, 255, 0);
      R3BeginPolygon();
      R3LoadPoint(p1->X(), p1->Y(), 0.0);
      R3LoadPoint(p1->X() - max_distance * n2.X(), p1->Y() - max_distance * n2.Y(), -max_distance);
      R3LoadPoint(p2->X() - max_distance * n2.X(), p2->Y() - max_distance * n2.Y(), -max_distance);
      R3LoadPoint(p2->X(), p2->Y(), 0.0);
      R3EndPolygon();

      // Draw left side of wedge
      glColor3ub(255, 0, 0);
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



}; // end namespace
