// Source file for the rgbd loader program



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

#include "RGBD/RGBD.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static const char *input_configuration_filename = NULL;
static const char *input_mesh_filename = NULL;
static const char *output_vertex_image_overlap_filename = NULL;
static const char *output_image_vertex_overlap_filename = NULL;
static const char *output_image_image_overlap_filename = NULL;
static const char *output_image_image_overlap_matrix = NULL;
static double min_overlap_fraction = RN_EPSILON;
// static int max_reprojection_distance = 0; // in pixels
static double max_depth_error = 0.05; // as fraction of depth
static int check_every_kth_pixel = 1;
static int print_verbose = 0;
static int print_debug = 0;



////////////////////////////////////////////////////////////////////////
// Input functions
////////////////////////////////////////////////////////////////////////

static RGBDConfiguration *
ReadConfigurationFile(const char *filename) 
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Reading configuration from %s ...\n", filename);
    fflush(stdout);
  }

  // Allocate configuration
  RGBDConfiguration *configuration = new RGBDConfiguration();
  if (!configuration) {
    fprintf(stderr, "Unable to allocate configuration for %s\n", filename);
    return NULL;
  }

  // Read file
  if (!configuration->ReadFile(filename)) {
    fprintf(stderr, "Unable to read configuration from %s\n", filename);
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration->NImages());
    fflush(stdout);
  }

  // Return configuration
  return configuration;
}



static R3Mesh *
ReadMeshFile(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate mesh
  R3Mesh *mesh = new R3Mesh();
  if (!mesh) {
    fprintf(stderr, "Unable to allocate mesh for %s\n", filename);
    return NULL;
  }

  // Read mesh from file
  if (!mesh->ReadFile(filename)) {
    fprintf(stderr, "Unable to read mesh from %s\n", filename);
    delete mesh;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read mesh ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Faces = %d\n", mesh->NFaces());
    printf("  # Edges = %d\n", mesh->NEdges());
    printf("  # Vertices = %d\n", mesh->NVertices());
    fflush(stdout);
  }

  // Return mesh
  return mesh;
}



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

static int
CountOverlaps(R3Mesh *mesh, const RNArray<R3MeshVertex *>& a0, const RNArray<R3MeshVertex *>& a1)
{
  // Note: this assumes that the inputs are sorted by VertexID
  
  // Initialize the count
  int count = 0;

  // Count overlaps
  int i0 = 0, i1 = 0;
  while ((i0 < a0.NEntries()) && (i1 < a1.NEntries())) {
    if (mesh->VertexID(a0[i0]) < mesh->VertexID(a1[i1])) i0++;
    else if (mesh->VertexID(a0[i0]) > mesh->VertexID(a1[i1])) i1++;
    else { count++; i0++; i1++; }
  }
  
  // Return the count
  return count;
}


////////////////////////////////////////////////////////////////////////
// Output functions
////////////////////////////////////////////////////////////////////////

