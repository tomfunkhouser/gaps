/* Source file for the R3 surfel image class */



////////////////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////////////////

#include "R3Surfels.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS/DESTRUCTORS
////////////////////////////////////////////////////////////////////////

R3SurfelImage::
R3SurfelImage(const char *name)
  : scene(NULL),
    scene_index(-1),
    scan(NULL),
    channels(),
    pose(R3Point(0,0,0), R3Triad(R3Vector(0,0,0), R3Vector(0,0,0))),
    timestamp(0),
    image_width(0), 
    image_height(0),
    image_center(0,0),
    xfocal(0),
    yfocal(0),
    distortion_type(R3_SURFEL_NO_DISTORTION),
    name((name) ? RNStrdup(name) : NULL),
    flags(0),
    data(NULL)
{
  // Intialize distortion parameters
  radial_distortion[0] = 0;
  radial_distortion[1] = 0;
  radial_distortion[2] = 0;
  tangential_distortion[0] = 0;
  tangential_distortion[1] = 0;

  // Initialize rolling shutter pose parameters
  rolling_shutter_poses[0] = pose;
  rolling_shutter_poses[1] = pose;
  rolling_shutter_timestamps[0] = timestamp;
  rolling_shutter_timestamps[1] = timestamp;
}



R3SurfelImage::
R3SurfelImage(const R3SurfelImage& image)
  : scene(NULL),
    scene_index(-1),
    scan(NULL),
    channels(),
    pose(image.pose),
    timestamp(image.timestamp),
    image_width(image.image_width), 
    image_height(image.image_height),
    image_center(image.image_center),
    xfocal(image.xfocal),
    yfocal(image.yfocal),
    distortion_type(R3_SURFEL_NO_DISTORTION),
    name((image.name) ? RNStrdup(image.name) : NULL),
    flags(0),
    data(NULL)
{
  // Intialize distortion parameters
  radial_distortion[0] = image.radial_distortion[0];
  radial_distortion[1] = image.radial_distortion[1];
  radial_distortion[2] = image.radial_distortion[2];
  tangential_distortion[0] = image.tangential_distortion[0];
  tangential_distortion[1] = image.tangential_distortion[1];

  // Initialize rolling shutter pose parameters
  rolling_shutter_poses[0] = image.rolling_shutter_poses[0];
  rolling_shutter_poses[1] = image.rolling_shutter_poses[1];
  rolling_shutter_timestamps[0] = image.rolling_shutter_timestamps[0];
  rolling_shutter_timestamps[1] = image.rolling_shutter_timestamps[1];

  // Copy all channels
  for (int i = 0; i < channels.NEntries(); i++) {
    R2Grid *channel = channels.Kth(i);
    if (channel) channels.Insert(new R2Grid(*channel));
    else channels.Insert(NULL);
  }
}



R3SurfelImage::
~R3SurfelImage(void)
{
  // Remove image from scan
  if (scan) SetScan(NULL);
  
  // Remove image from scene
  if (scene) scene->RemoveImage(this);

  // Delete all channels
  for (int i = 0; i < channels.NEntries(); i++) {
    R2Grid *channel = channels.Kth(i);
    if (channel) delete channel;
  }
  
  // Delete name
  if (name) free(name);
}



////////////////////////////////////////////////////////////////////////
// PROPERTY RETRIEVAL FUNCTIONS
////////////////////////////////////////////////////////////////////////

