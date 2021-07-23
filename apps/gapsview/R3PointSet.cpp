// Source file for 3D point set class



// Include files

#include "R3Shapes/R3Shapes.h"
#include "R3PointSet.h"



// Namespace

namespace gaps {



// Member functions

R3PointSet::
R3PointSet(void)
  : positions(),
    normals(),
    colors(),
    timestamps(),
    category_identifiers(),
    instance_identifiers(),
    values(),
    bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}



R3PointSet::
~R3PointSet(void)
{
}



R3Point R3PointSet::
Centroid(void) const
{
  // Check if no points
  if (positions.size() == 0) return R3zero_point;

  // Return centroid of point set
  R3Point centroid = R3zero_point;
  for (unsigned int i = 0; i < positions.size(); i++) 
    centroid += positions[i];
  centroid /= positions.size();
  return centroid;
}



int R3PointSet::
FindPointIndex(const R3Point& point, double tolerance) const
{
  // Return first point within tolerance
  double squared_tolerance = tolerance * tolerance;
  for (unsigned int i = 0; i < positions.size(); i++) {
    if (R3SquaredDistance(positions[i], point) <= squared_tolerance) {
      return i;
    }
  }

  // No point found
  return -1;
}



void R3PointSet::
RemovePoint(int index)
{
  // Remove point at given index
  if (index < (int) positions.size()) positions.erase(positions.begin()+index);
  if (index < (int) normals.size()) normals.erase(normals.begin()+index);
  if (index < (int) colors.size()) colors.erase(colors.begin()+index);
  if (index < (int) timestamps.size()) timestamps.erase(timestamps.begin()+index);
  if (index < (int) category_identifiers.size()) category_identifiers.erase(category_identifiers.begin()+index);
  if (index < (int) instance_identifiers.size()) instance_identifiers.erase(instance_identifiers.begin()+index);
  if (index < (int) values.size()) values.erase(values.begin()+index);
}



int R3PointSet::
InsertPoint(const R3Point& position)
{
  // Insert point
  int index = NPoints();
  SetPointPosition(index, position);
  return index;
}



void R3PointSet::
SetPointPosition(int index, const R3Point& position)
{
  // Set position for point with given index
  while (index >= (int) positions.size())
    positions.push_back(R3zero_point);
  positions[index] = position;
  bbox.Union(position);
}



void R3PointSet::
SetPointNormal(int index, const R3Vector& normal)
{
  // Set normal for point with given index
  while (index >= (int) normals.size())
    normals.push_back(R3zero_vector);
  normals[index] = normal;
}



void R3PointSet::
SetPointColor(int index, const RNRgb& color)
{
  // Set color for point with given index
  while (index >= (int) colors.size())
    colors.push_back(RNblack_rgb);
  colors[index] = color;
}



void R3PointSet::
SetPointTimestamp(int index, RNScalar timestamp)
{
  // Set timestamp for point with given index
  while (index >= (int) timestamps.size())
    timestamps.push_back(-1);
  timestamps[index] = timestamp;
}



void R3PointSet::
SetPointCategoryIdentifier(int index, int identifier)
{
  // Set category identifier for point with given index
  while (index >= (int) category_identifiers.size())
    category_identifiers.push_back(-1);
  category_identifiers[index] = identifier;
}



void R3PointSet::
SetPointInstanceIdentifier(int index, int identifier)
{
  // Set instance identifier for point with given index
  while (index >= (int) instance_identifiers.size())
    instance_identifiers.push_back(-1);
  instance_identifiers[index] = identifier;
}



void R3PointSet::
SetPointValue(int index, RNScalar value)
{
  // Set value for point with given index
  while (index >= (int) values.size())
    values.push_back(-1);
  values[index] = value;
}



