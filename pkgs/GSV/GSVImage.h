// Include file for the GSV image class 
#ifndef __GSV__IMAGE__H__
#define __GSV__IMAGE__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVImage {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVImage(const char *name = NULL);
  virtual ~GSVImage(void);


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

  // Tapestry access functions
  GSVTapestry *Tapestry(void) const;
  int TapestryIndex(void) const;

  // Panorama access functions
  GSVPanorama *Panorama(void) const;
  int PanoramaIndex(void) const;

  // Camera access functions
  GSVCamera *Camera(void) const;


  //////////////////////////
  //// PROPERTY FUNCTIONS ////
  //////////////////////////

  // Name property
  const char *Name(void) const;

  // Intrinsic camera properties
  RNAngle XFov(void) const;
  RNAngle YFov(void) const;
  int Width(void) const;
  int Height(void) const;

  // Image properties
  R2Image *DistortedImage(void) const;
  R2Image *UndistortedImage(void) const;

  // Extrinsic camera properties
  GSVPose Pose(int column_index = -1) const;

  // Timing properties
  RNScalar Timestamp(int column_index = -1) const;

  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Set name
  void SetName(const char *name);

  // Set extrinsic camera properties
  void SetPose(const GSVPose& pose0, const GSVPose& pose1);

  // Set timestamp
  void SetTimestamp(RNScalar timestamp0, RNScalar timestamp1);

  // Set user data 
  void SetData(void *data);
  

  ///////////////////////////
  //// MAPPING FUNCTIONS ////
  ///////////////////////////

  // Mapping between world, undistorted, and distorted positions
  R2Point UndistortedPosition(const R2Point& distorted_image_position) const;
  R2Point DistortedPosition(const R2Point& undistorted_image_position) const;
  R2Point UndistortedPosition(const R3Point& world_position) const;
  R2Point DistortedPosition(const R3Point& world_position) const;

  // Mapping between rays and image positions
  R3Ray RayThroughUndistortedPosition(const R2Point& undistorted_image_position) const;
  R3Ray RayThroughDistortedPosition(const R2Point& distorted_image_position) const;


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
  // Only use this if you know what you are doing
  R2Point UndistortedPosition(const R3Point& world_position, int column_index) const;

public:
  // Update functions
  void Update(void);

protected:
  friend class GSVTapestry;
  friend class GSVPanorama;
  GSVTapestry *tapestry;
  int tapestry_index;
  GSVPanorama *panorama;
  int panorama_index;
  GSVPose pose0, pose1;
  RNScalar timestamp0, timestamp1;
  char *name;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVTapestry *GSVImage::
Tapestry(void) const
{
  // Return tapestry
  return tapestry;
}



inline int GSVImage::
TapestryIndex(void) const
{
  // Return index of this image in tapestry
  return tapestry_index;
}



inline GSVPanorama *GSVImage::
Panorama(void) const
{
  // Return panorama
  return panorama;
}



inline int GSVImage::
PanoramaIndex(void) const
{
  // Return index of this image in panorama
  return panorama_index;
}



inline const char *GSVImage::
Name(void) const
{
  // Return name
  return name;
}



inline void *GSVImage::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVImage::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}


// End include guard
#endif
