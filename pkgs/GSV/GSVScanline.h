// Include file for the GSV scanline class 
#ifndef __GSV__SCANLINE__H__
#define __GSV__SCANLINE__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVScanline {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor functions
  GSVScanline(void);
  GSVScanline(const GSVPose& pose, RNScalar timestamp);
  virtual ~GSVScanline(void);


  //////////////////////////
  //// ACCESS FUNCTIONS ////
  //////////////////////////

  // Scene access functions
  GSVScene *Scene(void) const;
  int SceneIndex(void) const;

  // Run access functions
  GSVRun *Run(void) const;
  int RunIndex(void) const;

  // Laser access functions
  GSVLaser *Laser(void) const;
  int LaserIndex(void) const;

  // Segment access functions
  GSVSegment *Segment(void) const;
  int SegmentIndex(void) const;

  // Survey access functions
  GSVSurvey *Survey(void) const;
  int SurveyIndex(void) const;

  // Scan access functions
  GSVScan *Scan(void) const;
  int ScanIndex(void) const;

  
  ////////////////////////////
  //// SCANLINE PROPERTY FUNCTIONS ////
  ////////////////////////////

  // Extrinsic sensor property functions
  const GSVPose& Pose(void) const;

  // Timing property functions
  RNScalar Timestamp(void) const;

  // Index property functions
  int SweepIndex(void) const;
  int MaxBeamIndex(void) const;

  // Spatial property functions
  const R3Box& BBox(void) const;

  // Estimated property functions
  RNCoord EstimatedGroundZ(void) const;

  // Get user data 
  void *Data(void) const;


  //////////////////////////////////
  //// POINT PROPERTY FUNCTIONS ////
  //////////////////////////////////

  // Point access functions
  int NPoints(void) const;
  const GSVPoint& Point(int point_index) const;
  R3Point PointPosition(int point_index) const;
  R3Vector PointNormal(int point_index) const;
  R3Vector PointTangent(int point_index) const;
  RNLength PointRadius(int point_index, int k) const;
  RNScalar PointTimestamp(int point_index) const;
  RNScalar PointElevation(int point_index) const;
  RNAngle PointAngle(int point_index) const;
  RNRgb PointColor(int point_index) const;
  int PointReflectivity(int point_index) const;
  int PointBeamIndex(int point_index) const;
  int PointReturnType(int point_index) const;
  int PointIdentifier(int point_index) const;
  int PointClusterIdentifier(int point_index) const;
  int PointCategoryIdentifier(int point_index) const;
  RNScalar PointCategoryConfidence(int point_index) const;

  ////////////////////////////////
  //// SCANLINE MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Extrinsic sensor property functions
  void SetPose(const GSVPose& pose);

  // Index property functions
  void SetSweepIndex(int index);

  // Timing property functions
  void SetTimestamp(RNScalar timestamp);

  // Spatial property manipulation
  void SetBBox(const R3Box& box);

  // Set user data 
  void SetData(void *data);
  

  //////////////////////////////////////
  //// POINT MANIPULATION FUNCTIONS ////
  //////////////////////////////////////
  
  // Point manipulation functions
  void SetPoints(const GSVPoint *points, int npoints);
  void SetPoint(int point_index, const GSVPoint& point);
  void SetPointPosition(int point_index, const R3Point& position);
  void SetPointNormal(int point_index, const R3Vector& normal);
  void SetPointTangent(int point_index, const R3Vector& tangent);
  void SetPointRadius(int point_index, int k, RNLength radius);
  void SetPointReturnType(int point_index, int return_type);
  void SetPointTimestamp(int point_index, RNScalar timestamp);
  void SetPointElevation(int point_index, RNScalar elevation);
  void SetPointColor(int point_index, const RNRgb& color);
  void SetPointReflectivity(int point_index, int reflectivity);
  void SetPointBeamIndex(int point_index, int beam_index);
  void SetPointIdentifier(int point_index, int point_identifier);
  void SetPointClusterIdentifier(int point_index, int cluster_identifier);
  void SetPointCategoryIdentifier(int point_index, int category_identifier);
  void SetPointCategoryConfidence(int point_index, RNScalar category_confidence);


  ///////////////////////////
  //// DISPLAY FUNCTIONS ////
  ///////////////////////////

  // Draw function
  virtual void Draw(RNFlags flags = GSV_DEFAULT_DRAW_FLAGS) const;

  // Print function
  virtual void Print(FILE *fp = NULL, const char *prefix = NULL, const char *suffix = NULL) const;