R2Image R3SurfelImage::
ColorChannels(void) const
{
  // Allocate color image for result
  R2Image color_image;

  // Get RGB channels
  const R2Grid *red_channel = RedChannel();
  const R2Grid *green_channel = GreenChannel();
  const R2Grid *blue_channel = BlueChannel();
  if (red_channel && green_channel && blue_channel) {
    // Fill in RGB for each pixel
    color_image = R2Image(ImageWidth(), ImageHeight(), 3);
    for (int iy = 0; iy < ImageHeight(); iy++) {
      for (int ix = 0; ix < ImageWidth(); ix++) {
        RNScalar red = red_channel->GridValue(ix, iy);
        RNScalar green = green_channel->GridValue(ix, iy);
        RNScalar blue = blue_channel->GridValue(ix, iy);
        RNRgb color(red, green, blue);
        color_image.SetPixelRGB(ix, iy, color);
      }
    }
  }

  // Return color image
  return color_image;
}
 

  
R4Matrix R3SurfelImage::
ProjectionMatrix(RNScalar neardist, RNScalar fardist) const
{
  // Return projection transformation from camera coordinates to normalized image coordinates
  RNScalar fx = XFocal();
  RNScalar fy = YFocal();
  RNScalar cx = ImageCenter().X();
  RNScalar cy = ImageCenter().Y();
  RNScalar W = ImageWidth();
  RNScalar H = ImageHeight();
  return R4Matrix(
    2*fx/W, 0, -2*(cx/W)+1, 0,
    0, 2*fy/H, -2*(cy/H)+1, 0,
    0, 0, -(fardist + neardist) / (fardist - neardist),  -2.0 * neardist * fardist / (fardist - neardist),
    0, 0, -1, 0 );
}



R3Ray R3SurfelImage::
PixelWorldRay(const R2Point& image_position) const
{
  // Get/check intrinsics matrix
  if (RNIsZero(xfocal)) return R3null_ray;
  if (RNIsZero(yfocal)) return R3null_ray;

  // Get point at depth 1.0 along ray in camera coordinates
  R3Point camera_position;
  camera_position[0] = (image_position[0] - image_center.X()) / xfocal;
  camera_position[1] = (image_position[1] - image_center.Y()) / yfocal;
  camera_position[2] = -1.0;

  // Create ray and transform into world coordinates
  R3Ray ray(R3zero_point, camera_position);
  ray.Transform(R3Affine(CameraToWorld(), 0));

  // Return ray through pixel in world coordinates
  return ray;
}



R3Vector R3SurfelImage::
TrajectoryDirection(void) const
{
  // Check scene
  if (!scene) return R3zero_vector;
  if (scene_index < 0) return R3zero_vector;
  
  // Find images before/after this image
  int delta = 7;
  int index = scene_index;
  int indexA = index - delta;
  int indexB = index + delta;
  if (indexA < 0) indexA = 0;
  if (indexB > scene->NImages()-1) indexB = scene->NImages()-1;
  R3SurfelImage *imageA = scene->Image(indexA);
  R3SurfelImage *imageB = scene->Image(indexB);

  // Compute trajector direction
  R3Point viewpointA = imageA->Viewpoint();
  R3Point viewpointB = imageB->Viewpoint();
  R3Vector trajectory_direction = viewpointB - viewpointA;
  trajectory_direction.Normalize();

  // Return trajectory direction
  return trajectory_direction;
}



////////////////////////////////////////////////////////////////////////
// PROPERTY MANIPULATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelImage::
SetChannel(int channel_index, const R2Grid& channel)
{
  // Check channel index
  assert(channel_index >= 0);

  // Insert channels up to channel index
  for (int i = channels.NEntries(); i <= channel_index; i++) {
    channels.Insert(NULL);
  }

  // Delete previous channel
  if (channels[channel_index]) delete channels[channel_index];

  // Update image width and height
  if (image_width <= 0) image_width = channel.XResolution();
  if (image_height <= 0) image_height = channel.YResolution();
  
  // Copy image into channel
  channels[channel_index] = new R2Grid(channel);

  // Resample if necessary
  if ((image_width != channel.XResolution()) || (image_height != channel.YResolution())) {
    channels[channel_index]->Resample(image_width, image_height, TRUE);
  }
}


  
void R3SurfelImage::
SetRedChannel(const R2Grid& channel)
{
  // Set red channel
  SetChannel(R3_SURFEL_RED_CHANNEL, channel);
}



void R3SurfelImage::
SetGreenChannel(const R2Grid& channel)
{
  // Set green channel
  SetChannel(R3_SURFEL_GREEN_CHANNEL, channel);
}



