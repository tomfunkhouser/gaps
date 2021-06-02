// Source file for GSV image class



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

GSVImage::
GSVImage(const char *name)
  : tapestry(NULL),
    tapestry_index(-1),
    panorama(NULL),
    panorama_index(-1),
    pose0(0,0,0,0,0,0,0),
    pose1(0,0,0,0,0,0,0),
    timestamp0(-1),
    timestamp1(-1),
    name((name) ? RNStrdup(name) : NULL),
    data(NULL)
{
}



GSVImage::
~GSVImage(void)
{
  // Remove image from tapestry
  if (tapestry) tapestry->RemoveImage(this);

  // Remove image from panorama
  if (panorama) panorama->RemoveImage(this);

  // Delete name
  if (name) free(name);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVImage::
Scene(void) const
{
  // Return scene
  if (!tapestry) return NULL;
  return tapestry->Scene();
}



GSVRun *GSVImage::
Run(void) const
{
  // Return run
  if (!tapestry) return NULL;
  return tapestry->Run();
}



GSVCamera *GSVImage::
Camera(void) const
{
  // Return camera associated with image
  if (!tapestry) return NULL;
  return tapestry->Camera();
}



GSVSegment *GSVImage::
Segment(void) const
{
  // Return segment
  if (!tapestry) return NULL;
  return tapestry->Segment();
}



int GSVImage::
SceneIndex(void) const
{
  // Get convenient variables
  GSVRun *run = Run();
  if (!run) return -1;
  GSVScene *scene = run->Scene();
  if (!scene) return -1;

  // Compute scene index
  int scene_index = RunIndex();
  for (int ir = 0; ir < run->SceneIndex(); ir++) {
    GSVRun *r = scene->Run(ir);
    scene_index += r->NImages();
  }

  // Return scene index
  return scene_index;
}



int GSVImage::
RunIndex(void) const
{
  // Get convenient variables
  GSVSegment *segment = Segment();
  if (!segment) return -1;
  GSVRun *run = segment->Run();
  if (!run) return -1;

  // Compute run index
  int run_index = SegmentIndex();
  for (int i = 0; i < segment->RunIndex(); i++) {
    GSVSegment *s = run->Segment(i);
    run_index += s->NImages();
  }

  // Return runindex
  return run_index;
}



int GSVImage::
SegmentIndex(void) const
{
  // Get convenient variables
  GSVSegment *segment = Segment();
  if (!segment) return -1;

  // Compute segment index
  int segment_index = TapestryIndex();
  for (int i = 0; i < tapestry->SegmentIndex(); i++) {
    GSVTapestry *t = segment->Tapestry(i);
    segment_index += t->NImages();
  }

  // Return segment index
  return segment_index;
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

RNAngle GSVImage::
XFov(void) const
{
  // Return field of view in horizontal direction
  GSVCamera *camera = Camera();
  if (!camera) return 0;
  return camera->XFov();
}



RNAngle GSVImage::
YFov(void) const
{
  // Return field of view in vertical direction
  GSVCamera *camera = Camera();
  if (!camera) return 0;
  return camera->YFov();
}



int GSVImage::
Width(void) const
{
  // Return width of images taken with this image
  GSVCamera *camera = Camera();
  if (!camera) return 0;
  return camera->ImageWidth();
}



int GSVImage::
Height(void) const
{
  // Return height of images taken with this image
  GSVCamera *camera = Camera();
  if (!camera) return 0;
  return camera->ImageHeight();
}



GSVPose GSVImage::
Pose(int column_index) const
{
  // Return pose estimate for given column
  int ncolumns = Width();
  if (ncolumns <= 0) return GSVPose(0,0,0,0,0,0,0);
  if (column_index < 0) column_index = ncolumns/2;
  RNScalar t = (RNScalar) column_index / (RNScalar) (ncolumns-1);
  R3Point viewpoint = (1-t)*pose0.Viewpoint() + t*pose1.Viewpoint();
  R3Quaternion orientation = R3QuaternionSlerp(pose0.Orientation(), pose1.Orientation(), t);
  return GSVPose(viewpoint, orientation);
}



RNScalar GSVImage::
Timestamp(int column_index) const
{
  // Return timestamp estimate for given column
  int ncolumns = Width();
  if (ncolumns <= 0) return -1;
  if (column_index < 0) column_index = ncolumns/2;
  RNScalar t = (RNScalar) column_index / (RNScalar) (ncolumns-1);
  return (1-t)*timestamp0 + t*timestamp1;
}



R2Image *GSVImage::
DistortedImage(void) const
{
  // Get convenient variables
  int image_index = PanoramaIndex();
  GSVPanorama *panorama = Panorama();
  if (!panorama) return NULL;
  int panorama_index = panorama->RunIndex();
  GSVSegment *segment = panorama->Segment();
  if (!segment) return NULL;
  int segment_index = segment->RunIndex();
  GSVRun *run = segment->Run();
  if (!run) return NULL;
  GSVScene *scene = run->Scene();
  if (!scene) return NULL;
  char run_name[1024];
  if (run->Name()) sprintf(run_name, "%s", run->Name());
  else sprintf(run_name, "Run%d", run->SceneIndex());
  const char *raw_data_directory = scene->RawDataDirectoryName();
  if (!raw_data_directory) return NULL;
  
  // Construct distorted image name
  char distorted_image_name[4096];
  sprintf(distorted_image_name, "%s/%s/segment_%02d/unstitched_%06d_%02d.jpg", 
    raw_data_directory, run_name, segment_index, panorama_index, image_index);
    
  // Allocate distorted image
  R2Image *distorted_image = new R2Image();
  if (!distorted_image) {
    RNFail("Unable to allocate image for %s\n", distorted_image_name);
    return NULL;
  }

  // Read distorted image
  if (!distorted_image->Read(distorted_image_name)) {
    RNFail("Unable to read distorted image %s\n", distorted_image_name);
    delete distorted_image;
    return NULL;
  }

  // Return distorted image
  return distorted_image;
}




R2Image *GSVImage::
UndistortedImage(void) const
{
  // Get convenient variables
  int image_index = PanoramaIndex();
  GSVPanorama *panorama = Panorama();
  if (!panorama) return NULL;
  int panorama_index = panorama->RunIndex();
  GSVSegment *segment = panorama->Segment();
  if (!segment) return NULL;
  int segment_index = segment->RunIndex();
  GSVRun *run = segment->Run();
  if (!run) return NULL;
  char run_name[1024];
  if (run->Name()) sprintf(run_name, "%s", run->Name());
  else sprintf(run_name, "Run%d", run->SceneIndex());
  GSVScene *scene = run->Scene();
  if (!scene) return NULL;
  GSVCamera *camera = Camera();
  if (!camera) return NULL;
  R2Image *undistorted_image = NULL;
  
  // Construct undistorted image name
  if (scene->CacheDataDirectoryName()) {
    // Get cached undistorted image name
    char undistorted_image_name[4096];
    sprintf(undistorted_image_name, "%s/undistorted_images/%s/%02d_%06d_%02d_UndistortedImage.jpg", 
      scene->CacheDataDirectoryName(), run_name, segment_index, panorama_index, image_index);

    // Check if undistorted image file exists
    if (RNFileExists(undistorted_image_name)) { 
      // Allocate undistorted image
      undistorted_image = new R2Image();
      if (!undistorted_image) {
        RNFail("Unable to allocate image for %s\n", undistorted_image_name);
        return NULL;
      }

      // Read undistorted image
      if (!undistorted_image->Read(undistorted_image_name)) {
        RNFail("Unable to read undistorted image %s\n", undistorted_image_name);
        delete undistorted_image;
        return NULL;
      }
    }
  }

  // If did not read from cache ...
  if (!undistorted_image) {
    // Get distorted image
    R2Image *distorted_image = DistortedImage();
    if (!distorted_image) return NULL;

    // Allocate undistorted image with same resolution
    undistorted_image = new R2Image(distorted_image->Width(), distorted_image->Height());
    if (!undistorted_image) {
      RNFail("Unable to allocate undistorted image\n");
      return NULL;
    }

    // Undistort image
    if (!camera->UndistortImage(*distorted_image, *undistorted_image)) {
      RNFail("Unable to undistort image\n");
      delete undistorted_image;
      return NULL;
    }

    // Write undistorted image
    if (scene->CacheDataDirectoryName()) {
      if (scene->CreateCacheDataDirectory("undistorted_images", run)) {
        char undistorted_image_name[4096];
        sprintf(undistorted_image_name, "%s/undistorted_images/%s/%02d_%06d_%02d_UndistortedImage.jpg", 
          scene->CacheDataDirectoryName(), run_name, segment_index, panorama_index, image_index);
        if (!undistorted_image->Write(undistorted_image_name)) {
          RNFail("Unable to write undistorted image for %s\n", undistorted_image_name);
        }
      }
    }

    // Delete distorted image
    delete distorted_image;
  }

  // Return undistorted image
  return undistorted_image;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVImage::
SetName(const char *name)
{
  // Delete previous name
  if (this->name) free(this->name);

  // Set new name
  if (name) this->name = strdup(name);
  else this->name = NULL;
}



void GSVImage::
SetPose(const GSVPose& pose0, const GSVPose& pose1)
{
  // Set pose parameters
  this->pose0 = pose0;
  this->pose1 = pose1;

  // Invalidate bounding boxes
  if (tapestry) tapestry->InvalidateBBox();
}



void GSVImage::
SetTimestamp(RNScalar timestamp0, RNScalar timestamp1)
{
  // Set timestamp
  this->timestamp0 = timestamp0;
  this->timestamp1 = timestamp1;
}



////////////////////////////////////////////////////////////////////////
// Mapping functions
////////////////////////////////////////////////////////////////////////

R2Point GSVImage::
DistortedPosition(const R2Point& undistorted_image_position) const
{
  // Return the position after distortion by the camera lens
  GSVCamera *camera = Camera();
  if (camera) return camera->DistortedPosition(undistorted_image_position);
  else return undistorted_image_position;
}



R2Point GSVImage::
UndistortedPosition(const R2Point& distorted_image_position) const
{
  // Return the position after the inverse of distortion by the camera lens
  GSVCamera *camera = Camera();
  if (camera) return camera->UndistortedPosition(distorted_image_position);
  else return distorted_image_position;
}



R2Point GSVImage::
UndistortedPosition(const R3Point& world_position, int column_index) const
{
  // Get camera info
  GSVCamera *camera = Camera();
  if (!camera) return 0;
  RNCoord xcenter = camera->XCenter();
  RNCoord ycenter = camera->YCenter();
  RNAngle xfocal = camera->XFocal();
  RNAngle yfocal = camera->YFocal();

  // Transform world point into column's camera coordinate system
  const GSVPose& column_pose = Pose(column_index);
  R3CoordSystem cs(column_pose.Viewpoint(), R3Triad(column_pose.Towards(), column_pose.Up()));
  R3Point camera_position = cs.InverseMatrix() * world_position;
  if (RNIsPositiveOrZero(camera_position.Z())) return R2Point(-1,-1);

  // Compute coordinates of projection
  RNCoord x = xcenter + camera_position.X() * xfocal / -(camera_position.Z());
  RNCoord y = ycenter + camera_position.Y() * yfocal / -(camera_position.Z());
  
  // Fill in point projected onto image
  return R2Point(x, y);
}



R2Point GSVImage::
UndistortedPosition(const R3Point& world_position) const
{
  // Get camera info
  GSVCamera *camera = Camera();
  if (!camera) return 0;
  RNCoord xcenter = camera->XCenter();
  RNCoord ycenter = camera->YCenter();
  RNAngle xfocal = camera->XFocal();
  RNAngle yfocal = camera->YFocal();
  RNCoord x = RN_UNKNOWN;
  RNCoord y = RN_UNKNOWN;
  R3Point camera_position;

  // Search for x coordinate (there is a different camera pose for every column)
  int column_index = Width() / 2;
  int last_column_index = column_index;
  for (int i = 0; i < Width()/2; i++) {
    // Transform world point into column's camera coordinate system
    const GSVPose& column_pose = Pose(column_index);
    R3CoordSystem cs(column_pose.Viewpoint(), R3Triad(column_pose.Towards(), column_pose.Up()));
    camera_position = cs.InverseMatrix() * world_position;
    if (RNIsPositiveOrZero(camera_position.Z())) return R2Point(-1,-1);

    // Compute coordinates of projection
    x = xcenter + camera_position.X() * xfocal / -(camera_position.Z());
    y = ycenter + camera_position.Y() * yfocal / -(camera_position.Z());

    // Get/check column index
    R2Point distorted_position = DistortedPosition(R2Point(x,y));
    column_index = (int) (distorted_position.X() + 0.5);
    if (column_index < 0) column_index = 0;
    if (column_index >= Width()-1) column_index = Width()-1;
    if (column_index == last_column_index) break;
    last_column_index = column_index;
  }

  // Fill in point projected onto image
  return R2Point(x, y);
}



R2Point GSVImage::
DistortedPosition(const R3Point& world_position) const
{
  // Return position of 3D point in distorted image
  R2Point undistorted_position = UndistortedPosition(world_position);
  if (undistorted_position == R2Point(-1,-1)) return undistorted_position;
  return DistortedPosition(undistorted_position);
}



R3Ray GSVImage::
RayThroughUndistortedPosition(const R2Point& undistorted_image_position) const
{
  // Check camera
  if (!Camera()) return R3null_ray;

  // Compute distorted image position
  R2Point distorted_image_position = DistortedPosition(undistorted_image_position);
  if (distorted_image_position == R2Point(-1,-1)) return R3null_ray;
  
  // Get column index
  int column_index = (int) (distorted_image_position.X() + 0.5);
  if (column_index < 0) column_index = 0;
  if (column_index >= Width()) column_index = Width()-1;
      
  // Get camera pose
  GSVPose pose = Pose(column_index);
  const R3Point& viewpoint = pose.Viewpoint();
  const R3Vector& towards = pose.Towards();
  const R3Vector& up = pose.Up();
  R3Vector right = towards % up;

  // Compute point on ray
  RNScalar dx = (RNScalar) (2 * (undistorted_image_position.X() - 0.5 * Width())) / (RNScalar) Width();
  RNScalar dy = (RNScalar) (2 * (undistorted_image_position.Y() - 0.5 * Height())) / (RNScalar) Height();
  R3Point world_point = viewpoint + towards;
  world_point += dx * right * tan(0.5 * Camera()->XFov());
  world_point += dy * up * tan(0.5 * Camera()->YFov());

  // Return ray through undistorted image point
  return R3Ray(viewpoint, world_point);
}



R3Ray GSVImage::
RayThroughDistortedPosition(const R2Point& distorted_image_position) const
{
  // Return ray through distorted image point
  R2Point undistorted_position = UndistortedPosition(distorted_image_position);
  if (undistorted_position == R2Point(-1,-1)) return R3null_ray;
  return RayThroughUndistortedPosition(undistorted_position);
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVImage::
Draw(RNFlags flags) const
{
  // Draw image
  // ???
}



void GSVImage::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print image header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Image %d:", panorama_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

  // Print image stuff
  // ???
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void GSVImage::
Update(void) 
{
}



} // namespace gaps