////////////////////////////////////////////////////////////////////////
// INTERNAL STUFF BELOW HERE
////////////////////////////////////////////////////////////////////////

  // Query points by beam index
  int FindPointIndexWithClosestBeamIndex(int query_beam_index) const;

  // Load points (calls glColor and glVertex)
  void LoadPoints(RNFlags flags = GSV_DEFAULT_DRAW_FLAGS) const;
  
public:
  // Lower level access functions 
  const double *PointPositionCoords(int point_index) const;
  const float *PointNormalCoords(int point_index) const;
  const float *PointTangentCoords(int point_index) const;
  unsigned char PointFlags(int point_index) const;
  
  // Lower level manipulation functions
  void SetPointPositionCoords(int point_index, const double position[3]);
  void SetPointNormalCoords(int point_index, const float normal[3]);
  void SetPointTangentCoords(int point_index, const float tangent[3]);
  void SetPointFlags(int point_index, unsigned char flags);
  
  // BBox update functions 
  void UpdateBBox(void);
  void InvalidateBBox(void);
  RNBoolean DoesBBoxNeedUpdate(void) const;

  // I/O functions
  RNBoolean ArePointsResident(void) const;
  int ReadPoints(FILE *fp, RNBoolean seek = TRUE);
  int WritePoints(FILE *fp, RNBoolean seek = TRUE);
  int ReleasePoints(void);

  // File functions
  unsigned long long FileOffset(void) const;
  void SetFileOffset(unsigned long long file_offset);
  void SetNPoints(int npoints);

protected:
  friend class GSVScene; // for old version reading
  friend class GSVScan;
  GSVScan *scan;
  int scan_index;
  unsigned long long file_offset;
  unsigned int read_count;
  GSVPoint *points;
  int npoints;
  GSVPose pose;
  RNScalar timestamp;
  int sweep_index;
  R3Box bbox;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVScan *GSVScanline::
Scan(void) const
{
  // Return laser scan
  return scan;
}



inline int GSVScanline::
ScanIndex(void) const
{
  // Return index of this scanline in scan
  return scan_index;
}



inline const GSVPose& GSVScanline::
Pose(void) const
{
  // Return sensor pose
  return pose;
}



inline RNScalar GSVScanline::
Timestamp(void) const
{
  // Return timestamp 
  return timestamp;
}



inline int GSVScanline::
SweepIndex(void) const
{
  // Return index indicating where this scanline is within (angle) range of scan
  return sweep_index;
}



inline const R3Box& GSVScanline::
BBox(void) const
{
  // Return bounding box
  if (DoesBBoxNeedUpdate()) 
    ((GSVScanline *) this)->UpdateBBox();
  return bbox;
}



inline void *GSVScanline::
Data(void) const
{
  // Return user data
  return data;
}



inline int GSVScanline::
NPoints(void) const
{
  // Return number of points
  return npoints;
}



inline const GSVPoint& GSVScanline::
Point(int point_index) const
{
  // Return point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index];
}



inline R3Point GSVScanline::
PointPosition(int point_index) const
{
  // Return point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Position();
}



inline const double *GSVScanline::
PointPositionCoords(int point_index) const
{
  // Return point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].PositionCoords();
}



inline R3Vector GSVScanline::
PointNormal(int point_index) const
{
  // Return point normal
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Normal();
}



inline const float *GSVScanline::
PointNormalCoords(int point_index) const
{
  // Return point normal
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].NormalCoords();
}



inline R3Vector GSVScanline::
PointTangent(int point_index) const
{
  // Return point tangent
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Tangent();
}



inline const float *GSVScanline::
PointTangentCoords(int point_index) const
{
  // Return point tangent
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].TangentCoords();
}



inline RNLength GSVScanline::
PointRadius(int point_index, int k) const
{
  // Return point radius
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Radius(k);
}



inline int GSVScanline::
PointReturnType(int point_index) const
{
  // Return point return type
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].ReturnType();
}



inline RNScalar GSVScanline::
PointTimestamp(int point_index) const
{
  // Return point timestamp
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Timestamp();
}



inline RNScalar GSVScanline::
PointElevation(int point_index) const
{
  // Return point elevation
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Elevation();
}



inline RNRgb GSVScanline::
PointColor(int point_index) const
{
  // Return point color
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Color();
}



inline int GSVScanline::
PointReflectivity(int point_index) const
{
  // Return point reflectivity
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Reflectivity();
}