void R3SurfelImage::
SetBlueChannel(const R2Grid& channel)
{
  // Set blue channel
  SetChannel(R3_SURFEL_BLUE_CHANNEL, channel);
}



void R3SurfelImage::
SetDepthChannel(const R2Grid& channel)
{
  // Set depth channel
  SetChannel(R3_SURFEL_DEPTH_CHANNEL, channel);
}



void R3SurfelImage::
SetCategoryChannel(const R2Grid& channel)
{
  // Set category channel
  SetChannel(R3_SURFEL_CATEGORY_CHANNEL, channel);
}



void R3SurfelImage::
SetInstanceChannel(const R2Grid& channel)
{
  // Set instance channel
  SetChannel(R3_SURFEL_INSTANCE_CHANNEL, channel);
}



void R3SurfelImage::
SetColorChannels(const R2Image& color_image)
{
  // Initialize channels
  R2Grid red_channel(color_image.Width(), color_image.Height());
  R2Grid green_channel(color_image.Width(), color_image.Height());
  R2Grid blue_channel(color_image.Width(), color_image.Height());

  // Fill channels
  for (int iy = 0; iy < color_image.Height(); iy++) {
    for (int ix = 0; ix < color_image.Width(); ix++) {
      RNRgb rgb = color_image.PixelRGB(ix, iy);
      red_channel.SetGridValue(ix, iy, rgb.R());
      green_channel.SetGridValue(ix, iy, rgb.G());
      blue_channel.SetGridValue(ix, iy, rgb.B());
    }
  }

  // Set channels
  SetRedChannel(red_channel);
  SetGreenChannel(green_channel);
  SetBlueChannel(blue_channel);
}



void R3SurfelImage::
RemoveChannel(int channel_index)
{
  // Check channel index
  if (channel_index < 0) return;
  if (channel_index >= channels.NEntries()) return;
  if (!channels[channel_index]) return;

  // Remove channel
  delete channels[channel_index];
  channels[channel_index] = NULL;
}


  
void R3SurfelImage::
SetScan(R3SurfelScan *scan) 
{
  // Check if the same
  if (this->scan == scan) return;

  // Update old scan
  if (this->scan) {
    assert(this->scan->image == this);
    this->scan->image = NULL;
  }

  // Update new scan
  if (scan) {
    assert(scan->image == NULL);
    scan->image = this;
  }

  // Assign scan
  this->scan = scan;

  // Mark scene as dirty
  if (Scene()) Scene()->SetDirty();
}



void R3SurfelImage::
SetFocalLengths(RNLength focal_length) 
{
  // Set both focal lengths
  this->xfocal = focal_length;
  this->yfocal = focal_length;
}



void R3SurfelImage::
SetXFocal(RNLength focal_length) 
{
  // Set horizontal focal length
  this->xfocal = focal_length;
}



void R3SurfelImage::
SetYFocal(RNLength focal_length) 
{
  // Set vertical focal length
  this->yfocal = focal_length;
}



void R3SurfelImage::
SetImageDimensions(int width, int height) 
{
  // Set resolution
  this->image_width = width;
  this->image_height = height;
}



void R3SurfelImage::
SetImageCenter(const R2Point& center) 
{
  // Set image center
  this->image_center = center;
}



void R3SurfelImage::
SetPose(const R3CoordSystem& pose) 
{
  // Set pose
  this->pose = pose;

  // Update rollling shutter poses
  this->rolling_shutter_poses[0] = this->pose;
  this->rolling_shutter_poses[1] = this->pose;
  flags.Remove(R3_SURFEL_IMAGE_HAS_ROLLING_SHUTTER);
}



void R3SurfelImage::
SetViewpoint(const R3Point& viewpoint) 
{
  // Set viewpoint
  this->pose.SetOrigin(viewpoint);

  // Update rollling shutter poses
  this->rolling_shutter_poses[0] = this->pose;
  this->rolling_shutter_poses[1] = this->pose;
  flags.Remove(R3_SURFEL_IMAGE_HAS_ROLLING_SHUTTER);
}



