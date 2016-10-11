////////////////////////////////////////////////////////////////////////
// Source file for object point set class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////

ObjectPointSet::
ObjectPointSet(void)
  : points(),
    kdtree(NULL),
    origin(0,0,0)
{ 
}  

  

ObjectPointSet::
ObjectPointSet(const ObjectPointSet& pointset)
  : points(),
    kdtree(NULL),
    origin(pointset.origin)
{ 
  // Copy points
  for (int i = 0; i < pointset.NPoints(); i++) {
    ObjectPoint *point = pointset.Point(i);
    InsertPoint(*point);
  }
}  

  

ObjectPointSet::
~ObjectPointSet(void)
{ 
  // Delete kdtree
  if (kdtree) {
    delete kdtree;
  }

  // Delete points
  for (int i = 0; i < NPoints(); i++) {
    delete Point(i);
  }
}  



R3Point ObjectPointSet::
Centroid(void) const
{
  // Check number of points
  if (NPoints() == 0) return R3zero_point;

  // Sum points
  R3Point sum = R3zero_point;
  for (int i = 0; i < NPoints(); i++) {
    ObjectPoint *point = Point(i);
    sum += point->Position();
  }

  // Return centroid
  return sum / NPoints();
}

  

RNLength ObjectPointSet::
Radius(void) const
{
  // Find max radius
  RNLength max_rr = 0;
  for (int i = 0; i < NPoints(); i++) {
    ObjectPoint *point = Point(i);
    const R3Point& position = point->Position();
    RNLength dx = position.X() - origin.X();
    RNLength dy = position.Y() - origin.Y();
    RNLength rr = dx*dx + dy+dy;
    if (rr > max_rr) max_rr = rr;
  }

  // Return XY radius
  return sqrt(max_rr);
}

  

RNLength ObjectPointSet::
Height(void) const
{
  // Check number of points
  if (NPoints() == 0) return 0.0;

  // Find max height
  RNCoord zmin = FLT_MAX;
  RNCoord zmax = -FLT_MAX;
  for (int i = 0; i < NPoints(); i++) {
    ObjectPoint *point = Point(i);
    const R3Point& position = point->Position();
    if (position.Z() < zmin) zmin = position.Z();
    if (position.Z() > zmax) zmax = position.Z();
  }

  // Return Z height
  return zmax - zmin;
}

  

R3Box ObjectPointSet::
BBox(void) const
{
  // Union points
  R3Box bbox = R3null_box;
  for (int i = 0; i < NPoints(); i++) {
    ObjectPoint *point = Point(i);
    bbox.Union(point->Position());
  }

  // Return bounding box
  return bbox;
}

  

void ObjectPointSet::
Empty(void) 
{
  // Reset kdtree
  if (kdtree) { 
    delete kdtree; 
    kdtree = NULL; 
  }

  // Delete points
  for (int i = 0; i < NPoints(); i++) {
    delete Point(i);
  }

  // Empty points array
  points.Empty();
}



ObjectPointSet& ObjectPointSet::
operator=(const ObjectPointSet& pointset)
{
  // Empty points
  Empty();

  // Copy points
  for (int i = 0; i < pointset.NPoints(); i++) {
    ObjectPoint *point = pointset.Point(i);
    InsertPoint(*point);
  }

  // Set origin
  SetOrigin(pointset.Origin());

  // Return this
  return *this;
}



void ObjectPointSet::
InsertPoint(const ObjectPoint& p)
{
  // Create point
  ObjectPoint *point = new ObjectPoint(p);

  // Insert point
  points.Insert(point);

  // Reset kdtree
  if (kdtree) { delete kdtree; kdtree = NULL; }
}



void ObjectPointSet::
SetOrigin(const R3Point& origin) 
{
  // Set origin
  this->origin = origin;
}



ObjectPoint *ObjectPointSet::
FindClosest(ObjectPoint *query, 
    RNLength min_distance, RNLength max_distance, 
    int (*IsCompatible)(ObjectPoint *, ObjectPoint *, void *), void *compatible_data, 
    RNLength *closest_distance) const
{
  // Update kdtree
  if (!kdtree) {
    ObjectPointSet *pointset = (ObjectPointSet *) this;
    pointset->kdtree = new R3Kdtree<ObjectPoint *>(points, 0);
  }

  // Return closest point
  return kdtree->FindClosest(query, min_distance, max_distance, IsCompatible, compatible_data, closest_distance);
}



int ObjectPointSet::
FindClosest(ObjectPoint *query, 
    RNLength min_distance, RNLength max_distance, int max_points, 
    int (*IsCompatible)(ObjectPoint *, ObjectPoint *, void *), void *compatible_data, 
    RNArray<ObjectPoint *>& closest_points, RNLength *distances) const
{
  // Update kdtree
  if (!kdtree) {
    ObjectPointSet *pointset = (ObjectPointSet *) this;
    pointset->kdtree = new R3Kdtree<ObjectPoint *>(points, 0);
  }

  // Return closest point
  return kdtree->FindClosest(query, min_distance, max_distance, max_points,  
    IsCompatible, compatible_data, closest_points, distances);
}