inline int GSVScanline::
PointBeamIndex(int point_index) const
{
  // Return point beam index
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].BeamIndex();
}



inline int GSVScanline::
PointIdentifier(int point_index) const
{
  // Return point point identifier
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].PointIdentifier();
}



inline int GSVScanline::
PointClusterIdentifier(int point_index) const
{
  // Return point cluster identifier
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].ClusterIdentifier();
}



inline int GSVScanline::
PointCategoryIdentifier(int point_index) const
{
  // Return point category identifier
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].CategoryIdentifier();
}



inline RNScalar GSVScanline::
PointCategoryConfidence(int point_index) const
{
  // Return point category identifier
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].CategoryConfidence();
}



inline unsigned char GSVScanline::
PointFlags(int point_index) const
{
  // Return point flags
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  return points[point_index].Flags();
}



inline void GSVScanline::
SetTimestamp(RNScalar timestamp)
{
  // Set timestamp
  this->timestamp = timestamp;
}



inline void GSVScanline::
SetSweepIndex(int index)
{
  // Set sweep index
  this->sweep_index = index;
}



inline void GSVScanline::
SetBBox(const R3Box& box)
{
  // Set bbox
  this->bbox = box;
}



inline void GSVScanline::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



inline RNBoolean GSVScanline::
ArePointsResident(void) const
{
  // Return whether points are resident
  return (points) ? TRUE: FALSE;
}



inline unsigned long long GSVScanline::
FileOffset(void) const
{
  // Return offset of scanline points in GSV file
  return file_offset;
}



inline void GSVScanline::
SetFileOffset(unsigned long long file_offset)
{
  // Set offset of scanline points in GSV file
  this->file_offset = file_offset;
}



inline void GSVScanline::
SetNPoints(int npoints)
{
  // Set number of points
  this->npoints = npoints;
}



inline void GSVScanline::
SetPointPosition(int point_index, const R3Point& position)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetPosition(position);

  // Update bounding box
  bbox.Union(position);
}



inline void GSVScanline::
SetPointPositionCoords(int point_index, const double position[3])
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetPositionCoords(position);

  // Update bounding box
  bbox.Union(R3Point(position[0], position[1], position[2]));
}



inline void GSVScanline::
SetPointNormal(int point_index, const R3Vector& normal)
{
  // Set point normal
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetNormal(normal);
}



inline void GSVScanline::
SetPointNormalCoords(int point_index, const float normal[3])
{
  // Set point normal
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetNormalCoords(normal);
}



inline void GSVScanline::
SetPointTangent(int point_index, const R3Vector& tangent)
{
  // Set point tangent
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetTangent(tangent);
}



inline void GSVScanline::
SetPointTangentCoords(int point_index, const float tangent[3])
{
  // Set point tangent
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetTangentCoords(tangent);
}



inline void GSVScanline::
SetPointRadius(int point_index, int k, RNLength radius)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetRadius(k, radius);
}



inline void GSVScanline::
SetPointReturnType(int point_index, int return_type)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetReturnType(return_type);
}



inline void GSVScanline::
SetPointTimestamp(int point_index, RNScalar timestamp)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetTimestamp(timestamp);
}



inline void GSVScanline::
SetPointElevation(int point_index, RNScalar elevation)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetElevation(elevation);
}



inline void GSVScanline::
SetPointColor(int point_index, const RNRgb& color)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetColor(color);
}



inline void GSVScanline::
SetPointReflectivity(int point_index, int reflectivity)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetReflectivity(reflectivity);
}



inline void GSVScanline::
SetPointBeamIndex(int point_index, int beam_index)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetBeamIndex(beam_index);
}



inline void GSVScanline::
SetPointIdentifier(int point_index, int point_identifier)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetPointIdentifier(point_identifier);
}



inline void GSVScanline::
SetPointClusterIdentifier(int point_index, int cluster_identifier)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetClusterIdentifier(cluster_identifier);
}



inline void GSVScanline::
SetPointCategoryIdentifier(int point_index, int category_identifier)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetCategoryIdentifier(category_identifier);
}



inline void GSVScanline::
SetPointCategoryConfidence(int point_index, RNScalar category_confidence)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetCategoryConfidence(category_confidence);
}



inline void GSVScanline::
SetPointFlags(int point_index, unsigned char flags)
{
  // Set point position
  assert(points);
  assert((point_index >= 0) && (point_index < npoints));
  points[point_index].SetFlags(flags);
}



// End namespace
}


// End include guard
#endif