void R3SurfelImage::
SetOrientation(const R3Vector& towards, const R3Vector& up) 
{
  // Set orientation
  this->pose.SetAxes(R3Triad(towards, up));

  // Update rollling shutter poses
  this->rolling_shutter_poses[0] = this->pose;
  this->rolling_shutter_poses[1] = this->pose;
  flags.Remove(R3_SURFEL_IMAGE_HAS_ROLLING_SHUTTER);
}



void R3SurfelImage::
SetTimestamp(RNScalar timestamp) 
{
  // Set timestamp
  this->timestamp = timestamp;

  // Update rollling shutter timestamps
  this->rolling_shutter_timestamps[0] = timestamp;
  this->rolling_shutter_timestamps[1] = timestamp;
  flags.Remove(R3_SURFEL_IMAGE_HAS_ROLLING_SHUTTER);
}



void R3SurfelImage::
SetDistortionType(int distortion_type) 
{
  // Set distortion type
  this->distortion_type = distortion_type;
}



void R3SurfelImage::
SetRadialDistortion(const RNScalar k[3]) 
{
  // Set radial distortion parameters
  this->radial_distortion[0] = k[0];
  this->radial_distortion[1] = k[1];
  this->radial_distortion[2] = k[2];
}



void R3SurfelImage::
SetTangentialDistortion(const RNScalar p[2]) 
{
  // Set tangential distortion parameters
  this->tangential_distortion[0] = p[0];
  this->tangential_distortion[1] = p[1];
}



void R3SurfelImage::
SetRollingShutterTimestamps(RNScalar timestamp0, RNScalar timestamp1) 
{
  // Check if timestamps are different
  if (timestamp0 == timestamp1) {
    SetTimestamp(timestamp0);
  }
  else {
    // Set rollling shutter timestamps
    this->rolling_shutter_timestamps[0] = timestamp0;
    this->rolling_shutter_timestamps[1] = timestamp1;

    // Update center timestamp
    this->timestamp = 0.5 * (timestamp0 + timestamp1);

    // Update flags
    flags.Add(R3_SURFEL_IMAGE_HAS_ROLLING_SHUTTER);
  }
}



void R3SurfelImage::
SetRollingShutterPoses(const R3CoordSystem& pose0, const R3CoordSystem& pose1) 
{
  // Check if poses are different
  if (pose0 == pose1) {
    SetPose(pose0);
  }
  else {    
    // Set rollling shutter poses
    this->rolling_shutter_poses[0] = pose0;
    this->rolling_shutter_poses[1] = pose1;

    // Update center pose
    this->pose = R3CoordSystemSlerp(pose0, pose1, 0.5);

    // Update flags
    flags.Add(R3_SURFEL_IMAGE_HAS_ROLLING_SHUTTER);
  }
}



void R3SurfelImage::
SetName(const char *name)
{
  // Delete previous name
  if (this->name) free(this->name);
  this->name = (name) ? RNStrdup(name) : NULL;

  // Mark scene as dirty
  if (Scene()) Scene()->SetDirty();
}



void R3SurfelImage::
SetFlags(RNFlags flags)
{
  // Set flags
  this->flags = flags;
}



void R3SurfelImage::
SetData(void *data) 
{
  // Set user data
  this->data = data;
}



////////////////////////////////////////////////////////////////////////
// TRANSFORMATION MANIPULATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelImage::
Transform(const R3Affine& transformation)
{
  // Transform pose
  pose.Transform(transformation);

  // Transform rolling shutter poses
  rolling_shutter_poses[0].Transform(transformation);
  rolling_shutter_poses[1].Transform(transformation);
}


  
////////////////////////////////////////////////////////////////////////
// COORDINATE TRANSFORMATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