int ObjectPointSet::
FindAll(ObjectPoint *query, 
    RNLength min_distance, RNLength max_distance, 
    int (*IsCompatible)(ObjectPoint *, ObjectPoint *, void *), void *compatible_data, 
    RNArray<ObjectPoint *>& closest_points) const
{
  // Update kdtree
  if (!kdtree) {
    ObjectPointSet *pointset = (ObjectPointSet *) this;
    pointset->kdtree = new R3Kdtree<ObjectPoint *>(points, 0);
  }

  // Return closest point
  return kdtree->FindAll(query, min_distance, max_distance, 
    IsCompatible, compatible_data, closest_points);
}



////////////////////////////////////////////////////////////////////////
// Input/output functions
////////////////////////////////////////////////////////////////////////

int ObjectPointSet::
ReadFile(const char *filename)
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .txt)\n", filename);
    return 0;
  }

  // Read file of appropriate type
  if (!strncmp(extension, ".txt", 4)) {
    if (!ReadXYZFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".xyz", 4)) {
    if (!ReadXYZFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".xyzn", 5)) {
    if (!ReadXYZNFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".off", 4)) {
    if (!ReadMeshFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".ply", 4)) {
    if (!ReadMeshFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".obj", 4)) {
    if (!ReadMeshFile(filename)) return 0;
  }
  else {
    fprintf(stderr, "Unable to read file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Return success
  return 1;
}



int ObjectPointSet::
WriteFile(const char *filename) const
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .txt)\n", filename);
    return 0;
  }

  // Write file of appropriate type
  if (!strncmp(extension, ".txt", 4)) {
    if (!WriteXYZNFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".xyz", 5)) {
    if (!WriteXYZFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".xyzn", 5)) {
    if (!WriteXYZNFile(filename)) return 0;
  }
  else {
    fprintf(stderr, "Unable to write file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// XYZ input/output functions
////////////////////////////////////////////////////////////////////////

int ObjectPointSet::
ReadXYZFile(const char *filename)
{
  // Open points file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open points file: %s\n", filename);
    return 0;
  }
  
  // Read points
  char buffer[4096];
  while (fgets(buffer, 4096, fp)) {
    double px, py, pz;
    if (sscanf(buffer, "%lf%lf%lf", &px, &py, &pz) == (unsigned int) 3) {
      ObjectPoint point;
      point.SetPosition(R3Point(px, py, pz));
      point.SetValue(1.0);
      InsertPoint(point);
    }
  }
  
  // Close points file
  fclose(fp);

  // Return success
  return 1;
}


int ObjectPointSet::
WriteXYZFile(const char *filename) const
{
  // Open points file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open points file: %s\n", filename);
    return 0;
  }
  
  // Write points
  for (int i = 0; i < NPoints(); i++) {
    ObjectPoint *point = Point(i);
    const R3Point& position = point->Position();
    fprintf(fp, "%g %g %g   \n", position.X(), position.Y(), position.Z());
  }
  
  // Close points file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
//  XYZN input/output functions
////////////////////////////////////////////////////////////////////////

int ObjectPointSet::
ReadXYZNFile(const char *filename)
{
  // Open points file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open points file: %s\n", filename);
    return 0;
  }
  
  // Read points
  if (!ReadXYZN(fp)) return 0;
  
  // Close points file
  fclose(fp);

  // Return success
  return 1;
}



int ObjectPointSet::
ReadXYZN(FILE *fp)
{
  // Read points
  char buffer[4096];
  while (fgets(buffer, 4096, fp)) {
    double px, py, pz, nx, ny, nz;
    if (sscanf(buffer, "%lf%lf%lf%lf%lf%lf", &px, &py, &pz, &nx, &ny, &nz) == (unsigned int) 6) {
      ObjectPoint point;
      point.SetPosition(R3Point(px, py, pz));
      point.SetNormal(R3Vector(nx, ny, nz));
      point.SetValue(1.0);
      InsertPoint(point);
    }
  }
  
  // Return success
  return 1;
}



int ObjectPointSet::
WriteXYZNFile(const char *filename) const
{
  // Open points file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open points file: %s\n", filename);
    return 0;
  }
  
  // Write points
  if (!WriteXYZN(fp)) return 0;
  
  // Close points file
  fclose(fp);

  // Return success
  return 1;
}



