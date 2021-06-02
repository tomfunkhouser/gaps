// Source file for GSV camera class



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "GSV.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Constructor/destructor functions
////////////////////////////////////////////////////////////////////////

GSVCamera::
GSVCamera(void)
  : run(NULL),
    run_index(-1),
    tapestries(),
    distortion_type(-1),
    width(-1),
    height(-1),
    focal_x(-1), 
    focal_y(-1),
    center_x(-1),
    center_y(-1),
    k1(-1), k2(-1), k3(-1),
    p1(-1), p2(-1),
    fov_max(-1), 
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
}



GSVCamera::
~GSVCamera(void)
{
  // Remove camera from run
  if (run) run->RemoveCamera(this);

  // Remove camera tapestries from this camera
  while (NTapestries() > 0) RemoveTapestry(Tapestry(0));
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVCamera::
Scene(void) const
{
  // Return scene
  if (!run) return NULL;
  return run->Scene();
}



int GSVCamera::
SceneIndex(void) const
{
  // Get convenient variables
  GSVScene *scene = Scene();
  if (!scene) return -1;

  // Compute scene index
  int scene_index = RunIndex();
  for (int i = 0; i < run->SceneIndex(); i++) {
    GSVRun *r = scene->Run(i);
    scene_index += r->NCameras();
  }

  // Return scene index
  return scene_index;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVCamera::
InsertTapestry(GSVTapestry *tapestry)
{
  // Just checking
  assert(tapestry->camera_index == -1);
  assert(tapestry->camera == NULL);

  // Insert tapestry
  tapestry->camera = this;
  tapestry->camera_index = tapestries.NEntries();
  tapestries.Insert(tapestry);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVCamera::
RemoveTapestry(GSVTapestry *tapestry)
{
  // Just checking
  assert(tapestry->camera_index >= 0);
  assert(tapestry->camera_index < tapestries.NEntries());
  assert(tapestry->camera == this);

  // Remove tapestry
  RNArrayEntry *entry = tapestries.KthEntry(tapestry->camera_index);
  GSVTapestry *tail = tapestries.Tail();
  tail->camera_index = tapestry->camera_index;
  tapestries.EntryContents(entry) = tail;
  tapestries.RemoveTail();
  tapestry->camera_index = -1;
  tapestry->camera = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVCamera::
SetDistortionType(int distortion_type)
{
  // Set distorition type
  this->distortion_type = distortion_type;
}



void GSVCamera::
SetImageWidth(int width)
{
  // Set width
  this->width = width;
}



void GSVCamera::
SetImageHeight(int height)
{
  // Set height
  this->height = height;
}



void GSVCamera::
SetXFocal(RNLength focal_x)
{
  // Set distance from view plane to focal point (based on estimate in horizontal direction)
  this->focal_x = focal_x;
}



void GSVCamera::
SetYFocal(RNScalar focal_y)
{
  // Set distance from view plane to focal point (based on estimate in vertical direction)
  this->focal_y = focal_y;
}



void GSVCamera::
SetXCenter(RNScalar center_x)
{
  // Set center x coordinate
  this->center_x = center_x;
}



void GSVCamera::
SetYCenter(RNScalar center_y)
{
  // Set center y coordinate
  this->center_y = center_y;
}



void GSVCamera::
SetK1(RNScalar k1)
{
  // Set k1 parameter
  this->k1 = k1;
}



void GSVCamera::
SetK2(RNScalar k2)
{
  // Set k2 parameter
  this->k2 = k2;
}



void GSVCamera::
SetK3(RNScalar k3)
{
  // Set k3 parameter
  this->k3 = k3;
}



void GSVCamera::
SetP1(RNScalar p1)
{
  // Set p1 parameter
  this->p1 = p1;
}



void GSVCamera::
SetP2(RNScalar p2)
{
  // Set p2 parameter
  this->p2 = p2;
}



void GSVCamera::
SetMaxFov(RNScalar fov_max)
{
  // Set max field of view
  this->fov_max = fov_max;
}






////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVCamera::
Draw(RNFlags flags) const
{
  // Draw tapestries
  for (int i = 0; i < NTapestries(); i++) {
    GSVTapestry *tapestry = Tapestry(i);
    tapestry->Draw(flags);
  }
}



void GSVCamera::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print camera header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Camera %d:", run_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

   // Print tapestries
  for (int i = 0; i < NTapestries(); i++) {
    GSVTapestry *tapestry = Tapestry(i);
    tapestry->Print(fp, indented_prefix, suffix);
  }
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void GSVCamera::
UpdateBBox(void) 
{
  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NTapestries(); i++) {
    bbox.Union(Tapestry(i)->BBox());
  }
}


void GSVCamera::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (run) run->InvalidateBBox();
  }
}



////////////////////////////////////////////////////////////////////////
// Mapping functions
////////////////////////////////////////////////////////////////////////

R2Point GSVCamera::
DistortedPositionFromUndistortedNormalizedPosition(RNAngle x_dir, RNAngle y_dir) const
{
  // (xdir, y_dir) is undistorted position given in normalized coordinates
  // (x_pixel, y_pixel) is distorted position given in pixel coordinates 

  if (distortion_type == 0) { // PERSPECTIVE
    double x2 = x_dir * x_dir;
    double y2 = y_dir * y_dir;
    double r2 = x2 + y2;
    
    double r_max = tan( fov_max );
    double r2_max = r_max * r_max;
    if ( r2 > r2_max ) return R2Point(-1,-1);
    
    double xy = x_dir * y_dir;
    
    double dist_scale = 1 + k1*r2 + k2*r2*r2 + k3*r2*r2*r2;
    double x_distorted = dist_scale * x_dir + 2 * p1 * xy + p2 * (r2 + 2 * x2);
    double y_distorted = dist_scale * y_dir + p1 * ( r2 + 2 * y2 ) + 2 * p2 * xy;
    
    double x_pixel = focal_x * x_distorted + center_x;
    double y_pixel = focal_y * y_distorted + center_y;

    return R2Point(x_pixel, y_pixel);
  }
  else if (distortion_type == 1) {  // FISHEYE
    double x2 = x_dir * x_dir;
    double y2 = y_dir * y_dir;
    double r2 = x2 + y2;

    double r_max = tan( fov_max );
    double r2_max = r_max * r_max;
    if ( r2 > r2_max ) return R2Point(-1,-1);

    double r = sqrt ( r2 );

    double theta_tilde = 1;
    if (r > 1e-8) {
      double theta = atan( r );
      double theta2 = theta * theta;
      theta_tilde = theta * ( 1 + k1 * theta2 + k2 * theta2*theta2 ) / r;
    }

    double x_distorted = theta_tilde * x_dir;
    double y_distorted = theta_tilde * y_dir;

    double x_pixel = focal_x * x_distorted + center_x;
    double y_pixel = focal_y * y_distorted + center_y;

    return R2Point(x_pixel, y_pixel);
  }

  // Should never get here
  RNFail("Unrecognized distortion type: %d\n", distortion_type);
  return R2zero_point;
}



R2Point GSVCamera::
DistortedPosition(const R2Point& undistorted_image_position) const
{
  // Check focal lengths
  if (RNIsNegativeOrZero(focal_x) || RNIsNegativeOrZero(focal_y))
    return R2Point(-1,-1);
  
  // Check if there is no distortion
  if ((k1 == 0) && (k2 == 0) && (k3 == 0) && (p1 == 0) && (p2 == 0))
    return undistorted_image_position;
  
  // Undistorted_position and distorted position are both given in pixel coordinates 
  double delta_x = undistorted_image_position.X() - center_x;
  double delta_y = undistorted_image_position.Y() - center_y;
  double dir_x = delta_x / focal_x;
  double dir_y = delta_y / focal_y;
  return DistortedPositionFromUndistortedNormalizedPosition(dir_x, dir_y);
}



R2Point GSVCamera::
UndistortedPosition(const R2Point& distorted_image_position) const
{
  // Check if there is no distortion
  if ((k1 == 0) && (k2 == 0) && (k3 == 0) && (p1 == 0) && (p2 == 0))
    return distorted_image_position;
  
  // Not implemented yet
  RNAbort("Not implemented");
  return R2null_point;
}



int GSVCamera::
UndistortImage(const R2Image& input, R2Image& output) const
{
  // Get convenient variables
  double xscale = (double) output.Width() / (double) input.Width();
  double yscale = (double) output.Height() / (double) input.Height();
  R2Box input_bbox(0, 0, input.Width()-1, input.Height()-1);

  // Create undistorted image (output) from distorted image (input)
  // Removes radial distortion
  for (int x = 0; x < output.Width(); x++) {
    for (int y = 0; y < output.Height(); y++) {
      R2Point output_position(x,y);
      output_position[0] *= xscale;
      output_position[1] *= yscale;
      R2Point input_position = DistortedPosition(output_position);
      if (R2Contains(input_bbox, input_position)) {
        int ix1 = (int) input_position[0];
        int iy1 = (int) input_position[1];
        int ix2 = ix1 + 1;
        int iy2 = iy1 + 1;
        if (ix2 >= input.Width()) ix2 = ix1;
        if (iy2 >= input.Height()) iy2 = iy1;
        double dx = input_position[0] - ix1;
        double dy = input_position[1] - iy1;
        RNRgb pixel1 = (1-dx) * input.PixelRGB(ix1, iy1) + dx * input.PixelRGB(ix2, iy1);
        RNRgb pixel2 = (1-dx) * input.PixelRGB(ix1, iy2) + dx * input.PixelRGB(ix2, iy2);
        RNRgb pixel = (1-dy) * pixel1 + dy * pixel2;
        output.SetPixelRGB(x, y, pixel);
      }
      else {
        output.SetPixelRGB(x, y, RNblack_rgb);
      }
    }
  }

  // Return success
  return 1;
}



int GSVCamera::
DistortImage(const R2Image& input, R2Image& output) const
{
  // Get convenient variables
  double xscale = (double) output.Width() / (double) input.Width();
  double yscale = (double) output.Height() / (double) input.Height();
  R2Box input_bbox(0, 0, input.Width()-1, input.Height()-1);

  // Create distorted image (output) from undistorted image (input)
  // Adds radial distortion
  for (int x = 0; x < output.Width(); x++) {
    for (int y = 0; y < output.Height(); y++) {
      R2Point output_position(x,y);
      output_position[0] *= xscale;
      output_position[1] *= yscale;
      R2Point input_position = UndistortedPosition(output_position);
      if (R2Contains(input_bbox, input_position)) {
        int ix1 = (int) input_position[0];
        int iy1 = (int) input_position[1];
        int ix2 = ix1 + 1;
        int iy2 = iy1 + 1;
        if (ix2 >= input.Width()) ix2 = ix1;
        if (iy2 >= input.Height()) iy2 = iy1;
        double dx = input_position[0] - ix1;
        double dy = input_position[1] - iy1;
        RNRgb pixel1 = (1-dx) * input.PixelRGB(ix1, iy1) + dx * input.PixelRGB(ix2, iy1);
        RNRgb pixel2 = (1-dx) * input.PixelRGB(ix1, iy2) + dx * input.PixelRGB(ix2, iy2);
        RNRgb pixel = (1-dy) * pixel1 + dy * pixel2;
        output.SetPixelRGB(x, y, pixel);
      }
      else {
        output.SetPixelRGB(x, y, RNblack_rgb);
      }
    }
  }

  // Return success
  return 1;
}



} // namespace gaps




