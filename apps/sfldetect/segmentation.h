/////////////////////////////////////////////////////////////////////////
// Include file for object segmentation class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectSegmentation {
public:
  ////////////////////////////////////////
  // Constructor/destructor functions
  ////////////////////////////////////////

  // Constructor destructor
  ObjectSegmentation(const char *filename = NULL, const R3Point& origin = R3unknown_point, 
    RNScalar radius = RN_UNKNOWN, RNScalar height = RN_UNKNOWN, RNScalar score = RN_UNKNOWN);
  ObjectSegmentation(const ObjectPointSet& pointset, const R3Point& origin = R3unknown_point, 
    RNScalar radius = RN_UNKNOWN, RNScalar height = RN_UNKNOWN, RNScalar score = RN_UNKNOWN);
  ObjectSegmentation(const ObjectSegmentation& segmentation);
  ~ObjectSegmentation(void);


  ////////////////////////////////////////
  // Access functions
  ////////////////////////////////////////

  // Parse access
  ObjectParse *Parse(void) const;
  int ParseIndex(void) const;

  // Detection access
  ObjectDetection *Detection(void) const;
  int DetectionIndex(void) const;

  // Label access
  ObjectLabel *BestLabel(void) const;
  ObjectLabel *GroundTruthLabel(void) const;

  // Model access
  ObjectModel *BestModel(void) const;
  ObjectModel *GroundTruthModel(void) const;

  // Assignment access
  int NAssignments(void) const;
  ObjectAssignment *Assignment(int k) const;
  ObjectAssignment *BestAssignment(void) const;
  ObjectAssignment *BestAssignment(const ObjectLabel *label) const;
  ObjectAssignment *BestAssignment(const ObjectModel *model) const;
  ObjectAssignment *GroundTruthAssignment(void) const;

  // Property access
  const char *Filename(void) const;
  const ObjectPointSet& PointSet(void) const;
  const R3Point& Origin(void) const;
  RNLength Radius(void) const;
  RNLength Height(void) const;
  RNScalar Score(void) const;


  ////////////////////////////////////////
  // Manipulation functions
  ////////////////////////////////////////

  // Property manipulation
  void SetPointSet(const ObjectPointSet& pointset);
  void SetOrigin(const R3Point& origin);
  void SetRadius(RNLength radius);
  void SetHeight(RNLength height);
  void SetScore(RNScalar score);


  ////////////////////////////////////////
  // Internal stuff
  ////////////////////////////////////////

  // Assignment manipulation
  void EmptyAssignments(void);
  void InsertAssignment(ObjectAssignment *assignment);
  void RemoveAssignment(ObjectAssignment *assignment);


public:
  friend class ObjectParse;
  ObjectParse *parse;
  int parse_index;
  friend class ObjectDetection;
  ObjectDetection *detection;
  int detection_index;
  RNArray<ObjectAssignment *> assignments;
  char *filename;
  ObjectPointSet pointset;
  R3Point origin;
  RNLength radius;
  RNLength height;
  RNScalar score;
};



////////////////////////////////////////////////////////////////////////
// Extern functions (used for sorting)
////////////////////////////////////////////////////////////////////////

extern int CompareObjectSegmentations(const void *data1, const void *data2);



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline ObjectParse *ObjectSegmentation::
Parse(void) const
{
  // Return parse
  return parse;
}



inline int ObjectSegmentation::
ParseIndex(void) const
{
  // Return parse index
  return parse_index;
}



inline ObjectDetection *ObjectSegmentation::
Detection(void) const
{
  // Return detection
  return detection;
}



inline int ObjectSegmentation::
DetectionIndex(void) const
{
  // Return detection index
  return detection_index;
}



inline int ObjectSegmentation::
NAssignments(void) const
{
  // Return number of assignments
  return assignments.NEntries();
}



inline ObjectAssignment *ObjectSegmentation::
Assignment(int k) const
{
  // Return kth assignment
  return assignments.Kth(k);
}



inline const char *ObjectSegmentation::
Filename(void) const
{
  // Return filename
  return filename;
}



inline const ObjectPointSet& ObjectSegmentation::
PointSet(void) const
{
  // Return point set
  return pointset;
}



inline const R3Point& ObjectSegmentation::
Origin(void) const
{
  // Return origin
  return origin;
}



inline RNLength ObjectSegmentation::
Radius(void) const
{
  // Return radius
  return radius;
}



inline RNLength ObjectSegmentation::
Height(void) const
{
  // Return height
  return height;
}



inline RNScalar ObjectSegmentation::
Score(void) const
{
  // Return score
  return score;
}



