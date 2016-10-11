/////////////////////////////////////////////////////////////////////////
// Include file for object instance class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectInstance {
public:
  // Constructor destructor
  ObjectInstance(
    const R3Point& origin = R3unknown_point, RNScalar radius = 0, RNScalar height = 0, 
    const char *name = NULL);
  ObjectInstance(const ObjectPointSet& pset,
    const R3Point& origin = R3unknown_point, RNScalar radius = 0, RNScalar height = 0, 
    const char *name = NULL);
  ~ObjectInstance(void);

  // Properties
  const char *Name(void) const;
  const R3Point& Origin(void) const;
  RNLength Radius(void) const;
  RNLength Height(void) const;

  // Parse access
  ObjectParse *Parse(void) const;
  int ParseIndex(void) const;

  // Assignment access
  int NAssignments(void) const;
  ObjectAssignment *Assignment(int k) const;

  // Property manipulation
  void SetName(const char *name);
  void SetOrigin(const R3Point& origin);
  void SetRadius(RNLength radius);
  void SetHeight(RNLength height);

public:
  ObjectPointSet pointset;
  RNArray<ObjectAssignment *> assignments;
  friend class ObjectParse;
  ObjectParse *parse;
  int parse_index;
  R3Point origin;
  RNLength radius;
  RNLength height;
  char *name;
};



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline const char *ObjectInstance::
Name(void) const
{
  // Return name
  return name;
}



inline const R3Point& ObjectInstance::
Origin(void) const
{
  // Return origin
  return origin;
}



inline RNLength ObjectInstance::
Radius(void) const
{
  // Return radius
  return radius;
}



inline RNLength ObjectInstance::
Height(void) const
{
  // Return height
  return height;
}



inline ObjectParse *ObjectInstance::
Parse(void) const
{
  // Return parse
  return parse;
}



inline int ObjectInstance::
ParseIndex(void) const
{
  // Return parse index
  return parse_index;
}



inline int ObjectInstance::
NAssignments(void) const
{
  // Return number of assignments
  return assignments.NEntries();
}



inline ObjectAssignment *ObjectInstance::
Assignment(int k) const
{
  // Return kth assignment
  return assignments.Kth(k);
}



inline void ObjectInstance::
SetName(const char *name) 
{
  // Set name
  if (this->name) free(this->name);
  if (name) this->name = strdup(name);
  else this->name = NULL;
}



inline void ObjectInstance::
SetOrigin(const R3Point& origin)
{
  // Set origin
  this->origin = origin;
}



inline void ObjectInstance::
SetRadius(RNLength radius)
{
  // Set radius
  this->radius = radius;
}



inline void ObjectInstance::
SetHeight(RNLength height)
{
  // Set height
  this->height = height;
}
