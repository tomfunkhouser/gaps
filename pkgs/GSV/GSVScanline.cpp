// Source file for GSV scanline class



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

GSVScanline::
GSVScanline(void)
  : scan(NULL),
    scan_index(-1),
    file_offset(0),
    read_count(0),
    points(NULL),
    npoints(0),
    pose(0,0,0,0,0,0,0),
    timestamp(-1),
    sweep_index(0),
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
}



GSVScanline::
GSVScanline(const GSVPose& pose, RNScalar timestamp)
  : scan(NULL),
    scan_index(-1),
    file_offset(0),
    read_count(0),
    points(NULL),
    npoints(0),
    pose(pose),
    timestamp(timestamp),
    sweep_index(-1),
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
}



GSVScanline::
~GSVScanline(void)
{
  // Delete points
  if (points) delete [] points;

  // Remove scanline from laser scan
  if (scan) scan->RemoveScanline(this);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVScanline::
Scene(void) const
{
  // Return scene
  if (!scan) return NULL;
  return scan->Scene();
}



GSVRun *GSVScanline::
Run(void) const
{
  // Return run
  if (!scan) return NULL;
  return scan->Run();
}



GSVLaser *GSVScanline::
Laser(void) const
{
  // Return laser
  if (!scan) return NULL;
  return scan->Laser();
}



GSVSurvey *GSVScanline::
Survey(void) const
{
  // Return survey
  if (!scan) return NULL;
  return scan->Survey();
}



GSVSegment *GSVScanline::
Segment(void) const
{
  // Return segment
  if (!scan) return NULL;
  return scan->Segment();
}



int GSVScanline::
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
    scene_index += r->NScanlines();
  }

  // Return scene index
  return scene_index;
}



int GSVScanline::
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
    run_index += s->NScanlines();
  }

  // Return runindex
  return run_index;
}



int GSVScanline::
LaserIndex(void) const
{
  // Get convenient variables
  GSVLaser *laser = Laser();
  if (!laser) return -1;
  GSVSurvey *survey = Survey();
  if (!survey) return -1;

  // Compute laser index
  int laser_index = SurveyIndex();
  for (int i = 0; i < survey->LaserIndex(); i++) {
    GSVSurvey *s = laser->Survey(i);
    laser_index += s->NScanlines();
  }

  // Return laser index
  return laser_index;
}



int GSVScanline::
SurveyIndex(void) const
{
  // Get convenient variables
  GSVScan *scan = Scan();
  if (!scan) return -1;
  GSVSurvey *survey = Survey();
  if (!survey) return -1;

  // Compute survey index
  int survey_index = ScanIndex();
  for (int i = 0; i < scan->SurveyIndex(); i++) {
    GSVScan *s = survey->Scan(i);
    survey_index += s->NScanlines();
  }

  // Return survey index
  return survey_index;
}