R3Point R3SurfelImage::
TransformFromWorldToCamera(const R3Point& world_position) const
{
  // Initialize result
  R3Point camera_position = pose.InverseMatrix() * world_position;

  // Check if has rolling shutter
  if (HasRollingShutter()) {
    // Search for x coordinate (there is a different camera pose for every column)
    int column_index = ImageWidth() / 2;
    int last_column_index = column_index;
    for (int i = 0; i < ImageWidth()/2; i++) {
      // Transform world point into column's camera coordinate system
      R3CoordSystem cs = RollingShutterPoseAtImagePosition(R2Point(column_index, 0));
      R3Point camera_p = cs.InverseMatrix() * world_position;
      if (RNIsPositiveOrZero(camera_p.Z())) return camera_position;
      camera_position = camera_p;
      
      // Get/check column index
      R2Point distorted_position = TransformFromCameraToImage(camera_position);
      column_index = (int) (distorted_position.X() + 0.5);
      if (column_index < 0) column_index = 0;
      if (column_index >= ImageWidth()-1) column_index = ImageWidth()-1;
      if (column_index == last_column_index) break;
      last_column_index = column_index;
    }
  }

  // Return camera position
  return camera_position;
}

  

R2Point R3SurfelImage::
TransformFromWorldToImage(const R3Point& world_position) const
{
  // Transform into camera coordinates
  R3Point camera_position = TransformFromWorldToCamera(world_position);
  if (RNIsPositiveOrZero(camera_position.Z())) return R2unknown_point;

  // Transform into image coordinates
  return TransformFromCameraToImage(camera_position);
}



R2Point R3SurfelImage::
TransformFromCameraToImage(const R3Point& camera_position) const
{
  // Project 3D point in camera coordinates into pixels
  const R2Point c = ImageCenter();
  RNCoord x = c.X() + xfocal * camera_position.X() / -camera_position.Z();
  RNCoord y = c.Y() + yfocal * camera_position.Y() / -camera_position.Z();
  R2Point undistorted_image_position(x, y);
  
  // Distort image position
  return DistortImagePosition(undistorted_image_position);
}



R3Point R3SurfelImage::
TransformFromCameraToWorld(const R3Point& camera_position) const
{
  // Check if rolling shutter
  if (HasRollingShutter()) {
    // Get projected image position
    R2Point image_position = TransformFromCameraToImage(camera_position);

    // Get rolling shutter pose
    R3CoordSystem rs_pose = RollingShutterPoseAtImagePosition(image_position);

    // Transform 3D point from camera into world coordinate system
    return rs_pose.Matrix() * camera_position;
  }
  else {
    // Transform 3D point from camera into world coordinate system
    return CameraToWorld() * camera_position;
  }
}



R3Point R3SurfelImage::
TransformFromImageToWorld(const R2Point& image_position, RNLength depth) const
{
  // Transform from image to camera coordinates
  R3Point camera_position = TransformFromImageToCamera(image_position, depth);

  // Transform from camera to world coordinates
  if (HasRollingShutter()) {
    // Get rolling shutter pose
    R3CoordSystem rs_pose = RollingShutterPoseAtImagePosition(image_position);

    // Return position in world coordinates
    return rs_pose.Matrix() * camera_position;
  }
  else {
    // Return position in world coordinates
    return TransformFromCameraToWorld(camera_position);
  }
}



R3Point R3SurfelImage::
TransformFromImageToCamera(const R2Point& image_position, RNLength depth) const
{
  // Check camera parameters
  if ((xfocal <= 0) || (yfocal <= 0)) return R3unknown_point;

  // Undistort image position
  R2Point undistorted_image_position = UndistortImagePosition(image_position);
  if (undistorted_image_position == R2unknown_point) return R3unknown_point;

  // Get depth
  if (depth < 0) {
    if (channels.NEntries() <= R3_SURFEL_DEPTH_CHANNEL) return R3unknown_point;
    if (!channels[R3_SURFEL_DEPTH_CHANNEL]) return R3unknown_point;
    int ix = image_position.X() + 0.5;
    int iy = image_position.Y() + 0.5;
    depth = channels[R3_SURFEL_DEPTH_CHANNEL]->GridValue(ix, iy);
  }

  // Backproject from pixels into camera coordinate system
  const R2Point c = ImageCenter();
  RNCoord x = (undistorted_image_position.X() - c.X()) * depth / xfocal;
  RNCoord y = (undistorted_image_position.Y() - c.Y()) * depth / yfocal;
  return R3Point(x, y, -depth);
}



