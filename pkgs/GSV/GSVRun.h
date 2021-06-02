// Include file for the GSV run class 
#ifndef __GSV__RUN__H__
#define __GSV__RUN__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVRun {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVRun(const char *name = NULL);
  virtual ~GSVRun(void);


  //////////////////////////
  //// ACCESS FUNCTIONS ////
  //////////////////////////

  // Scene access functions
  GSVScene *Scene(void) const;
  int SceneIndex(void) const;

  // Segment access functions
  int NSegments(void) const;
  GSVSegment *Segment(int segment_index) const;
  GSVSegment *Segment(const char *name) const;

  // Camera access functions
  int NCameras(void) const;
  GSVCamera *Camera(int index) const;

  // Laser access functions
  int NLasers(void) const;
  GSVLaser *Laser(int laser_index) const;

  // Indirect access functions
  int NTapestries(void) const;
  int NPanoramas(void) const;
  int NImages(void) const;
  int NSurveys(void) const;
  int NScans(void) const;
  int NScanlines(void) const;


  ////////////////////////////
  //// PROPERTY FUNCTIONS ////
  ////////////////////////////

  // Name property functions
  const char *Name(void) const;

  // Spatial property functions
  const R3Box& BBox(void) const;

  // Statistics functions
  int NPoints(void) const;
  
  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Segment manipulation functions
  void InsertSegment(GSVSegment *segment);
  void RemoveSegment(GSVSegment *segment);

  // Camera manipulation functions
  void InsertCamera(GSVCamera *camera);
  void RemoveCamera(GSVCamera *camera);

  // Laser manipulation functions
  void InsertLaser(GSVLaser *laser);
  void RemoveLaser(GSVLaser *laser);

  // Name manipulation functions
  void SetName(const char *name);

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

  // ASCII file reading
  int ReadCameraInfoFile(void);
  int ReadImageSegmentFile(RNArray<GSVPanorama *>& panoramas);
  int ReadImagePoseFile(const RNArray<GSVPanorama *>& panoramas);
  int ReadLaserObjFile(int laser_index, RNArray<GSVScanline *>& scanlines, RNArray<GSVScan *>& scanline_scans, RNBoolean read_points = TRUE);
  int ReadLaserPoseFile(int laser_index, const RNArray<GSVScanline *>& scanlines, RNArray<GSVScan *>& scanline_scans);


public:
  // Update functions
  void UpdateBBox(void);
  void InvalidateBBox(void);
  void UpdateNormals(void);
  void InvalidateNormals(void);

protected:
  friend class GSVScene;
  GSVScene *scene;
  int scene_index;
  RNArray<GSVSegment *> segments;
  RNArray<GSVCamera *> cameras;
  RNArray<GSVLaser *> lasers;
  char *name;
  R3Box bbox;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVScene *GSVRun::
Scene(void) const
{
  // Return scene
  return scene;
}



inline int GSVRun::
SceneIndex(void) const
{
  // Return index of this panaorama in scene
  return scene_index;
}



inline int GSVRun::
NSegments(void) const
{
  // Return number of segments
  return segments.NEntries();
}



inline GSVSegment *GSVRun::
Segment(int segment_index) const
{
  // Return kth segment
  return segments.Kth(segment_index);
}



inline int GSVRun::
NCameras(void) const
{
  // Return number of cameras
  return cameras.NEntries();
}



inline GSVCamera *GSVRun::
Camera(int index) const
{
  // Return kth camera
  return cameras.Kth(index);
}



inline int GSVRun::
NLasers(void) const
{
  // Return number of lasers
  return lasers.NEntries();
}



inline GSVLaser *GSVRun::
Laser(int laser_index) const
{
  // Return kth laser
  return lasers.Kth(laser_index);
}



inline const char *GSVRun::
Name(void) const
{
  // Return name
  return name;
}



inline const R3Box& GSVRun::
BBox(void) const
{
  // Return bounding box
  if (bbox.XMin() == FLT_MAX) 
    ((GSVRun *) this)->UpdateBBox();
  return bbox;
}



inline void *GSVRun::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVRun::
SetBBox(const R3Box& box)
{
  // Union box
  this->bbox = box;
}



inline void GSVRun::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}


// End include guard
#endif
