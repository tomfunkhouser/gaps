// Source file for GSV survey class



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

GSVSurvey::
GSVSurvey(void)
  : segment(NULL),
    segment_index(-1),
    laser(NULL),
    laser_index(-1),
    scans(),
    travel_distances(NULL),
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
}



GSVSurvey::
~GSVSurvey(void)
{
  // Remove survey from segment
  if (segment) segment->RemoveSurvey(this);

  // Remove survey from laser
  if (laser) laser->RemoveSurvey(this);

  // Delete scans
  while (NScans()) delete Scan(0);

  // Delete travel distances
  if (travel_distances) delete [] travel_distances;
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVSurvey::
Scene(void) const
{
  // Return scene
  if (!segment) return NULL;
  return segment->Scene();
}



int GSVSurvey::
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



GSVRun *GSVSurvey::
Run(void) const
{
  // Return run
  if (!segment) return NULL;
  return segment->Run();
}



int GSVSurvey::
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



int GSVSurvey::
NScanlines(void) const
{
  // Return number of scanlines in survey
  int count = 0;
  for (int i = 0; i < NScans(); i++) {
    GSVScan *scan = Scan(i);
    count += scan->NScanlines();
  }
  return count;
}



GSVScanline *GSVSurvey::
Scanline(int scanline_index) const
{
  // Return scanline
  for (int i = 0; i < NScans(); i++) {
    GSVScan *scan = Scan(i);
    if (scanline_index >= scan->NScanlines()) scanline_index -= scan->NScanlines();
    else return scan->Scanline(scanline_index);
  }

  // Index too high
  return NULL;
}



GSVScan *GSVSurvey::
FindScanBeforeTimestamp(RNScalar timestamp) const
{
  // Check number of scans
  if (NScans() == 0) return NULL;

  // Check all scans
  RNBoolean first = TRUE;
  for (int i = 0; i < NScans(); i++) {
    GSVScan *scan = Scan(i);
    if (scan->NScanlines() == 0) continue;
    if (first && (timestamp < scan->Scanline(0)->Timestamp())) { first = FALSE; continue; }
    if (timestamp > scan->Scanline(scan->NScanlines()-1)->Timestamp()) continue;
    return scan;
  }

  // Otherwise not found
  return NULL;
}


  
GSVScan *GSVSurvey::
FindScanBeforeTravelDistance(RNScalar travel_distance, int imin, int imax) const
{
  // Binary search
  int i = (imin + imax) / 2;
  if (i == imin) return Scan(imin);
  assert(i < imax);
  GSVScan *scan = Scan(i);
  RNScalar t = TravelDistance(scan->SurveyIndex());
  if (t > travel_distance) return FindScanBeforeTravelDistance(travel_distance, imin, i);
  else if (t < travel_distance) return FindScanBeforeTravelDistance(travel_distance, i, imax);
  else return scan;
}



GSVScan *GSVSurvey::
FindScanBeforeTravelDistance(RNScalar travel_distance) const
{
  // Binary search
  if (NScans() == 0) return NULL;
  if (travel_distance <= TravelDistance(0)) return Scan(0);
  if (travel_distance >= TravelDistance(NScans()-1)) return Scan(NScans()-1);
  return FindScanBeforeTravelDistance(travel_distance, 0, NScans()-1);
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

int GSVSurvey::
NPoints(void) const
{
  // Return number of points in all scans
  int npoints = 0;
  for (int i = 0; i < NScans(); i++) 
    npoints += Scan(i)->NPoints();
  return npoints;
}

  

int GSVSurvey::
MaxBeamIndex(void) const
{
  // Initialize result
  int max_beam_index = -1;

  // Find maximum beam index of any point in any scan
  for (int i = 0; i < NScans(); i++) {
    GSVScan *scan = Scan(i);
    int m = scan->MaxBeamIndex();
    if (m > max_beam_index) max_beam_index = m;
  }

  // Return result
  return max_beam_index;
}

  

////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVSurvey::
InsertScan(GSVScan *scan)
{
  // Just checking
  assert(scan->survey_index == -1);
  assert(scan->survey == NULL);

  // Insert laser scan
  scan->survey = this;
  scan->survey_index = scans.NEntries();
  scans.Insert(scan);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVSurvey::
RemoveScan(GSVScan *scan)
{
  // Just checking
  assert(scan->survey_index >= 0);
  assert(scan->survey_index < scans.NEntries());
  assert(scan->survey == this);

  // Remove scan
  RNArrayEntry *entry = scans.KthEntry(scan->survey_index);
  GSVScan *tail = scans.Tail();
  tail->survey_index = scan->survey_index;
  scans.EntryContents(entry) = tail;
  scans.RemoveTail();
  scan->survey_index = -1;
  scan->survey = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVSurvey::
Draw(RNFlags flags) const
{
  // Draw scans
  for (int i = 0; i < NScans(); i++) {
    GSVScan *scan = Scan(i);
    scan->Draw(flags);
  }
}



void GSVSurvey::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print survey header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Survey %d:", segment_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

   // Print scans
  for (int i = 0; i < NScans(); i++) {
    GSVScan *scan = Scan(i);
    scan->Print(fp, indented_prefix, suffix);
  }
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void GSVSurvey::
UpdateBBox(void) 
{
  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NScans(); i++) {
    bbox.Union(Scan(i)->BBox());
  }
}



void GSVSurvey::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (segment) segment->InvalidateBBox();
    if (laser) laser->InvalidateBBox();
  }
}



void GSVSurvey::
UpdateTravelDistances(void) 
{
  // Check stuff
  if (travel_distances) return;
  if (NScans() == 0) return;

  // Allocate travel distances
  travel_distances = new RNLength [ NScans() ];
  if (!travel_distances) {
    RNFail("Unable to allocate travel distances\n");
    return;
  }

  // Compute travel distances
  RNLength travel_distance = 0;
  R3Point prev_viewpoint = Scan(0)->Viewpoint();
  for (int i = 0; i < NScans(); i++) {
    GSVScan *scan = Scan(i);
    const R3Point& viewpoint = scan->Viewpoint();
    travel_distance += R3Distance(viewpoint, prev_viewpoint);
    travel_distances[i] = travel_distance;
    prev_viewpoint = viewpoint;
  }
}



void GSVSurvey::
InvalidateTravelDistances(void) 
{
  // Invalidate travel distances
  if (!travel_distances) return;
  delete [] travel_distances;
  travel_distances = NULL;
}



} // namespace gaps
