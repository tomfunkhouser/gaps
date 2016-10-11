/////////////////////////////////////////////////////////////////////////
// Include file for object pointset class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Point class definition
////////////////////////////////////////////////////////////////////////

class ObjectDescriptor {
public:
  // Constructor destructor
  ObjectDescriptor(const double *values = NULL, int nvalues = 0);
  ObjectDescriptor(const ObjectDescriptor& descriptor);
  ~ObjectDescriptor(void);

  // Properties
  int NValues(void) const;
  double Value(int k) const;

  // Relationships
  double SquaredDistance(const ObjectDescriptor& descriptor) const;
  double Distance(const ObjectDescriptor& descriptor) const;

  // Property manipulation
  ObjectDescriptor& operator=(const ObjectDescriptor& descriptor);
  void Reset(const double *values = NULL, int nvalues = 0);
  void SetValue(int k, double value);

public:
  int nvalues;
  double *values;
};



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline int ObjectDescriptor::
NValues(void) const
{
  // Return number of values
  return nvalues;
}



inline double ObjectDescriptor::
Value(int k) const
{
  // Return kth value
  assert(values);
  assert((0 <= k) && (k < nvalues));
  return values[k];
}



inline void ObjectDescriptor::
SetValue(int k, double value) 
{
  // Set kth value
  assert(values);
  assert((0 <= k) && (k < nvalues));
  values[k] = value;
}



inline double ObjectDescriptor::
Distance(const ObjectDescriptor& descriptor) const
{
  // Return L2 distance
  return sqrt(SquaredDistance(descriptor));
}


