/////////////////////////////////////////////////////////////////////////
// Include file for object detection class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectDetection {
public:
  ////////////////////////////////////////
  // Constructor/destructor functions
  ////////////////////////////////////////

  // Constructor destructor
  ObjectDetection(const char *name = NULL, const R3Point& origin = R3unknown_point, 
    RNScalar max_radius = RN_UNKNOWN, RNScalar max_height = RN_UNKNOWN);
  ~ObjectDetection(void);


  ////////////////////////////////////////
  // Access functions
  ////////////////////////////////////////

  // Parse access
  ObjectParse *Parse(void) const;
  int ParseIndex(void) const;

  // Segmentation access
  int NSegmentations(void) const;
  ObjectSegmentation *Segmentation(int k) const;
  ObjectSegmentation *BestSegmentation(void) const;
  ObjectSegmentation *GroundTruthSegmentation(void) const;

  // Label access
  ObjectLabel *BestLabel(void) const;
  ObjectLabel *GroundTruthLabel(void) const;

  // Model access
  ObjectModel *BestModel(void) const;
  ObjectModel *GroundTruthModel(void) const;

  // Assignment access
  ObjectAssignment *BestAssignment(void) const;
  ObjectAssignment *BestAssignment(ObjectLabel *label) const;
  ObjectAssignment *BestAssignment(ObjectModel *model) const;
  ObjectAssignment *GroundTruthAssignment(void) const;

  // Property access
  const char *Name(void) const;
  const R3Point& Origin(void) const;
  RNLength MaxRadius(void) const;
  RNLength MaxHeight(void) const;


  ////////////////////////////////////////
  // Manipulation functions
  ////////////////////////////////////////

  // Segmentation manipulation
  void InsertSegmentation(ObjectSegmentation *segmentation);
  void RemoveSegmentation(ObjectSegmentation *segmentation);
  void SetGroundTruthSegmentation(ObjectSegmentation *segmentation);

  // Property manipulation
  void SetName(const char *name);
  void SetOrigin(const R3Point& origin);
  void SetMaxRadius(RNLength radius);
  void SetMaxHeight(RNLength height);


  ////////////////////////////////////////
  // Internal stuff
  ////////////////////////////////////////

public:
  friend class ObjectParse;
  ObjectParse *parse;
  int parse_index;
  RNArray<ObjectSegmentation *> segmentations;
  ObjectSegmentation *ground_truth_segmentation;
  char *name;
  R3Point origin;
  RNLength max_radius;
  RNLength max_height;
};



////////////////////////////////////////////////////////////////////////
// Extern functions (used for sorting)
////////////////////////////////////////////////////////////////////////

extern int CompareObjectDetections(const void *data1, const void *data2);



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline ObjectParse *ObjectDetection::
Parse(void) const
{
  // Return parse
  return parse;
}



inline int ObjectDetection::
ParseIndex(void) const
{
  // Return parse index
  return parse_index;
}



inline int ObjectDetection::
NSegmentations(void) const
{
  // Return number of segmentations
  return segmentations.NEntries();
}



inline ObjectSegmentation *ObjectDetection::
Segmentation(int k) const
{
  // Return kth segmentation
  return segmentations.Kth(k);
}



inline ObjectSegmentation *ObjectDetection::
GroundTruthSegmentation(void) const
{
  // Return ground truth segmentation
  return ground_truth_segmentation;
}



inline const char *ObjectDetection::
Name(void) const
{
  // Return name
  return name;
}



inline const R3Point& ObjectDetection::
Origin(void) const
{
  // Return origin
  return origin;
}



inline RNLength ObjectDetection::
MaxRadius(void) const
{
  // Return maximum radius
  return max_radius;
}



inline RNLength ObjectDetection::
MaxHeight(void) const
{
  // Return maximum height
  return max_height;
}