int ObjectPointSet::
WriteXYZN(FILE *fp) const
{
  // Write points
  for (int i = 0; i < NPoints(); i++) {
    ObjectPoint *point = Point(i);
    const R3Point& position = point->Position();
    const R3Vector& normal = point->Normal();
    fprintf(fp, "%g %g %g   ", position.X(), position.Y(), position.Z());
    fprintf(fp, "%g %g %g\n", normal.X(), normal.Y(), normal.Z());
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Mesh input/output functions
////////////////////////////////////////////////////////////////////////

int ObjectPointSet::
ReadMeshFile(const char *filename)
{
  // Read mesh
  R3Mesh *mesh = new R3Mesh();
  if (!mesh->ReadFile(filename)) {
    delete mesh;
    return 0;
  }

  // Load mesh
  if (!LoadMesh(mesh)) {
    delete mesh;
    return 0;
  }

  // Delete mesh
  delete mesh;

  // Return success
  return 1;
}



int ObjectPointSet::
LoadMesh(R3Mesh *mesh, int max_points)
{ 
  // Update origin
  this->origin = mesh->Centroid();
  this->origin[2] = mesh->BBox().ZMin();

  // Count cumulative area of faces
  RNArea total_area = 0;
  RNArea *cumulative_area = new RNArea [ mesh->NFaces() ];
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    RNArea face_area = mesh->FaceArea(face);
    total_area += face_area;
    cumulative_area[i] = total_area;;
  }
    
  // Generate points
  RNSeedRandomScalar();
  while (points.NEntries() < max_points) {
    // Generate a random number
    RNScalar r = RNRandomScalar() * total_area;

    // Find face ID
    int min_face_id = 0;
    int max_face_id = mesh->NFaces()-1;
    int face_id = (min_face_id + max_face_id) / 2;
    while (min_face_id < max_face_id) {
      if (min_face_id + 1 >= max_face_id) { 
        if (r < cumulative_area[min_face_id]) { 
          face_id = min_face_id; 
          break; 
        }
        else { 
          face_id = max_face_id; 
          break; 
        }
      }
      else {
        if (cumulative_area[face_id] < r) {
          min_face_id = face_id + 1;
          face_id = (min_face_id + max_face_id) / 2;
        }
        else if (cumulative_area[face_id] > r) {
          max_face_id = face_id;
          face_id = (min_face_id + max_face_id) / 2;
        }
        else {
          break;
        }
      }
    }

    // Find point on face
    R3MeshFace *face = mesh->Face(face_id);
    R3MeshVertex *v0 = mesh->VertexOnFace(face, 0);
    R3MeshVertex *v1 = mesh->VertexOnFace(face, 1);
    R3MeshVertex *v2 = mesh->VertexOnFace(face, 2);
    const R3Point& p0 = mesh->VertexPosition(v0);
    const R3Point& p1 = mesh->VertexPosition(v1);
    const R3Point& p2 = mesh->VertexPosition(v2);
    const R3Vector& n0 = mesh->VertexNormal(v0);
    const R3Vector& n1 = mesh->VertexNormal(v1);
    const R3Vector& n2 = mesh->VertexNormal(v2);
    RNScalar r1 = sqrt(RNRandomScalar());
    RNScalar r2 = RNRandomScalar();
    RNScalar t0 = (1.0 - r1);
    RNScalar t1 = r1 * (1.0 - r2);
    RNScalar t2 = r1 * r2;
    R3Point position = t0*p0 + t1*p1 + t2*p2;
    R3Vector normal = t0*n0 + t1*n1 + t2*n2;

    // Create point
    ObjectPoint point;
    point.SetPosition(position);
    point.SetNormal(normal);
    point.SetValue(1.0);
    InsertPoint(point);
  }

  // Delete temporary data
  delete [] cumulative_area;

  // Return success
  return 1;
}  



void ObjectPointSet::
UpdatePoints(RNScalar neighborhood_radius)
{ 
  // Update normals and descriptors based on positions in this pointset

  // Allocate memory 
  const int max_neighbors = 16;
  RNArray<R3Point *> positions;
  RNArray<ObjectPoint *> neighbors;
  positions.Resize(max_neighbors);
  neighbors.Resize(max_neighbors);
  const int descriptor_nvalues = 5;
  double descriptor_values[descriptor_nvalues];
    
  // Update point normals based on neighborhood of pointset
  for (int i = 0; i < NPoints(); i++) {
    ObjectPoint *point = Point(i);
    const R3Point& position = point->Position();

    // Find neighbors
    neighbors.Empty();
    R3Vector normal(0, 0, 0);
    R3Point centroid(position);
    RNScalar variances[3] = { 0, 0, 0 };
    if (FindClosest(point, 0, neighborhood_radius, max_neighbors, NULL, NULL, neighbors)) {
      // Create list of positions (for R3Centroid and R3PrincipleAxes)
      positions.Empty();
      for (int j = 0; j < neighbors.NEntries(); j++) {
        positions.Insert(&(neighbors[j]->position));
      }

      // Compute properties of neighborhood
      centroid = R3Centroid(positions);
      R3Triad triad = R3PrincipleAxes(centroid, positions, NULL, variances);
      normal = triad[2];
    }

    // Update normal
    point->SetNormal(normal);

    // Update descriptor
    descriptor_values[0] = position.Z() - origin.Z();
    descriptor_values[1] = fabs(normal.Z());
    descriptor_values[2] = (variances[0] > 0) ? variances[1] / variances[0] : 1.0;
    descriptor_values[3] = (variances[0] > 0) ? variances[2] / variances[0] : 1.0;
    descriptor_values[4] = (variances[1] > 0) ? variances[2] / variances[1] : 1.0;
    point->SetDescriptor(ObjectDescriptor(descriptor_values, descriptor_nvalues));
  }
}






