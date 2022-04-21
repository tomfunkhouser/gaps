// Source file for GSV scan class


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

GSVScan::
GSVScan(void)
  : survey(NULL),
    survey_index(-1),
    read_count(0),
    scanlines(),
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
}



GSVScan::
~GSVScan(void)
{
  // Remove scan from survey
  if (survey) survey->RemoveScan(this);

  // Delete scanlines
  while (NScanlines()) delete Scanline(0);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVScan::
Scene(void) const
{
  // Return scene
  if (!survey) return NULL;
  return survey->Scene();
}



GSVRun *GSVScan::
Run(void) const
{
  // Return run
  if (!survey) return NULL;
  return survey->Run();
}



GSVLaser *GSVScan::
Laser(void) const
{
  // Return laser
  if (!survey) return NULL;
  return survey->Laser();
}



GSVSegment *GSVScan::
Segment(void) const
{
  // Return segment
  if (!survey) return NULL;
  return survey->Segment();
}



int GSVScan::
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
    scene_index += r->NScans();
  }

  // Return scene index
  return scene_index;
}



int GSVScan::
RunIndex(void) const
{
  // Get convenient variables
  GSVSegment *segment = Segment();
  if (!segment) return -1;
  GSVRun *run = Run();
  if (!run) return -1;

  // Compute run index
  int run_index = SegmentIndex();
  for (int i = 0; i < segment->RunIndex(); i++) {
    GSVSegment *s = run->Segment(i);
    run_index += s->NScans();
  }

  // Return run index
  return run_index;
}



int GSVScan::
LaserIndex(void) const
{
  // Get convenient variables
  GSVLaser *laser = Laser();
  if (!laser) return -1;

  // Compute laser index
  int laser_index = SurveyIndex();
  for (int i = 0; i < survey->LaserIndex(); i++) {
    GSVSurvey *s = laser->Survey(i);
    laser_index += s->NScans();
  }

  // Return laser index
  return laser_index;
}