RNBoolean R3SurfelImage::
ContainsImagePosition(const R2Point& image_position) const
{
  // Check if within bounds
  if (image_position.X() < -0.49) return FALSE;
  if (image_position.Y() < -0.49) return FALSE;
  if (image_position.X() >= ImageWidth() - 0.51) return FALSE;
  if (image_position.Y() >= ImageHeight() - 0.51) return FALSE;
  return TRUE;
}

  

////////////////////////////////////////////////////////////////////////
// DISTORTION FUNCTIONS
////////////////////////////////////////////////////////////////////////

R2Point R3SurfelImage::
DistortImagePosition(const R2Point& undistorted_image_position) const
{
  // Check distortion parameters
  if (distortion_type == R3_SURFEL_NO_DISTORTION)
    return undistorted_image_position;

  // Check distortion parameters
  if ((radial_distortion[0] == 0) && (radial_distortion[1] == 0) && (radial_distortion[2] == 0) &&
      (tangential_distortion[0] == 0) && (tangential_distortion[1] == 0))
    return undistorted_image_position;

  // Check focal lengths
  if ((xfocal <= 0) || (yfocal <= 0))
    return undistorted_image_position;
  
  // Check distortion type
  if (distortion_type == R3_SURFEL_PERSPECTIVE_DISTORTION) {
    // Convert to normalized coordinates
    double ux = (undistorted_image_position.X() - XCenter()) / xfocal;
    double uy = (undistorted_image_position.Y() - YCenter()) / yfocal;

    // Get convenient variables
    const RNScalar *k = radial_distortion;
    const RNScalar *p = tangential_distortion;
    double x2 = ux * ux;
    double y2 = uy * uy;
    double xy = ux * uy;
    double r2 = x2 + y2;

    // Distort position
    double scale = 1 + k[0]*r2 + k[1]*r2*r2 + k[2]*r2*r2*r2;
    double dx = scale * ux + 2 * p[0] * xy + p[1] * (r2 + 2 * x2);
    double dy = scale * uy + p[0] * ( r2 + 2 * y2 ) + 2 * p[1] * xy;

    // Convert back to image coordinates
    double distorted_x = xfocal * dx + XCenter();
    double distorted_y = yfocal * dy + YCenter();

    // Return distorted position
    return R2Point(distorted_x, distorted_y);
  }
  else if (distortion_type == R3_SURFEL_FISHEYE_DISTORTION) {
    // Convert to normalized coordinates
    double ux = (undistorted_image_position.X() - XCenter()) / xfocal;
    double uy = (undistorted_image_position.Y() - YCenter()) / yfocal;

    // Get convenient variables
    const RNScalar *k = radial_distortion;
    double x2 = ux * ux;
    double y2 = uy * uy;
    double r2 = x2 + y2;
    double r = sqrt ( r2 );
    if (RNIsZero(r)) return undistorted_image_position;

    // Distort position
    double theta = atan( r );
    double theta2 = theta * theta;
    double theta_tilde = theta * ( 1 + k[0] * theta2 + k[1] * theta2*theta2 ) / r;
    double dx = theta_tilde * ux;
    double dy = theta_tilde * uy;

    // Convert back to image coordinates
    double distorted_x = xfocal * dx + XCenter();
    double distorted_y = yfocal * dy + YCenter();

    // Return distorted position
    return R2Point(distorted_x, distorted_y);
  }

  // Otherwise, apply no distortion
  return undistorted_image_position;
}



