/* Source file for the GSV mesh class */




////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "GSV.h"
#include "R3Shapes/ply.h"





////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



GSVMesh::
GSVMesh(void)
  : gsv_vertex_block(NULL),
    search_tree(NULL)
{
}



GSVMesh::
~GSVMesh(void)
{
  // Delete everything
  Empty();

  // Delete search tree
  if (search_tree) delete search_tree;

  // Delete vertices
  if (gsv_vertex_block) delete [] gsv_vertex_block;
}



////////////////////////////////////////////////////////////////////////
// Vertex properties
////////////////////////////////////////////////////////////////////////

RNScalar GSVMesh::
VertexTimestamp(R3MeshVertex *v) const
{
  // Return vertex timestamp
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return RN_UNKNOWN;
  return scanline->Timestamp();
}



RNScalar GSVMesh::
VertexHeight(R3MeshVertex *v) const
{
  // Return vertex height
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return RN_UNKNOWN;
  RNCoord groundZ = scanline->EstimatedGroundZ();
  const R3Point& position = VertexPosition(vertex);
  return position.Z() - groundZ;
}



RNScalar GSVMesh::
VertexLaserAngle(R3MeshVertex *v) const
{
  // Return vertex angle
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return RN_UNKNOWN;
  const GSVPose& pose = scanline->Pose();
  const R3Point& viewpoint = pose.Viewpoint();
  const R3Vector& towards = pose.Towards();
  R3Vector up = pose.Up();
  const R3Point& position = VertexPosition(vertex);
  R3Vector vec = position - viewpoint;
  RNLength distance = vec.Length();
  if (RNIsZero(distance)) return RN_UNKNOWN;
  vec /= distance;
  RNScalar dot = vec.Dot(towards);
  RNAngle angle = acos(dot);
  if (vec.Dot(up) > 0) angle = RN_PI_OVER_TWO + angle;
  else angle = RN_PI_OVER_TWO - angle;
  return angle;
}



RNScalar GSVMesh::
VertexLaserDepth(R3MeshVertex *v) const
{
  // Return vertex depth
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return RN_UNKNOWN;
  const GSVPose& pose = scanline->Pose();
  R3Vector towards = pose.Towards();
  const R3Point& viewpoint = pose.Viewpoint();
  const R3Point& position = VertexPosition(vertex);
  R3Vector vec = position - viewpoint;
  RNScalar depth = vec.Dot(towards);
  return depth;
}



RNScalar GSVMesh::
VertexLaserDistance(R3MeshVertex *v) const
{
  // Return vertex distance
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return RN_UNKNOWN;
  const GSVPose& pose = scanline->Pose();
  const R3Point& viewpoint = pose.Viewpoint();
  const R3Point& position = VertexPosition(vertex);
  return R3Distance(position, viewpoint);
}



R3Point GSVMesh::
VertexLaserViewpoint(R3MeshVertex *v) const
{
  // Return vertex viewpoint
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return R3zero_point;
  const GSVPose& pose = scanline->Pose();
  return pose.Viewpoint();
}



R3Vector GSVMesh::
VertexLaserTowards(R3MeshVertex *v) const
{
  // Return vertex viewpoint
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return R3zero_vector;
  const GSVPose& pose = scanline->Pose();
  return pose.Towards();
}



R3Vector GSVMesh::
VertexLaserUp(R3MeshVertex *v) const
{
  // Return vertex viewpoint
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return R3zero_vector;
  const GSVPose& pose = scanline->Pose();
  return pose.Up();
}



int GSVMesh::
VertexScanlineIndex(R3MeshVertex *v) const
{
  // Return vertex viewpoint
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  GSVScanline *scanline = vertex->scanline;
  if (!scanline) return -1;
  return scanline->ScanIndex();
}



////////////////////////////////////////////////////////////////////////
// GSV-specific manipulation functions 
////////////////////////////////////////////////////////////////////////