int GSVScan::
SegmentIndex(void) const
{
  // Get convenient variables
  GSVSegment *segment = Segment();
  if (!segment) return -1;

  // Compute segment index
  int segment_index = SurveyIndex();
  for (int i = 0; i < survey->SegmentIndex(); i++) {
    GSVSurvey *s = segment->Survey(i);
    segment_index += s->NScans();
  }

  // Return segment index
  return segment_index;
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

int GSVScan::
NPoints(void) const
{
  // Return number of points in all scanlines
  int npoints = 0;
  for (int i = 0; i < NScanlines(); i++) 
    npoints += Scanline(i)->NPoints();
  return npoints;
}

  

int GSVScan::
MaxSweepIndex(void) const
{
  // Initialize result
  int max_sweep_index = -1;

  // Find maximum sweep index of any point in any scanline
  for (int i = 0; i < NScanlines(); i++) {
    GSVScanline *scanline = Scanline(i);
    int m = scanline->SweepIndex();
    if (m > max_sweep_index) max_sweep_index = m;
  }

  // Return result
  return max_sweep_index;
}

  

int GSVScan::
MaxBeamIndex(void) const
{
  // Initialize result
  int max_beam_index = -1;

  // Find maximum beam index of any point in any scanline
  for (int i = 0; i < NScanlines(); i++) {
    GSVScanline *scanline = Scanline(i);
    int m = scanline->MaxBeamIndex();
    if (m > max_beam_index) max_beam_index = m;
  }

  // Return result
  return max_beam_index;
}

  

RNLength GSVScan::
TravelDistance(void) const
{
  // Return distance viewpoint of laser of survey travelled up to scan
  if (!survey) return 0.0;
  return survey->TravelDistance(survey_index);
}



GSVScanline *GSVScan::
FindScanlineBeforeTimestamp(RNScalar timestamp, int imin, int imax) const
{
  // Binary search
  int i = (imin + imax) / 2;
  if (i == imin) return Scanline(imin);
  assert(i < imax);
  GSVScanline *scanline = Scanline(i);
  RNScalar t = scanline->Timestamp();
  if (t > timestamp) return FindScanlineBeforeTimestamp(timestamp, imin, i);
  else if (t < timestamp) return FindScanlineBeforeTimestamp(timestamp, i, imax);
  else return scanline;
}



GSVScanline *GSVScan::
FindScanlineBeforeTimestamp(RNScalar timestamp) const
{
  // Binary search
  if (NScanlines() == 0) return NULL;
  if (timestamp <= Scanline(0)->Timestamp()) return Scanline(0);
  if (timestamp >= Scanline(NScanlines()-1)->Timestamp()) return Scanline(NScanlines()-1);
  return FindScanlineBeforeTimestamp(timestamp, 0, NScanlines()-1);
}



GSVPose GSVScan::
Pose(RNScalar timestamp) const
{
  // Find scanlines before and after timestamp
  GSVScanline *scanline1 = FindScanlineBeforeTimestamp(timestamp);
  if (!scanline1) return GSVPose(R3zero_point, R3zero_quaternion);
  int index1 = scanline1->ScanIndex();
  if (index1 < 0) return Scanline(0)->Pose();
  if (index1 >= NScanlines()-1) return Scanline(NScanlines()-1)->Pose();
  int index2 = index1 + 1;
  GSVScanline *scanline2 = Scanline(index2);

  // Compute interpolation parameter (0 <= t <= 1)
  RNScalar timestamp1 = scanline1->Timestamp();
  RNScalar timestamp2 = scanline2->Timestamp();
  RNScalar delta_timestamp = timestamp2 - timestamp1;
  if (delta_timestamp == 0) return scanline1->Pose();
  RNScalar t = (timestamp - timestamp1) / delta_timestamp;

  // Interpolate poses (would be better with quaternions)
  const GSVPose& pose1 = scanline1->Pose();
  const GSVPose& pose2 = scanline2->Pose();
  const R3Point& viewpoint1 = pose1.Viewpoint();
  const R3Point& viewpoint2 = pose2.Viewpoint();
  const R3Quaternion& orientation1 = pose1.Orientation();
  const R3Quaternion& orientation2 = pose2.Orientation();
  R3Point viewpoint = (1-t)*viewpoint1 + t*viewpoint2;
  R3Quaternion orientation = R3QuaternionSlerp(orientation1, orientation2, t);

  // Return interpolated pose
  return GSVPose(viewpoint, orientation);
}



R2Grid *GSVScan::
Grid(int channel) const
{
  // Check dimensions
  int nx = MaxSweepIndex() + 1;
  int ny = MaxBeamIndex() + 1;
  RNBoolean use_sweep_index = TRUE;
  if (nx <= 1) { use_sweep_index = FALSE; nx = NScanlines(); }
  if (nx * ny == 0) return NULL;
                     
  // Allocate image
  R2Grid *grid = new R2Grid(nx, ny);
  if (!grid) {
    RNFail("Unable to allocate grid");
    return NULL;
  }

  // Fill grid
  grid->Clear(R2_GRID_UNKNOWN_VALUE);
  for (int i = 0; i < NScanlines(); i++) {
    GSVScanline *scanline = Scanline(i);
    int ix = (use_sweep_index) ? scanline->SweepIndex() : i;
    const R3Point& sensor_position = scanline->Pose().Viewpoint();
    for (int j = 0; j < scanline->NPoints(); j++) {
      // Get point
      const GSVPoint& point = scanline->Point(j);

      // Compute value
      RNScalar value = R2_GRID_UNKNOWN_VALUE;
      switch (channel) {
      case GSV_POSITION_X_CHANNEL:
        value = point.Position().X();
        break;
      case GSV_POSITION_Y_CHANNEL:
        value = point.Position().Y();
        break;
      case GSV_POSITION_Z_CHANNEL:
        value = point.Position().Z();
        break;
      case GSV_NORMAL_X_CHANNEL:
        value = point.Normal().X();
        break;
      case GSV_NORMAL_Y_CHANNEL:
        value = point.Normal().Y();
        break;
      case GSV_NORMAL_Z_CHANNEL:
        value = point.Normal().Z();
        break;
      case GSV_TANGENT_X_CHANNEL:
        value = point.Tangent().X();
        break;
      case GSV_TANGENT_Y_CHANNEL:
        value = point.Tangent().Y();
        break;
      case GSV_TANGENT_Z_CHANNEL:
        value = point.Tangent().Z();
        break;
      case GSV_RADIUS_0_CHANNEL:
        value = point.Radius(0);
        break;
      case GSV_RADIUS_1_CHANNEL:
        value = point.Radius(1);
        break;
      case GSV_RED_CHANNEL:
        value = point.Color().R();
        break;
      case GSV_GREEN_CHANNEL:
        value = point.Color().G();
        break;
      case GSV_BLUE_CHANNEL:
        value = point.Color().B();
        break;
      case GSV_DISTANCE_CHANNEL:
        value = R3Distance(point.Position(), sensor_position);
        break;
      case GSV_REFLECTIVITY_CHANNEL:
        value = point.Reflectivity();
        break;
      case GSV_POINT_IDENTIFIER_CHANNEL:
        value = point.PointIdentifier();
        break;        
      case GSV_CLUSTER_IDENTIFIER_CHANNEL:
        value = point.ClusterIdentifier();
        break;        
      case GSV_CATEGORY_IDENTIFIER_CHANNEL:
        value = point.CategoryIdentifier();
        break;
      case GSV_CATEGORY_CONFIDENCE_CHANNEL:
        value = point.CategoryConfidence();
        break;
      case GSV_BEAM_INDEX_CHANNEL:
        value = point.BeamIndex();
        break;
      }        

      // Set value
      grid->SetGridValue(ix, point.BeamIndex(), value);
    }
  }
  
  // Return grid
  return grid;
}

  

GSVMesh *GSVScan::
Mesh(void) const
{
  // Get convenient variables (parameters)
  GSVScan *scan = (GSVScan *) this;
  if (NScanlines() == 0) return NULL;
  int scan_index = SegmentIndex();
  GSVSegment *segment = Segment();
  if (!segment) return NULL;
  int segment_index = segment->RunIndex();
  GSVRun *run = segment->Run();
  if (!run) return NULL;
  char run_name[1024];
  if (run->Name()) sprintf(run_name, "%s", run->Name());
  else sprintf(run_name, "Run%d", run->SceneIndex());
  GSVScene *scene = run->Scene();
  if (!scene) return NULL;

  // Initialize result
  GSVMesh *mesh = NULL;

  // Check if cached
  if (scene->CacheDataDirectoryName()) {
    // Construct mesh filename
    char mesh_name[4096];
    sprintf(mesh_name, "%s/laser_meshes/%s/%02d_%02d_Mesh.ply", 
      scene->CacheDataDirectoryName(), run_name, segment_index, scan_index);

    // Check if mesh file exists
    if (RNFileExists(mesh_name)) {
      // Allocate mesh
      mesh = new GSVMesh();
      if (!mesh) {
        RNFail("Unable to allocate mesh\n");
        return NULL;
      }
      
      // Read file 
      if (!mesh->ReadPlyFile(scan, mesh_name)) {
        RNFail("Unable to read mesh file %s\n", mesh_name);
        delete mesh;
        return NULL;
      }
    }
  }

  // If did not read from cache ...
  if (!mesh) {
    // Allocate mesh
    mesh = new GSVMesh();
    if (!mesh) {
      RNFail("Unable to allocate mesh\n");
      return NULL;
    }
      
    // Compute mesh 
    if (!mesh->LoadScan(scan)) {
      RNFail("Unable to create mesh\n");
      delete mesh;
      return NULL;
    }

    // Write mesh to file
    if (scene->CacheDataDirectoryName()) {
      if (scene->CreateCacheDataDirectory("laser_meshes", run)) {
        char mesh_name[4096];
        sprintf(mesh_name, "%s/laser_meshes/%s/%02d_%02d_Mesh.ply", 
          scene->CacheDataDirectoryName(), run_name, segment_index, scan_index);
        if (!mesh->WritePlyFile(scan, mesh_name, TRUE)) {
          RNFail("Unable to write mesh file %s\n", mesh_name);
        }
      }
    }
  }

  // Return mesh
  return mesh;
}



////////////////////////////////////////////////////////////////////////
//Input/Output functions
////////////////////////////////////////////////////////////////////////

int GSVScan::
ReadPoints(void) 
{
  // Check read counter
  if (++read_count > 1) return 1;

  // Get the scene filename
  if (NScanlines() == 0) return 1;
  GSVSegment *segment = Segment();
  if (!segment) return 0;
  GSVRun *run = segment->Run();
  if (!run) return 0;
  GSVScene *scene = run->Scene();
  if (!scene) return 0;
  const char *filename = scene->Filename();
  if (!filename) return 0;

  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    RNFail("Unable to open GSV scene file: %s\n", filename);
    return 0;
  }

  // Read scanlines
  for (int i = 0; i < NScanlines(); i++) {
    GSVScanline *scanline = Scanline(i);
    if (!scanline->ReadPoints(fp)) return 0;
  }

  // Close file 
  fclose(fp);

  // Return success
  return 1;
}



