// Source file for the mesh segmentation program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Shapes/R3Shapes.h"
#include "R3Utils/R3Utils.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static char *input_mesh_name = NULL;
static char *output_mesh_name = NULL;
static char *output_json_name = NULL;
static char *output_ascii_name = NULL;
static double max_neighbor_normal_angle = 0;
static double max_neighbor_color_difference = 0;
static int set_face_materials = 0;
static int refine_mesh_boundaries = 1;
static R3Segmentation options;
static int print_verbose = 0;



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

  // Initialize (remember segments in face materials)
  if (set_face_materials) {
    for (int i = 0; i < mesh->NFaces(); i++) {
      R3MeshFace *face = mesh->Face(i);
      int segment = mesh->FaceSegment(face);
      mesh->SetFaceMaterial(face, segment);
      mesh->SetFaceSegment(face, -1);
    }
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

  // Swap face segments and materials
  if (set_face_materials) {
    for (int i = 0; i < mesh->NFaces(); i++) {
      R3MeshFace *face = mesh->Face(i);
      int material = mesh->FaceMaterial(face);
      int segment = mesh->FaceSegment(face);
      mesh->SetFaceMaterial(face, segment);
      mesh->SetFaceSegment(face, material);
    }
  }

  // Write mesh
  if (!mesh->WriteFile(filename)) {
    RNFail("Unable to write mesh to %s\n", filename);
    return 0;
  }

  // Swap back face segments and materials
  if (set_face_materials) {
    for (int i = 0; i < mesh->NFaces(); i++) {
      R3MeshFace *face = mesh->Face(i);
      int material = mesh->FaceMaterial(face);
      int segment = mesh->FaceSegment(face);
      mesh->SetFaceMaterial(face, segment);
      mesh->SetFaceSegment(face, material);
    }
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



static int
WriteJson(R3Mesh *mesh, const char *filename)
{
  // Check filename
  if (!filename) return 1;

  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Open file
  FILE* fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open json file %s\n", filename);
    return 0;
  }

  // Write segments
  fprintf(fp, "{\n");
  fprintf(fp, "  \"sceneId\": \"xxx\",\n");
  fprintf(fp, "  \"segIndices\": [\n");
  int junk_segment_id = 9999999;
  for (int i = 0; i < mesh->NVertices(); i++) {
    R3MeshVertex *vertex = mesh->Vertex(i);

    // Find best segment for vertex
    int best_segment = -1;
    for (int j = 0; j < mesh->VertexValence(vertex); j++) {
      R3MeshEdge *edge = mesh->EdgeOnVertex(vertex, j);
      R3MeshFace *face = mesh->FaceOnEdge(edge, vertex, RN_CW);
      if (!face) continue;
      int segment = mesh->FaceSegment(face);
      if (segment < 0) continue;
      if ((best_segment > 0) && (segment > best_segment)) continue;
      best_segment = segment;
    }
    if (i > 0) fprintf(fp, ",\n");
    if (best_segment >= 0) fprintf(fp, "%d", best_segment); 
    else fprintf(fp, "%d", junk_segment_id);
  }
  fprintf(fp, "\n]\n");
  fprintf(fp, "}\n");

  // Close file
  fclose(fp);

  // Print statistics
  if (print_verbose) {
    printf("Wrote segments to %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
WriteAscii(R3Segmentation *segmentation, const char *filename)
{
  // Check filename
  if (!filename) return 1;

  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Write segmentation
  if (!segmentation->WriteFile(filename)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("Wrote segments to %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Segmentation functions
////////////////////////////////////////////////////////////////////////

static int 
CreatePoints(R3Mesh *mesh, R3Segmentation *segmentation)
{
  // Allocate points
  int npoints = mesh->NFaces();
  segmentation->point_buffer = new R3SegmentationPoint [ npoints ];
  if (!segmentation->point_buffer) {
    RNFail("Unable to allocate points\n");
    return 0;
  }

  // Fill points
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    R3SegmentationPoint *point = &segmentation->point_buffer[i];
    point->position = mesh->FaceCentroid(face);
    point->normal = mesh->FaceNormal(face);
    point->radius1 = 2 * sqrt(mesh->FaceArea(face));
    point->radius2 = point->radius2;
    point->area = mesh->FaceArea(face);
    point->color = RNblack_rgb;
    point->boundary = 0;
    point->data_index = i;
    segmentation->points.Insert(point);

    // Compute face color
    point->color.Reset(0,0,0);
    point->color += mesh->VertexColor(mesh->VertexOnFace(face, 0));
    point->color += mesh->VertexColor(mesh->VertexOnFace(face, 1));
    point->color += mesh->VertexColor(mesh->VertexOnFace(face, 2));
    point->color /= 3;
  }

  // Create kdtree of points
  R3SegmentationPoint tmp; int position_offset = (unsigned char *) &(tmp.position) - (unsigned char *) &tmp;
  segmentation->kdtree = new R3Kdtree<R3SegmentationPoint *>(segmentation->points, position_offset);
  if (!segmentation->kdtree) {
    RNFail("Unable to create kdtree\n");
    return 0;
  }
  
  // Create arrays of neighbor points
  for (int i = 0; i <  mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    R3SegmentationPoint *point = segmentation->points.Kth(i);
    for (int j = 0; j < 3; j++) {
      // Get neighbor face
      R3MeshFace *neighbor_face = mesh->FaceOnFace(face, j);
      if (!neighbor_face) continue;

      // Get neighbor point
      R3SegmentationPoint *neighbor_point = segmentation->points.Kth(mesh->FaceID(neighbor_face));

      // Check normal angle
      if (max_neighbor_normal_angle > 0) {
        RNScalar dot = point->normal.Dot(neighbor_point->normal);
        RNAngle normal_angle = (dot < 1) ? acos(dot) : 0;
        if (normal_angle > max_neighbor_normal_angle) continue;
      }
      
      // Check color difference
      if (max_neighbor_color_difference > 0) {
        RNLength color_difference = 0;
        color_difference += fabs(point->color.R() - neighbor_point->color.R());
        color_difference += fabs(point->color.R() - neighbor_point->color.R());
        color_difference += fabs(point->color.R() - neighbor_point->color.R());
        if (color_difference > max_neighbor_color_difference) continue;
      }

      // Insert neighbor point
      point->neighbors.Insert(neighbor_point);
    }
  }

  // Return success
  return 1;
}
               


static int 
UpdateMesh(R3Mesh *mesh, R3Segmentation *segmentation)
{
  // Clear mesh info
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    mesh->SetFaceSegment(face, -1);
  }    

  // Update mesh info
  for (int i = 0; i < segmentation->clusters.NEntries(); i++) {
    R3SegmentationCluster *cluster = segmentation->clusters.Kth(i);
    for (int j = 0; j < cluster->points.NEntries(); j++) {
      R3SegmentationPoint *point = cluster->points.Kth(j);
      R3MeshFace *face = mesh->Face(point->data_index);
      mesh->SetFaceSegment(face, i);
    }
  }

  // Return success
  return 1;
}



static R3Segmentation *
CreateSegmentation(R3Mesh *mesh)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate segmentation
  R3Segmentation *segmentation = new R3Segmentation(options);
  if (!segmentation) {
    RNFail("Unable to allocation segmentation\n");
    return NULL;
  }

  // Create points
  if (!CreatePoints(mesh, segmentation)) return NULL;

  // Create clusters
  if (!segmentation->CreateClusters(R3_SEGMENTATION_PLANE_PRIMITIVE_TYPE)) return NULL;

  // Print statistics
  if (print_verbose) {
    printf("Created segmentation ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Segments = %d\n", segmentation->clusters.NEntries());
    fflush(stdout);
  }

  // Return segmentation
  return segmentation;
}



static int
RefineBoundaries(R3Mesh *mesh)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Refine boundaries
  RNBoolean done = FALSE;
  while (!done) {
    done = TRUE;
    for (int i = 0; i < mesh->NFaces(); i++) {
      R3MeshFace *face = mesh->Face(i);
      R3MeshEdge *edge0 = mesh->EdgeOnFace(face, 0);
      R3MeshEdge *edge1 = mesh->EdgeOnFace(face, 1);
      R3MeshEdge *edge2 = mesh->EdgeOnFace(face, 2);
      R3MeshFace *face0 = mesh->FaceAcrossEdge(edge0, face);
      R3MeshFace *face1 = mesh->FaceAcrossEdge(edge1, face);
      R3MeshFace *face2 = mesh->FaceAcrossEdge(edge2, face);
      if (!face0 || !face1 || !face2) continue;
      int segment = mesh->FaceSegment(face);
      int segment0 = mesh->FaceSegment(face0);
      int segment1 = mesh->FaceSegment(face1);
      int segment2 = mesh->FaceSegment(face2);
      R3Vector normal = mesh->FaceNormal(face);
      R3Vector normal0 = mesh->FaceNormal(face0);
      R3Vector normal1 = mesh->FaceNormal(face1);
      R3Vector normal2 = mesh->FaceNormal(face2);
      RNAngle angle0 = R3InteriorAngle(normal, normal0);
      RNAngle angle1 = R3InteriorAngle(normal, normal1);
      RNAngle angle2 = R3InteriorAngle(normal, normal2);
      if (angle0 > options.max_pair_normal_angle) segment0 = -1;
      if (angle1 > options.max_pair_normal_angle) segment1 = -1;
      if (angle2 > options.max_pair_normal_angle) segment2 = -1;
      if ((segment != segment0) && (segment0 >= 0) && (segment1 >= 0) && (segment0 == segment1)) {
        mesh->SetFaceSegment(face, segment0);
        done = FALSE;
      }
      else if ((segment != segment0) && (segment0 >= 0) && (segment2 >= 0) && (segment0 == segment2)) {
        mesh->SetFaceSegment(face, segment0);
        done = FALSE;
      }
      else if ((segment != segment1) && (segment1 >= 0) && (segment2 >= 0) && (segment1 == segment2)) {
        mesh->SetFaceSegment(face, segment1);
        done = FALSE;
      }
    }
  }
  
  // Print statistics
  if (print_verbose) {
    printf("Refined boundaries ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    fflush(stdout);
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
      else if (!strcmp(*argv, "-debug")) options.print_progress = 1;
      else if (!strcmp(*argv, "-dont_refine_boundaries")) refine_mesh_boundaries = 0;
      else if (!strcmp(*argv, "-refine_boundaries")) refine_mesh_boundaries = 1;
      else if (!strcmp(*argv, "-set_face_materials")) set_face_materials = 1;
      else if (!strcmp(*argv, "-initialize_with_region_growing")) {
        options.initialize_hierarchically = 0;
      }
      else if (!strcmp(*argv, "-allow_outlier_points")) {
        options.allow_outlier_points = 1;
      }
      else if (!strcmp(*argv, "-min_segments")) {
        argc--; argv++; options.min_clusters = atoi(*argv);
      }
      else if (!strcmp(*argv, "-max_segments")) {
        argc--; argv++; options.max_clusters = atoi(*argv);
      }
      else if (!strcmp(*argv, "-min_pair_affinity")) {
        argc--; argv++; options.min_pair_affinity = atof(*argv);
      }
      else if (!strcmp(*argv, "-max_refinement_iterations")) {
        argc--; argv++; options.max_refinement_iterations = atoi(*argv);
      }
      else if (!strcmp(*argv, "-max_reassignment_iterations")) {
        argc--; argv++; options.max_reassignment_iterations = atoi(*argv);
      }
      else if (!strcmp(*argv, "-max_distance")) {
        argc--; argv++; RNScalar max_distance = atof(*argv);
        options.max_cluster_primitive_distance = max_distance;
        options.max_pair_primitive_distance = max_distance;
      }
      else if (!strcmp(*argv, "-max_angle")) {
        argc--; argv++; RNScalar max_angle = atof(*argv);
        options.max_cluster_normal_angle = max_angle;
        options.max_pair_normal_angle = max_angle;
      }
      else if (!strcmp(*argv, "-max_color_difference")) {
        argc--; argv++; RNScalar max_color_difference = atof(*argv);
        options.max_cluster_color_difference = max_color_difference;
        options.max_pair_color_difference = max_color_difference;
      }
      else if (!strcmp(*argv, "-max_neighbor_normal_angle")) {
        argc--; argv++; max_neighbor_normal_angle = atof(*argv);
      }
      else if (!strcmp(*argv, "-max_neighbor_color_difference")) {
        argc--; argv++; max_neighbor_color_difference = atof(*argv);
      }
      else if (!strcmp(*argv, "-min_faces_per_segment")) {
        argc--; argv++; options.min_cluster_points = atoi(*argv);
      }
      else if (!strcmp(*argv, "-min_area_per_segment")) {
        argc--; argv++; options.min_cluster_area = atof(*argv);
      }
      else if (!strcmp(*argv, "-output_json")) {
        argc--; argv++; output_json_name = *argv;
      }
      else if (!strcmp(*argv, "-output_ascii")) {
        argc--; argv++; output_ascii_name = *argv;
      }
      else {
        RNFail("Invalid program argument: %s", *argv);
        exit(1);
      }
      argv++; argc--;
    }
    else {
      if (!input_mesh_name) input_mesh_name = *argv;
      else if (!output_mesh_name) output_mesh_name = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!input_mesh_name || !output_mesh_name) {
    RNFail("Usage: mshseg inputmesh outputmesh\n");
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

  // Create segmentation
  R3Segmentation *segmentation = CreateSegmentation(mesh);
  if (!segmentation) exit(-1);

  // Update mesh
  if (!UpdateMesh(mesh, segmentation)) return 0;
  
  // Refine boundaries
  if (refine_mesh_boundaries) {
    if (!RefineBoundaries(mesh)) exit(-1);
  }

  // Write mesh
  if (!WriteMesh(mesh, output_mesh_name)) exit(-1);

  // Write segments to json file
  if (!WriteJson(mesh, output_json_name)) exit(-1);

  // Write segments to ascii file
  if (!WriteAscii(segmentation, output_ascii_name)) exit(-1);

  // Delete mesh
  delete mesh;
  
  // Return success 
  return 0;
}



