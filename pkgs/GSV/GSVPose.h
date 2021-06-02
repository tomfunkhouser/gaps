// Include file for the GSV pose class 
#ifndef __GSV__POSE__H__
#define __GSV__POSE__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVPose {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVPose(void);
  GSVPose(const R3Point& viewpoint, const R3Quaternion& orientation);
  GSVPose(const R3Point& viewpoint, const R3Triad& triad);
  GSVPose(const R3Point& viewpoint, const R3Vector& towards, const R3Vector& up);
  GSVPose(double px, double py, double pz, double qx, double qy, double qz, double qw);
  virtual ~GSVPose(void);


  ///////////////////////////
  //// PROPERTY FUNCTIONS ////
  ///////////////////////////

  // Pose property functions
  const R3Point& Viewpoint(void) const;
  const R3Quaternion& Orientation(void) const;
  const R3Vector Towards(void) const;
  const R3Vector Up(void) const;
  const R3Vector Right(void) const;
  const R4Matrix Matrix(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Camera property manipulation
  void SetViewpoint(const R3Point& viewpoint);
  void SetOrientation(const R3Quaternion& orientation);
  void Translate(const R3Vector& translation);
  void Rotate(const R3Quaternion& rotation);
  void Rotate(const R3Vector& xyz_angles);
  void Transform(const R3Affine& affine);


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

protected:
  R3Point viewpoint;
  R3Quaternion orientation;
};



////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS ////
////////////////////////////////////////////////////////////////////////

GSVPose GSVInterpolatedPose(const GSVPose& pose0, const GSVPose& pose1, RNScalar t);



////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
////////////////////////////////////////////////////////////////////////

extern GSVPose GSVnull_pose;



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline const R3Point& GSVPose::
Viewpoint(void) const
{
  // Return viewpoint
  return viewpoint;
}



inline const R3Quaternion& GSVPose::
Orientation(void) const
{
  // Return orientation
  return orientation;
}



// End namespace
}


// End include guard
#endif
