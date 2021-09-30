// Source file for GSV pose class



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "GSV.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Public variables
////////////////////////////////////////////////////////////////////////

GSVPose GSVnull_pose(R3Point(0,0,0), R3Quaternion(0,0,0,0));



////////////////////////////////////////////////////////////////////////
// constructor/destructor functions
////////////////////////////////////////////////////////////////////////

GSVPose::
GSVPose(void)
  : viewpoint(0,0,0),
    orientation(0,0,0,0)
{
}



GSVPose::
GSVPose(const R3Point& viewpoint, const R3Quaternion& orientation)
  : viewpoint(viewpoint),
    orientation(orientation)
{
}



GSVPose::
GSVPose(const R3Point& viewpoint, const R3Triad& triad)
  : viewpoint(viewpoint),
    orientation(triad.Matrix(), 0)
{
}



GSVPose::
GSVPose(const R3Point& viewpoint, const R3Vector& towards, const R3Vector& up)
  : viewpoint(viewpoint),
    orientation(R3Triad(towards, up).Matrix(), 0)
{
}



GSVPose::
GSVPose(double px, double py, double pz, double qx, double qy, double qz, double qw)
  : viewpoint(px,py,pz),
    orientation(qw,qx,qy,qz)
{
}



GSVPose::
~GSVPose(void)
{
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

const R3Vector GSVPose::
Towards(void) const
{
  // Return vector along view direction
  R4Matrix rotation = orientation.Matrix();
  R3Vector towards = rotation * R3negz_vector;
  towards.Normalize();
  return towards;
}



const R3Vector GSVPose::
Up(void) const
{
  // Return up vector
  R4Matrix rotation = orientation.Matrix();
  R3Vector up = rotation * R3posy_vector;
  up.Normalize();
  return up;
}



const R3Vector GSVPose::
Right(void) const
{
  // Return up vector
  R4Matrix rotation = orientation.Matrix();
  R3Vector right = rotation * R3posx_vector;
  right.Normalize();
  return right;
}



const R4Matrix GSVPose::
Matrix(void) const
{
  // Return matrix (std -> pose)
  R4Matrix matrix = R4identity_matrix;
  matrix.Translate(viewpoint.Vector());
  matrix.Multiply(orientation.Matrix());
  return matrix;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVPose::
SetViewpoint(const R3Point& viewpoint)
{
  // Set viewpoint
  this->viewpoint = viewpoint;
}



void GSVPose::
SetOrientation(const R3Quaternion& orientation)
{
  // Set orientation
  this->orientation = orientation;
}



void GSVPose::
Translate(const R3Vector& translation)
{
  // Translate viewpoint
  viewpoint.Translate(translation);
}



void GSVPose::
Rotate(const R3Quaternion& rotation)
{
  // Rotate orientation
  orientation.Rotate(rotation);
}



void GSVPose::
Rotate(const R3Vector& xyz_angles)
{
  // Rotate orientation
  orientation.Rotate(xyz_angles);
}



void GSVPose::
Transform(const R3Affine& affine)
{
  // Transform viewpoint and orientation
  viewpoint.Transform(affine);
  orientation = R3Quaternion(affine.Matrix(), 0) * orientation;
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVPose::
Draw(RNFlags flags) const
{
  // Draw viewpoint
  R3Sphere(viewpoint, 0.1).Draw();

  // Draw towards vector
  R3Span(viewpoint, viewpoint + Towards()).Draw();

  // Draw up vector
  R3Span(viewpoint, viewpoint + 0.5 * Up()).Draw();
}



void GSVPose::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print pose header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Pose:   %g %g %g   %g %g %g %g", 
    viewpoint[0], viewpoint[1], viewpoint[2], 
    orientation[0], orientation[1], orientation[2], orientation[3]);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");
}



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

GSVPose 
GSVInterpolatedPose(const GSVPose& pose0, const GSVPose& pose1, RNScalar t)
{
  // Return interpolated pose (with spherical interpolation of orientation)
  R3Point viewpoint = (1-t)*pose0.Viewpoint() + t*pose1.Viewpoint();
  R3Quaternion orientation = R3QuaternionSlerp(pose0.Orientation(), pose1.Orientation(), t);
  return GSVPose(viewpoint, orientation);
}



} // namespace gaps