int R3PointSet::
ReadFile(const char *filename)
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .mm)\n", filename);
    return 0;
  }

  // Read file of appropriate type
  if (!strcmp(extension, ".xyz")) return ReadXYZFile(filename);
  else if (!strcmp(extension, ".xyzn")) return ReadXYZNFile(filename);
  else if (!strcmp(extension, ".pset")) return ReadBinaryFile(filename);
  else if (!strcmp(extension, ".pts")) return ReadPTSFile(filename);
  else if (!strcmp(extension, ".sdf")) return ReadSDFFile(filename);
  else if (!strcmp(extension, ".vts")) return ReadVTSFile(filename);
  else return ReadMeshFile(filename);

  // Should not get here
  RNFail("Unrecognized file extension in %s\n", filename);
  return 0;
}


int R3PointSet::
WriteFile(const char *filename) const
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .mm)\n", filename);
    return 0;
  }

  // Write file of appropriate type
  if (!strcmp(extension, ".xyz")) return WriteXYZFile(filename);
  else if (!strcmp(extension, ".xyzn")) return WriteXYZNFile(filename);
  else if (!strcmp(extension, ".pset")) return WriteBinaryFile(filename);
  else if (!strcmp(extension, ".pts")) return WritePTSFile(filename);
  else if (!strcmp(extension, ".sdf")) return WriteSDFFile(filename);
  else return WriteMeshFile(filename);

  // Should not get here
  RNFail("Unrecognized file extension in %s\n", filename);
  return 0;
}



////////////////////////////////////////////////////////////////////////