R2Point R3SurfelImage::
UndistortImagePosition(const R2Point& distorted_image_position) const
{
  // Check distortion type
  if (distortion_type == R3_SURFEL_NO_DISTORTION)
    return distorted_image_position;

  // Check distortion parameters
  if ((radial_distortion[0] == 0) && (radial_distortion[1] == 0) && (radial_distortion[2] == 0) &&
      (tangential_distortion[0] == 0) && (tangential_distortion[1] == 0))
    return distorted_image_position;

  // Check focal lengths
  if ((xfocal <= 0) || (yfocal <= 0))
    return distorted_image_position;
  
  // Not implemented yet
  RNWarning("R3SurfelImage::UndistortImagePosition not implemented");
  return distorted_image_position;
}



////////////////////////////////////////////////////////////////////////
// ROLLING SHUTTER POSES
////////////////////////////////////////////////////////////////////////

R3CoordSystem R3SurfelImage::
RollingShutterPoseAtTimestamp(RNScalar t) const
{
  // Check rolling shutter parameters
  if (!HasRollingShutter()) return pose;
  double dt = rolling_shutter_timestamps[1] - rolling_shutter_timestamps[0];
  if (RNIsZero(dt)) return rolling_shutter_poses[0];

  // Get interpolation parameter
  double u = (t - rolling_shutter_timestamps[0]) / dt;
  if (u <= 0) return rolling_shutter_poses[0];
  if (u >= 1) return rolling_shutter_poses[1];
  
  // Return slerped pose
  return R3CoordSystemSlerp(rolling_shutter_poses[0], rolling_shutter_poses[1], u);
}



R3CoordSystem R3SurfelImage::
RollingShutterPoseAtImagePosition(const R2Point& image_position) const
{
  // Check rolling shutter parameters
  if (!HasRollingShutter()) return pose;
  if (image_width <= 1) return rolling_shutter_poses[0];

  // Get interpolation parameter
  double u = (double) image_position.X() / (double) (image_width-1);
  if (u <= 0) return rolling_shutter_poses[0];
  if (u >= 1) return rolling_shutter_poses[1];

  // Return slerped pose
  return R3CoordSystemSlerp(rolling_shutter_poses[0], rolling_shutter_poses[1], u);
}



RNScalar R3SurfelImage::
RollingShutterTimestampAtImagePosition(const R2Point& image_position) const
{
  // Check rolling shutter parameters
  if (!HasRollingShutter()) return timestamp;
  if (image_width <= 1) return rolling_shutter_timestamps[0];

  // Get interpolation parameter
  double u = (double) image_position.X() / (double) (image_width-1);
  if (u <= 0) return rolling_shutter_timestamps[0];
  if (u >= 1) return rolling_shutter_timestamps[1];

  // Return interpolated timestamp
  return (1-u)*rolling_shutter_timestamps[0] + u*rolling_shutter_timestamps[1];
}



////////////////////////////////////////////////////////////////////////
// DISPLAY FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelImage::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print image
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "%d %s", SceneIndex(), (Name()) ? Name() : "-");
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");
}



void R3SurfelImage::
Draw(RNFlags flags) const
{
#if 1
  // Draw towards and up
  RNGrfxBegin(RN_GRFX_LINES);
  const R3Point& viewpoint = Viewpoint();
  const R3Vector towards = Towards();
  const R3Vector up = Up();
  R3LoadPoint(viewpoint);
  R3LoadPoint(viewpoint + towards);
  R3LoadPoint(viewpoint);
  R3LoadPoint(viewpoint + 0.5 * up);
  RNGrfxEnd();
#else
  // Draw camera
  RNScalar scale = 1.0;
  R3Point org = Viewpoint() + Towards() * scale;
  R3Vector dx = Right() * scale * tan(XFOV());
  R3Vector dy = Up() * scale * tan(YFOV());
  R3Point ur = org + dx + dy;
  R3Point lr = org + dx - dy;
  R3Point ul = org - dx + dy;
  R3Point ll = org - dx - dy;
  R3BeginLine();
  R3LoadPoint(ur);
  R3LoadPoint(ul);
  R3LoadPoint(ll);
  R3LoadPoint(lr);
  R3LoadPoint(ur);
  R3LoadPoint(Viewpoint());
  R3LoadPoint(lr);
  R3LoadPoint(ll);
  R3LoadPoint(Viewpoint());
  R3LoadPoint(ul);
  R3EndLine();
#endif
}



