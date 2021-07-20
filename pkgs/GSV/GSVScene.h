// Include file for the GSV scene class 
#ifndef __GSV__SCENE__H__
#define __GSV__SCENE__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVScene {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  GSVScene(void);
  virtual ~GSVScene(void);


  //////////////////////////
  //// ACCESS FUNCTIONS ////
  //////////////////////////

  // Run access functions
  int NRuns(void) const;
  GSVRun *Run(int run_index) const;
  GSVRun *Run(const char *name) const;

  // Indirect access functions 
  int NSegments(void) const;
  int NCameras(void) const;
  int NTapestries(void) const;
  int NPanoramas(void) const;
  int NImages(void) const;
  int NLasers(void) const;
  int NSurveys(void) const;
  int NScans(void) const;
  int NScanlines(void) const;
  int NPoints(void) const;


  ////////////////////////////
  //// PROPERTY FUNCTIONS ////
  ////////////////////////////

  // Spatial property functions 
  const R3Box& BBox(void) const;

  // Transformation properties (local to world)
  const R3Affine& Transformation(void) const;

  // Get user data 
  void *Data(void) const;


  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Run manipulation
  void InsertRun(GSVRun *run);
  void RemoveRun(GSVRun *run);

  // Spatial property manipulation
  void SetBBox(const R3Box& box);

  // Transformation manipulation 
  void SetTransformation(const R3Affine& transformation);

  // Set user data 
  void SetData(void *data);
  

  ///////////////////////////
  //// DISPLAY FUNCTIONS ////
  ///////////////////////////

  // Draw function
  virtual void Draw(RNFlags flags = GSV_DEFAULT_DRAW_FLAGS) const;

  // Print function
  virtual void Print(FILE *fp = NULL, const char *prefix = NULL, const char *suffix = NULL) const;


  ///////////////////////
  //// I/O FUNCTIONS ////
  ///////////////////////

  // File read/write functions
  virtual int ReadFile(const char *filename, RNBoolean read_points = TRUE);
  virtual int ReadLaserFile(const char *filename, RNBoolean read_points = TRUE);
  virtual int ReadASCIIFile(const char *filename, RNBoolean read_points = TRUE);
  virtual int ReadGSFFile(const char *filename, RNBoolean read_points = TRUE);
  virtual int ReadGSVFile(const char *filename, RNBoolean read_points = TRUE);
  virtual int WriteFile(const char *filename) const;
  virtual int WriteGSVFile(const char *filename) const;

  // Stream read/write functions
  virtual int ReadGSVStream(FILE *fp, RNBoolean read_points = TRUE);
  virtual int WriteGSVStream(FILE *fp) const;


////////////////////////////////////////////////////////////////////////
// INTERNAL STUFF BELOW HERE
////////////////////////////////////////////////////////////////////////

public:
  // File name functions
  const char *Filename(void) const;

  // Data direcory functions
  const char *RawDataDirectoryName(void) const;
  void SetRawDataDirectoryName(const char *directory_name);
  const char *CacheDataDirectoryName(void) const;
  void SetCacheDataDirectoryName(const char *directory_name);
  int CreateCacheDataDirectory(const char *subdirectory = NULL, GSVRun *run = NULL) const;

  // Update functions
  void UpdateBBox(void);
  void InvalidateBBox(void);
  void UpdateNormals(void);
  void InvalidateNormals(void);
  void UpdatePointIdentifiers(void);

  // Backward compatibility for reading old versions
  int GetGSVMajorVersion(const char *filename);
  int ReadGSVStreamFromOldVersion(FILE *fp, int read_points = TRUE);
  int ReadScanlinePointsFromOldVersion(FILE *fp, GSVScanline *scanline);
  
protected:
  RNArray<GSVRun *> runs;
  char *filename;
  char *raw_data_directory_name;
  char *cache_data_directory_name;
  R3Box bbox;
  R3Affine transformation;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline int GSVScene::
NRuns(void) const
{
  // Return number of runs
  return runs.NEntries();
}



inline GSVRun *GSVScene::
Run(int run_index) const
{
  // Return kth run
  return runs.Kth(run_index);
}



inline const R3Box& GSVScene::
BBox(void) const
{
  // Return bounding box 
  if (bbox.XMin() == FLT_MAX) 
    ((GSVScene *) this)->UpdateBBox();
  return bbox;
}



inline const R3Affine& GSVScene::
Transformation(void) const
{
  // Return transformation
  return transformation;
}



inline void *GSVScene::
Data(void) const
{
  // Return user data
  return data;
}



inline void GSVScene::
SetBBox(const R3Box& box)
{
  // Set bbox
  this->bbox = box;
}



inline void GSVScene::
SetTransformation(const R3Affine& transformation)
{
  // Set transformation
  this->transformation = transformation;
}



inline void GSVScene::
SetData(void *data)
{
  // Set user data
  this->data = data;
}



inline const char *GSVScene::
Filename(void) const
{
  // Return name of input file
  return filename;
}



inline const char *GSVScene::
RawDataDirectoryName(void) const
{
  // Return name of directory with rawd data
  return raw_data_directory_name ;
}



inline const char *GSVScene::
CacheDataDirectoryName(void) const
{
  // Return name of directory with cached data
  return cache_data_directory_name ;
}



// End namespace
}


// End include guard
#endif
