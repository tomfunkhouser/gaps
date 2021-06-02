// Source file for GSV tapestry class



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

GSVTapestry::
GSVTapestry(void)
  : segment(NULL),
    segment_index(-1),
    camera(NULL),
    camera_index(-1),
    images(),
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
}



GSVTapestry::
~GSVTapestry(void)
{
  // Remove tapestry from segment
  if (segment) segment->RemoveTapestry(this);

  // Remove tapestry from camera
  if (camera) camera->RemoveTapestry(this);

  // Delete images
  while (NImages()) delete Image(0);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVTapestry::
Scene(void) const
{
  // Return scene
  if (!segment) return NULL;
  return segment->Scene();
}



int GSVTapestry::
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
    scene_index += r->NTapestries();
  }

  // Return scene index
  return scene_index;
}



GSVRun *GSVTapestry::
Run(void) const
{
  // Return run
  if (!segment) return NULL;
  return segment->Run();
}



int GSVTapestry::
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
    run_index += s->NTapestries();
  }

  // Return run index
  return run_index;
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVImage *GSVTapestry::
FindImageBeforeTimestamp(RNScalar timestamp, int imin, int imax) const
{
  // Binary search
  int i = (imin + imax) / 2;
  if (i == imin) return Image(imin);
  assert(i < imax);
  GSVImage *image = Image(i);
  RNScalar t = image->Timestamp();
  if (t > timestamp) return FindImageBeforeTimestamp(timestamp, imin, i);
  else if (t < timestamp) return FindImageBeforeTimestamp(timestamp, i, imax);
  else return image;
}



GSVImage *GSVTapestry::
FindImageBeforeTimestamp(RNScalar timestamp) const
{
  // Binary search
  if (NImages() == 0) return NULL;
  if (timestamp <= Image(0)->Timestamp()) return Image(0);
  if (timestamp >= Image(NImages()-1)->Timestamp()) return Image(NImages()-1);
  return FindImageBeforeTimestamp(timestamp, 0, NImages()-1);
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

GSVPose GSVTapestry::
Pose(RNScalar timestamp) const
{
  // Find images before and after timestamp
  GSVImage *image1 = FindImageBeforeTimestamp(timestamp);
  if (!image1) return GSVPose(R3zero_point, R3null_quaternion);
  int index1 = image1->TapestryIndex();
  if (index1 < 0) return Image(0)->Pose();
  if (index1 >= NImages()-1) return Image(NImages()-1)->Pose();
  int index2 = index1 + 1;
  GSVImage *image2 = Image(index2);

  // Compute interpolation parameter (0 <= t <= 1)
  RNScalar timestamp1 = image1->Timestamp();
  RNScalar timestamp2 = image2->Timestamp();
  RNScalar delta_timestamp = timestamp2 - timestamp1;
  if (delta_timestamp == 0) return image1->Pose();
  RNScalar t = (timestamp - timestamp1) / delta_timestamp;

  // Interpolate poses (would be better with quaternions)
  const GSVPose& pose1 = image1->Pose();
  const GSVPose& pose2 = image2->Pose();
  const R3Point& viewpoint1 = pose1.Viewpoint();
  const R3Point& viewpoint2 = pose2.Viewpoint();
  const R3Quaternion& orientation1 = pose1.Orientation();
  const R3Quaternion& orientation2 = pose2.Orientation();
  R3Point viewpoint = (1-t)*viewpoint1 + t*viewpoint2;
  R3Quaternion orientation = R3QuaternionSlerp(orientation1, orientation2, t);

  // Return interpolated pose
  return GSVPose(viewpoint, orientation);
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVTapestry::
InsertImage(GSVImage *image)
{
  // Just checking
  assert(image->tapestry_index == -1);
  assert(image->tapestry == NULL);

  // Insert camera image
  image->tapestry = this;
  image->tapestry_index = images.NEntries();
  images.Insert(image);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVTapestry::
RemoveImage(GSVImage *image)
{
  // Just checking
  assert(image->tapestry_index >= 0);
  assert(image->tapestry_index < images.NEntries());
  assert(image->tapestry == this);

  // Remove image
  RNArrayEntry *entry = images.KthEntry(image->tapestry_index);
  GSVImage *tail = images.Tail();
  tail->tapestry_index = image->tapestry_index;
  images.EntryContents(entry) = tail;
  images.RemoveTail();
  image->tapestry_index = -1;
  image->tapestry = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVTapestry::
Draw(RNFlags flags) const
{
  // Draw images
  for (int i = 0; i < NImages(); i++) {
    GSVImage *image = Image(i);
    image->Draw(flags);
  }
}



void GSVTapestry::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print tapestry header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Tapestry %d:", segment_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

   // Print images
  for (int i = 0; i < NImages(); i++) {
    GSVImage *image = Image(i);
    image->Print(fp, indented_prefix, suffix);
  }
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void GSVTapestry::
UpdateBBox(void) 
{
  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NImages(); i++) {
    bbox.Union(Image(i)->Pose().Viewpoint());
  }
}



void GSVTapestry::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (segment) segment->InvalidateBBox();
    if (camera) camera->InvalidateBBox();
  }
}



} // namespace gaps
