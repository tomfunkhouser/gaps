// Include file for the GSV tapestry class 
#ifndef __GSV__TAPESTRY__H__
#define __GSV__TAPESTRY__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVTapestry {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVTapestry(void);
  virtual ~GSVTapestry(void);


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

  // Camera access functions
  GSVCamera *Camera(void) const;
  int CameraIndex(void) const;

  // Image access functions
  int NImages(void) const;
  GSVImage *Image(int image_index) const;
  GSVImage *FindImageBeforeTimestamp(RNScalar timestamp) const;


  ///////////////////////////
  //// PROPERTY FUNCTIONS ////
  ///////////////////////////

  // Spatial property functions
  const R3Box& BBox(void) const;

  // Pose property functions
  GSVPose Pose(RNScalar timestamp) const;

  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Image manipulation
  void InsertImage(GSVImage *image);
  void RemoveImage(GSVImage *image);

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
  // Update functions
  void UpdateBBox(void);
  void InvalidateBBox(void);

protected:
  // Internal support functions
  GSVImage *FindImageBeforeTimestamp(RNScalar timestamp, int imin, int imax) const;

protected:
  friend class GSVSegment;
  friend class GSVCamera;
  GSVSegment *segment;
  int segment_index;
  GSVCamera *camera;
  int camera_index;
  RNArray<GSVImage *> images;
  R3Box bbox;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVSegment *GSVTapestry::
Segment(void) const
{
  // Return segment
  return segment;
}



inline int GSVTapestry::
SegmentIndex(void) const
{
  // Return index of this tapestry in segment
  return segment_index;
}



inline GSVCamera *GSVTapestry::
Camera(void) const
{
  // Return camera
  return camera;
}



inline int GSVTapestry::
CameraIndex(void) const
{
  // Return index of this tapestry in camera
  return camera_index;
}



inline int GSVTapestry::
NImages(void) const
{
  // Return number of images
  return images.NEntries();
}



inline GSVImage *GSVTapestry::
Image(int image_index) const
{
  // Return kth image
  return images.Kth(image_index);
}



inline const R3Box& GSVTapestry::
BBox(void) const
{
  // Return bounding box
  if (bbox.XMin() == FLT_MAX) 
    ((GSVTapestry *) this)->UpdateBBox();
  return bbox;
}



inline void *GSVTapestry::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVTapestry::
SetBBox(const R3Box& box)
{
  // Set bbox
  this->bbox = box;
}



inline void GSVTapestry::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}


// End include guard
#endif
