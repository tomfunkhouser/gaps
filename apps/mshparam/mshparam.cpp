// Source file for the mesh segmentation program

////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Shapes/R3Shapes.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static char *input_mesh_name = NULL;
static char *output_mesh_name = NULL;
static int separate_face_segments = 0;
static int separate_face_materials = 0;
static int flatten = 0;
static int print_verbose = 0;



////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////

struct Segment {
  R3Mesh *mesh;
  RNArray<R3MeshFace *> faces;
  RNArray<R3MeshVertex *> vertices;
  RNMap<R3MeshVertex *, R3MeshVertex *> vertex_map;
  R4Matrix matrix;
  R2Box bbox;
  int id;
  int index;
};



////////////////////////////////////////////////////////////////////////
// Input/output stuff
////////////////////////////////////////////////////////////////////////

static R3Mesh *
ReadMesh(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate mesh
  R3Mesh *mesh = new R3Mesh();
  assert(mesh);

  // Read mesh from file
  if (!mesh->ReadFile(filename)) {
    delete mesh;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read mesh from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Faces = %d\n", mesh->NFaces());
    printf("  # Edges = %d\n", mesh->NEdges());
    printf("  # Vertices = %d\n", mesh->NVertices());
    fflush(stdout);
  }

  // Return mesh
  return mesh;
}



