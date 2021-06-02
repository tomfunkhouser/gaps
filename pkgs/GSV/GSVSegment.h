// Include file for the GSV segment class 
#ifndef __GSV__SEGMENT__H__
#define __GSV__SEGMENT__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVSegment {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVSegment(const char *name = NULL);
  virtual ~GSVSegment(void);


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

  // Scan access functions
  int NScans(void) const;
  GSVScan *Scan(int scan_index) const;

  // Scanline access functions (indirect - i.e., slow) 
  int NScanlines(void) const;
  GSVScanline *Scanline(int scanline_index) const;

  // Tapestry access functions
  int NTapestries(void) const;
  GSVTapestry *Tapestry(int tapestry_index) const;

  // Panorama access functions
  int NPanoramas(void) const;
  GSVPanorama *Panorama(int panorama_index) const;
  GSVPanorama *Panorama(const char *name) const;
  GSVPanorama *FindPanoramaBeforeTimestamp(RNScalar timestamp) const;

  // Image access functions (indirect - i.e., slow) 
  int NImages(void) const;
  GSVImage *Image(int image_index) const;


  //////////////////////////
  //// PROPERTY FUNCTIONS ////
  //////////////////////////

  // Name property
  const char *Name(void) const;
  
  // Pose property functions
  R3Point Viewpoint(RNScalar timestamp) const;
  R3Vector Front(RNScalar timestamp) const;
  R3Vector Right(RNScalar timestamp) const;
  R3Vector Up(RNScalar timestamp) const;
  
  // Spatial property functions
  const R3Box& BBox(void) const;

  // Statistics functions
  int NPoints(void) const;
  
  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Survey manipulation functions
  void InsertSurvey(GSVSurvey *survey);
  void RemoveSurvey(GSVSurvey *survey);

  // Tapestry manipulation functions
  void InsertTapestry(GSVTapestry *tapestry);
  void RemoveTapestry(GSVTapestry *tapestry);

  // Panorama manipulation functions
  void InsertPanorama(GSVPanorama *panorama);
  void RemovePanorama(GSVPanorama *panorama);

  // Name manipulation
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

public:
  // Update functions
  void UpdateBBox(void);
  void InvalidateBBox(void);
  void UpdateNormals(void);
  void InvalidateNormals(void);

protected:
  // Internal support functions
  GSVPanorama *FindPanoramaBeforeTimestamp(RNScalar timestamp, int imin, int imax) const;

protected:
  friend class GSVScene;
  friend class GSVRun;
  int scene_index;
  GSVRun *run;
  int run_index;
  RNArray<GSVSurvey *> surveys;
  RNArray<GSVTapestry *> tapestries;
  RNArray<GSVPanorama *> panoramas;
  R3Box bbox;
  char *name;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline GSVRun *GSVSegment::
Run(void) const
{
  // Return run
  return run;
}



inline int GSVSegment::
RunIndex(void) const
{
  // Return index of this segment in run
  return run_index;
}



inline int GSVSegment::
NSurveys(void) const
{
  // Return number of surveys
  return surveys.NEntries();
}



inline GSVSurvey *GSVSegment::
Survey(int survey_index) const
{
  // Return kth survey
  return surveys.Kth(survey_index);
}



inline int GSVSegment::
NTapestries(void) const
{
  // Return number of tapestries
  return tapestries.NEntries();
}



inline GSVTapestry *GSVSegment::
Tapestry(int tapestry_index) const
{
  // Return kth tapestry
  return tapestries.Kth(tapestry_index);
}



inline int GSVSegment::
NPanoramas(void) const
{
  // Return number of panoramas
  return panoramas.NEntries();
}



inline GSVPanorama *GSVSegment::
Panorama(int panorama_index) const
{
  // Return kth panorama
  return panoramas.Kth(panorama_index);
}



inline R3Vector GSVSegment::
Up(RNScalar timestamp) const
{
  // Up is always positive z
  return R3posz_vector;
}


  
inline const char *GSVSegment::
Name(void) const
{
  // Return name
  return name;
}



inline const R3Box& GSVSegment::
BBox(void) const
{
  // Return bounding box
  if (bbox.XMin() == FLT_MAX) 
    ((GSVSegment *) this)->UpdateBBox();
  return bbox;
}



inline void *GSVSegment::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVSegment::
SetBBox(const R3Box& box)
{
  // Set bbox
  this->bbox = box;
}



inline void GSVSegment::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



// End namespace
}


// End include guard
#endif