int GSVMesh::
LoadScan(GSVScan *scan, 
  RNLength min_viewpoint_movement,
  RNLength max_depth_discontinuity)
{
  // Just checking
  if (!scan) return 0;
  if (scan->NScanlines() == 0) return 1;

  // Read points
  scan->ReadPoints();

  // Compute whether faces sould be flipped (clockwise)
  RNBoolean flip = (scan->SegmentIndex() == 2) ? 1 : 0;

  // Compute total number of vertices and length of scan viewpoint path
  int total_vertex_count = 0;
  RNScalar total_travel_distance = 0;
  R3Point prev_viewpoint = scan->Scanline(0)->Pose().Viewpoint();
  for (int ie = 0; ie < scan->NScanlines(); ie++) {
    GSVScanline *scanline = scan->Scanline(ie);
    const GSVPose& pose = scanline->Pose();
    const R3Point& viewpoint = pose.Viewpoint();
    RNLength movement = R3Distance(viewpoint, prev_viewpoint);
    total_travel_distance += movement;
    if (movement < min_viewpoint_movement) continue;
    prev_viewpoint = viewpoint;
    total_vertex_count += scanline->NPoints();
  }

  // Compute number of rows and columns
  if (total_travel_distance == 0) { scan->ReleasePoints(); return 1; }
  int ncols = (int) (total_travel_distance / min_viewpoint_movement + 0.5);
  if (ncols == 0) { scan->ReleasePoints(); return 1; }
  const int nrows = 180;
  int col = 0;

  // Allocate vertex data
  gsv_vertex_block = new GSVMeshVertex [ total_vertex_count ];
  if (!gsv_vertex_block) { 
    RNFail("Unable to allocate GSV Mesh vertex array\n"); 
    scan->ReleasePoints(); 
    return 0; 
  }

  // Initialize vertex buffer
  int buffer_index = 0;
  GSVMeshVertex *vertex_buffer[2][nrows];
  for (int k = 0; k < nrows; k++) {
    vertex_buffer[0][k] = NULL;
    vertex_buffer[1][k] = NULL;
  }

  // Create mesh
  int vertex_count = 0;
  RNScalar travel_distance = 0;
  prev_viewpoint = scan->Scanline(0)->Pose().Viewpoint();
  for (int ie = 0; ie < scan->NScanlines(); ie++) {
    GSVScanline *scanline = scan->Scanline(ie);
    const GSVPose& pose = scanline->Pose();
    const R3Point& viewpoint = pose.Viewpoint();
    RNLength movement = R3Distance(viewpoint, prev_viewpoint);
    if (movement < min_viewpoint_movement) continue;
    travel_distance += movement;
    prev_viewpoint = viewpoint;
    const R3Vector& towards = pose.Towards();
    R3Vector up = pose.Up();

    // Initialize buffer of vertices in scanline indexed by row
    for (int k = 0; k < nrows; k++) {
      vertex_buffer[buffer_index][k] = NULL;
    }

    // Create vertices for scanline
    for (int j = 0; j < scanline->NPoints(); j++) {
      R3Point position = scanline->PointPosition(j);
      R3Vector v = position - viewpoint;
      RNLength distance = v.Length();
      if (RNIsZero(distance)) continue;
      v /= distance;
      RNScalar dot = v.Dot(towards);
      RNAngle angle = acos(dot);
      if (v.Dot(up) > 0) angle = RN_PI_OVER_TWO + angle;
      else angle = RN_PI_OVER_TWO - angle;
      int row = (int) (nrows * angle / RN_PI + 0.5);
      if ((row < 0) || (row >= nrows)) continue;
      if (vertex_buffer[buffer_index][row]) continue;
      assert(vertex_count < total_vertex_count);
      GSVMeshVertex *vertex = &gsv_vertex_block[vertex_count];
      vertex_buffer[buffer_index][row] = vertex;
      CreateVertex(position, vertex);
      vertex->scanline = scanline;
      vertex->point_index = j;
      vertex->boundary_type = GSV_MESH_NO_BOUNDARY;
      if (j == 0) vertex->boundary_type = GSV_MESH_BOTTOM_BOUNDARY;
      else if (j == (scanline->NPoints()-1)) vertex->boundary_type = GSV_MESH_TOP_BOUNDARY;
      else if (ie == 0) vertex->boundary_type = GSV_MESH_LEFT_BOUNDARY;
      else if (ie == (scan->NScanlines()-1)) vertex->boundary_type = GSV_MESH_RIGHT_BOUNDARY;
      vertex_count++;
    }

    // Create faces between this and previous scanline
    if (col > 0) {
      int row0 = 0;
      int row1 = 0;
      while ((row0 < nrows) && (!vertex_buffer[1-buffer_index][row0])) row0++;
      while ((row1 < nrows) && (!vertex_buffer[buffer_index][row1])) row1++;
      if ((row0 < nrows) && (row1 < nrows)) {
        GSVMeshVertex *vertex0 = vertex_buffer[1-buffer_index][row0];
        GSVMeshVertex *vertex1 = vertex_buffer[buffer_index][row1];
        while ((row0 < nrows-1) || (row1 < nrows-1)) {
          // Get next vertex
          int row = 0;
          GSVMeshVertex *vertex = NULL;
          if (row0 < row1) {
            row0++;
            vertex = vertex_buffer[1-buffer_index][row0];
            row = 0;
          }
          else {
            row1++;
            vertex = vertex_buffer[buffer_index][row1];
            row = 1;
          }

          // Check vertex
          if (vertex && vertex0 && vertex1) {
            // Mark vertex types
            RNBoolean curtain = FALSE;
            RNLength depth = VertexLaserDepth(vertex);
            RNLength depth0 = VertexLaserDepth(vertex0);
            RNLength depth1 = VertexLaserDepth(vertex1);
            if (max_depth_discontinuity > 0) {
              const R3Point& position0 = VertexPosition(vertex0);
              const R3Point& position1 = VertexPosition(vertex1);
              R3Vector normal_vector0 = VertexNormal(vertex0);
              R3Vector normal_vector1 = VertexNormal(vertex1);
              R3Vector normal_vector = 0.5 * (normal_vector0 + normal_vector1);
              normal_vector.Normalize();
              R3Vector laser_vector0 = VertexLaserViewpoint(vertex0) - position0;
              laser_vector0.Normalize();
              R3Vector laser_vector1 = VertexLaserViewpoint(vertex1)- position1;
              laser_vector1.Normalize();
              R3Vector laser_vector = 0.5 * (laser_vector0 + laser_vector1);
              laser_vector.Normalize();
              RNScalar dot = fabs(laser_vector.Dot(normal_vector));
              if (dot < 0.5) dot = 0.5;
              RNLength max_depth_diff = max_depth_discontinuity / dot;
              if (depth - depth0 > max_depth_diff) {
                vertex0->boundary_type = GSV_MESH_SILHOUETTE_BOUNDARY;
                vertex->boundary_type = GSV_MESH_SHADOW_BOUNDARY;
                curtain = TRUE;
              }
              else if (depth0 - depth > max_depth_diff) {
                vertex->boundary_type = GSV_MESH_SILHOUETTE_BOUNDARY;
                vertex0->boundary_type = GSV_MESH_SHADOW_BOUNDARY;
                curtain = TRUE;
              }
              if (depth - depth1 > max_depth_diff) {
                vertex1->boundary_type = GSV_MESH_SILHOUETTE_BOUNDARY;
                vertex->boundary_type = GSV_MESH_SHADOW_BOUNDARY;
                curtain = TRUE;
              }
              else if (depth1 - depth > max_depth_diff) {
                vertex->boundary_type = GSV_MESH_SILHOUETTE_BOUNDARY;
                vertex1->boundary_type = GSV_MESH_SHADOW_BOUNDARY;
                curtain = TRUE;
              }
            }
            
            // Create face
            if (!curtain) {
              R3MeshFace *face = NULL;
              if (flip) face = CreateFace(vertex0, vertex, vertex1);
              else face = CreateFace(vertex0, vertex1, vertex);
              if (face) SetFaceSegment(face, 1);
            }
          }
          
          // Update stuff for next triangle
          if (vertex) {
            if (row == 0) vertex0 = vertex;
            else vertex1 = vertex;
          }
        }
      }
    }

    // Update stuff for next scanline
    buffer_index = 1 - buffer_index;
    col++;
  }

  // Release points
  scan->ReleasePoints(); 

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// R3Mesh Manipulation functions 
// (override R3Mesh functions to return GSVMeshVertex instead of R3MeshVertex)
////////////////////////////////////////////////////////////////////////

GSVMeshVertex *GSVMesh::
MergeVertex(R3MeshVertex *v1, R3MeshVertex *v2)
{
  // Get GSV vertex info
  GSVScanline *scanline = VertexScanline(v1);
  int point_index = VertexPointIndex(v1);
  int boundary_type = VertexBoundaryType(v1);

  // Merge vertices
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::MergeVertex(v1, v2);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
CollapseEdge(R3MeshEdge *edge, const R3Point& point)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnEdge(edge, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Collapse edge
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::CollapseEdge(edge, point);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
CollapseFace(R3MeshFace *face, const R3Point& point)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnFace(face, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Collapse face
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::CollapseFace(face, point);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
CollapseEdge(R3MeshEdge *edge)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnEdge(edge, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Collapse edge
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::CollapseEdge(edge);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
CollapseFace(R3MeshFace *face)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnFace(face, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Collapse face
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::CollapseFace(face);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
SplitEdge(R3MeshEdge *edge, const R3Point& point, R3MeshEdge **e0, R3MeshEdge **e1)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnEdge(edge, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Split edge
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::SplitEdge(edge, point, e0, e1);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
SplitEdge(R3MeshEdge *edge, const R3Plane& plane)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnEdge(edge, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Split edge
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::SplitEdge(edge, plane);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
SubdivideEdge(R3MeshEdge *edge)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnEdge(edge, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Subdivide edge
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::SubdivideEdge(edge);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



GSVMeshVertex *GSVMesh::
SplitFace(R3MeshFace *face, const R3Point& point, 
  R3MeshFace **f0, R3MeshFace **f1, R3MeshFace **f2)
{
  // Get GSV vertex info
  GSVMeshVertex *v0 = VertexOnFace(face, 0);
  GSVScanline *scanline = VertexScanline(v0);
  int point_index = VertexPointIndex(v0);
  int boundary_type = VertexBoundaryType(v0);

  // Split face
  GSVMeshVertex *vertex = (GSVMeshVertex *) R3Mesh::SplitFace(face, point, f0, f1, f2);

  // Assign GSV properties to new vertex
  if (vertex) {
    vertex->scanline = scanline;
    vertex->point_index = point_index;
    vertex->boundary_type = boundary_type;
  }

  // Return new vertex
  return vertex;
}



////////////////////////////////////////////////////////////////////////
// Vertex creation function 
// (override R3Mesh functions to create GSVMeshVertex instead of R3MeshVertex)
////////////////////////////////////////////////////////////////////////

R3MeshVertex *GSVMesh::
CreateVertex(const R3Point& position, R3MeshVertex *v)
{
  // Create GSV vertex
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  if (!v) {
    vertex = new GSVMeshVertex();
    vertex->flags.Add(R3_MESH_VERTEX_ALLOCATED);
  }

  // Fill in GSV stuff
  vertex->scanline = NULL;
  vertex->point_index = -1;
  vertex->boundary_type = GSV_MESH_NO_BOUNDARY;

  // Fill in R3Mesh stuff 
  return R3Mesh::CreateVertex(position, vertex);
}



R3MeshVertex *GSVMesh::
CreateVertex(const R3Point& position, const R3Vector& normal, R3MeshVertex *v)
{
  // Create GSV vertex
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  if (!vertex) {
    vertex = new GSVMeshVertex();
    vertex->flags.Add(R3_MESH_VERTEX_ALLOCATED);
  }

  // Fill in GSV stuff
  vertex->scanline = NULL;
  vertex->point_index = -1;
  vertex->boundary_type = GSV_MESH_NO_BOUNDARY;

  // Fill in R3Mesh stuff 
  return R3Mesh::CreateVertex(position, normal, vertex);
}



////////////////////////////////////////////////////////////////////////
// Draw functions
////////////////////////////////////////////////////////////////////////

void GSVMesh::
Draw(void) const
{
  // Draw without texture coordinates by default
  Draw(FALSE);
}



void GSVMesh::
Draw(RNBoolean texture_coordinates) const
{
  if (texture_coordinates) {
    RNGrfxBegin(RN_GRFX_TRIANGLES);
    for (int i = 0; i < NFaces(); i++) {
      GSVMeshFace *face = Face(i);
      R3LoadNormal(FaceNormal(face));
      GSVMeshVertex *v0 = VertexOnFace(face, 0);
      int s0 = VertexScanlineIndex(v0);
      int t0 = (int) (180.0 * VertexLaserAngle(v0) / RN_PI + 0.5);
      R3LoadTextureCoords(s0, t0);
      R3LoadPoint(VertexPosition(v0));
      GSVMeshVertex *v1 = VertexOnFace(face, 1);
      int s1 = VertexScanlineIndex(v1);
      int t1 = (int) (180.0 * VertexLaserAngle(v1) / RN_PI + 0.5);
      R3LoadTextureCoords(s1, t1);
      R3LoadPoint(VertexPosition(v1));
      GSVMeshVertex *v2 = VertexOnFace(face, 2);
      int s2 = VertexScanlineIndex(v2);
      int t2 = (int) (180.0 * VertexLaserAngle(v2) / RN_PI + 0.5);
      R3LoadTextureCoords(s2, t2);
      R3LoadPoint(VertexPosition(v2));
    }
    RNGrfxEnd();
  }
  else {
    RNGrfxBegin(RN_GRFX_TRIANGLES);
    for (int i = 0; i < NFaces(); i++) {
      GSVMeshFace *face = Face(i);
      R3LoadNormal(FaceNormal(face));
      GSVMeshVertex *v0 = VertexOnFace(face, 0);
      R3LoadPoint(VertexPosition(v0));
      GSVMeshVertex *v1 = VertexOnFace(face, 1);
      R3LoadPoint(VertexPosition(v1));
      GSVMeshVertex *v2 = VertexOnFace(face, 2);
      R3LoadPoint(VertexPosition(v2));
    }
    RNGrfxEnd();
  }
}



////////////////////////////////////////////////////////////////////////
// Intersection functions
////////////////////////////////////////////////////////////////////////

R3MeshType GSVMesh::
Intersection(const R3Ray& ray, R3MeshIntersection *intersection) const
{
  // Return intersection 
  return Intersection(ray, intersection, 0, RN_INFINITY, NULL, NULL);
}



R3MeshType GSVMesh::
Intersection(const R3Ray& ray, R3MeshIntersection *intersection, 
  RNScalar min_t, RNScalar max_t,
  int (*IsCompatible)(const R3Point&, const R3Vector&, R3Mesh *, R3MeshFace *, void *), 
  void *compatible_data) const
{
  // Create search tree
  if (!search_tree) {
    GSVMesh *mesh = (GSVMesh *) this;
    mesh->search_tree = new R3MeshSearchTree((R3Mesh *) this);
    if (!search_tree) {
      RNFail("Unable to build mesh search tree\n");
      return R3_MESH_NULL_TYPE;
    }
  }

  // Use search tree to find intersection
  R3MeshIntersection closest;
  search_tree->FindIntersection(ray, closest, min_t, max_t, IsCompatible, compatible_data);
  if (intersection) *intersection = closest;
  return closest.type;
}



R3MeshType GSVMesh::
Intersection(const R3Ray& ray, R3MeshFace *face, R3MeshIntersection *intersection) const
{
  // Return ray-face intersection
  return R3Mesh::Intersection(ray, face, intersection);
}



R3MeshType GSVMesh::
ClosestPoint(const R3Point& query, R3MeshIntersection *closest, 
  RNScalar min_distance, RNScalar max_distance,
  int (*IsCompatible)(const R3Point&, const R3Vector&, R3Mesh *, R3MeshFace *, void *), 
  void *compatible_data)
{
  // Create search tree
  if (!search_tree) {
    GSVMesh *mesh = (GSVMesh *) this;
    mesh->search_tree = new R3MeshSearchTree((R3Mesh *) this);
    if (!search_tree) {
      RNFail("Unable to build mesh search tree\n");
      return R3_MESH_NULL_TYPE;
    }
  }

  // Use search tree to find closest point
  R3MeshIntersection result;
  search_tree->FindClosest(query, result, min_distance, max_distance, IsCompatible, compatible_data);
  if (closest) *closest = result;
  return result.type;
}  



////////////////////////////////////////////////////////////////////////
// Ply read/write functions
////////////////////////////////////////////////////////////////////////

int GSVMesh::
ReadPlyFile(const char *filename)
{
  // Overridden
  return R3Mesh::ReadPlyFile(filename);
}



int GSVMesh::
ReadPlyFile(GSVScan *scan, const char *filename)
{
  FILE *fp;
  int i,j;
  PlyFile *ply;
  int nelems;
  PlyProperty **plist;
  char **elist;
  int file_type;
  int nprops;
  int num_elems;
  char *elem_name;
  float version;

  typedef struct PlyVertex {
    float x, y, z;
    int scanline_index;
    int point_index;
    int boundary_type;
  } PlyVertex;

  typedef struct PlyFace {
    unsigned char nverts;
    int *verts;
    int segment;
  } PlyFace;

  // List of property information for a vertex 
  // List of property information for a vertex 
  static PlyProperty vert_props[] = { 
    {(char *) "x", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,x), 0, 0, 0, 0},
    {(char *) "y", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,y), 0, 0, 0, 0},
    {(char *) "z", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,z), 0, 0, 0, 0},
    {(char *) "scanline_index", PLY_INT, PLY_INT, offsetof(PlyVertex,scanline_index), 0, 0, 0, 0},
    {(char *) "point_index", PLY_INT, PLY_INT, offsetof(PlyVertex,point_index), 0, 0, 0, 0},
    {(char *) "boundary_type", PLY_INT, PLY_INT, offsetof(PlyVertex,boundary_type), 0, 0, 0, 0}
  };

  // List of property information for a vertex 
  static PlyProperty face_props[] = { 
    {(char *) "vertex_indices", PLY_INT, PLY_INT, offsetof(PlyFace,verts), 1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyFace,nverts)},
    {(char *) "segment", PLY_INT, PLY_INT, offsetof(PlyFace,segment), 0, 0, 0, 0}
  };

  // Open file 
  fp = fopen(filename, "rb");
  if (!fp) {
    RNFail("Unable to open file: %s", filename);
    return 0;
  }

  // Read PLY header
  ply = ply_read (fp, &nelems, &elist);
  if (!ply) {
    RNFail("Unable to read ply file: %s", filename);
    fclose(fp);
    return 0;
  }
  
  // Get header info
  ply_get_info (ply, &version, &file_type);

  // Read obj infos
  int scan_index = 0;
  int segment_index = 0;
  char run_name[1024];
  int num_obj_infos = 0;
  char **obj_infos = ply_get_obj_info(ply, &num_obj_infos);
  for (int i = 0; i < num_obj_infos; i++) {
    char obj_info[1024];
    strncpy(obj_info, obj_infos[i], 1023);
    char *info_type = strtok(obj_info, " ");
    if (!info_type) continue;
    char *valuep = strtok(NULL, " ");
    if (!strcmp(info_type, "scan_index")) scan_index = atoi(valuep);
    else if (!strcmp(info_type, "segment_index")) segment_index = atoi(valuep);
    else if (!strcmp(info_type, "run_name")) strcpy(run_name, valuep);
  }

  // To silence compiler about unused variables
  if (scan_index == -146) printf("Wow\n");
  if (segment_index == -146) printf("Wow\n");

  // Read all elements
  for (i = 0; i < nelems; i++) {
    // Get the description of the element 
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    // Check element type
    if (equal_strings ("vertex", elem_name)) {
      // Allocate block of vertices
      gsv_vertex_block = new GSVMeshVertex [num_elems];
      if (!gsv_vertex_block) {
        RNFail("Unable to allocate vertex matrix\n");
        return 0;
      }

      // Resize array of vertices
      vertices.Resize(num_elems);

      // set up for getting vertex elements 
      for (j = 0; j < nprops; j++) {
	if (equal_strings("x", plist[j]->name)) ply_get_property (ply, elem_name, &vert_props[0]);
	else if (equal_strings("y", plist[j]->name)) ply_get_property (ply, elem_name, &vert_props[1]);
	else if (equal_strings("z", plist[j]->name)) ply_get_property (ply, elem_name, &vert_props[2]);
	else if (equal_strings("scanline_index", plist[j]->name)) ply_get_property (ply, elem_name, &vert_props[3]);
	else if (equal_strings("point_index", plist[j]->name)) ply_get_property (ply, elem_name, &vert_props[4]);
	else if (equal_strings("boundary_type", plist[j]->name)) ply_get_property (ply, elem_name, &vert_props[5]);
      }

      // Grab all the vertex elements 
      for (j = 0; j < num_elems; j++) {
        // Read vertex into local struct
        PlyVertex plyvertex;
        plyvertex.scanline_index = -1;
        plyvertex.point_index = -1;
        plyvertex.boundary_type = 0;
        ply_get_element(ply, (void *) &plyvertex);

        // Create mesh vertex
        R3Point position(plyvertex.x, plyvertex.y, plyvertex.z);
        GSVMeshVertex *v = (GSVMeshVertex *) CreateVertex(position, &gsv_vertex_block[j]);
        v->scanline = (scan && (plyvertex.scanline_index >= 0)) ? scan->Scanline(plyvertex.scanline_index) : NULL;
        v->point_index = plyvertex.point_index;
        v->boundary_type = plyvertex.boundary_type;
      }
    }
    else if (equal_strings ("face", elem_name)) {
      // Resize array of faces
      faces.Resize(num_elems);

      // set up for getting face elements 
      for (j = 0; j < nprops; j++) {
	if (equal_strings("vertex_indices", plist[j]->name)) ply_get_property (ply, elem_name, &face_props[0]);
	else if (equal_strings("segment", plist[j]->name)) ply_get_property (ply, elem_name, &face_props[1]);
      }

      // grab all the face elements 
      for (j = 0; j < num_elems; j++) {
        // Read face into local struct
        PlyFace plyface;
        plyface.nverts = 0;
        plyface.verts = NULL;
        plyface.segment = 0;
        ply_get_element(ply, (void *) &plyface);

        // Create mesh face(s)
        R3MeshVertex *v1 = vertices[plyface.verts[0]];
        for (int k = 2; k < plyface.nverts; k++) {
          // Get vertices
          R3MeshVertex *v2 = vertices[plyface.verts[k-1]];
          R3MeshVertex *v3 = vertices[plyface.verts[k]];

          // Check plyface
          assert(plyface.verts[0] >= 0);
          assert(plyface.verts[k-1] >= 0);
          assert(plyface.verts[k] >= 0);
          assert(plyface.verts[0] < vertices.NEntries());
          assert(plyface.verts[k-1] < vertices.NEntries());
          assert(plyface.verts[k] < vertices.NEntries());

          // Check vertices
          if ((v1 == v2) || (v2 == v3) || (v1 == v3)) continue;

          // Create face
          R3MeshFace *f = CreateFace(v1, v2, v3);
          if (!f) {
            // Must have been degeneracy (e.g., three faces sharing an edge), create new vertices
            // Note: these vertices are allocated separately, and so they will not be deleted (memory leak)
            // R3MeshVertex *v1a = CreateVertex(VertexPosition(v1));
            // R3MeshVertex *v2a = CreateVertex(VertexPosition(v2));
            // R3MeshVertex *v3a = CreateVertex(VertexPosition(v3));
            // f = CreateFace(v1a, v2a, v3a);
          } 

          // Set boundary_type
          if (f) SetFaceSegment(f, plyface.segment);
        }

        // Free face data allocated by ply 
        if (plyface.verts) free(plyface.verts);
      }
    }
    else {
      ply_get_other_element (ply, elem_name, num_elems);
    }
  }

  // Close the file 
  ply_close (ply);

  // Return number of faces created
  return faces.NEntries();
}



int GSVMesh::
WritePlyFile(const char *filename, RNBoolean binary) const
{
  // Overridden
  return R3Mesh::WritePlyFile(filename, binary);
}



int GSVMesh::
WritePlyFile(GSVScan *scan, const char *filename, RNBoolean binary) const
{
  typedef struct PlyVertex {
    float x, y, z;
    int scanline_index;
    int point_index;
    int boundary_type;
  } PlyVertex;

  typedef struct PlyFace {
    unsigned char nverts;
    int *verts;
    int segment;
  } PlyFace;

  // Element names
  char *elem_names[] = { (char *) "vertex", (char *) "face" };

  // List of property information for a vertex 
  static PlyProperty vert_props[] = { 
    {(char *) "x", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,x), 0, 0, 0, 0},
    {(char *) "y", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,y), 0, 0, 0, 0},
    {(char *) "z", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,z), 0, 0, 0, 0},
    {(char *) "scanline_index", PLY_INT, PLY_INT, offsetof(PlyVertex,scanline_index), 0, 0, 0, 0},
    {(char *) "point_index", PLY_INT, PLY_INT, offsetof(PlyVertex,point_index), 0, 0, 0, 0},
    {(char *) "boundary_type", PLY_INT, PLY_INT, offsetof(PlyVertex,boundary_type), 0, 0, 0, 0}
  };

  // List of property information for a vertex 
  static PlyProperty face_props[] = { 
    {(char *) "vertex_indices", PLY_INT, PLY_INT, offsetof(PlyFace,verts), 1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyFace,nverts)},
    {(char *) "segment", PLY_INT, PLY_INT, offsetof(PlyFace,segment), 0, 0, 0, 0}
  };

  // Open ply file
  float version;
  int file_type = (binary) ? PLY_BINARY_NATIVE : PLY_ASCII;
  PlyFile *ply = ply_open_for_writing((char *) filename, 2, elem_names, file_type, &version);
  if (!ply) return -1;

  // Describe vertex properties
  ply_element_count(ply, (char *) "vertex", NVertices());
  for (unsigned int i = 0; i < sizeof(vert_props) / sizeof(PlyProperty); i++) {
    ply_describe_property(ply, (char *) "vertex", &vert_props[i]);
  }

  // Describe face properties
  ply_element_count(ply, (char *) "face", NFaces());
  for (unsigned int i = 0; i < sizeof(face_props) / sizeof(PlyProperty); i++) {
    ply_describe_property(ply, (char *) "face", &face_props[i]);
  }

  // Describe scan properties
  char buffer[1024];
  GSVSegment *segment = (scan) ? scan->Segment() : NULL;
  GSVRun *run = (segment) ? segment->Run() : NULL;
  sprintf(buffer, "run_name %s", (run) ? run->Name() : "None");
  ply_put_obj_info(ply, buffer);
  sprintf(buffer, "segment_index %d", (segment) ? segment->RunIndex() : -1);
  ply_put_obj_info(ply, buffer);
  sprintf(buffer, "scan_index %d", (scan) ? scan->SegmentIndex() : -1);
  ply_put_obj_info(ply, buffer);

  // Complete header
  ply_header_complete(ply);

  // Write vertices
  ply_put_element_setup(ply, (char *) "vertex");
  for (int i = 0; i < NVertices(); i++) {
    GSVMeshVertex *v = (GSVMeshVertex *) Vertex(i);
    const R3Point& position = VertexPosition(v);
    PlyVertex ply_vertex;
    ply_vertex.x = position.X();
    ply_vertex.y = position.Y();
    ply_vertex.z = position.Z();
    ply_vertex.scanline_index = (v->scanline) ? v->scanline->ScanIndex() : -1;
    ply_vertex.point_index = v->point_index;
    ply_vertex.boundary_type = v->boundary_type;
    ply_put_element(ply, (void *) &ply_vertex);
  }

  // Write faces
  ply_put_element_setup(ply, (char *) "face");
  for (int i = 0; i < NFaces(); i++) {
    R3MeshFace *f = Face(i);
    static int verts[3];
    static PlyFace ply_face = { 3, verts, 0 };
    ply_face.verts[0] = VertexID(VertexOnFace(f, 0));
    ply_face.verts[1] = VertexID(VertexOnFace(f, 1));
    ply_face.verts[2] = VertexID(VertexOnFace(f, 2));
    ply_face.segment = FaceSegment(f);
    ply_put_element(ply, (void *) &ply_face);
  }

  // Close the file 
  ply_close(ply);

  // Return number of faces written
  return NFaces();
}



////////////////////////////////////////////////////////////////////////
// ARFF read/write functions
////////////////////////////////////////////////////////////////////////

int GSVMesh::
WriteARFFFile(const char *filename) const
{
  // Overridden version (can pass NULL because WriteARFFFile doesn't actually use scan)
  return GSVMesh::WriteARFFFile(NULL, filename);
}



int GSVMesh::
WriteARFFFile(GSVScan *scan, const char *filename) const
{
  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open ARFF file %s\n", filename);
    return 0;
  }
  
  // Write header
  fprintf(fp, "@relation GSV\n");
  fprintf(fp, "\n");

  // Write property names
  fprintf(fp, "@attribute ScanlineIndex real\n");
  fprintf(fp, "@attribute PointIndex real\n");
  fprintf(fp, "@attribute Timestamp real\n");
  fprintf(fp, "@attribute PositionX real\n");
  fprintf(fp, "@attribute PositionY real\n");
  fprintf(fp, "@attribute PositionZ real\n");
  fprintf(fp, "@attribute NormalX real\n");
  fprintf(fp, "@attribute NormalY real\n");
  fprintf(fp, "@attribute NormalZ real\n");
  fprintf(fp, "@attribute GroundZ real\n");
  fprintf(fp, "@attribute Height real\n");
  fprintf(fp, "@attribute LaserDepth real\n");
  fprintf(fp, "@attribute LaserDistance real\n");
  fprintf(fp, "@attribute LaserAngle real\n");
  fprintf(fp, "@attribute LaserViewpointX real\n");
  fprintf(fp, "@attribute LaserViewpointY real\n");
  fprintf(fp, "@attribute LaserViewpointZ real\n");
  fprintf(fp, "@attribute LaserTowardsX real\n");
  fprintf(fp, "@attribute LaserTowardsY real\n");
  fprintf(fp, "@attribute LaserTowardsZ real\n");
  fprintf(fp, "@attribute LaserUpX real\n");
  fprintf(fp, "@attribute LaserUpY real\n");
  fprintf(fp, "@attribute LaserUpZ real\n");
  fprintf(fp, "@attribute BoundaryType real\n");
  fprintf(fp, "\n");
  
  // Write property values (one line per vertex)
  fprintf(fp, "@data\n\n");
  for (int i = 0; i < NVertices(); i++) {
    GSVMeshVertex *vertex = Vertex(i);
    const R3Point& position = VertexPosition(vertex);
    const R3Vector& normal = VertexNormal(vertex);
    GSVScanline *scanline = VertexScanline(vertex);
    RNCoord groundZ = (scanline) ? scanline->EstimatedGroundZ() : -1;
    RNLength height = position.Z() - groundZ;
    R3Point viewpoint = VertexLaserViewpoint(vertex);
    R3Vector towards = VertexLaserTowards(vertex);
    R3Vector up = VertexLaserUp(vertex);
    fprintf(fp, "%d ", VertexScanlineIndex(vertex));
    fprintf(fp, "%d ", VertexPointIndex(vertex));
    fprintf(fp, "%g ", VertexTimestamp(vertex));
    fprintf(fp, "%g ", position.X());
    fprintf(fp, "%g ", position.Y());
    fprintf(fp, "%g ", position.Z());
    fprintf(fp, "%g ", normal.X());
    fprintf(fp, "%g ", normal.Y());
    fprintf(fp, "%g ", normal.Z());
    fprintf(fp, "%g ", groundZ);
    fprintf(fp, "%g ", height);
    fprintf(fp, "%g ", VertexLaserDepth(vertex));
    fprintf(fp, "%g ", VertexLaserDistance(vertex));
    fprintf(fp, "%g ", VertexLaserAngle(vertex));
    fprintf(fp, "%g ", viewpoint.X());
    fprintf(fp, "%g ", viewpoint.Y());
    fprintf(fp, "%g ", viewpoint.Z());
    fprintf(fp, "%g ", towards.X());
    fprintf(fp, "%g ", towards.Y());
    fprintf(fp, "%g ", towards.Z());
    fprintf(fp, "%g ", up.X());
    fprintf(fp, "%g ", up.Y());
    fprintf(fp, "%g ", up.Z());
    fprintf(fp, "%d ", VertexBoundaryType(vertex));
    fprintf(fp, "\n");
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Vertex member functions
////////////////////////////////////////////////////////////////////////

GSVMeshVertex::
GSVMeshVertex(void)
  : R3MeshVertex(),
    scanline(NULL),
    point_index(-1),
    boundary_type(GSV_MESH_NO_BOUNDARY)
{
}



GSVMeshVertex::
~GSVMeshVertex(void)
{
}



} // namespace gaps
