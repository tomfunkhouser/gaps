/////////////////////////////////////////////////////////////////////////
// Include file for object model class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectModel {
public:
  ////////////////////////////////////////
  // Constructor/destructor functions
  ////////////////////////////////////////

  // Constructor destructor
  ObjectModel(const char *filename = NULL,
    const R3Point& origin = R3unknown_point, RNScalar radius = RN_UNKNOWN, RNScalar height = RN_UNKNOWN);
  ObjectModel(const ObjectPointSet& pointset, 
    const R3Point& origin = R3unknown_point, RNScalar radius = RN_UNKNOWN, RNScalar height = RN_UNKNOWN);
  ~ObjectModel(void);


  ////////////////////////////////////////
  // Access functions
  ////////////////////////////////////////

  // Parse access
  ObjectParse *Parse(void) const;
  int ParseIndex(void) const;

  // Label access
  ObjectLabel *Label(void) const;
  int LabelIndex(void) const;

  // Assignment access
  int NAssignments(void) const;
  ObjectAssignment *Assignment(int k) const;
  ObjectAssignment *Assignment(const ObjectSegmentation *segmentation) const;

  // Property access
  const char *Filename(void) const;
  const ObjectPointSet& PointSet(void) const;
  const R3Point& Origin(void) const;
  RNLength Radius(void) const;
  RNLength Height(void) const;


  ////////////////////////////////////////
  // Manipulation functions
  ////////////////////////////////////////

  // Property manipulation
  void SetPointSet(const ObjectPointSet& pointset);
  void SetOrigin(const R3Point& origin);
  void SetRadius(RNLength radius);
  void SetHeight(RNLength height);


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
  friend class ObjectLabel;
  ObjectLabel *label;
  int label_index;
  RNArray<ObjectAssignment *> assignments;
  char *filename;
  ObjectPointSet pointset;
  R3Point origin;
  RNLength radius;
  RNLength height;
};



////////////////////////////////////////////////////////////////////////
// Extern functions (used for sorting)
////////////////////////////////////////////////////////////////////////

extern int CompareObjectModels(const void *data1, const void *data2);



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline ObjectParse *ObjectModel::
Parse(void) const
{
  // Return parse
  return parse;
}



inline int ObjectModel::
ParseIndex(void) const
{
  // Return parse index
  return parse_index;
}



inline ObjectLabel *ObjectModel::
Label(void) const
{
  // Return label
  return label;
}



inline int ObjectModel::
LabelIndex(void) const
{
  // Return label index
  return label_index;
}



inline int ObjectModel::
NAssignments(void) const
{
  // Return number of assignments
  return assignments.NEntries();
}



inline ObjectAssignment *ObjectModel::
Assignment(int k) const
{
  // Return kth assignment
  return assignments.Kth(k);
}



inline const char *ObjectModel::
Filename(void) const
{
  // Return filename
  return filename;
}



inline const ObjectPointSet& ObjectModel::
PointSet(void) const
{
  // Return point set
  return pointset;
}



inline const R3Point& ObjectModel::
Origin(void) const
{
  // Return origin
  return origin;
}



inline RNLength ObjectModel::
Radius(void) const
{
  // Return radius
  return radius;
}



inline RNLength ObjectModel::
Height(void) const
{
  // Return height
  return height;
}



