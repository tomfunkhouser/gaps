// Include file for 3D point set


// Include files

#include <vector>



// Class definition

class R3PointSet {
public:
  // Constructor/deconstructor
  R3PointSet(void);
  ~R3PointSet(void);

  // Point access
  int NPoints(void) const;
  const R3Point& PointPosition(int index) const;
  const R3Vector& PointNormal(int index) const;
  const RNRgb& PointColor(int index) const;
  int PointCategoryIdentifier(int index) const;
  int PointInstanceIdentifier(int index) const;
  RNScalar PointValue(int index) const;

  // Point set properties
  R3Point Centroid(void) const;
  const R3Box& BBox(void) const;

  // Point queries
  int FindPointIndex(const R3Point& position, double tolerance = 0) const;

  // Point manipulation
  void RemovePoint(int index);
  int InsertPoint(const R3Point& position);
  void SetPointPosition(int index, const R3Point& position);
  void SetPointNormal(int index, const R3Vector& normal);
  void SetPointColor(int index, const RNRgb& color);
  void SetPointCategoryIdentifier(int index, int category);
  void SetPointInstanceIdentifier(int index, int instance);
  void SetPointValue(int index, RNScalar value);

  // Input/output
  int ReadFile(const char *filename);
  int ReadASCIIFile(const char *filename);
  int ReadBinaryFile(const char *filename);
  int ReadMeshFile(const char *filename);
  int ReadSDFFile(const char *filename);
  int WriteFile(const char *filename) const;
  int WriteASCIIFile(const char *filename) const;
  int WriteBinaryFile(const char *filename) const;
  int WriteSDFFile(const char *filename) const;
  int WriteMeshFile(const char *filename) const;

private:
  std::vector<R3Point> positions;
  std::vector<R3Vector> normals;
  std::vector<RNRgb> colors;
  std::vector<int> category_identifiers;
  std::vector<int> instance_identifiers;
  std::vector<RNScalar> values;
  R3Box bbox;
};



// Inline functions

inline int R3PointSet::
NPoints(void) const
{
  // Return number of points
  return (int) positions.size();
}



inline const R3Point& R3PointSet::
PointPosition(int index) const
{
  // Return position of point with given index
  if (index >= (int) positions.size()) return R3zero_point;
  return positions[index];
}



inline const R3Vector& R3PointSet::
PointNormal(int index) const
{
  // Return normal of point with given index
  if (index >= (int) normals.size()) return R3zero_vector;
  return normals[index];
}



inline const RNRgb& R3PointSet::
PointColor(int index) const
{
  // Return color of point with given index
  if (index >= (int) colors.size()) return RNblack_rgb;
  return colors[index];
}



inline int R3PointSet::
PointCategoryIdentifier(int index) const
{
  // Return category_identifier of point with given index
  if (index >= (int) category_identifiers.size()) return -1;
  return category_identifiers[index];
}



inline int R3PointSet::
PointInstanceIdentifier(int index) const
{
  // Return instance_identifier of point with given index
  if (index >= (int) instance_identifiers.size()) return -1;
  return instance_identifiers[index];
}



inline RNScalar R3PointSet::
PointValue(int index) const
{
  // Return value of point with given index
  if (index >= (int) values.size()) return -1;
  return values[index];
}



inline const R3Box& R3PointSet::
BBox(void) const
{
  // Return bounding box of point set
  return bbox;
}


