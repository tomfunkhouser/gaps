// Include file for 3D point set
#ifndef __R3__POINTSET__H__
#define __R3__POINTSET__H__



// Include files

#include <vector>



// Begin namespace

namespace gaps {



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
  const R3Vector& PointTangent(int index) const;
  const RNRgb& PointColor(int index) const;
  RNLength PointRadius(int index, int axis) const;
  RNScalar PointTimestamp(int index) const;
  int PointCategoryIdentifier(int index) const;
  int PointInstanceIdentifier(int index) const;
  RNScalar PointWeight(int index) const;
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
  void SetPointTangent(int index, const R3Vector& tangent);
  void SetPointRadius(int index, int axis, RNLength radius);
  void SetPointColor(int index, const RNRgb& color);
  void SetPointTimestamp(int index, RNScalar timestamp);
  void SetPointCategoryIdentifier(int index, int category);
  void SetPointInstanceIdentifier(int index, int instance);
  void SetPointWeight(int index, RNScalar weight);
  void SetPointValue(int index, RNScalar value);

  // Draw functions
  void Draw(const R3DrawFlags draw_flags = R3_VERTICES_DRAW_FLAG |
    R3_VERTEX_NORMALS_DRAW_FLAG | R3_VERTEX_COLORS_DRAW_FLAG) const;

  // Input/output
  int ReadFile(const char *filename);
  int ReadXYZFile(const char *filename);
  int ReadXYZNFile(const char *filename);
  int ReadBinaryFile(const char *filename);
  int ReadMeshFile(const char *filename);
  int ReadPTSFile(const char *filename);
  int ReadSDFFile(const char *filename);
  int ReadVTSFile(const char *filename);
  int WriteFile(const char *filename) const;
  int WriteXYZFile(const char *filename) const;
  int WriteXYZNFile(const char *filename) const;
  int WriteBinaryFile(const char *filename) const;
  int WritePTSFile(const char *filename) const;
  int WriteSDFFile(const char *filename) const;
  int WriteMeshFile(const char *filename) const;

public:
  // OpenGL buffer object access
  unsigned int GLPositionBufferObject(void) const;
  unsigned int GLNormalBufferObject(void) const;
  unsigned int GLRgbColorBufferObject(void) const;
  unsigned int GLPickColorBufferObject(void) const;

private:
  std::vector<R3Point> positions;
  std::vector<R3Vector> normals;
  std::vector<R3Vector> tangents;
  std::vector<RNLength> radii[2];
  std::vector<RNRgb> colors;
  std::vector<RNScalar> timestamps;
  std::vector<int> category_identifiers;
  std::vector<int> instance_identifiers;
  std::vector<RNScalar> weights;
  std::vector<RNScalar> values;
  unsigned int vbo_position_buffer;
  unsigned int vbo_normal_buffer;
  unsigned int vbo_rgb_color_buffer;
  unsigned int vbo_pick_color_buffer;
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
  assert(index >= 0);
  if (index >= (int) positions.size()) return R3zero_point;
  return positions[index];
}



inline const R3Vector& R3PointSet::
PointNormal(int index) const
{
  // Return normal of point with given index
  assert(index >= 0);
  if (index >= (int) normals.size()) return R3zero_vector;
  return normals[index];
}



inline const R3Vector& R3PointSet::
PointTangent(int index) const
{
  // Return tangent of point with given index
  assert(index >= 0);
  if (index >= (int) tangents.size()) return R3zero_vector;
  return tangents[index];
}



inline RNLength R3PointSet::
PointRadius(int index, int axis) const
{
  // Return radius of point with given index for the given tangent
  assert(index >= 0);
  if (index >= (int) radii[axis].size()) return 0;
  return radii[axis][index];
}



inline const RNRgb& R3PointSet::
PointColor(int index) const
{
  // Return color of point with given index
  assert(index >= 0);
  if (index >= (int) colors.size()) return RNblack_rgb;
  return colors[index];
}



inline RNScalar R3PointSet::
PointTimestamp(int index) const
{
  // Return timestamp of point with given index
  assert(index >= 0);
  if (index >= (int) timestamps.size()) return -1;
  return timestamps[index];
}



inline int R3PointSet::
PointCategoryIdentifier(int index) const
{
  // Return category_identifier of point with given index
  assert(index >= 0);
  if (index >= (int) category_identifiers.size()) return -1;
  return category_identifiers[index];
}



inline int R3PointSet::
PointInstanceIdentifier(int index) const
{
  // Return instance_identifier of point with given index
  assert(index >= 0);
  if (index >= (int) instance_identifiers.size()) return -1;
  return instance_identifiers[index];
}



inline RNScalar R3PointSet::
PointWeight(int index) const
{
  // Return value of point with given index
  assert(index >= 0);
  if (index >= (int) weights.size()) return 0;
  return weights[index];
}



inline RNScalar R3PointSet::
PointValue(int index) const
{
  // Return value of point with given index
  assert(index >= 0);
  if (index >= (int) values.size()) return -1;
  return values[index];
}



inline const R3Box& R3PointSet::
BBox(void) const
{
  // Return bounding box of point set
  return bbox;
}



// End namespace
}


// End include guard
#endif
