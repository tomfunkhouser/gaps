// Include file for the GSV camera class 
#ifndef __GSV__CAMERA__H__
#define __GSV__CAMERA__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVCamera {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVCamera(void);
  virtual ~GSVCamera(void);


  //////////////////////////
  //// ACCESS FUNCTIONS ////
  //////////////////////////

  // Scene access functions
  GSVScene *Scene(void) const;
  int SceneIndex(void) const;

  // Run access functions
  GSVRun *Run(void) const;
  int RunIndex(void) const;

  // Camera tapestry access functions
  int NTapestries(void) const;
  GSVTapestry *Tapestry(int tapestry_index) const;


  ///////////////////////////
  //// PROPERTY FUNCTIONS ////
  ///////////////////////////

  // Intrinsic camera property functions
  int DistortionType(void) const;
  RNLength XFocal(void) const;
  RNLength YFocal(void) const;
  RNCoord XCenter(void) const;
  RNCoord YCenter(void) const;
  RNScalar K1(void) const;
  RNScalar K2(void) const;
  RNScalar K3(void) const;
  RNScalar P1(void) const;
  RNScalar P2(void) const;
  RNAngle MaxFov(void) const;
  RNAngle XFov(void) const;
  RNAngle YFov(void) const;
  int ImageWidth(void) const;
  int ImageHeight(void) const;

  // Spatial property functions
  const R3Box& BBox(void) const;

  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Camera tapestry manipulation
  void InsertTapestry(GSVTapestry *tapestry);
  void RemoveTapestry(GSVTapestry *tapestry);

  // Intrinsic property manipulation
  void SetDistortionType(int distortion_type);
  void SetXFocal(RNLength focal_x);
  void SetYFocal(RNLength focal_y);
  void SetXCenter(RNCoord center_x);
  void SetYCenter(RNCoord center_y);
  void SetK1(RNScalar k1);
  void SetK2(RNScalar k2);
  void SetK3(RNScalar k3);
  void SetP1(RNScalar p1);
  void SetP2(RNScalar p2);
  void SetMaxFov(RNAngle fov_max);
  void SetImageWidth(int width);
  void SetImageHeight(int height);

  // Spatial property manipulation
  void SetBBox(const R3Box& box);

  // Set user data 
  void SetData(void *data);
  

  ///////////////////////////
  //// MAPPING FUNCTIONS ////
  ///////////////////////////

  // Mapping between undistorted and distorted positions (account for radial distortion)
  R2Point DistortedPosition(const R2Point& undistorted_image_position) const;
  R2Point UndistortedPosition(const R2Point& distorted_image_position) const;

  // Warping between undistorted and distorted images (account for radial distortion)
  int UndistortImage(const R2Image& input, R2Image& output) const;
  int DistortImage(const R2Image& input, R2Image& output) const;


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

public:
  // Internal mapping function
  R2Point DistortedPositionFromUndistortedNormalizedPosition(RNAngle x_dir, RNAngle y_dir) const;

  // For backward compatibility
  RNLength FocalLength(void) const;

protected:
  friend class GSVRun;
  GSVRun *run;
  int run_index;
  RNArray<GSVTapestry *> tapestries;
  int distortion_type;
  int width, height;
  RNLength focal_x, focal_y;
  RNCoord center_x, center_y;
  RNScalar k1, k2, k3;
  RNScalar p1, p2;
  RNAngle fov_max;
  R3Box bbox;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVRun *GSVCamera::
Run(void) const
{
  // Return run
  return run;
}



inline int GSVCamera::
RunIndex(void) const
{
  // Return index of this camera in run
  return run_index;
}



inline int GSVCamera::
NTapestries(void) const
{
  // Return number of camera tapestries
  return tapestries.NEntries();
}



inline GSVTapestry *GSVCamera::
Tapestry(int tapestry_index) const
{
  // Return kth camera tapestry
  return tapestries.Kth(tapestry_index);
}



inline RNLength GSVCamera::
XFocal(void) const
{
  // Return focal X
  return focal_x;
}



inline RNLength GSVCamera::
YFocal(void) const
{
  // Return focal Y
  return focal_y;
}



inline RNLength GSVCamera::
FocalLength(void) const
{
  // Return focal length
  return 0.5 * (focal_x + focal_y);
}



inline RNCoord GSVCamera::
XCenter(void) const
{
  // Return center pixel X
  return center_x;
}



inline RNCoord GSVCamera::
YCenter(void) const
{
  // Return center pixel Y
  return center_y;
}



inline int GSVCamera::
DistortionType(void) const
{
  // Return distortion type
  return distortion_type;
}



inline RNScalar GSVCamera::
K1(void) const
{
  // Return k1 parameter used for rectifying image distortion
  return k1;
}



inline RNScalar GSVCamera::
K2(void) const
{
  // Return k2 parameter used for rectifying image distortion
  return k2;
}



inline RNScalar GSVCamera::
K3(void) const
{
  // Return k3 parameter used for rectifying image distortion
  return k3;
}



inline RNScalar GSVCamera::
P1(void) const
{
  // Return p1 parameter used for rectifying image distortion
  return p1;
}



inline RNScalar GSVCamera::
P2(void) const
{
  // Return p2 parameter used for rectifying image distortion
  return p2;
}



inline RNAngle GSVCamera::
MaxFov(void) const
{
  // Return maximum field of view
  return fov_max;
}



inline int GSVCamera::
ImageWidth(void) const
{
  // Return width of images taken with this camera

  // For backward compatibility with R7
  if (width < 0) return 1936;

  // Return width
  return width;
}



inline int GSVCamera::
ImageHeight(void) const
{
  // Return height of images taken with this camera

  // For backward compatibility with R7
  if (height < 0) return 2592;

  // Return height
  return height;
}



inline RNAngle GSVCamera::
XFov(void) const
{
  // Return field of view in horizontal direction
  RNLength focal_length = FocalLength();
  if (RNIsZero(focal_length)) return 0;
  // if (distortion_type != 0) return 0; // XXX TEMPORARY XXX
  return 2.0 * atan(0.5 * ImageWidth() / focal_length);
}



inline RNAngle GSVCamera::
YFov(void) const
{
  // Return field of view in vertical direction
  RNLength focal_length = FocalLength();
  if (RNIsZero(focal_length)) return 0;
  // if (distortion_type != 0) return 0; // XXX TEMPORARY XXX
  return 2.0 * atan(0.5 * ImageHeight() / focal_length);
}



inline const R3Box& GSVCamera::
BBox(void) const
{
  // Return bounding box 
  if (bbox.XMin() == FLT_MAX) 
    ((GSVCamera *) this)->UpdateBBox();
  return bbox;
}



inline void *GSVCamera::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVCamera::
SetBBox(const R3Box& box)
{
  // Set bbox
  this->bbox = box;
}



inline void GSVCamera::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}


// End include guard
#endif