int GSVScan::
ReleasePoints(void) 
{
  // Check read counter
  if (--read_count > 0) return 1;

  // Release points
  for (int i = 0; i < NScanlines(); i++) {
    GSVScanline *scanline = Scanline(i);
    if (!scanline->ReleasePoints()) return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVScan::
InsertScanline(GSVScanline *scanline)
{
  // Just checking
  assert(scanline->scan_index == -1);
  assert(scanline->scan == NULL);

  // Insert scanline
  scanline->scan = this;
  scanline->scan_index = scanlines.NEntries();
  scanlines.Insert(scanline);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVScan::
RemoveScanline(GSVScanline *scanline)
{
  // Just checking
  assert(scanline->scan_index >= 0);
  assert(scanline->scan_index < scanlines.NEntries());
  assert(scanline->scan == this);

  // Remove scanline
  RNArrayEntry *entry = scanlines.KthEntry(scanline->scan_index);
  GSVScanline *tail = scanlines.Tail();
  tail->scan_index = scanline->scan_index;
  scanlines.EntryContents(entry) = tail;
  scanlines.RemoveTail();
  scanline->scan_index = -1;
  scanline->scan = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVScan::
Draw(RNFlags flags) const
{
  if (flags & GSV_DRAW_POINTS_WITH_LASER_INDEX_COLOR) {
    // Draw points  colored by laser index
    const int ncolors = 3;
    GLfloat color[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
    int laser_index = (Laser()) ? Laser()->RunIndex() : 0;
    RNLoadRgb(color[laser_index % ncolors]);
    for (int i = 0; i < NScanlines(); i++) {
      GSVScanline *scanline = Scanline(i);
      scanline->Draw(flags);
    }
  }
  else if (flags & GSV_DRAW_MESHES_WITHOUT_COLOR) {
    glEnable(GL_CULL_FACE);
    static GSVMesh *last_mesh = NULL;
    static const GSVScan *last_scan = NULL;
    if (this != last_scan) {
      if (last_mesh) delete last_mesh;
      last_mesh = Mesh();
      last_scan = this;
    }
    if (last_mesh) last_mesh->Draw();
    glDisable(GL_CULL_FACE);
  }
  else {
    // Draw scanlines
    for (int i = 0; i < NScanlines(); i++) {
      GSVScanline *scanline = Scanline(i);
      scanline->Draw(flags);
    }
  }
}



void GSVScan::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print scan header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Scan %d:", survey_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

   // Print scanlines
  for (int i = 0; i < NScanlines(); i++) {
    GSVScanline *scanline = Scanline(i);
    scanline->Print(fp, indented_prefix, suffix);
  }
}



void GSVScan::
UpdateBBox(void) 
{
  // Read points (assumes all scanlines are in memory or not) 
  RNBoolean read_points = FALSE;
  if (NScanlines() > 0) {
    GSVScanline *scanline = Scanline(0);
    if (scanline->DoesBBoxNeedUpdate()) {
      ReadPoints();
      read_points = TRUE;
    }
  }

  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NScanlines(); i++) {
    bbox.Union(Scanline(i)->BBox());
  }

  // Release points
  if (read_points) {
    ReleasePoints();
  }
}



void GSVScan::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (survey) survey->InvalidateBBox();
  }
}



void GSVScan::
UpdatePoseOrientations(void)
{
  // Get useful variables
  if (NScanlines() == 0) return;
  if (!survey) return;
  GSVLaser *laser = survey->Laser();
  if (!laser) return;
  GSVSegment *segment = survey->Segment();
  if (!segment) return;
  int mid_beam_index = laser->MaxBeamIndex()/2;
  if (mid_beam_index < 0) return;
  R3Vector front = segment->Front(Timestamp());

  // Read points
  if (!ReadPoints()) return;
  
  // Create list of points on middle beam index
  int npositions = 0;
  const int max_positions = 4096;
  R3Point positions[max_positions];
  for (int e = 0; e < NScanlines(); e++) {
    GSVScanline *scanline = Scanline(e);
    if (npositions >= max_positions) break;
    for (int k = 0; k < scanline->NPoints(); k++) {
      const GSVPoint& point = scanline->Point(k);
      if (point.BeamIndex() != mid_beam_index) continue;
      R3Vector v = point.Position() - scanline->Pose().Viewpoint();
      v.Normalize();
      if (npositions >= max_positions) break;
      positions[npositions++] = R3zero_point + v;
    }
  }

  // Release points
  if (!ReleasePoints()) return;
  
  // Compute PCA coordinate system of points
  if (npositions < 3) return;
  R3Point centroid = R3Centroid(npositions, positions);
  R3Triad principle_axes = R3PrincipleAxes(centroid, npositions, positions);

  // Compute scan orientation
  R3Vector axis2 = principle_axes[2];
  if (axis2.Dot(front) < 0) axis2.Flip();
  R3Vector axis0 = axis2 % R3posz_vector;
  axis0.Normalize();
  R3Vector axis1 = axis2 % axis0;
  axis1.Normalize();
  R3Triad triad(axis0, axis1, axis2);

  // Update pose orientation for every scanline
  for (int i = 0; i < NScanlines(); i++) {
    GSVScanline *scanline = Scanline(i);
    R3Point viewpoint = scanline->Pose().Viewpoint();
    GSVPose pose(viewpoint, triad);
    scanline->SetPose(pose);
  }
}



} // namespace gaps
