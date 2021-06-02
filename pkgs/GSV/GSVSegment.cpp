// Source file for GSV segment class



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

GSVSegment::
GSVSegment(const char *name)
  : scene_index(-1),
    run(NULL),
    run_index(-1),
    surveys(), 
    tapestries(), 
    panoramas(), 
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    name((name) ? RNStrdup(name) : NULL),
    data(NULL)
{
}



GSVSegment::
~GSVSegment(void)
{
  // Remove segment from scene
  assert(scene_index == -1);

  // Remove segment from run
  if (run) run->RemoveSegment(this);

  // Delete surveys
  while (NSurveys() > 0) delete Survey(0);

  // Delete tapestries
  while (NTapestries() > 0) delete Tapestry(0);

  // Delete panoramas
  while (NPanoramas() > 0) delete Panorama(0);

  // Delete name
  if (name) free(name);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVSegment::
Scene(void) const
{
  // Return scene
  if (!run) return NULL;
  return run->Scene();
}



int GSVSegment::
SceneIndex(void) const
{
  // Get convenient variables
  GSVScene *scene = Scene();
  if (!scene) return -1;

  // Compute scene index
  int scene_index = RunIndex();
  for (int i = 0; i < run->SceneIndex(); i++) {
    GSVRun *r = scene->Run(i);
    scene_index += r->NSegments();
  }

  // Return scene index
  return scene_index;
}



int GSVSegment::
NImages(void) const
{
  // Return number of images in segment
  int count = 0;
  for (int i = 0; i < NTapestries(); i++) {
    GSVTapestry *tapestry = Tapestry(i);
    count += tapestry->NImages();
  }
  return count;
}



GSVImage *GSVSegment::
Image(int image_index) const
{
  // Return image
  for (int i = 0; i < NTapestries(); i++) {
    GSVTapestry *tapestry = Tapestry(i);
    if (image_index >= tapestry->NImages()) image_index -= tapestry->NImages();
    else return tapestry->Image(image_index);
  }

  // Index too high
  return NULL;
}




int GSVSegment::
NScans(void) const
{
  // Return number of scans in segment
  int count = 0;
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    count += survey->NScans();
  }
  return count;
}



GSVScan *GSVSegment::
Scan(int scan_index) const
{
  // Return scan
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    if (scan_index >= survey->NScans()) scan_index -= survey->NScans();
    else return survey->Scan(scan_index);
  }

  // Index too high
  return NULL;
}




int GSVSegment::
NScanlines(void) const
{
  // Return number of scanlines in segment
  int count = 0;
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    count += survey->NScanlines();
  }
  return count;
}



GSVScanline *GSVSegment::
Scanline(int scanline_index) const
{
  // Return scanline
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    if (scanline_index >= survey->NScanlines()) scanline_index -= survey->NScanlines();
    else return survey->Scanline(scanline_index);
  }

  // Index too high
  return NULL;
}
 



GSVPanorama *GSVSegment::
Panorama(const char *name) const
{
  // Return panorama with matching name
  for (int i = 0; i < NPanoramas(); i++) {
    GSVPanorama *panorama = Panorama(i);
    if (!panorama->Name()) continue;
    if (!strcmp(panorama->Name(), name)) return panorama;
  }

  // None found
  return NULL;
}



GSVPanorama *GSVSegment::
FindPanoramaBeforeTimestamp(RNScalar timestamp, int imin, int imax) const
{
  // Binary search
  int i = (imin + imax) / 2;
  if (i == imin) return Panorama(imin);
  assert(i < imax);
  GSVPanorama *panorama = Panorama(i);
  RNScalar t = panorama->Timestamp();
  if (t > timestamp) return FindPanoramaBeforeTimestamp(timestamp, imin, i);
  else if (t < timestamp) return FindPanoramaBeforeTimestamp(timestamp, i, imax);
  else return panorama;
}



