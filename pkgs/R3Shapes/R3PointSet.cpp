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
    weights(),
    values(),
    vbo_position_buffer(0),
    vbo_normal_buffer(0),
    vbo_rgb_color_buffer(0),
    vbo_pick_color_buffer(0),
    bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}



R3PointSet::
~R3PointSet(void)
{
  // Delete OpenGL buffers
  if (vbo_position_buffer > 0) glDeleteBuffers(1, &vbo_position_buffer);
  if (vbo_normal_buffer > 0) glDeleteBuffers(1, &vbo_normal_buffer);
  if (vbo_rgb_color_buffer > 0) glDeleteBuffers(1, &vbo_rgb_color_buffer);
  if (vbo_pick_color_buffer > 0) glDeleteBuffers(1, &vbo_pick_color_buffer);
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
  if (index < (int) weights.size()) values.erase(weights.begin()+index);
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
SetPointTangent(int index, const R3Vector& tangent)
{
  // Set tangent for point with given index
  while (index >= (int) tangents.size())
    tangents.push_back(R3zero_vector);
  tangents[index] = tangent;
}



void R3PointSet::
SetPointRadius(int index, int axis, RNLength radius)
{
  // Set radius for point with given index
  while (index >= (int) radii[axis].size())
    radii[axis].push_back(0);
  radii[axis][index] = radius;
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
SetPointWeight(int index, RNScalar weight)
{
  // Set weight for point with given index
  while (index >= (int) weights.size())
    weights.push_back(0);
  weights[index] = weight;
}



void R3PointSet::
SetPointValue(int index, RNScalar value)
{
  // Set value for point with given index
  while (index >= (int) values.size())
    values.push_back(-1);
  values[index] = value;
}



unsigned int  R3PointSet::
GLPositionBufferObject(void) const
{
  // Check if nothing to be done
  if (NPoints() == 0) return 0;

  // Update position buffer object
  if (vbo_position_buffer == 0) {
    glGenBuffers(1, (GLuint *) &vbo_position_buffer);
    if (vbo_position_buffer > 0) {
      std::vector<GLfloat> positions;
      for (int i = 0; i < NPoints(); i++) {
        const R3Point& position = PointPosition(i);
        positions.push_back(position.X());
        positions.push_back(position.Y());
        positions.push_back(position.Z());
      }
      glBindBuffer(GL_ARRAY_BUFFER, vbo_position_buffer);
      glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), &positions[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }
  
  // Return buffer object identifier
  return vbo_position_buffer;
}


  
unsigned int R3PointSet::
GLNormalBufferObject(void) const
{
  // Check if nothing to be done
  if (NPoints() == 0) return 0;

  // Update normal buffer
  if (vbo_normal_buffer == 0) {
    glGenBuffers(1, (GLuint *) &vbo_normal_buffer);
    if (vbo_normal_buffer > 0) {
      std::vector<GLfloat> normals;
      for (int i = 0; i < NPoints(); i++) {
        const R3Vector& normal = PointNormal(i);
        normals.push_back(normal.X());
        normals.push_back(normal.Y());
        normals.push_back(normal.Z());
      }
      glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_buffer);
      glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }

  // Return buffer object identifier
  return vbo_normal_buffer;
}

  

unsigned int R3PointSet::
GLRgbColorBufferObject(void) const
{
  // Check if nothing to be done
  if (NPoints() == 0) return 0;

  // Update rgb color buffer
  if (vbo_rgb_color_buffer == 0) {
    glGenBuffers(1, (GLuint *) &vbo_rgb_color_buffer);
    if (vbo_rgb_color_buffer > 0) {
      std::vector<GLubyte> rgb_colors;
      for (int i = 0; i < NPoints(); i++) {
        const RNRgb& rgb_color = PointColor(i);
        rgb_colors.push_back(255.0 * rgb_color.R());
        rgb_colors.push_back(255.0 * rgb_color.G());
        rgb_colors.push_back(255.0 * rgb_color.B());
        rgb_colors.push_back(255);
      }
      glBindBuffer(GL_ARRAY_BUFFER, vbo_rgb_color_buffer);
      glBufferData(GL_ARRAY_BUFFER, rgb_colors.size() * sizeof(GLubyte), &rgb_colors[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }

  // Return buffer object identifier
  return vbo_rgb_color_buffer;
}


  
unsigned int R3PointSet::
GLPickColorBufferObject(void) const
{
  // Check if nothing to be done
  if (NPoints() == 0) return 0;

  // Update pick color buffer
  if (vbo_pick_color_buffer == 0) {
    glGenBuffers(1, (GLuint *) &vbo_pick_color_buffer);
    if (vbo_pick_color_buffer > 0) {
      std::vector<GLubyte> pick_colors;
      for (int i = 0; i < NPoints(); i++) {
        unsigned char pick_color[4];
        EncodePickColor(i, 255, pick_color);
        pick_colors.push_back(pick_color[0]);
        pick_colors.push_back(pick_color[1]);
        pick_colors.push_back(pick_color[2]);
        pick_colors.push_back(pick_color[3]);
      }
      glBindBuffer(GL_ARRAY_BUFFER, vbo_pick_color_buffer);
      glBufferData(GL_ARRAY_BUFFER, pick_colors.size() * sizeof(GLubyte), &pick_colors[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }

  // Return buffer object identifier
  return vbo_pick_color_buffer;
}



void R3PointSet::
Draw(const RNFlags draw_flags) const
{
  // Check if nothing to be done
  if (NPoints() == 0) return;

  // Bind vertex position buffer
  if (GLPositionBufferObject() <= 0) return;
  glBindBuffer(GL_ARRAY_BUFFER, vbo_position_buffer);
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  // Bind vertex normal buffer
  if (draw_flags[R3_VERTEX_NORMALS_DRAW_FLAG]) {
    if (GLNormalBufferObject() > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_buffer);
      glNormalPointer(GL_FLOAT, 0, 0);
      glEnableClientState(GL_NORMAL_ARRAY);
    }
  }
  
  // Bind vertex color buffer
  if (draw_flags[R3_VERTEX_PICK_DRAW_FLAG]) {
    if (GLPickColorBufferObject() > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_pick_color_buffer);
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
      glEnableClientState(GL_COLOR_ARRAY);
    }
  }
  else if (draw_flags[R3_VERTEX_COLORS_DRAW_FLAG]) {
    if (GLRgbColorBufferObject() > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_rgb_color_buffer);
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
      glEnableClientState(GL_COLOR_ARRAY);
    }
  }
  
  // Draw points
  glDrawArrays(GL_POINTS, 0, NPoints());

  // Reset everything
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
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
