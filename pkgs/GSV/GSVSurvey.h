// Include file for the GSV laser class 
#ifndef __GSV__SURVEY__H__
#define __GSV__SURVEY__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVSurvey {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVSurvey(void);
  virtual ~GSVSurvey(void);


  //////////////////////////
  //// ACCESS FUNCTIONS ////
  //////////////////////////

  // Scene access functions
  GSVScene *Scene(void) const;
  int SceneIndex(void) const;

  // Run access functions
  GSVRun *Run(void) const;
  int RunIndex(void) const;

  // Segment access functions
  GSVSegment *Segment(void) const;
  int SegmentIndex(void) const;

  // Laser access functions
  GSVLaser *Laser(void) const;
  int LaserIndex(void) const;

  // Scan access functions
  int NScans(void) const;
  GSVScan *Scan(int scan_index) const;
  GSVScan *FindScanBeforeTimestamp(RNScalar timestamp) const;
  GSVScan *FindScanBeforeTravelDistance(RNScalar travel_distance) const;

  // Scanline access functions
  int NScanlines(void) const;
  GSVScanline *Scanline(int scanline_index) const;


  ///////////////////////////
  //// PROPERTY FUNCTIONS ////
  ///////////////////////////

  // Spatial property functions
  const R3Box& BBox(void) const;

  // Statistics functions
  int NPoints(void) const;
  int MaxBeamIndex(void) const;

  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Scan manipulation
  void InsertScan(GSVScan *scan);
  void RemoveScan(GSVScan *scan);

  // Spatial property manipulation
  void SetBBox(const R3Box& box);

  // Set user data 
  void SetData(void *data);
  

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

public:
  // Travel distances
  RNLength TravelDistance(int scan_index) const;

  // Update functions
  void UpdateBBox(void);
  void InvalidateBBox(void);
  void InvalidateTravelDistances(void);
  void UpdateTravelDistances(void);

protected:
  // Internal support functions
  GSVScan *FindScanBeforeTimestamp(RNScalar timestamp, int imin, int imax) const;
  GSVScan *FindScanBeforeTravelDistance(RNScalar travel_distance, int imin, int imax) const;

protected:
  friend class GSVSegment;
  friend class GSVLaser;
  GSVSegment *segment;
  int segment_index;
  GSVLaser *laser;
  int laser_index;
  RNArray<GSVScan *> scans;
  RNLength *travel_distances;
  R3Box bbox;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVSegment *GSVSurvey::
Segment(void) const
{
  // Return segment
  return segment;
}



inline int GSVSurvey::
SegmentIndex(void) const
{
  // Return index of this survey in segment
  return segment_index;
}



inline GSVLaser *GSVSurvey::
Laser(void) const
{
  // Return laser
  return laser;
}



inline int GSVSurvey::
LaserIndex(void) const
{
  // Return index of this survey in laser
  return laser_index;
}



inline int GSVSurvey::
NScans(void) const
{
  // Return number of scans
  return scans.NEntries();
}



inline GSVScan *GSVSurvey::
Scan(int scan_index) const
{
  // Return kth scan
  return scans.Kth(scan_index);
}



inline const R3Box& GSVSurvey::
BBox(void) const
{
  // Return bounding box
  if (bbox.XMin() == FLT_MAX) 
    ((GSVSurvey *) this)->UpdateBBox();
  return bbox;
}



inline RNLength GSVSurvey::
TravelDistance(int scan_index) const
{
  // Return distance car laser viewpoint traveled before arriving at scan
  assert((scan_index >= 0) && (scan_index < NScans()));
  if (!travel_distances) ((GSVSurvey *) this)->UpdateTravelDistances();
  return travel_distances[scan_index];
}



inline void *GSVSurvey::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVSurvey::
SetBBox(const R3Box& box)
{
  // Set bbox
  this->bbox = box;
}



inline void GSVSurvey::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}



// End include guard
#endif
