// Source file for the mesh to mesh distance program



// Include files 

namespace gaps {}
using namespace gaps;
#include "R3Shapes/R3Shapes.h"
#include <vector>
#include <algorithm>



// Program variables

static char *input_mesh1_filename = NULL;
static char *input_mesh2_filename = NULL;
static char *output_property_filename = NULL;
static int print_verbose = 0;



////////////////////////////////////////////////////////////////////////
// Input/Output
////////////////////////////////////////////////////////////////////////

static R3Mesh *
ReadMesh(char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate mesh
  R3Mesh *mesh = new R3Mesh();
  if (!mesh) {
    RNFail("Unable to allocate mesh for %s\n", filename);
    return NULL;
  }

  // Read mesh from file
  if (!mesh->ReadFile(filename)) {
    delete mesh;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read mesh from %s\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Faces = %d\n", mesh->NFaces());
    printf("  # Edges = %d\n", mesh->NEdges());
    printf("  # Vertices = %d\n", mesh->NVertices());
    fflush(stdout);
  }

  // Return success
  return mesh;
}


static int
WriteProperties(R3MeshPropertySet *properties, char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Write properties
  if (!properties->Write(filename)) exit(-1);

  // Print statistics
  if (print_verbose) {
    printf("Wrote properties to %s\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Vertices = %d\n", properties->Mesh()->NVertices());
    printf("  # Properties = %d\n", properties->NProperties());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Property computation
////////////////////////////////////////////////////////////////////////

static R3MeshPropertySet *
CreateMeshPropertySet(R3Mesh *mesh1, R3Mesh *mesh2)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating mesh properties ...\n");
    fflush(stdout);
  }

  // Create property set
  R3MeshPropertySet *properties = new R3MeshPropertySet(mesh1);
  if (!properties) {
    fprintf(stderr, "Unable to open allocate mesh property set\n");
    return NULL;
  }

  // Create properties
  R3MeshProperty *p;
  p = new R3MeshProperty(mesh1, "distance"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "NdotN"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "type1"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "id1"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "category1"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "curvature1"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "boundary1"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "position1.x"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "position1.y"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "position1.z"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "normal1.x"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "normal1.y"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "normal1.z"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "type2"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "id2"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "category2"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "curvature2"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "boundary2"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "position2.x"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "position2.y"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "position2.z"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "normal2.x"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "normal2.y"); properties->Insert(p);
  p = new R3MeshProperty(mesh1, "normal2.z"); properties->Insert(p);
  
  // Compute distances from vertices of mesh1 to closest face in mesh2
  R3MeshIntersection closest2;
  R3MeshSearchTree kdtree2(mesh2);
  for (int i = 0; i < mesh1->NVertices(); i++) {
    R3MeshVertex *vertex1 = mesh1->Vertex(i);
    R3MeshFace *face1 = mesh1->FaceOnVertex(vertex1);
    const R3Point& position1 = mesh1->VertexPosition(vertex1);
    int id1 = mesh1->VertexID(vertex1);
    int type1 = R3_MESH_VERTEX_TYPE;
    R3Vector normal1 = mesh1->VertexNormal(vertex1);
    RNScalar curvature1 = mesh1->VertexMeanCurvature(vertex1);
    RNBoolean boundary1 = mesh1->IsVertexOnBoundary(vertex1);
    int category1 = (face1) ? mesh1->FaceCategory(face1) : -1;
    kdtree2.FindClosest(position1, closest2);
    if (closest2.type == R3_MESH_NULL_TYPE) RNAbort("Error");
    R3MeshFace *face2 = closest2.face;
    int type2 = closest2.type;
    RNLength distance = closest2.t;
    int category2 = (face2) ? mesh2->FaceCategory(face2) : -1;
    R3Point position2 = closest2.point;
    int id2 = 0;
    R3Vector normal2 = R3zero_vector;
    RNScalar curvature2 =0;
    RNBoolean boundary2 = FALSE;
    if (closest2.type == R3_MESH_VERTEX_TYPE) {
      id2 = mesh2->VertexID(closest2.vertex);
      normal2 = mesh2->VertexNormal(closest2.vertex);
      curvature2 = mesh2->VertexMeanCurvature(closest2.vertex);
      boundary2 = mesh2->IsVertexOnBoundary(closest2.vertex);
    }
    else if (closest2.type == R3_MESH_EDGE_TYPE) {
      R3MeshVertex *vA = mesh2->VertexOnEdge(closest2.edge, 0);
      R3MeshVertex *vB = mesh2->VertexOnEdge(closest2.edge, 1);
      RNScalar curvatureA = mesh2->VertexMeanCurvature(vA);
      RNScalar curvatureB = mesh2->VertexMeanCurvature(vB);
      id2 = mesh2->EdgeID(closest2.edge);
      normal2 = mesh2->EdgeNormal(closest2.edge);
      curvature2 = (curvatureA + curvatureB) / 2.0;
      boundary2 = mesh2->IsEdgeOnBoundary(closest2.edge);
    }
    else if (closest2.type == R3_MESH_FACE_TYPE) {
      R3MeshVertex *vA = mesh2->VertexOnFace(closest2.face, 0);
      R3MeshVertex *vB = mesh2->VertexOnFace(closest2.face, 1);
      R3MeshVertex *vC = mesh2->VertexOnFace(closest2.face, 2);
      RNScalar curvatureA = mesh2->VertexMeanCurvature(vA);
      RNScalar curvatureB = mesh2->VertexMeanCurvature(vB);
      RNScalar curvatureC = mesh2->VertexMeanCurvature(vC);
      id2 = mesh2->FaceID(closest2.face);
      normal2 = mesh2->FaceNormal(closest2.face);
      curvature2 = (curvatureA + curvatureB + curvatureC) / 3.0;
      boundary2 = FALSE;
    }

    // Add entries to properties
    int cnt = 0;
    properties->Property(cnt++)->SetVertexValue(i, distance);
    properties->Property(cnt++)->SetVertexValue(i, normal1.Dot(normal2));
    properties->Property(cnt++)->SetVertexValue(i, type1);
    properties->Property(cnt++)->SetVertexValue(i, id1);
    properties->Property(cnt++)->SetVertexValue(i, category1);
    properties->Property(cnt++)->SetVertexValue(i, curvature1);
    properties->Property(cnt++)->SetVertexValue(i, boundary1);
    properties->Property(cnt++)->SetVertexValue(i, position1.X());
    properties->Property(cnt++)->SetVertexValue(i, position1.Y());
    properties->Property(cnt++)->SetVertexValue(i, position1.Z());
    properties->Property(cnt++)->SetVertexValue(i, normal1.X());
    properties->Property(cnt++)->SetVertexValue(i, normal1.Y());
    properties->Property(cnt++)->SetVertexValue(i, normal1.Z());
    properties->Property(cnt++)->SetVertexValue(i, type2);
    properties->Property(cnt++)->SetVertexValue(i, id2);
    properties->Property(cnt++)->SetVertexValue(i, category2);
    properties->Property(cnt++)->SetVertexValue(i, curvature2);
    properties->Property(cnt++)->SetVertexValue(i, boundary2);
    properties->Property(cnt++)->SetVertexValue(i, position2.X());
    properties->Property(cnt++)->SetVertexValue(i, position2.Y());
    properties->Property(cnt++)->SetVertexValue(i, position2.Z());
    properties->Property(cnt++)->SetVertexValue(i, normal2.X());
    properties->Property(cnt++)->SetVertexValue(i, normal2.Y());
    properties->Property(cnt++)->SetVertexValue(i, normal2.Z());
    assert(cnt == properties->NProperties());
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Vertices = %d\n", mesh1->NVertices());
    printf("  # Properties = %d\n", properties->NProperties());
    fflush(stdout);
  }

  // Return properties
  return properties;
}