////////////////////////////////////////////////////////////////////////
// RENDERING FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3SurfelImage::
RenderImage(R2Image *color_image,
  R2Grid *depth_image, R2Grid *height_image,
  R2Grid *xnormal_image, R2Grid *ynormal_image, R2Grid *znormal_image,
  R2Grid *label_image, R2Grid *object_image,
  R2Grid *node_image, R2Grid *block_image) const
{
  // Only for scans now
  R3SurfelScan *scan = Scan();
  if (!scan) return 0;
  R3SurfelNode *node = scan->Node();
  if (!node) return 0;
  
  // Initialize images
  R2Grid tmp_image(ImageWidth(), ImageHeight());
  tmp_image.Clear(R2_GRID_UNKNOWN_VALUE);
  if (color_image) *color_image = R2Image(ImageWidth(), ImageHeight());
  if (depth_image) *depth_image = tmp_image;
  if (height_image) *height_image = tmp_image;
  if (xnormal_image) *xnormal_image = tmp_image;
  if (ynormal_image) *ynormal_image = tmp_image;
  if (znormal_image) *znormal_image = tmp_image;
  if (label_image) *label_image = tmp_image;
  if (object_image) *object_image = tmp_image;
  if (node_image) *node_image = tmp_image;
  if (block_image) *block_image = tmp_image;

  // Check stuff
  if (!scene || !scene->Tree()) return 0;

  // Visit leaf nodes
  RNArray<const R3SurfelNode *> stack;
  stack.Insert(node);
  while (!stack.IsEmpty()) {
    const R3SurfelNode *decendent = stack.Tail();
    stack.RemoveTail();
    if (decendent->NParts() > 0) {
      // Visit decendent nodes
      for (int i = 0; i < decendent->NParts(); i++) {
        R3SurfelNode *part = decendent->Part(i);
        stack.Insert(part);
      }
    }
    else {
      // Render all blocks of leaf node
      for (int i = 0; i < decendent->NBlocks(); i++) {
        R3SurfelBlock *block = decendent->Block(i);
        R3SurfelNode *node = (block) ? block->Node() : NULL;
        R3SurfelObject *object = (node) ? node->Object(TRUE) : NULL;
        R3SurfelLabel *label = (object) ? object->CurrentLabel() : NULL;

        // Read block
        scene->Tree()->Database()->ReadBlock(block);

        // Render block
        for (int j = 0; j < block->NSurfels(); j++) {
          const R3Surfel *surfel = block->Surfel(j);

          // Get image position
          R2Point image_position = TransformFromWorldToImage(block->SurfelPosition(j));
          if (!ContainsImagePosition(image_position)) continue;
          int ix = (int) (image_position.X() + 0.5);
          int iy = (int) (image_position.Y() + 0.5);

          // Fill pixel at image index
          if (color_image) color_image->SetPixelRGB(ix, iy, block->SurfelColor(j));
          if (depth_image) depth_image->SetGridValue(ix, iy, (block->SurfelPosition(j) - Viewpoint()).Dot(Towards()));
          if (height_image) height_image->SetGridValue(ix, iy, block->SurfelPosition(j).Z() - Viewpoint().Z());
          if (xnormal_image) xnormal_image->SetGridValue(ix, iy, surfel->NX());
          if (ynormal_image) ynormal_image->SetGridValue(ix, iy, surfel->NY());
          if (znormal_image) znormal_image->SetGridValue(ix, iy, surfel->NZ());
          if (block_image) block_image->SetGridValue(ix, iy, block->DatabaseIndex());
          if (node_image && node) node_image->SetGridValue(ix, iy, node->TreeIndex());
          if (object_image && object) object_image->SetGridValue(ix, iy, object->SceneIndex());
          if (label_image && label) label_image->SetGridValue(ix, iy, label->SceneIndex());
        }

        // Release block
        scene->Tree()->Database()->ReleaseBlock(block);
      }
    }
  }

  // Return success
  return 1;
}



} // namespace gaps