int R3PointSet::
ReadXYZFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Read points
  double px, py, pz;
  while (fscanf(fp, "%lf%lf%lf", &px, &py, &pz) == (unsigned int) 3) {
    R3Point position(px,py,pz);
    InsertPoint(position);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3PointSet::
WriteXYZFile(const char *filename) const
{
 // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Write points
  for (int i = 0; i < NPoints(); i++) {
    const R3Point& p = PointPosition(i);
    fprintf(fp, "%g %g %g\n", p[0], p[1], p[2]);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////

int R3PointSet::
ReadXYZNFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Read points
  double px, py, pz, nx, ny, nz;
  while (fscanf(fp, "%lf%lf%lf%lf%lf%lf", &px, &py, &pz, &nx, &ny, &nz) == (unsigned int) 6) {
    R3Point position(px,py,pz);
    R3Vector normal(nx,ny,nz);
    normal.Normalize();
    int index = InsertPoint(position);
    SetPointNormal(index, normal);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3PointSet::
WriteXYZNFile(const char *filename) const
{
 // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Write points
  for (int i = 0; i < NPoints(); i++) {
    const R3Point& p = PointPosition(i);
    const R3Vector& n = PointNormal(i);
    fprintf(fp, "%g %g %g %g %g %g\n", p[0], p[1], p[2], n[0], n[1], n[2]);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////

int R3PointSet::
ReadBinaryFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Read points
  float buf[16];
  while (fread(buf, sizeof(float), 16, fp) == (unsigned int) 16) {
    R3Point position(buf[0], buf[1], buf[2]);
    R3Vector normal(buf[3], buf[4], buf[5]);
    RNRgb color(buf[6], buf[7], buf[8]);
    double timestamp = buf[9];
    int category_identifier = buf[10] + 0.5;
    int instance_identifier = buf[11] + 0.5;
    double value = buf[12];
    normal.Normalize();
    int index = InsertPoint(position);
    SetPointNormal(index, normal);
    SetPointColor(index, color);
    SetPointTimestamp(index, timestamp);
    SetPointCategoryIdentifier(index, category_identifier);
    SetPointInstanceIdentifier(index, instance_identifier);
    SetPointValue(index, value);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3PointSet::
WriteBinaryFile(const char *filename) const
{
 // Open file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Write points
  float buf[16];
  for (int i = 0; i < NPoints(); i++) {
    const R3Point& p = PointPosition(i);
    const R3Vector& n = PointNormal(i);
    const RNRgb& c = PointColor(i);
    buf[0] = p[0]; buf[1] = p[1]; buf[2] = p[2];
    buf[3] = n[0]; buf[4] = n[1]; buf[5] = n[2];
    buf[6] = c[0]; buf[7] = c[1]; buf[8] = c[2];
    buf[9] = PointTimestamp(i);
    buf[10] = PointCategoryIdentifier(i);
    buf[11] = PointInstanceIdentifier(i);
    buf[12] = PointValue(i);
    buf[13] = buf[14] = buf[15] = 0;
    if (fwrite(buf, sizeof(float), 16, fp) != (unsigned int) 16) {
      RNFail("Unable to write point %d to %s\n", i, filename);
      return 0;
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////

int R3PointSet::
ReadPTSFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Read points
  float buf[6];
  while (fread(buf, sizeof(float), 6, fp) == (unsigned int) 6) {
    R3Point position(buf[0], buf[1], buf[2]);
    R3Vector normal(buf[3], buf[4], buf[5]);
    normal.Normalize();
    int index = InsertPoint(position);
    SetPointNormal(index, normal);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3PointSet::
WritePTSFile(const char *filename) const
{
 // Open file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Write points
  float buf[6];
  for (int i = 0; i < NPoints(); i++) {
    const R3Point& p = PointPosition(i);
    const R3Vector& n = PointNormal(i);
    buf[0] = p[0]; buf[1] = p[1]; buf[2] = p[2];
    buf[3] = n[0]; buf[4] = n[1]; buf[5] = n[2];
    if (fwrite(buf, sizeof(float), 6, fp) != (unsigned int) 6) {
      RNFail("Unable to write point %d to %s\n", i, filename);
      return 0;
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////

int R3PointSet::
ReadSDFFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Read points
  float buf[4];
  while (fread(buf, sizeof(float), 4, fp) == (unsigned int) 4) {
    R3Point position(buf[0], buf[1], buf[2]);
    int index = InsertPoint(position);
    SetPointValue(index, buf[3]);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3PointSet::
WriteSDFFile(const char *filename) const
{
 // Open file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Write points
  float buf[4];
  for (int i = 0; i < NPoints(); i++) {
    const R3Point& p = PointPosition(i);
    buf[0] = p[0]; buf[1] = p[1]; buf[2] = p[2];
    buf[3] = PointValue(i);
    if (fwrite(buf, sizeof(float), 4, fp) != (unsigned int) 4) {
      RNFail("Unable to write point %d to %s\n", i, filename);
      return 0;
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////

int R3PointSet::
ReadVTSFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open %s\n", filename);
    return 0;
  }

  // Read points
  int vertex_index;
  double px, py, pz;
  while (fscanf(fp, "%d%lf%lf%lf", &vertex_index, &px, &py, &pz) == (unsigned int) 4) {
    R3Point position(px,py,pz);
    InsertPoint(position);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////

int R3PointSet::
ReadMeshFile(const char *filename)
{
  // Read mesh file
  R3Mesh mesh;
  if (!mesh.ReadFile(filename)) {
    RNFail("Unable to read %s\n", filename);
    return 0;
  }

  // Create point for each vertex
  for (int i = 0; i < mesh.NVertices(); i++) {
    R3MeshVertex *vertex = mesh.Vertex(i);
    int index = InsertPoint(mesh.VertexPosition(vertex));
    SetPointNormal(index, mesh.VertexNormal(vertex));
    SetPointColor(index, mesh.VertexColor(vertex));
  }

  // Return success
  return 1;
}



int R3PointSet::
WriteMeshFile(const char *filename) const
{
  // Create mesh
  R3Mesh mesh;
  for (int i = 0; i < NPoints(); i++) {
    mesh.CreateVertex(PointPosition(i), PointNormal(i), PointColor(i));
  }

  // Write mesh
  return mesh.WriteFile(filename);
}




}; // end namespace