static int
ComputeOverlaps(RGBDConfiguration *configuration, R3Mesh *mesh,
  RNArray<R3MeshVertex *> **image_to_vertex_overlaps = NULL,
  RNArray<RGBDImage *> **vertex_to_image_overlaps = NULL) 
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  unsigned long overlap_count = 0;
  if (print_verbose) {
    printf("Computing overlaps ...\n");
    fflush(stdout);
  }

  // Consider every image
  for (int i0 = 0; i0 < configuration->NImages(); i0++) {
    RGBDImage *image = configuration->Image(i0);

    // Read image depth channel
    if (!image->ReadDepthChannel()) continue;
    const R3Box& image_bbox = image->WorldBBox();
    if (image_bbox.IsEmpty()) { image->ReleaseDepthChannel(); continue; }
    if (!R3Intersects(image_bbox, mesh->BBox())) { image->ReleaseDepthChannel(); continue; }
    
    // Consider every mesh vertex
    for (int i1 = 0; i1 < mesh->NVertices(); i1++) {
      R3MeshVertex *vertex = mesh->Vertex(i1);
      const R3Point& vertex_position = mesh->VertexPosition(vertex);
      if (!R3Intersects(image_bbox, vertex_position)) continue;
      
      // Compute mapping from vertex into image
      R2Point image_position;
      if (!RGBDTransformWorldToImage(vertex_position, image_position, image)) continue;

      // Check for depth consistency
      if (max_depth_error > 0) {
        RNScalar vertex_depth = (vertex_position - image->WorldViewpoint()).Dot(image->WorldTowards());
        RNScalar pixel_depth = image->DepthChannel()->GridValue(image_position);
        RNScalar depth_difference = fabs(vertex_depth - pixel_depth);
        RNScalar depth_maximum = (pixel_depth > vertex_depth) ? pixel_depth : vertex_depth;
        if (depth_maximum <= 0) continue;
        RNScalar depth_error = depth_difference / depth_maximum;
        if (depth_error > max_depth_error) continue;
      }

      // Insert overlap
      if (image_to_vertex_overlaps) (*image_to_vertex_overlaps)[i0].Insert(vertex);
      if (vertex_to_image_overlaps) (*vertex_to_image_overlaps)[i1].Insert(image);

      // Update statistics
      overlap_count++;
    }

    // Release image depth channel
    image->ReleaseDepthChannel();
  }
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration->NImages());
    printf("  # Vertices = %d\n", mesh->NVertices());
    printf("  # Overlaps = %lu\n", overlap_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
WriteImageVertexOverlapFile(RGBDConfiguration *configuration, R3Mesh *mesh,
  const RNArray<R3MeshVertex *> *image_to_vertex_overlaps, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Writing image->vertex overlaps to %s ...\n", filename);
    fflush(stdout);
  }
  
  // Open output file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open overlap file %s\n", filename);
    return 0;
  }
  
  // Write overlaps
  for (int i0 = 0; i0 < configuration->NImages(); i0++) {
    fprintf(fp, "%d   ", i0);
    for (int i1 = 0; i1 < image_to_vertex_overlaps[i0].NEntries(); i1++) {
      R3MeshVertex *vertex = image_to_vertex_overlaps[i0].Kth(i1);
      fprintf(fp, "%d ", mesh->VertexID(vertex));
    }
    fprintf(fp, "\n");
  }
  
  // Close info file
  fclose(fp);
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration->NImages());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
WriteVertexImageOverlapFile(RGBDConfiguration *configuration, R3Mesh *mesh,
  const RNArray<RGBDImage *> *vertex_to_image_overlaps, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Writing vertex->image overlaps to %s ...\n", filename);
    fflush(stdout);
  }
  
  // Open output file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open overlap file %s\n", filename);
    return 0;
  }
  
  // Write overlaps
  for (int i0 = 0; i0 < mesh->NVertices(); i0++) {
    fprintf(fp, "%d   ", i0);
    for (int i1 = 0; i1 < vertex_to_image_overlaps[i0].NEntries(); i1++) {
      RGBDImage *image = vertex_to_image_overlaps[i0].Kth(i1);
      fprintf(fp, "%d ", image->ConfigurationIndex());
    }
    fprintf(fp, "\n");
  }
  
  // Close info file
  fclose(fp);
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Vertices = %d\n", mesh->NVertices());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
WriteImageImageOverlapFile(RGBDConfiguration *configuration, R3Mesh *mesh,
  const RNArray<R3MeshVertex *> *image_to_vertex_overlaps, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Writing image->image overlaps to %s ...\n", filename);
    fflush(stdout);
  }
  
  // Open output file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open overlap file %s\n", filename);
    return 0;
  }
  
  // Write overlaps
  for (int i0 = 0; i0 < configuration->NImages(); i0++) {
    for (int i1 = 0; i1 < configuration->NImages(); i1++) {
      if (image_to_vertex_overlaps[i0].IsEmpty()) continue;
      if (image_to_vertex_overlaps[i1].IsEmpty()) continue;
      int overlap_count = CountOverlaps(mesh, image_to_vertex_overlaps[i0], image_to_vertex_overlaps[i1]);
      if (overlap_count == 0) continue;
      double overlap_fraction = (double) overlap_count / (double) image_to_vertex_overlaps[i0].NEntries();
      if (overlap_fraction < min_overlap_fraction) continue;
      fprintf(fp, "%d %d %d %g\n", i0, i1, overlap_count, overlap_fraction);
      count++;
    }
  }

  // Close info file
  fclose(fp);
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Overlaps = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
WriteImageImageOverlapMatrix(RGBDConfiguration *configuration, R3Mesh *mesh,
  const RNArray<R3MeshVertex *> *image_to_vertex_overlaps, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Writing image->image overlaps to %s ...\n", filename);
    fflush(stdout);
  }
  
  // Create matrix
  R2Grid matrix(configuration->NImages(), configuration->NImages());
  for (int i0 = 0; i0 < configuration->NImages(); i0++) {
    for (int i1 = 0; i1 < configuration->NImages(); i1++) {
      if (image_to_vertex_overlaps[i0].IsEmpty()) continue;
      if (image_to_vertex_overlaps[i1].IsEmpty()) continue;
      int overlap_count = CountOverlaps(mesh, image_to_vertex_overlaps[i0], image_to_vertex_overlaps[i1]);
      if (overlap_count == 0) continue;
      double overlap_fraction = (double) overlap_count / (double) image_to_vertex_overlaps[i0].NEntries();
      if (overlap_fraction < min_overlap_fraction) continue;
      matrix.SetGridValue(i0, i1, overlap_fraction);
    }
  }

  // Write matrix
  if (!matrix.WriteFile(filename)) return 0;
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Overlaps = %d\n", matrix.Cardinality());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Check if have an output
  RNBoolean output = FALSE;

  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-debug")) print_debug = 1;
      else if (!strcmp(*argv, "-min_overlap")) { argc--; argv++; min_overlap_fraction = atof(*argv); }
      else if (!strcmp(*argv, "-check_every_kth_pixel")) { argc--; argv++; check_every_kth_pixel = atoi(*argv); }
      else if (!strcmp(*argv, "-output_image_vertex_overlaps")) { argc--; argv++; output_image_vertex_overlap_filename = *argv; output = TRUE; }
      else if (!strcmp(*argv, "-output_vertex_image_overlaps")) { argc--; argv++; output_vertex_image_overlap_filename = *argv; output = TRUE; }
      else if (!strcmp(*argv, "-output_image_image_overlaps")) { argc--; argv++; output_image_image_overlap_filename = *argv; output = TRUE; }
      else if (!strcmp(*argv, "-output_image_image_matrix")) { argc--; argv++; output_image_image_overlap_matrix = *argv; output = TRUE; }
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
    else {
      if (!input_configuration_filename) input_configuration_filename = *argv;
      else if (!input_mesh_filename) input_mesh_filename = *argv;
      else if (!output_image_image_overlap_filename) output_image_image_overlap_filename = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!input_configuration_filename || !input_mesh_filename || !output) {
    fprintf(stderr, "Usage: conf2overlap inputconfigurationfile inputmeshfile output [options]\n");
    return 0;
  }

  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
  // Check number of arguments
  if (!ParseArgs(argc, argv)) exit(1);

  // Read configuration
  RGBDConfiguration *configuration = ReadConfigurationFile(input_configuration_filename);
  if (!configuration) exit(-1);

  // Read mesh
  R3Mesh *mesh = ReadMeshFile(input_mesh_filename);
  if (!mesh) exit(-1);

  // Compute overlaps
  RNArray<R3MeshVertex *> *image_to_vertex_overlaps = new RNArray<R3MeshVertex *> [ configuration->NImages() ];
  RNArray<RGBDImage *> *vertex_to_image_overlaps = new RNArray<RGBDImage *> [ mesh->NVertices() ]; 
  if (!ComputeOverlaps(configuration, mesh, &image_to_vertex_overlaps, &vertex_to_image_overlaps)) return 0;

  // Write image to vertex overlaps
  if (output_image_vertex_overlap_filename) {
    if (!WriteImageVertexOverlapFile(configuration, mesh, image_to_vertex_overlaps, output_image_vertex_overlap_filename)) exit(-1);
  }
  
  // Write vertex to image overlaps
  if (output_vertex_image_overlap_filename) {
    if (!WriteVertexImageOverlapFile(configuration, mesh, vertex_to_image_overlaps, output_vertex_image_overlap_filename)) exit(-1);
  }
  
  // Write image to image overlaps
  if (output_image_image_overlap_filename) {
    if (!WriteImageImageOverlapFile(configuration, mesh, image_to_vertex_overlaps, output_image_image_overlap_filename)) exit(-1);
  }
  
  // Write image to image matrix
  if (output_image_image_overlap_matrix) {
    if (!WriteImageImageOverlapMatrix(configuration, mesh, image_to_vertex_overlaps, output_image_image_overlap_matrix)) exit(-1);
  }
  
  // Return success 
  return 0;
}



