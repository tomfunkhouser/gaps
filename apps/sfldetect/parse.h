/////////////////////////////////////////////////////////////////////////
// Include file for object parse class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectParse {
public:
  // Constructor destructor
  ObjectParse(const char *model_directory = NULL, const char *segmentation_directory = NULL);
  ~ObjectParse(void);


  ////////////////////////////////////////
  // Access functions
  ////////////////////////////////////////

  // Label access
  int NLabels(void) const;
  ObjectLabel *Label(int k) const;
  ObjectLabel *Label(const char *name) const;

  // Model access
  int NModels(void) const;
  ObjectModel *Model(int k) const;

  // Segmentation access
  int NSegmentations(void) const;
  ObjectSegmentation *Segmentation(int k) const;

  // Detection access
  int NDetections(void) const;
  ObjectDetection *Detection(int k) const;
  ObjectDetection *Detection(const char *name) const;

  // Assignment access
  int NAssignments(void) const;
  ObjectAssignment *Assignment(int k) const;

  // Directory names
  const char *ModelDirectory(void) const;
  const char *SegmentationDirectory(void) const;


  ////////////////////////////////////////
  // Structure manipulation functions
  ////////////////////////////////////////

  // Basic manipulation
  void Sort(void);
  void Empty(void);

  // Label manipulation
  void EmptyLabels(void);
  void InsertLabel(ObjectLabel *label);
  void RemoveLabel(ObjectLabel *label);

  // Model manipulation
  void EmptyModels(void);
  void InsertModel(ObjectModel *model);
  void RemoveModel(ObjectModel *model);

  // Segmentation manipulation
  void EmptySegmentations(void);
  void InsertSegmentation(ObjectSegmentation *segmentation);
  void RemoveSegmentation(ObjectSegmentation *segmentation);

  // Detection manipulation
  void EmptyDetections(void);
  void InsertDetection(ObjectDetection *detection);
  void RemoveDetection(ObjectDetection *detection);

  // Assignment manipulation
  void EmptyAssignments(void);
  void InsertAssignment(ObjectAssignment *assignment);
  void RemoveAssignment(ObjectAssignment *assignment);


  ////////////////////////////////////////
  // Property manpulation functions
  ////////////////////////////////////////

  // Directory manipulation
  void SetModelDirectory(const char *dirname);
  void SetSegmentationDirectory(const char *dirname);


  ////////////////////////////////////////
  // Input/output functions
  ////////////////////////////////////////

  // Input/output
  int ReadFile(const char *filename);
  int ReadAsciiFile(const char *filename);
  int ReadMarkersFile(const char *filename);
  int ReadGSVFeaturesFile(const char *filename);
  int WriteFile(const char *filename) const;
  int WriteAsciiFile(const char *filename) const;
  int WriteMarkersFile(const char *filename) const;
  int WriteArffFile(const char *filename) const;


public:
  RNArray<ObjectLabel *> labels;
  RNArray<ObjectModel *> models;
  RNArray<ObjectSegmentation *> segmentations;
  RNArray<ObjectDetection *> detections;
  RNArray<ObjectAssignment *> assignments;
  char *model_directory;
  char *segmentation_directory;
};



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline int ObjectParse::
NLabels(void) const
{
  // Return number of labels
  return labels.NEntries();
}



inline struct ObjectLabel *ObjectParse::
Label(int k) const
{
  // Return kth label
  return labels.Kth(k);
}



inline int ObjectParse::
NModels(void) const
{
  // Return number of models
  return models.NEntries();
}



inline struct ObjectModel *ObjectParse::
Model(int k) const
{
  // Return kth model
  return models.Kth(k);
}



inline int ObjectParse::
NSegmentations(void) const
{
  // Return number of segmentations
  return segmentations.NEntries();
}



inline struct ObjectSegmentation *ObjectParse::
Segmentation(int k) const
{
  // Return kth segmentation
  return segmentations.Kth(k);
}



inline int ObjectParse::
NDetections(void) const
{
  // Return number of detections
  return detections.NEntries();
}



inline struct ObjectDetection *ObjectParse::
Detection(int k) const
{
  // Return kth detection
  return detections.Kth(k);
}



inline int ObjectParse::
NAssignments(void) const
{
  // Return number of assignments
  return assignments.NEntries();
}



inline ObjectAssignment *ObjectParse::
Assignment(int k) const
{
  // Return kth assignment
  return assignments.Kth(k);
}



inline const char *ObjectParse::
ModelDirectory(void) const
{
  // Return name of directory containing model files
  return model_directory;
}



inline const char *ObjectParse::
SegmentationDirectory(void) const
{
  // Return name of directory containing segmentation files
  return segmentation_directory;
}



