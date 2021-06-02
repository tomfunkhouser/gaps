// Source file for GSV run class



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

GSVRun::
GSVRun(const char *name)
  : scene(NULL),
    scene_index(-1),
    segments(),
    cameras(),
    lasers(),
    name((name) ? RNStrdup(name) : NULL),
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
}



GSVRun::
~GSVRun(void)
{
  // Remove run from scene
  if (scene) scene->RemoveRun(this);

  // Delete segments
  while (NSegments() > 0) delete Segment(0);

  // Delete cameras
  while (NCameras() > 0) delete Camera(0);

  // Delete lasers
  while (NLasers() > 0) delete Laser(0);

  // Delete name
  if (name) free(name);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVSegment *GSVRun::
Segment(const char *name) const
{
  // Return segment with matching name
  for (int i = 0; i < NSegments(); i++) {
    GSVSegment *segment = Segment(i);
    if (!segment->Name()) continue;
    if (!strcmp(segment->Name(), name)) return segment;
  }

  // None found
  return NULL;
}



int GSVRun::
NTapestries(void) const
{
  // Return number of tapestries in run
  int count = 0;
  for (int is = 0; is < NSegments(); is++) {
    GSVSegment *segment = Segment(is);
    count += segment->NTapestries();
  }
  return count;
}



int GSVRun::
NPanoramas(void) const
{
  // Return number of panoramas in run
  int count = 0;
  for (int is = 0; is < NSegments(); is++) {
    GSVSegment *segment = Segment(is);
    count += segment->NPanoramas();
  }
  return count;
}



int GSVRun::
NImages(void) const
{
  // Return number of images in run
  int count = 0;
  for (int is = 0; is < NSegments(); is++) {
    GSVSegment *segment = Segment(is);
    count += segment->NImages();
  }
  return count;
}



int GSVRun::
NSurveys(void) const
{
  // Return number of surveys in run
  int count = 0;
  for (int is = 0; is < NSegments(); is++) {
    GSVSegment *segment = Segment(is);
    count += segment->NSurveys();
  }
  return count;
}



int GSVRun::
NScans(void) const
{
  // Return number of scans in run
  int count = 0;
  for (int is = 0; is < NSegments(); is++) {
    GSVSegment *segment = Segment(is);
    count += segment->NScans();
  }
  return count;
}



int GSVRun::
NScanlines(void) const
{
  // Return number of scanlines in run
  int count = 0;
  for (int is = 0; is < NSegments(); is++) {
    GSVSegment *segment = Segment(is);
    count += segment->NScanlines();
  }
  return count;
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

int GSVRun::
NPoints(void) const
{
  // Return number of points in all segments
  int npoints = 0;
  for (int i = 0; i < NSegments(); i++) 
    npoints += Segment(i)->NPoints();
  return npoints;
}

  

////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVRun::
InsertSegment(GSVSegment *segment)
{
  // Just checking
  assert(segment->run_index == -1);
  assert(segment->run == NULL);

  // Insert segment
  segment->run = this;
  segment->run_index = segments.NEntries();
  segments.Insert(segment);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVRun::
RemoveSegment(GSVSegment *segment)
{
  // Just checking
  assert(segment->run_index >= 0);
  assert(segment->run_index < segments.NEntries());
  assert(segment->run == this);

  // Remove segment
  RNArrayEntry *entry = segments.KthEntry(segment->run_index);
  GSVSegment *tail = segments.Tail();
  tail->run_index = segment->run_index;
  segments.EntryContents(entry) = tail;
  segments.RemoveTail();
  segment->run_index = -1;
  segment->run = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVRun::
InsertCamera(GSVCamera *camera)
{
  // Just checking
  assert(camera->run_index == -1);
  assert(camera->run == NULL);

  // Insert camera
  camera->run = this;
  camera->run_index = cameras.NEntries();
  cameras.Insert(camera);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVRun::
RemoveCamera(GSVCamera *camera)
{
  // Just checking
  assert(camera->run_index >= 0);
  assert(camera->run_index < cameras.NEntries());
  assert(camera->run == this);

  // Remove camera
  RNArrayEntry *entry = cameras.KthEntry(camera->run_index);
  GSVCamera *tail = cameras.Tail();
  tail->run_index = camera->run_index;
  cameras.EntryContents(entry) = tail;
  cameras.RemoveTail();
  camera->run_index = -1;
  camera->run = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVRun::
InsertLaser(GSVLaser *laser)
{
  // Just checking
  assert(laser->run_index == -1);
  assert(laser->run == NULL);

  // Insert laser
  laser->run = this;
  laser->run_index = lasers.NEntries();
  lasers.Insert(laser);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVRun::
RemoveLaser(GSVLaser *laser)
{
  // Just checking
  assert(laser->run_index >= 0);
  assert(laser->run_index < lasers.NEntries());
  assert(laser->run == this);

  // Remove laser
  RNArrayEntry *entry = lasers.KthEntry(laser->run_index);
  GSVLaser *tail = lasers.Tail();
  tail->run_index = laser->run_index;
  lasers.EntryContents(entry) = tail;
  lasers.RemoveTail();
  laser->run_index = -1;
  laser->run = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVRun::
SetName(const char *name)
{
  // Set name
  if (this->name) free(this->name);
  if (name) this->name = RNStrdup(name);
  else this->name = NULL;
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVRun::
Draw(RNFlags flags) const
{
  // Draw segments
  for (int i = 0; i < NSegments(); i++) {
    GSVSegment *segment = Segment(i);
    segment->Draw(flags);
  }
}



void GSVRun::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print run 
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Run %d:", scene_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

   // Print segments
  for (int i = 0; i < NSegments(); i++) {
    GSVSegment *segment = Segment(i);
    segment->Print(fp, indented_prefix, suffix);
  }
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void GSVRun::
UpdateBBox(void) 
{
  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NSegments(); i++) bbox.Union(Segment(i)->BBox());
  for (int i = 0; i < NLasers(); i++) bbox.Union(Laser(i)->BBox());
  for (int i = 0; i < NCameras(); i++) bbox.Union(Camera(i)->BBox());
}



void GSVRun::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (scene) scene->InvalidateBBox();
  }
}



void GSVRun::
UpdateNormals(void) 
{
  // Update normals
  for (int i = 0; i < NSegments(); i++) {
    GSVSegment *segment = Segment(i);
    segment->UpdateNormals();
  }
}



void GSVRun::
InvalidateNormals(void) 
{
  // Invalidate normals
  for (int i = 0; i < NSegments(); i++) {
    GSVSegment *segment = Segment(i);
    segment->InvalidateNormals();
  }
}



////////////////////////////////////////////////////////////////////////
// ASCII file read functions
////////////////////////////////////////////////////////////////////////

int GSVRun::
ReadCameraInfoFile(void)
{
  // Create filename
  char filename[4096];
  sprintf(filename, "%s/%s/camera_info.txt", scene->RawDataDirectoryName(), Name());

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open camera info file: %s\n", filename);
    return 0;
  }

  // Read camera information
  int line_count = 0;
  GSVCamera *camera = NULL;
  const int buffer_size = 16*1024;
  char buffer[buffer_size];
  while (fgets(buffer, buffer_size, fp)) {
    // Parse line
    line_count++;
    char *keyword = strtok(buffer, ": \t\n");
    if (!keyword) continue;
    if (keyword[0] == '#') continue;
    char *string = strtok(NULL, ": \t\n");
    RNScalar value = atof(string);

    // First line should have number
    if (!strcmp(keyword, "camera_number")) {
      int ncameras = atoi(string);
      for (int i = 0; i < ncameras; i++) {
        GSVCamera *camera = new GSVCamera();
        InsertCamera(camera);
      }
    }
    else if (!strcmp(keyword, "camera_index")) {
      int index = atoi(string);
      if ((index >= 0) && (index < NCameras())) {
        camera = Camera(index);
      }
    }
    else if (camera) {
      if (!strcmp(keyword, "focal_x")) camera->SetXFocal(value);
      else if (!strcmp(keyword, "focal_y")) camera->SetYFocal(value);
      else if (!strcmp(keyword, "center_x")) camera->SetXCenter(value);
      else if (!strcmp(keyword, "center_y")) camera->SetYCenter(value);
      else if (!strcmp(keyword, "distortion_type")) camera->SetDistortionType((!strcmp(string, "PERSPECTIVE")) ? 0 : 1);
      else if (!strcmp(keyword, "k1")) camera->SetK1(value);
      else if (!strcmp(keyword, "k2")) camera->SetK2(value);
      else if (!strcmp(keyword, "k3")) camera->SetK3(value);
      else if (!strcmp(keyword, "p1")) camera->SetP1(value);
      else if (!strcmp(keyword, "p2")) camera->SetP2(value);
      else if (!strcmp(keyword, "fov_max")) camera->SetMaxFov(RN_PI*value/180.0);
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int GSVRun::
ReadImageSegmentFile(RNArray<GSVPanorama *>& panoramas)
{
  // Create filename
  char filename[4096];
  sprintf(filename, "%s/%s/image_seg.txt", scene->RawDataDirectoryName(), Name());

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open segment info file: %s\n", filename);
    return 0;
  }

  // Read range of panorama indices for each segment
  int line_count = 0;
  const int buffer_size = 16*1024;
  char buffer[buffer_size];
  while (fgets(buffer, buffer_size, fp)) {
    // Parse line
    line_count++;
    if (buffer[0] == '\0') continue;
    if (buffer[0] == '#') continue;
    int segment_index;
    int start_panorama_index;
    int end_panorama_index;
    if (sscanf(buffer, "%d%d%d", &segment_index, &start_panorama_index, &end_panorama_index) == 3) {
      // Create segment
      while (NSegments() <= segment_index) {
        GSVSegment *segment = new GSVSegment();
        InsertSegment(segment);
        for (int ic = 0; ic < NCameras(); ic++) {
          GSVCamera *camera = Camera(ic);
          GSVTapestry *tapestry = new GSVTapestry();
          segment->InsertTapestry(tapestry);
          camera->InsertTapestry(tapestry);
        }
      }

      // Get segment
      GSVSegment *segment = Segment(segment_index);

      // Create panoramas
      for (int panorama_index = start_panorama_index; panorama_index <= end_panorama_index; panorama_index++) {
        while (panoramas.NEntries() <= panorama_index) {
          GSVPanorama *panorama = new GSVPanorama();
          segment->InsertPanorama(panorama);
          panoramas.Insert(panorama);
        }
      }
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int GSVRun::
ReadImagePoseFile(const RNArray<GSVPanorama *>& panoramas)
{
  // Create filename
  char filename[4096];
  sprintf(filename, "%s/%s/image_pose.txt", scene->RawDataDirectoryName(), Name());

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open image pose file: %s\n", filename);
    return 0;
  }

  // Read pose for each column
  int line_count = 0;
  const int buffer_size = 16*1024;
  char buffer[buffer_size];
  GSVCamera *camera = NULL;
  GSVSegment *segment = NULL;
  GSVTapestry *tapestry = NULL;
  GSVPanorama *panorama = NULL;
  GSVImage *image = NULL;
  while (fgets(buffer, buffer_size, fp)) {
    // Parse line
    line_count++;
    if (buffer[0] == '\0') continue;
    if (buffer[0] == '#') continue;
    int panorama_index, camera_index, column_index;
    double timestamp, position_x, position_y, position_z;
    double quaternion_x, quaternion_y, quaternion_z, quaternion_w; 
    if (sscanf(buffer, "%d%d%d%lf%lf%lf%lf%lf%lf%lf%lf", 
      &panorama_index, &camera_index, &column_index, &timestamp,
      &position_x, &position_y, &position_z,
      &quaternion_x, &quaternion_y, &quaternion_z, &quaternion_w) != 11) {
      RNFail("Error reading line %d from %s\n", line_count, filename);
      return 0;
    }

    // Get camera
    if (column_index == 0) {
      camera = NULL;
      if (camera_index < NCameras()) {
        camera = Camera(camera_index);
      }
    }

     // Get panorama and segment
    if (column_index == 0) {
      panorama = NULL;
      segment = NULL;
      if (panorama_index < panoramas.NEntries()) {
        panorama = panoramas.Kth(panorama_index);
        segment = panorama->Segment();
      }
    }

    // Get tapestry
    if (column_index == 0) {
      tapestry = NULL;
      if (segment) {
        if (camera_index < segment->NTapestries()) {
          tapestry = segment->Tapestry(camera_index);
        }
      }
    }

    // Create image
    if (column_index == 0) {
      image = new GSVImage();
      if (panorama) panorama->InsertImage(image);
      if (tapestry) tapestry->InsertImage(image);
    }

    // Create pose
    R3Point viewpoint(position_x, position_y, position_z);
    R3Quaternion orientation(quaternion_w, quaternion_x, quaternion_y, quaternion_z);
    orientation.Rotate(R3Quaternion(R3posx_vector, RN_PI));
    GSVPose pose(viewpoint, orientation);

    // Set image pose and timestamp
    if (camera) {
      static GSVPose pose0;
      static RNScalar timestamp0;
      if (column_index == 0) {
        pose0 = pose;
        timestamp0 = timestamp;
      }
      else if (column_index == camera->ImageWidth()-1) {
        image->SetPose(pose0, pose);
        image->SetTimestamp(timestamp0, timestamp);
      }
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int GSVRun::
ReadLaserObjFile(int laser_index, RNArray<GSVScanline *>& scanlines, RNArray<GSVScan *>& scanline_scans, RNBoolean read_points)
{
  // Create filename
  char filename[4096];
  sprintf(filename, "%s/%s/laser_point_cloud_%d.obj", scene->RawDataDirectoryName(), Name(), laser_index+1);

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open laser pose file: %s\n", filename);
    return 0;
  }

  // Create laser
  GSVLaser *laser = NULL;
  while (NLasers() <= laser_index) 
    InsertLaser(new GSVLaser(GSV_SICK_LASER));
  laser = Laser(laser_index);

  // Read points for each scanline
  int line_count = 0;
  const int buffer_size = 16*1024;
  char buffer[buffer_size];
  char keyword[1024];
  GSVScanline *scanline  = NULL;
  int num_scanline_points = 0;
  const int max_scanline_points = 1024;
  GSVPoint scanline_points[max_scanline_points];
  while (fgets(buffer, buffer_size, fp)) {
    line_count++;
    if (buffer[0] == '\0') continue;
    if (buffer[0] == '#') continue;
    if (buffer[0] == 'v') {
      // Parse point
      double x, y, z; 
      if (sscanf(buffer, "%s%lf%lf%lf", keyword, &x, &y, &z) != 4) {
        RNFail("Error reading line %d from %s\n", line_count, filename);
        return 0;
      }

      // Insert point
      if (scanline) {
        if (num_scanline_points < max_scanline_points) {
          scanline_points[num_scanline_points].SetPosition(R3Point(x, y, z));
          num_scanline_points++;
        }
      }
    }
    else if (buffer[0] == 'g') {
      // Set points in previous scanline
      if (scanline) {
        if (read_points) {
          // Project car points onto ground
          if (num_scanline_points > 25) {
            for (int i = num_scanline_points - 25; i < num_scanline_points; i++) {
              R3Point position =  scanline_points[i].Position();
              position[2] = scanline_points[num_scanline_points-25].Position()[2];
              scanline_points[i].SetPosition(position);
            }
          }
          scanline->SetPoints(scanline_points, num_scanline_points);
        }
        num_scanline_points = 0;
        scanline = NULL;
      }

      // Get scanline string
      char scanline_string[1024];
      if (sscanf(buffer, "%s%s", keyword, scanline_string) != 2) {
        RNFail("Error reading line %d from %s\n", line_count, filename);
        return 0;
      }

      // Parse scanline string
      int segment_index = 0;
      // int scanline_index = 0;
      char *bufferp = strtok(scanline_string, "_ \n\t");
      if (bufferp) { 
        assert(!strcmp(bufferp, "seg"));
        bufferp = strtok(NULL, "_ \n\t"); 
        if (bufferp) {
          segment_index = atoi(bufferp);
          bufferp = strtok(NULL, "_ \n\t"); 
          if (bufferp) {
            assert(!strcmp(bufferp, "scanline"));
            bufferp = strtok(NULL, "_ \n\t"); 
            if (bufferp) {
              // scanline_index = atoi(bufferp);
            }
          }
        }
      }

      // Get/create segment
      GSVSegment *segment = NULL;
      while (NSegments() <= segment_index) 
        InsertSegment(new GSVSegment());
      segment = Segment(segment_index);
        
      // Get/create survey
      GSVSurvey *survey = NULL;
      while (segment->NSurveys() <= laser_index) 
        segment->InsertSurvey(new GSVSurvey());
      survey = segment->Survey(laser_index);

      // Insert survey into laser
      if (survey->Laser() == NULL) laser->InsertSurvey(survey);
      assert(survey->Laser() == laser);

      // Get/create scan
      GSVScan *scan = new GSVScan();
      survey->InsertScan(scan);

      // Create scanline
      scanline = new GSVScanline();

      // Insert scanline into array to be returned
      scanlines.Insert(scanline);

      // Insert scan into array to be returned (so that can insert scanlines conditionally based on pose)
      scanline_scans.Insert(scan);
    }
  }

  // Set points in last scanline
  if (scanline) {
    if (read_points) {
      // Project car points onto ground
      if (num_scanline_points > 25) {
        for (int i = num_scanline_points - 25; i < num_scanline_points; i++) {
          R3Point position =  scanline_points[i].Position();
          position[2] = scanline_points[num_scanline_points-25].Position()[2];
          scanline_points[i].SetPosition(position);
        }
      }
      scanline->SetPoints(scanline_points, num_scanline_points);
    }
    num_scanline_points = 0;
    scanline = NULL;
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int GSVRun::
ReadLaserPoseFile(int laser_index, const RNArray<GSVScanline *>& scanlines, RNArray<GSVScan *>& scanline_scans)
{
  // Create filename
  char filename[4096];
  sprintf(filename, "%s/%s/laser_pose_%d.txt", scene->RawDataDirectoryName(), Name(), laser_index+1);

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open laser pose file: %s\n", filename);
    return 0;
  }

  // Read pose for each scanline
  int line_count = 0;
  const int buffer_size = 16*1024;
  char buffer[buffer_size];
  while (fgets(buffer, buffer_size, fp)) {
    // Parse line
    line_count++;
    if (buffer[0] == '\0') continue;
    if (buffer[0] == '#') continue;
    int scanline_index;
    double timestamp, position_x, position_y, position_z;
    double quaternion_x, quaternion_y, quaternion_z, quaternion_w; 
    if (sscanf(buffer, "%d%lf%lf%lf%lf%lf%lf%lf%lf", 
               &scanline_index, &timestamp,
               &position_x, &position_y, &position_z,
               &quaternion_x, &quaternion_y, &quaternion_z, &quaternion_w) != 9) {
      RNFail("Error reading line %d from %s\n", line_count, filename);
      return 0;
    }

    // Check scanline index (newer files have scanline == -1 in first line)
    if (scanline_index < 0) continue;

    // Get scan 
    GSVScan *scan = scanline_scans.Kth(scanline_index);

    // Get scanline
    GSVScanline *scanline = scanlines.Kth(scanline_index);

    // Set scanline pose
    R3Point viewpoint(position_x, position_y, position_z);
    R3Quaternion orientation(quaternion_w, quaternion_x, quaternion_y, quaternion_z);
    orientation.Rotate(R3Quaternion(R3posz_vector, -RN_PI_OVER_TWO));
    orientation.Rotate(R3Quaternion(R3posy_vector, RN_PI_OVER_TWO));
    GSVPose pose(viewpoint, orientation);
    scanline->SetPose(pose);
    
    // Set scanline timestamp
    scanline->SetTimestamp(timestamp);

#if 0
    // Check viewpoint movement
    const RNLength min_viewpoint_movement = 0.01;
    static R3Point previous_viewpoint(0, 0, 0);
    RNLength viewpoint_movement = R3Distance(viewpoint, previous_viewpoint);
    if (viewpoint_movement < min_viewpoint_movement) continue;
    previous_viewpoint = viewpoint;
#endif

    // Insert scanline into scan
    scan->InsertScanline(scanline);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



} // namespace gaps
