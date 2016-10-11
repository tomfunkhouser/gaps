/////////////////////////////////////////////////////////////////////////
// Include file for object pointset class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectPointSet {
public:
  // Constructor destructor
  ObjectPointSet(void);
  ObjectPointSet(const ObjectPointSet& pointset);
  ~ObjectPointSet(void);

  // Properties
  const R3Point& Origin(void) const;
  RNLength Radius(void) const;
  RNLength Height(void) const;
  R3Point Centroid(void) const;
  R3Box BBox(void) const;

  // Point access
  int NPoints(void) const;
  ObjectPoint *Point(int k) const;

  // Manipulation
  void Empty(void);
  ObjectPointSet& operator=(const ObjectPointSet& pointset);
  void InsertPoint(const ObjectPoint& point);
  void SetOrigin(const R3Point& origin);

  // Alignment transformation functions in 2D
  R3Affine PCATransformation2D(void) const;
  R3Affine ICPTransformation2D(const ObjectPointSet& pointset, 
    RNLength start_max_distance = FLT_MAX, RNLength end_max_distance = FLT_MAX,
    int niterations = 1) const;

  // Closest point queries
  ObjectPoint *FindClosest(ObjectPoint *query_point, 
    RNLength min_distance = 0, RNLength max_distance = FLT_MAX, 
    int (*IsCompatible)(ObjectPoint *, ObjectPoint *, void *) = NULL, void *compatible_data = NULL, 
    RNLength *closest_distance = NULL) const;
  int FindClosest(ObjectPoint *query_point, 
    RNLength min_distance, RNLength max_distance, int max_points, 
    int (*IsCompatible)(ObjectPoint *, ObjectPoint *, void *), void *compatible_data, 
    RNArray<ObjectPoint *>& closest_points, RNLength *distances = NULL) const;
  int FindAll(ObjectPoint *query_point, 
    RNLength min_distance, RNLength max_distance, 
    int (*IsCompatible)(ObjectPoint *, ObjectPoint *, void *), void *compatible_data, 
    RNArray<ObjectPoint *>& closest_points) const;

  // Input/output
  int ReadFile(const char *filename);
  int ReadXYZFile(const char *filename);
  int ReadXYZNFile(const char *filename);
  int ReadXYZN(FILE *fp);
  int ReadMeshFile(const char *filename);
  int WriteFile(const char *filename) const;
  int WriteXYZFile(const char *filename) const;
  int WriteXYZNFile(const char *filename) const;
  int WriteXYZN(FILE *fp) const;

  // Loading
  int LoadMesh(R3Mesh *mesh, int max_points = 1024);

public:
  // Update functions 
  void UpdatePoints(RNLength neighborhood_radius = FLT_MAX);

public:
  RNArray<ObjectPoint *> points;
  R3Kdtree<ObjectPoint *> *kdtree;
  R3Point origin;
};



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline int ObjectPointSet::
NPoints(void) const
{
  // Return number of points
  return points.NEntries();
}



inline ObjectPoint *ObjectPointSet::
Point(int k) const
{
  // Return kth point
  return points.Kth(k);
}



inline const R3Point& ObjectPointSet::
Origin(void) const
{
  // Return origin
  return origin;
}



