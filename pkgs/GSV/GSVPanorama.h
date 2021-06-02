// Include file for the GSV panorama class 
#ifndef __GSV__PANORAMA__H__
#define __GSV__PANORAMA__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVPanorama {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVPanorama(const char *name = NULL);
  virtual ~GSVPanorama(void);


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

  // Image access functions
  int NImages(void) const;
  GSVImage *Image(int k) const;
  GSVImage *Image(const char *name) const;
  

  ///////////////////////////
  //// PROPERTY FUNCTIONS ////
  ///////////////////////////

  // Get name
  const char *Name(void) const;

  // Pose property functions
  const R3Point& Viewpoint(void) const;
  const R3Vector& Front(void) const;
  const R3Vector& Right(void) const;
  const R3Vector Up(void) const;
  
  // Timestamp property functions
  RNScalar Timestamp(void) const;

  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Camera image manipulation
  void InsertImage(GSVImage *image);
  void RemoveImage(GSVImage *image);

  // Set name
  void SetName(const char *name);
  
  // Extrinsic sensor property functions
  void SetViewpoint(const R3Point& viewpoint);
  void SetFront(const R3Vector& front);
  void SetRight(const R3Vector& right);
  
  // Timing property functions
  void SetTimestamp(RNScalar timestamp);

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
  // Update functions
  void UpdatePose(void);
  void UpdateTimestamp(void);

protected:
  friend class GSVSegment;
  GSVSegment *segment;
  int segment_index;
  RNArray<GSVImage *> images;
  R3Point viewpoint;
  R3Vector front;
  R3Vector right;
  RNScalar timestamp;
  char *name;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVSegment *GSVPanorama::
Segment(void) const
{
  // Return segment
  return segment;
}



inline int GSVPanorama::
SegmentIndex(void) const
{
  // Return index of this panaorama in segment
  return segment_index;
}



inline int GSVPanorama::
NImages(void) const
{
  // Return number of images
  return images.NEntries();
}



inline GSVImage *GSVPanorama::
Image(int k) const
{
  // Return kth image
  return images.Kth(k);
}



inline const R3Point& GSVPanorama::
Viewpoint(void) const
{
  // Return (average) viewpoint
  if (viewpoint.X() == FLT_MAX) 
    ((GSVPanorama *) this)->UpdatePose();
  return viewpoint;
}



inline const R3Vector& GSVPanorama::
Front(void) const
{
  // Return front
  if (front == R3zero_vector)
    ((GSVPanorama *) this)->UpdatePose();
  return front;
}



inline const R3Vector& GSVPanorama::
Right(void) const
{
  // Return right vector
  if (right == R3zero_vector)
    ((GSVPanorama *) this)->UpdatePose();
  return right;
}



inline const R3Vector GSVPanorama::
Up(void) const
{
  // Return up vector 
  return R3posz_vector;
}



inline RNScalar GSVPanorama::
Timestamp(void) const
{
  // Return (average) timestamp 
  if (timestamp == -1) 
    ((GSVPanorama *) this)->UpdateTimestamp();
  return timestamp;
}



inline const char *GSVPanorama::
Name(void) const
{
  // Return name
  return name;
}



inline void *GSVPanorama::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVPanorama::
SetViewpoint(const R3Point& viewpoint)
{
  // Set viewpoint
  this->viewpoint = viewpoint;
}



inline void GSVPanorama::
SetFront(const R3Vector& front)
{
  // Set front
  this->front = front;
}



inline void GSVPanorama::
SetRight(const R3Vector& right)
{
  // Set right
  this->right = right;
}



inline void GSVPanorama::
SetTimestamp(RNScalar timestamp)
{
  // Set timestamp
  this->timestamp = timestamp;
}



inline void GSVPanorama::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}


// End include guard
#endif
