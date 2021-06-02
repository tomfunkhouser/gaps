// Include file for the GSV laser class 
#ifndef __GSV__LASER__H__
#define __GSV__LASER__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVLaser {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVLaser(int device_type = 0);
  virtual ~GSVLaser(void);


  //////////////////////////
  //// ACCESS FUNCTIONS ////
  //////////////////////////

  // Scene access functions
  GSVScene *Scene(void) const;
  int SceneIndex(void) const;

  // Run access functions
  GSVRun *Run(void) const;
  int RunIndex(void) const;

  // Survey access functions
  int NSurveys(void) const;
  GSVSurvey *Survey(int survey_index) const;


  ///////////////////////////
  //// PROPERTY FUNCTIONS ////
  ///////////////////////////

  // Device type
  int DeviceType(void) const;

  // Sweep ranges
  int MaxSweepIndex(void) const;
  int MaxBeamIndex(void) const;

  // Spatial property functions
  const R3Box& BBox(void) const;

  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Survey manipulation
  void InsertSurvey(GSVSurvey *survey);
  void RemoveSurvey(GSVSurvey *survey);

  // Device type manipulation
  void SetDeviceType(int device_type);

  // Sweep ranges
  void SetMaxSweepIndex(int index);
  void SetMaxBeamIndex(int index);

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
  friend class GSVRun;
  GSVRun *run;
  int run_index;
  RNArray<GSVSurvey *> surveys;
  int device_type;
  int max_sweep_index;
  int max_beam_index;
  R3Box bbox;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// DEVICE TYPES
////////////////////////////////////////////////////////////////////////

enum {
  GSV_UNKNOWN_LASER,
  GSV_VLP16_LASER,
  GSV_SICK_LASER,
  NUM_LASER_TYPES
};


  
////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVRun *GSVLaser::
Run(void) const
{
  // Return run
  return run;
}



inline int GSVLaser::
RunIndex(void) const
{
  // Return index of this laser in run
  return run_index;
}



inline int GSVLaser::
NSurveys(void) const
{
  // Return number of surveys
  return surveys.NEntries();
}



inline GSVSurvey *GSVLaser::
Survey(int survey_index) const
{
  // Return kth survey
  return surveys.Kth(survey_index);
}



inline const R3Box& GSVLaser::
BBox(void) const
{
  // Return bounding box
  if (bbox.XMin() == FLT_MAX) 
    ((GSVLaser *) this)->UpdateBBox();
  return bbox;
}



inline int GSVLaser::
DeviceType(void) const
{
  // Return device type
  return device_type;
}



inline int GSVLaser::
MaxSweepIndex(void) const
{
  // Return max sweep index
  return max_sweep_index;
}



inline int GSVLaser::
MaxBeamIndex(void) const
{
  // Return max beam index
  return max_beam_index;
}



inline void *GSVLaser::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVLaser::
SetDeviceType(int device_type)
{
  // Set device type
  this->device_type = device_type;
}



inline void GSVLaser::
SetMaxSweepIndex(int index)
{
  // Set max sweep index
  this->max_sweep_index = index;
}



inline void GSVLaser::
SetMaxBeamIndex(int index)
{
  // Set max beam index
  this->max_beam_index = index;
}



inline void GSVLaser::
SetBBox(const R3Box& box)
{
  // Set bbox
  this->bbox = box;
}



inline void GSVLaser::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}


// End include guard
#endif