int GSVScanline::
SegmentIndex(void) const
{
  // Get convenient variables
  GSVSegment *segment = Segment();
  if (!segment) return -1;
  GSVSurvey *survey = Survey();
  if (!survey) return -1;

  // Compute segment index
  int segment_index = SurveyIndex();
  for (int i = 0; i < survey->SegmentIndex(); i++) {
    GSVSurvey *s = segment->Survey(i);
    segment_index += s->NScanlines();
  }

  // Return segment index
  return segment_index;
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

int GSVScanline::
MaxBeamIndex(void) const
{
  // Check first and last points
  if (npoints == 0) return -1;
  int i0 = points[0].BeamIndex();
  int i1 = points[npoints-1].BeamIndex();
  return (i0 > i1) ? i0 : i1;
}

  

RNCoord GSVScanline::
EstimatedGroundZ(void) const
{
  // Get convenient variables
  GSVLaser *laser = Laser();
  const double vlp16_laser_height = 2.0;
  const int sick_scan_ground_index = 25;
  const int sick_radius = 1;

  // Check device
  if ((laser->DeviceType() == GSV_SICK_LASER) &&
      (NPoints() > 0) &&
      (NPoints() > sick_scan_ground_index+sick_radius) &&
      points) {

    // Get three points around scan_ground_index
    R3Point p0 = PointPosition(NPoints() - sick_scan_ground_index - sick_radius);
    R3Point p1 = PointPosition(NPoints() - sick_scan_ground_index);
    R3Point p2 = PointPosition(NPoints() - sick_scan_ground_index + sick_radius);

    // Find median Z coordinate
    RNCoord z;
    if (p0.Z() < p1.Z()) {
      if (p1.Z() < p2.Z()) {
        z = p1.Z();
      }
      else {
        if (p0.Z() < p2.Z()) z = p2.Z();
        else z = p0.Z();
      }
    }
    else {
      if (p0.Z() < p2.Z()) {
        z = p0.Z();
      }
      else {
        if (p1.Z() < p2.Z()) z = p2.Z();
        else z = p1.Z();
      }
    }
  
    // Return estimate of ground Z coordinate
    return z;
  }
  else {
    const GSVPose& pose = Pose();
    const R3Point& viewpoint = pose.Viewpoint();
    return viewpoint.Z() - vlp16_laser_height;
  }

  // Should not get here
  return RN_UNKNOWN;
}



RNAngle GSVScanline::
PointAngle(int point_index) const
{
  // Return angle of laser when point was sampled (0 - PI)
  const R3Point& position = PointPosition(point_index);
  const R3Point& viewpoint = pose.Viewpoint();
  const R3Vector& towards = pose.Towards();
  const R3Vector& up = pose.Up();
  R3Vector right = towards % up;
  R3Vector v = position - viewpoint;
  RNLength distance = v.Length();
  if (RNIsZero(distance)) return 0;
  v /= distance;
  RNScalar dot = -(v.Dot(up));
  RNAngle angle = acos(dot);
  if (v.Dot(right) > 0) angle = RN_PI_OVER_TWO + angle;
  else angle = RN_PI_OVER_TWO - angle;
  return angle;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVScanline::
SetPoints(const GSVPoint *points, int npoints) 
{
  // Delete previous points
  if (this->points) delete [] this->points;

  // Reset everything
  this->npoints = npoints;
  this->points = NULL;
  this->bbox = R3null_box;

  // Copy points
  if (points && (npoints > 0)) {
    // Allocate points 
    this->points = new GSVPoint [ npoints ];
    if (!this->points) {
      RNAbort("Unable to allocate points for scanline\n");
    }
    
    // Copy points
    for (int i = 0; i < npoints; i++) {
      this->points[i] = points[i];
    }

    // Update bounding box
    for (int i = 0; i < npoints; i++) {
      bbox.Union(this->points[i].Position());
    }
  }

  // Set read count
  read_count = 1;
}



void GSVScanline::
SetPose(const GSVPose& pose)
{
  // Update points
  if ((NPoints() > 0) && (points) && (!(this->pose.Orientation().IsZero()))) {
    // Determine transformation of points
    R4Matrix matrix = R4identity_matrix;
    matrix.Translate(pose.Viewpoint().Vector());
    matrix.Multiply(pose.Orientation().Matrix());
    matrix.Multiply(this->pose.Orientation().Inverse().Matrix());
    matrix.Translate(-this->pose.Viewpoint().Vector());
    R3Affine transformation(matrix, 0);
    
    // Transform points
    for (int i = 0; i < npoints; i++) {
      points[i].Transform(transformation);
    }
  }

  // Set pose
  this->pose = pose;

  // Invalidate bounding box
  InvalidateBBox();
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

static void
LoadColor(int k)
{
#if 1
  // Make array of colors
  static const int ncolors = 72;
  static const RNRgb colors[ncolors] = {
    RNRgb(0.5, 0.2, 0.2), RNRgb(0, 1, 0), RNRgb(0, 0, 1), 
    RNRgb(0.3, 0.6, 0), RNRgb(0, 1, 1), RNRgb(1, 0, 1), 
    RNRgb(1, 0.5, 0), RNRgb(0, 1, 0.5), RNRgb(0.5, 0, 1), 
    RNRgb(0.5, 1, 0), RNRgb(0, 0.5, 1), RNRgb(1, 0, 0.5), 
    RNRgb(0.5, 0, 0), RNRgb(0, 0.5, 0), RNRgb(0, 0, 0.5), 
    RNRgb(0.5, 0.5, 0), RNRgb(0, 0.5, 0.5), RNRgb(0.5, 0, 0.5),
    RNRgb(0.7, 0, 0), RNRgb(0, 0.7, 0), RNRgb(0, 0, 0.7), 
    RNRgb(0.7, 0.7, 0), RNRgb(0, 0.7, 0.7), RNRgb(0.7, 0, 0.7), 
    RNRgb(0.7, 0.3, 0), RNRgb(0, 0.7, 0.3), RNRgb(0.3, 0, 0.7), 
    RNRgb(0.3, 0.7, 0), RNRgb(0, 0.3, 0.7), RNRgb(0.7, 0, 0.3), 
    RNRgb(0.3, 0, 0), RNRgb(0, 0.3, 0), RNRgb(0, 0, 0.3), 
    RNRgb(0.3, 0.3, 0), RNRgb(0, 0.3, 0.3), RNRgb(0.3, 0, 0.3),
    RNRgb(1, 0.3, 0.3), RNRgb(0.3, 1, 0.3), RNRgb(0.3, 0.3, 1), 
    RNRgb(1, 0.0, 0.3), RNRgb(0.3, 1, 1), RNRgb(1, 0.3, 1), 
    RNRgb(1, 0.2, 0.7), RNRgb(0.3, 1, 0.5), RNRgb(0.5, 0.3, 1), 
    RNRgb(0.5, 1, 0.3), RNRgb(0.3, 0.5, 1), RNRgb(1, 0.3, 0.5), 
    RNRgb(0.5, 0.3, 0.3), RNRgb(0.3, 0.5, 0.3), RNRgb(0.3, 0.3, 0.5), 
    RNRgb(0.5, 0.5, 0.3), RNRgb(0.3, 0.5, 0.5), RNRgb(0.5, 0.3, 0.5),
    RNRgb(0.3, 0.5, 0.5), RNRgb(0.5, 0.3, 0.5), RNRgb(0.5, 0.5, 0.3), 
    RNRgb(0.3, 0.3, 0.5), RNRgb(0.5, 0.3, 0.3), RNRgb(0.3, 0.5, 0.3), 
    RNRgb(0.3, 0.8, 0.5), RNRgb(0.5, 0.3, 0.8), RNRgb(0.8, 0.5, 0.3), 
    RNRgb(0.8, 0.3, 0.5), RNRgb(0.5, 0.8, 0.3), RNRgb(0.3, 0.5, 0.8), 
    RNRgb(0.8, 0.5, 0.5), RNRgb(0.5, 0.8, 0.5), RNRgb(0.5, 0.5, 0.8), 
    RNRgb(0.8, 0.8, 0.5), RNRgb(0.5, 0.8, 0.8), RNRgb(0.8, 0.5, 0.8)
  };

  // Return color based on k
  if (k <= 0) RNLoadRgb(colors[0]);
  else RNLoadRgb(colors[1 + (k % (ncolors-1))]);
#else
  // Load identifier color
  if (k < 0) {
    RNLoadRgb(0.5, 0.5, 0.5);
  }
  else {
    unsigned char r = (224 + 67*k) % 256;
    unsigned char g = (32  + 47*k) % 256;
    unsigned char b = (32  + 29*k) % 256;
    RNLoadRgb(r, g, b);
  }
#endif
}


static void
LoadColor(double value)
{
  // Load heatmap color
  GLdouble r, g, b;
  if (value <= 0) {
    // Red
    r = 1;
    g = 0;
    b = 0;
  }
  if (value < 0.25) {
    // Red to yellow
    value *= 4;
    r = 1;
    g = value;
    b = 0;
  }
  else if (value < 0.5) {
    // Yellow to green
    value = (value - 0.25) * 4;
    r = 1 - value;
    g = 1;
    b = 0;
  }
  else if (value < 0.75) {
    // Green to cyan
    value = (value - 0.5) * 4;
    r = 0;
    g = 1;
    b = value;
  }
  else if (value < 1) {
    // Cyan to blue
    value = (value - 0.75) * 4;
    r = 0;
    g = 1 - value;
    b = 1;
  }
  else {
    // Blue
    r = 0;
    g = 0;
    b = 1;
  }
  RNLoadRgb(r, g, b);
}



void GSVScanline::
Draw(RNFlags flags) const
{
  // Just checking
  if (NPoints() == 0) return;
  assert(points);

  // Draw scanline
  if (flags & GSV_DRAW_SCANLINES_WITH_POINT_INDEX_COLOR) {
    const int ncolors = 3;
    GLfloat color[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
    RNLength max_edge_length_squared = 1;
    RNGrfxBegin(RN_GRFX_LINES);
    R3Point prev_point = PointPosition(0);
    for (int ik = 1; ik < NPoints(); ik++) {
      const R3Point& current_point = PointPosition(ik);
      RNScalar dx = prev_point[0] - current_point[0];
      RNScalar dy = prev_point[1] - current_point[1];
      RNScalar dd = dx*dx + dy*dy;
      if (dd < max_edge_length_squared) {
        RNLoadRgb(color[(int) (0.1 * (NPoints()-ik)) % ncolors]);
        R3LoadPoint(prev_point);
        R3LoadPoint(current_point);
      }
      prev_point = current_point;
    }
    RNGrfxEnd();
  }
  else {
    // Draw points
    RNGrfxBegin(RN_GRFX_POINTS);
    LoadPoints(flags);
    RNGrfxEnd();
  }
}



void GSVScanline::
LoadPoints(RNFlags flags) const
{
  // Just checking
  if (NPoints() == 0) return;
  assert(points);

  // Draw points with different color schemes
  if (flags & GSV_DRAW_POINTS_WITH_VIEWPOINT_DISTANCE_COLOR) {
    // Get viewpoint
    R3Point v = Pose().Viewpoint();

    // Load points
    for (int i = 0; i < NPoints(); i++) {
      const R3Point& p = PointPosition(i);
      double dx = v[0] - p[0];
      double dy = v[1] - p[1];
      double dz = v[2] - p[2];
      double d2 = dx*dx + dy*dy + dz*dz;
      // double value = 0.0025 * d2;
      double value = 0.001 * d2;
      LoadColor(value);
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_HEIGHT_COLOR) {
    // Get height
    RNCoord ground_z = EstimatedGroundZ();

    // Load points
    for (int i = 0; i < NPoints(); i++) {
      const R3Point& p = PointPosition(i);
      double dz = p[2] - ground_z;
      double value = 0.25 * dz;
      LoadColor(value);
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_ELEVATION_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      LoadColor(0.5 * sqrt(PointElevation(i)));
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_REFLECTIVITY_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      LoadColor(sqrt(PointReflectivity(i)/255.0));
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_GROUND_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      if (PointFlags(i) & GSV_POINT_ON_GROUND_FLAG) RNLoadRgb(1, 0, 0);
      else RNLoadRgb(0, 0, 1);
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_RGB_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      RNLoadRgb(PointColor(i));
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_NORMAL_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      R3Vector vector = 0.5 * (PointNormal(i) + R3ones_vector);
      RNLoadRgb(vector.Coords());
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_POINT_IDENTIFIER_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      LoadColor(PointIdentifier(i));
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_CLUSTER_IDENTIFIER_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      LoadColor(PointClusterIdentifier(i));
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_CATEGORY_IDENTIFIER_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      LoadColor(PointCategoryIdentifier(i));
      R3LoadPoint(PointPosition(i));
    }
  }
  else if (flags & GSV_DRAW_POINTS_WITH_CATEGORY_CONFIDENCE_COLOR) {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      LoadColor(PointCategoryConfidence(i));
      R3LoadPoint(PointPosition(i));
    }
  }
  else {
    // Load points
    for (int i = 0; i < NPoints(); i++) {
      R3LoadPoint(PointPosition(i));
    }
  }
}



void GSVScanline::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print scanline header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Scanline %d:", scan_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

  // Print points
  // ???
}



////////////////////////////////////////////////////////////////////////
// Point query  functions
////////////////////////////////////////////////////////////////////////

int GSVScanline::
FindPointIndexWithClosestBeamIndex(int query_beam_index) const
{
  // Initialize result
  int closest_delta = INT_MAX;
  int closest_point_index = -1;

  // Search for closest point (by beam index)
  for (int i = 0; i < npoints; i++) {
    const GSVPoint *point = &points[i];
    int delta = abs(point->BeamIndex() - query_beam_index);
    if (delta < closest_delta) {
      closest_delta = delta;
      closest_point_index = i;
      if (closest_delta == 0) break;
    }
  }

  // Return closest point index
  return closest_point_index;
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

RNBoolean GSVScanline::
DoesBBoxNeedUpdate(void) const
{
  // Return whether bounding box needs update
  if (bbox.XMin() == FLT_MAX) return TRUE;
  else return FALSE;
}



void GSVScanline::
UpdateBBox(void) 
{
  // Just checking
  assert(read_count > 0);
  assert(points);

  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NPoints(); i++) {
    bbox.Union(PointPosition(i));
  }
}



void GSVScanline::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (scan) scan->InvalidateBBox();
  }
}



////////////////////////////////////////////////////////////////////////
//Input/Output functions
////////////////////////////////////////////////////////////////////////

int GSVScanline::
ReadPoints(FILE *fp, RNBoolean seek) 
{
  // Check read counter
  if (++read_count > 1) return 1;
  assert(!points);

  // Seek within file
  if (seek && (file_offset > 0)) {
    if (!RNFileSeek(fp, file_offset, RN_FILE_SEEK_SET)) {
      RNFail("Unable to seek to scanline points\n");
      return 0;
    }
  }
  else {
    // Update file offset
    file_offset = RNFileTell(fp);
  }

  // Read number of points 
  if (fread(&npoints, sizeof(int), 1, fp) != (unsigned int) 1) {
    RNFail("Unable to read number of scanline points\n");
    return 0;
  }

  // Check number of points
  if (npoints > 0) {
    // Allocate points 
    points = new GSVPoint [ npoints ];
    if (!points) {
      RNFail("Unable to allocate scanline points\n");
      return 0;
    }
    
    // Read points
    if (fread(points, sizeof(GSVPoint), npoints, fp) != (unsigned int) npoints) {
      RNFail("Unable to read scanline points\n");
      return 0;
    }
    
    // Update bounding box
    bbox = R3null_box;
    for (int i = 0; i < npoints; i++) {
      bbox.Union(points[i].Position());
    }
  }
  else {
    // No points
    points = NULL;
    bbox = R3null_box;
  }

  // Return success
  return 1;
}



int GSVScanline::
WritePoints(FILE *fp, RNBoolean seek) 
{
  // Seek within file
  if (seek && (file_offset > 0)) {
    // Seek to file offset
    if (!RNFileSeek(fp, file_offset, RN_FILE_SEEK_SET)) {
      RNFail("Unable to seek to scanline points\n");
      return 0;
    }
  }
  else {
    // Update file offset
    file_offset = RNFileTell(fp);
  }

  // Write number of points 
  if (fwrite(&npoints, sizeof(int), 1, fp) != (unsigned int) 1) {
    RNFail("Unable to write number of scanline points\n");
    return 0;
  }

  // Write points
  if (npoints > 0) {
    if (fwrite(points, sizeof(GSVPoint), npoints, fp) != (unsigned int) npoints) {
      RNFail("Unable to write scanline points\n");
      return 0;
    }
  }

  // Return success
  return 1;
}



int GSVScanline::
ReleasePoints(void) 
{
  // Check read counter
  if (--read_count > 0) return 1;
  assert(points);

  // Release points
  delete [] points;
  points = NULL;

  // Return success
  return 1;
}



} // namespace gaps