static int
WriteMesh(R3Mesh *mesh, char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Write mesh
  if (!mesh->WriteFile(filename)) {
    fprintf(stderr, "Unable to write mesh to %s\n", filename);
    return 0;
  }

  // Print statistics
  if (print_verbose) {
    printf("Wrote mesh to %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Faces = %d\n", mesh->NFaces());
    printf("  # Edges = %d\n", mesh->NEdges());
    printf("  # Vertices = %d\n", mesh->NVertices());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Segmentation stuff
////////////////////////////////////////////////////////////////////////

static int 
CompareSegments(const void *data1, const void *data2)
{
  // Sort by bbox.xlength
  Segment *segment1 = *((Segment **) data1);
  Segment *segment2 = *((Segment **) data2);
  if (segment1->bbox.XLength() > segment2->bbox.XLength()) return -1;
  else if (segment1->bbox.XLength() < segment2->bbox.XLength()) return 1;
  else return 0;
}
  


static int 
CreateSegments(R3Mesh *mesh, RNArray<Segment *>& segments)
{
  // Remember original vertices
  RNArray<R3MeshVertex *> original_vertices;
  for (int i = 0; i < mesh->NVertices(); i++) {
    R3MeshVertex *original_vertex = mesh->Vertex(i);
    original_vertices.Insert(original_vertex);
  }

  // Remember original faces
  RNArray<R3MeshFace *> original_faces;
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *original_face = mesh->Face(i);
    original_faces.Insert(original_face);
  }
  
  // Create segments
  RNMap<int, Segment *> segment_map;
  R3MeshVertex *segment_face_vertices[3];
  for (int i = 0; i < original_faces.NEntries(); i++) {
    R3MeshFace *original_face = original_faces.Kth(i);
    int id = 0;
    if (separate_face_segments) id = mesh->FaceSegment(original_face);
    else if (separate_face_materials) id = mesh->FaceMaterial(original_face);

    // Find segment
    Segment *segment = NULL;
    if (!segment_map.Find(id, &segment)) {
      segment = new Segment();
      segment->mesh = mesh;
      segment->matrix = R4identity_matrix;
      segment->bbox = R2null_box;
      segment->id = id;
      segment->index = segments.NEntries();
      segment_map.Insert(id, segment);
      segments.Insert(segment);
    }

    // Find/insert copy of vertices into segment
    for (int j = 0; j < 3; j++) {
      R3MeshVertex *original_vertex = mesh->VertexOnFace(original_face, j);

      // Find/insert vertex in segment
      R3MeshVertex *segment_vertex = NULL;
      if (!segment->vertex_map.Find(original_vertex, &segment_vertex)) {
        segment_vertex = mesh->CreateVertex(*original_vertex);
        assert(segment_vertex);
        segment->vertex_map.Insert(original_vertex, segment_vertex);
        segment->vertices.Insert(segment_vertex);
      }

      // Remember vertex for face
      segment_face_vertices[j] = segment_vertex;
    }
      
    // Insert copy of face into segment
    R3MeshFace *segment_face = mesh->CreateFace(segment_face_vertices[0], segment_face_vertices[1], segment_face_vertices[2]);
    assert(segment_face);
    mesh->SetFaceSegment(segment_face, mesh->FaceSegment(original_face));
    mesh->SetFaceMaterial(segment_face, mesh->FaceMaterial(original_face));
    segment->faces.Insert(segment_face);
  }

  // Empty segment vertex maps (because will delete original vertices next)
  for (int i = 0; i < segments.NEntries(); i++) {
    Segment *segment = segments.Kth(i);
    segment->vertex_map.Empty();
  }

  // Delete original faces
  for (int i = 0; i < original_faces.NEntries(); i++) {
    R3MeshFace *original_face = original_faces.Kth(i);
    mesh->DeleteFace(original_face);
  }

  // Delete original vertices
  for (int i = 0; i < original_vertices.NEntries(); i++) {
    R3MeshVertex *original_vertex = original_vertices.Kth(i);
    mesh->DeleteVertex(original_vertex);
  }

  // Process segments
  for (int i = 0; i < segments.NEntries(); i++) {
    Segment *segment = segments.Kth(i);

    // Gather info for vertices of segment
    RNArray<R3Point *> points;
    R3Vector sum_of_normals = R3zero_vector;
    for (int i = 0; i < segment->vertices.NEntries(); i++) {
      R3MeshVertex *vertex = segment->vertices.Kth(i);
      sum_of_normals += mesh->VertexNormal(vertex);
      points.Insert((R3Point *) &(mesh->VertexPosition(vertex)));
    }

    // Create a coordinate system for segment
    R3Point centroid = R3Centroid(points);
    R3Triad triad = R3PrincipleAxes(centroid, points);
    if (triad[2].Dot(sum_of_normals) < 0) triad.Rotate(triad[0], RN_PI);
    R3CoordSystem cs(centroid, triad);
    segment->matrix = cs.InverseMatrix();

    // SOMEHOW THE PARAMETERIZATION FOR 1/2 OF SEGMENTS IS FLIPPED BACKWARDS

    // Determine extent of segment in texture coordinates
    segment->bbox = R2null_box;
    for (int j = 0; j < segment->vertices.NEntries(); j++) {
      R3MeshVertex *vertex = segment->vertices.Kth(j);
      R3Point position = mesh->VertexPosition(vertex);
      position = segment->matrix * position;
      R2Point texcoords(position.X(), position.Y());
      segment->bbox.Union(texcoords);
    }
  }

  // Sort segments by ylength
  segments.Sort(CompareSegments);
  
  // Return success
  return 1;
}
    


static int 
DeleteSegments(R3Mesh *mesh, const RNArray<Segment *>& segments)
{
  // Delete each segment
  for (int i = 0; i < segments.NEntries(); i++) {
    Segment *segment = segments.Kth(i);
    delete segment;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Parameterization stuff
////////////////////////////////////////////////////////////////////////

static int 
ParameterizeSegments(R3Mesh *mesh, const RNArray<Segment *>& segments)
{
  // Initialize location of segment in texture coordinates
  R2Point origin(0,0);
  RNLength max_xlength = 0;
  RNLength max_yorigin = 10;

  // Parameterize each segment
  for (int i = 0; i < segments.NEntries(); i++) {
    Segment *segment = segments.Kth(i);

    // Update matrix to start texcoords bbox for segment at origin
    R4Matrix matrix = segment->matrix;
    matrix[0][3] += origin.X() - segment->bbox.XMin();    
    matrix[1][3] += origin.Y() - segment->bbox.YMin();    
      
    // Assign texture coordinates to vertices
    for (int j = 0; j < segment->vertices.NEntries(); j++) {
      R3MeshVertex *vertex = segment->vertices.Kth(j);
      R3Point position = mesh->VertexPosition(vertex);
      position = matrix * position;
      R2Point texcoords(position.X(), position.Y());
      mesh->SetVertexTextureCoords(vertex, texcoords);
    }

    // Update max xlength
    if (segment->bbox.XLength() > max_xlength) max_xlength = segment->bbox.XLength();

    // Update origin
    origin[1] += segment->bbox.YLength();
    if (origin[1] > max_yorigin) {
      origin[0] += max_xlength;
      origin[1] = 0;
      max_xlength = 0;
    }
  }
  
  // Return success
  return 1;
}



static int
ParameterizeMesh(R3Mesh *mesh)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Create segments
  RNArray<Segment *> segments;
  if (!CreateSegments(mesh, segments)) return 0;

  // Parameterize segments
  if (!ParameterizeSegments(mesh, segments)) return 0;
  
  // Delete segments
  if (!DeleteSegments(mesh, segments)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("Parameterized mesh ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Segments = %d\n", segments.NEntries());
    fflush(stdout);
  }

  // Return success
  return 1;
}




static int
FlattenMesh(R3Mesh *mesh)
{
  // This is useful for debugging
  if (!flatten) return 1;

  for (int i = 0; i < mesh->NVertices(); i++) {
    R3MeshVertex *vertex = mesh->Vertex(i);
    R2Point texcoords = mesh->VertexTextureCoords(vertex);
    R3Point flattened_position(texcoords.X(), texcoords.Y(), 0);
    mesh->SetVertexPosition(vertex, flattened_position);
  }

  // Return success
  return 1;
}




////////////////////////////////////////////////////////////////////////
// Argument Parsing Stuff
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-flatten")) flatten = 1;
      else if (!strcmp(*argv, "-separate_face_segments")) separate_face_segments = 1;
      else if (!strcmp(*argv, "-separate_face_materials")) separate_face_materials = 1;
      else if (!strcmp(*argv, "-flatten")) flatten = 1;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
    else {
      if (!input_mesh_name) input_mesh_name = *argv;
      else if (!output_mesh_name) output_mesh_name = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!input_mesh_name || !output_mesh_name) {
    fprintf(stderr, "Usage: mshparam inputmesh outputmesh\n");
    return 0;
  }

  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////

int 
main(int argc, char **argv)
{
  // Check number of arguments
  if (!ParseArgs(argc, argv)) exit(1);

  // Read input mesh
  R3Mesh *mesh = ReadMesh(input_mesh_name);
  if (!mesh) exit(-1);

  // Segment mesh
  if (!ParameterizeMesh(mesh)) exit(-1);

  // Flatten mesh
  if (!FlattenMesh(mesh)) exit(-1);

  // Write mesh
  if (!WriteMesh(mesh, output_mesh_name)) exit(-1);

  // Return success 
  return 0;
}