GSVPanorama *GSVSegment::
FindPanoramaBeforeTimestamp(RNScalar timestamp) const
{
  // Binary search
  if (NPanoramas() == 0) return NULL;
  if (timestamp <= Panorama(0)->Timestamp()) return Panorama(0);
  if (timestamp >= Panorama(NPanoramas()-1)->Timestamp()) return Panorama(NPanoramas()-1);
  return FindPanoramaBeforeTimestamp(timestamp, 0, NPanoramas()-1);
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

int GSVSegment::
NPoints(void) const
{
  // Return number of points in all surveys
  int npoints = 0;
  for (int i = 0; i < NSurveys(); i++) 
    npoints += Survey(i)->NPoints();
  return npoints;
}

  
////////////////////////////////////////////////////////////////////////
// Pose functions
////////////////////////////////////////////////////////////////////////

R3Point GSVSegment::
Viewpoint(RNScalar timestamp) const
{  
  // Return "viewpoint" of the car at timestamp
  if (NPanoramas() == 0) return R3zero_point;
  GSVPanorama *panorama0 = FindPanoramaBeforeTimestamp(timestamp);
  if (!panorama0) return R3zero_point;
  int i0 = panorama0->SegmentIndex();
  if (i0 >= NPanoramas()-1) return panorama0->Viewpoint();
  GSVPanorama *panorama1 = Panorama(i0 + 1);
  RNScalar t0 = panorama0->Timestamp();
  RNScalar t1 = panorama1->Timestamp();
  if (t1 <= t0) return panorama0->Viewpoint();
  RNScalar t = (timestamp - t0) / (t1 - t0);
  if (t <= 0) return panorama0->Viewpoint();
  else if (t >= 1) return panorama1->Viewpoint();
  return (1 - t) * panorama0->Viewpoint() + t * panorama1->Viewpoint();
}


  
R3Vector GSVSegment::
Front(RNScalar timestamp) const
{
  // Interpolate panorama front vectors
  if (NPanoramas() == 0) return R3zero_vector;
  GSVPanorama *panorama0 = FindPanoramaBeforeTimestamp(timestamp);
  if (!panorama0) return R3zero_vector;
  int i0 = panorama0->SegmentIndex();
  if (i0 >= NPanoramas()-1) return panorama0->Front();
  GSVPanorama *panorama1 = Panorama(i0 + 1);
  RNScalar t0 = panorama0->Timestamp();
  RNScalar t1 = panorama1->Timestamp();
  if (t1 <= t0) return panorama0->Front();
  RNScalar t = (timestamp - t0) / (t1 - t0);
  if (t <= 0) return panorama0->Front();
  else if (t >= 1) return panorama1->Front();
  R3Vector front = (1 - t) * panorama0->Front() + t * panorama1->Front();

  // Make sure orhtogonal to positive z (up)
  front[2] = 0; front.Normalize();

  // Return front direction of the car at timestamp
  return front;
}



R3Vector GSVSegment::
Right(RNScalar timestamp) const
{
  // Return vector to right of car
  R3Vector front = Front(timestamp);
  R3Vector up = Up(timestamp);
  R3Vector right = front % up;
  right.Normalize();
  return right;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVSegment::
InsertSurvey(GSVSurvey *survey)
{
  // Just checking
  assert(survey->segment_index == -1);
  assert(survey->segment == NULL);

  // Insert laser survey
  survey->segment = this;
  survey->segment_index = surveys.NEntries();
  surveys.Insert(survey);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVSegment::
RemoveSurvey(GSVSurvey *survey)
{
  // Just checking
  assert(survey->segment_index >= 0);
  assert(survey->segment_index < surveys.NEntries());
  assert(survey->segment == this);

  // Remove laser survey
  RNArrayEntry *entry = surveys.KthEntry(survey->segment_index);
  GSVSurvey *tail = surveys.Tail();
  tail->segment_index = survey->segment_index;
  surveys.EntryContents(entry) = tail;
  surveys.RemoveTail();
  survey->segment_index = -1;
  survey->segment = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVSegment::
InsertTapestry(GSVTapestry *tapestry)
{
  // Just checking
  assert(tapestry->segment_index == -1);
  assert(tapestry->segment == NULL);

  // Insert camera tapestry
  tapestry->segment = this;
  tapestry->segment_index = tapestries.NEntries();
  tapestries.Insert(tapestry);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVSegment::
RemoveTapestry(GSVTapestry *tapestry)
{
  // Just checking
  assert(tapestry->segment_index >= 0);
  assert(tapestry->segment_index < tapestries.NEntries());
  assert(tapestry->segment == this);

  // Remove camera tapestry
  RNArrayEntry *entry = tapestries.KthEntry(tapestry->segment_index);
  GSVTapestry *tail = tapestries.Tail();
  tail->segment_index = tapestry->segment_index;
  tapestries.EntryContents(entry) = tail;
  tapestries.RemoveTail();
  tapestry->segment_index = -1;
  tapestry->segment = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVSegment::
InsertPanorama(GSVPanorama *panorama)
{
  // Just checking
  assert(panorama->segment_index == -1);
  assert(panorama->segment == NULL);

  // Insert camera panorama
  panorama->segment = this;
  panorama->segment_index = panoramas.NEntries();
  panoramas.Insert(panorama);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVSegment::
RemovePanorama(GSVPanorama *panorama)
{
  // Just checking
  assert(panorama->segment_index >= 0);
  assert(panorama->segment_index < panoramas.NEntries());
  assert(panorama->segment == this);

  // Remove camera panorama
  RNArrayEntry *entry = panoramas.KthEntry(panorama->segment_index);
  GSVPanorama *tail = panoramas.Tail();
  tail->segment_index = panorama->segment_index;
  panoramas.EntryContents(entry) = tail;
  panoramas.RemoveTail();
  panorama->segment_index = -1;
  panorama->segment = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVSegment::
SetName(const char *name)
{
  // Delete previous name
  if (this->name) free(this->name);

  // Set new name
  if (name) this->name = strdup(name);
  else this->name = NULL;
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVSegment::
Draw(RNFlags flags) const
{
  // Draw surveys
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    survey->Draw(flags);
  }

  // Draw tapestries
  for (int i = 0; i < NTapestries(); i++) {
    GSVTapestry *tapestry = Tapestry(i);
    tapestry->Draw(flags);
  }
}



void GSVSegment::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print segment 
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Segment %d:", run_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

  // Print surveys
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    survey->Print(fp, indented_prefix, suffix);
  }

  // Print tapestries
  for (int i = 0; i < NTapestries(); i++) {
    GSVTapestry *tapestry = Tapestry(i);
    tapestry->Print(fp, indented_prefix, suffix);
  }

  // Print panorama
  for (int i = 0; i < NPanoramas(); i++) {
    GSVPanorama *panorama = Panorama(i);
    panorama->Print(fp, indented_prefix, suffix);
  }
}



////////////////////////////////////////////////////////////////////////
// BBox update functions
////////////////////////////////////////////////////////////////////////

void GSVSegment::
UpdateBBox(void) 
{
  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NSurveys(); i++) bbox.Union(Survey(i)->BBox());
  for (int i = 0; i < NTapestries(); i++) bbox.Union(Tapestry(i)->BBox());
  for (int i = 0; i < NPanoramas(); i++) bbox.Union(Panorama(i)->Viewpoint());
}



void GSVSegment::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (run) run->InvalidateBBox();
  }
}



////////////////////////////////////////////////////////////////////////
// Normal update functions
////////////////////////////////////////////////////////////////////////

static R3Point
GSVPointPosition(const GSVPoint *point, void *data)
{
  // Return position of point
  return point->Position();
}



static int
IsGSVPointCompatible(const GSVPoint *point1, const GSVPoint *point2, void *data)
{
  // Check timestamp difference
  static const double max_timestamp_difference = 0.5;
  double timestamp1 = point1->Timestamp();
  double timestamp2 = point2->Timestamp();
  if (fabs(timestamp1 - timestamp2) > max_timestamp_difference) return 0;
  return 1;
}



static int
CreateArrayOfPoints(const GSVSegment *segment, RNArray<const GSVPoint *>& points)
{
  // Create array of GSVPoints
  for (int iu = 0; iu < segment->NSurveys(); iu++) {
    GSVSurvey *survey = segment->Survey(iu);
    for (int ia = 0; ia < survey->NScans(); ia++) {
      GSVScan *scan = survey->Scan(ia);
      for (int ie = 0; ie < scan->NScanlines(); ie++) {
        GSVScanline *scanline = scan->Scanline(ie);
        if (!scanline->ArePointsResident()) continue;
        for (int ik = 0; ik < scanline->NPoints(); ik++) {
          const GSVPoint *point = &(scanline->Point(ik));
          points.Insert(point);
        }
      }
    }
  }

  // Return success
  return 1;
}



void GSVSegment::
UpdateNormals(void)
{
  // Parameters
  const int max_iter = 4;
  const double min_radius = 0.25;
  const int max_neighbors = 32;

  // Create kdtree of points
  RNArray<const GSVPoint *> points;
  CreateArrayOfPoints(this, points);
  R3Kdtree<const GSVPoint *> kdtree(points, GSVPointPosition);
  
  // Estimate normals for all points
  for (int iu = 0; iu < NSurveys(); iu++) {
    GSVSurvey *survey = Survey(iu);
    for (int ia = 0; ia < survey->NScans(); ia++) {
      GSVScan *scan = survey->Scan(ia);
      for (int ie = 0; ie < scan->NScanlines(); ie++) {
        GSVScanline *scanline = scan->Scanline(ie);
        if (!scanline->ArePointsResident()) continue;
        R3Point viewpoint = scanline->Pose().Viewpoint();
        for (int ik = 0; ik < scanline->NPoints(); ik++) {
          GSVPoint& point = (GSVPoint&) scanline->Point(ik);
          if (!point.Normal().IsZero()) continue;

          // Get point info
          R3Point position = point.Position();
          R3Vector vector = position - viewpoint;

          // Iterate over increasing max distances
          for (int iter = 0; iter <= max_iter; iter++) {
            // Compute max distance
            double max_distance = min_radius * pow(2, iter);
            double min_stddev = 0.1 * max_distance;
            if (min_stddev > 0.1) min_stddev = 0.1;
            if (min_stddev < 0.05) min_stddev = 0.05;
            double min_variance = min_stddev * min_stddev;
                
            // Find neighbors
            RNArray<const GSVPoint *> neighbors;
            if (!kdtree.FindAll(&point, 0, max_distance,
                                IsGSVPointCompatible, NULL, neighbors)) 
              continue;
              
            // Check if too few neighbors
            if (neighbors.NEntries() < 3) continue;

            // Sample point positions 
            int npositions = 0;
            static R3Point positions[ max_neighbors ];
            int dk = neighbors.NEntries() / max_neighbors + 1;
            for (int k = 0; k < neighbors.NEntries(); k += dk) {
              if (npositions >= max_neighbors) break;
              positions[npositions++] = neighbors[k]->Position();
            }

            // Compute normal from neighbor positions
            RNScalar variances[3];
            R3Point centroid = R3Centroid(npositions, positions);
            R3Triad triad = R3PrincipleAxes(centroid, npositions, positions, NULL, variances);
            R3Vector normal = triad[2];
            normal.Normalize();

            // Check variances
            if (variances[0] < min_variance) continue;
            if (variances[1] < min_variance) continue;

            // Flip to face laser
            if (normal.Dot(vector) > 0) normal.Flip();

            // Update normal
            point.SetNormal(normal);

            // Update tangent
            if (point.Tangent() == R3zero_vector) {
              R3Vector tangent = triad[0];
              tangent.Normalize();
              point.SetTangent(tangent);
            }

            // Update radius
            if ((point.Radius(0) == 0) || (point.Radius(1) == 0)) {
              RNScalar radius0 = 0.25 * max_distance;
              if (radius0 <= 0) radius0 = 0.01;
              else if (radius0 > 1) radius0 = 1;
              RNLength aspect = sqrt(variances[1] / variances[0]);
              RNScalar radius1 = aspect * radius0;
              point.SetRadius(0, radius0);
              point.SetRadius(1, radius1);
            }

            // Set normal for point
            break;
          }
              
          // Set normal to default, if otherwise missed
          if (point.Normal().IsZero()) {
            point.SetNormal((vector.Z() > 0) ? R3posz_vector : R3negz_vector);
            point.SetTangent(R3posx_vector);
            point.SetRadius(0.1);
          }
        }
      }
    }
  }
}



void GSVSegment::
InvalidateNormals(void)
{
  // Invalidate all normals
  for (int iu = 0; iu < NSurveys(); iu++) {
    GSVSurvey *survey = Survey(iu);
    for (int ia = 0; ia < survey->NScans(); ia++) {
      GSVScan *scan = survey->Scan(ia);
      for (int ie = 0; ie < scan->NScanlines(); ie++) {
        GSVScanline *scanline = scan->Scanline(ie);
        if (!scanline->ArePointsResident()) continue;
        for (int ik = 0; ik < scanline->NPoints(); ik++) {
          GSVPoint& point = (GSVPoint&) scanline->Point(ik);
          point.SetNormal(R3zero_vector);
        }
      }
    }
  }
}



} // namespace gaps