////////////////////////////////////////////////////////////////////////
// Program argument parsing
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1; 
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
    }
    else {
      if (!input_mesh1_filename) input_mesh1_filename = *argv;
      else if (!input_mesh2_filename) input_mesh2_filename = *argv;
      else if (!output_property_filename) output_property_filename = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
    }
    argv++; argc--;
  }

  // Check filenames
  if (!input_mesh1_filename || !input_mesh2_filename || !output_property_filename) {
    RNFail("Usage: mshcompare mesh1 mesh2 output [options]\n");
    return 0;
  }

  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Main program
////////////////////////////////////////////////////////////////////////

int 
main(int argc, char **argv)
{
  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);

  // Read the first mesh 
  R3Mesh *mesh1 = ReadMesh(input_mesh1_filename);
  if (!mesh1) exit(-1);

  // Read the second mesh
  R3Mesh *mesh2 = ReadMesh(input_mesh2_filename);
  if (!mesh2) exit(-1);

  // Create the output properties
  R3MeshPropertySet *properties = CreateMeshPropertySet(mesh1, mesh2);
  if (!properties) exit(-1);

  // Write the properties to a file
  if (!WriteProperties(properties, output_property_filename)) exit(-1);

  // Delete the properties
  delete properties;
  
  // Return success
  return 0;
}
