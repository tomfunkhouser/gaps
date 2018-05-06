// Source file for the rgbd loader program



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

#include "RGBD/RGBD.h"
#include "texture.cpp"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static const char *input_configuration_filename = NULL;
static const char *input_mesh_filename = NULL;
static const char *input_depth_directory = NULL;
static const char *input_color_directory = NULL;
static const char *input_texture_directory = NULL;
static const char *output_configuration_filename = NULL;
static const char *output_depth_directory = NULL;
static const char *output_color_directory = NULL;
static const char *output_texture_directory = NULL;
static R3Box viewpoint_bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
static int load_images_starting_at_index = 0;
static int load_images_ending_at_index = INT_MAX;
static int load_every_kth_image = 1;
static int write_every_kth_image = 1;
static double texel_spacing = 0;
static int print_verbose = 0;



////////////////////////////////////////////////////////////////////////
// Image selection functions
////////////////////////////////////////////////////////////////////////

static int
DeleteUnwantedImages(RGBDConfiguration *configuration)
{
  // Make list of unwanted images
  RNArray<RGBDImage *> unwanted_images;
  for (int i = 0; i < configuration->NImages(); i++) {
    RGBDImage *image = configuration->Image(i);

    // Check if should load range of image indices
    if ((i < load_images_starting_at_index) || (i > load_images_ending_at_index)) {
      unwanted_images.Insert(image);
      continue;
    }

    // Check if should load every kth image  
    if (load_every_kth_image > 1) {
      if ((i % load_every_kth_image) != 0) {
        unwanted_images.Insert(image);
        continue;
      }
    }

    // Check if should load images within bbox
    if (!viewpoint_bbox.IsEmpty()) {
      if (!R3Contains(viewpoint_bbox, image->WorldViewpoint())) {
        unwanted_images.Insert(image);
        continue;
      }
    }
  }

  // Delete unwanted images
  for (int i = 0; i < unwanted_images.NEntries(); i++) {
    RGBDImage *image = unwanted_images.Kth(i);
    delete image;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Read/Write functions
////////////////////////////////////////////////////////////////////////

static RGBDConfiguration *
ReadConfiguration(const char *filename) 
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

  // Set input directories if specified on command line
  if (input_depth_directory) configuration->SetDepthDirectory(input_depth_directory);
  if (input_color_directory) configuration->SetColorDirectory(input_color_directory);
  if (input_texture_directory) configuration->SetTextureDirectory(input_texture_directory);

  // Delete unwanted images
  if (!DeleteUnwantedImages(configuration)) return NULL;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration->NImages());
    printf("  # Surfaces = %d\n", configuration->NSurfaces());
    fflush(stdout);
  }

  // Return configuration
  return configuration;
}



static int
WriteConfiguration(RGBDConfiguration *configuration, const char *filename) 
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Writing configuration to %s ...\n", filename);
    fflush(stdout);
  }

  // Set output directories if specified on command line
  if (output_depth_directory) configuration->SetDepthDirectory(output_depth_directory);
  if (output_color_directory) configuration->SetColorDirectory(output_color_directory);
  if (output_texture_directory) configuration->SetTextureDirectory(output_texture_directory);

  // Write file
  if (!configuration->WriteFile(filename, write_every_kth_image)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration->NImages());
    printf("  # Surfaces = %d\n", configuration->NSurfaces());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static R3Mesh *
ReadMesh(RGBDConfiguration *configuration, const char *filename)
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

  // Read mesh
  if (!mesh->ReadFile(filename)) {
    delete mesh;
    return NULL;
  }

  // Create surface      
  RGBDSurface *surface = new RGBDSurface("-", mesh, texel_spacing);
  if (!surface) {
    fprintf(stderr, "Unable to allocate surface for %s\n", filename);
    delete mesh;
    return NULL;
  }

  // Remember mesh filename
  surface->SetMeshFilename(input_mesh_filename);

  // Insert surface into configuration
  configuration->InsertSurface(surface);

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



////////////////////////////////////////////////////////////////////////
// PROCESSING FUNCTIONS
////////////////////////////////////////////////////////////////////////

#if 0
static void 
AlignWithAxes(RGBDConfiguration *configuration)
{
  // Check images and surfaces
  if (configuration->NImages() == 0) return;
  
  // Compute posz = average image up vector
  R3Vector posz = R3zero_vector;
  for (int i = 0; i < configuration->NImages(); i++) {
    RGBDImage *image = configuration->Image(i);
    posz += image->WorldUp();
  }

  // Compute origin and posy by averaging positions and normals of pixels
  int count = 0;
  R3Point origin = R3zero_point;
  R3Vector posy = R3zero_vector;
  for (int i = 0; i < configuration->NImages(); i++) {
    RGBDImage *image = configuration->Image(i);
    int ix = image->NPixels(RN_X)/2;
    int iy = image->NPixels(RN_Y)/2;
    R3Point world_position;
    if (RGBDTransformImageToWorld(R2Point(ix, iy), world_position, image)) {
      posy += -(image->PixelWorldNormal(ix, iy));
      origin += world_position;
      count++;
    }
  }

  // Compute averages
  if (count > 0) origin /= count;
  else return;

  // Compute orthogonal triad of axes
  posy.Normalize();
  posz.Normalize();
  R3Vector posx = posy % posz; posx.Normalize();
  posz = posx % posy; posz.Normalize();
  R3Triad axes(posx, posy, posz);
  
  // Compute transformation
  R3CoordSystem cs(origin, axes);
  R3Affine transformation(cs.InverseMatrix(), 0);

  // Apply transformation
  configuration->Transform(transformation);
}
#endif





static int
ComputeSurfaceTextures(RGBDConfiguration *configuration)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Computing surface textures ...\n");
    fflush(stdout);
  }

  // Set texture directory 
  if (output_texture_directory) configuration->SetTextureDirectory(output_texture_directory);
  if (!configuration->TextureDirectory()) configuration->SetTextureDirectory("texture");

  // Create texture directory
  if (configuration->TextureDirectory()) {
    char cmd[4096];
    sprintf(cmd, "mkdir -p %s", configuration->TextureDirectory());
    system(cmd);
  }

  // Set texel spacing if specified on command line                       
  if (texel_spacing > 0) {
    for (int i = 0; i < configuration->NSurfaces(); i++) {
      RGBDSurface *surface = configuration->Surface(i);
      surface->SetWorldTexelSpacing(texel_spacing);
    }
  }

  // Compute surface textures
  if (!RGBDComputeSurfaceTextures(configuration)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Texture directory = %s\n", configuration->TextureDirectory());
    printf("  # Surfaces = %d\n", configuration->NSurfaces());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
NegateYZ(RGBDConfiguration *configuration)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Negating Y and Z ...\n");
    fflush(stdout);
  }

  // Create transformation
  R3Affine transformation = R3identity_affine;
  transformation.YScale(-1);
  transformation.ZScale(-1);
  
  // Negate Y and Z in camera coordinate system
  for (int i = 0; i < configuration->NImages(); i++) {
    RGBDImage *image = configuration->Image(i);
    image->Transform(transformation);
  }

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
ResetTransformations(RGBDConfiguration *configuration)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Reseting transformations to identity ...\n");
    fflush(stdout);
  }

  // Reset transformations
  for (int i = 0; i < configuration->NImages(); i++) {
    RGBDImage *image = configuration->Image(i);
    image->SetCameraToWorld(R3identity_affine);
  }

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
Transform(RGBDConfiguration *configuration, const R4Matrix& matrix)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Transforming ...\n");
    fflush(stdout);
  }

  // Transform all images
  R3Affine transformation(matrix, 0);
  for (int i = 0; i < configuration->NImages(); i++) {
    RGBDImage *image = configuration->Image(i);
    image->Transform(transformation);
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration->NImages());
    printf("  Matrix =\n    %g %g %g %g\n    %g %g %g %g\n    %g %g %g %g\n    %g %g %g %g\n",
      matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
      matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
      matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
      matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
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
  // Parse arguments (this has to mimic the code in main)
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-input_mesh")) { argc--; argv++; input_mesh_filename = *argv; }
      else if (!strcmp(*argv, "-input_depth_directory")) { argc--; argv++; input_depth_directory = *argv; }
      else if (!strcmp(*argv, "-input_color_directory")) { argc--; argv++; input_color_directory = *argv; }
      else if (!strcmp(*argv, "-input_texture_directory")) { argc--; argv++; input_texture_directory = *argv; }
      else if (!strcmp(*argv, "-output_depth_directory")) { argc--; argv++; output_depth_directory = *argv; }
      else if (!strcmp(*argv, "-output_color_directory")) { argc--; argv++; output_color_directory = *argv; }
      else if (!strcmp(*argv, "-output_texture_directory")) { argc--; argv++; output_texture_directory = *argv; }
      else if (!strcmp(*argv, "-load_images_starting_at_index")) { argc--; argv++; load_images_starting_at_index = atoi(*argv); }
      else if (!strcmp(*argv, "-load_images_ending_at_index")) { argc--; argv++; load_images_ending_at_index = atoi(*argv); }
      else if (!strcmp(*argv, "-load_every_kth_image")) { argc--; argv++; load_every_kth_image = atoi(*argv); }
      else if (!strcmp(*argv, "-write_every_kth_image")) { argc--; argv++; write_every_kth_image = atoi(*argv); }
      else if (!strcmp(*argv, "-texel_spacing")) { argc--; argv++; texel_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-compute_surface_textures")) {}
      else if (!strcmp(*argv, "-negate_yz")) {}
      else if (!strcmp(*argv, "-transform")) { argc-=16; argv+=16; }
      else if (!strcmp(*argv, "-viewpoint_bbox")) {
        argc--; argv++; viewpoint_bbox[0][0] = atof(*argv);
        argc--; argv++; viewpoint_bbox[0][1] = atof(*argv);
        argc--; argv++; viewpoint_bbox[0][2] = atof(*argv);
        argc--; argv++; viewpoint_bbox[1][0] = atof(*argv);
        argc--; argv++; viewpoint_bbox[1][1] = atof(*argv);
        argc--; argv++; viewpoint_bbox[1][2] = atof(*argv);
      }    
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
    else {
      if (!input_configuration_filename) input_configuration_filename = *argv;
      else if (!output_configuration_filename) output_configuration_filename = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!input_configuration_filename || !output_configuration_filename) {
    fprintf(stderr, "Usage: texture inputconfigurationfile outputconfigurationfile [options]\n");
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
  RGBDConfiguration *configuration = ReadConfiguration(input_configuration_filename);
  if (!configuration) exit(-1);

  // Read mesh
  if (input_mesh_filename) {
    if (!ReadMesh(configuration, input_mesh_filename)) exit(-1);
  }
  
  // Execute commands (this has to mimic the code in ParseArgs)
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) {}
      else if (!strcmp(*argv, "-color_textures")) {}
      else if (!strcmp(*argv, "-semantic_textures")) {}
      else if (!strcmp(*argv, "-input_mesh")) { argc--; argv++; }
      else if (!strcmp(*argv, "-input_depth_directory")) { argc--; argv++; }
      else if (!strcmp(*argv, "-input_color_directory")) { argc--; argv++; }
      else if (!strcmp(*argv, "-input_texture_directory")) { argc--; argv++; }
      else if (!strcmp(*argv, "-output_depth_directory")) { argc--; argv++; }
      else if (!strcmp(*argv, "-output_color_directory")) { argc--; argv++; }
      else if (!strcmp(*argv, "-output_texture_directory")) { argc--; argv++; }
      else if (!strcmp(*argv, "-load_images_starting_at_index")) { argc--; argv++; }
      else if (!strcmp(*argv, "-load_images_ending_at_index")) { argc--; argv++; }
      else if (!strcmp(*argv, "-load_every_kth_image")) { argc--; argv++; }
      else if (!strcmp(*argv, "-write_every_kth_image")) { argc--; argv++; }
      else if (!strcmp(*argv, "-texel_spacing")) { argc--; argv++; }
      else if (!strcmp(*argv, "-viewpoint_bbox")) { argc -= 6; argv += 6; }
      else if (!strcmp(*argv, "-compute_surface_textures")) {
        if (!ComputeSurfaceTextures(configuration)) exit(-1);
      }
      else if (!strcmp(*argv, "-negate_yz")) {
        if (!NegateYZ(configuration)) exit(-1);
      }
      else if (!strcmp(*argv, "-identity_transformations")) {
        if (!ResetTransformations(configuration)) exit(-1);
      }
      else if (!strcmp(*argv, "-transform")) {
        R4Matrix matrix;
        argv++; argc--; matrix[0][0] = atof(*argv);
        argv++; argc--; matrix[0][1] = atof(*argv);
        argv++; argc--; matrix[0][2] = atof(*argv);
        argv++; argc--; matrix[0][3] = atof(*argv);
        argv++; argc--; matrix[1][0] = atof(*argv);
        argv++; argc--; matrix[1][1] = atof(*argv);
        argv++; argc--; matrix[1][2] = atof(*argv);
        argv++; argc--; matrix[1][3] = atof(*argv);
        argv++; argc--; matrix[2][0] = atof(*argv);
        argv++; argc--; matrix[2][1] = atof(*argv);
        argv++; argc--; matrix[2][2] = atof(*argv);
        argv++; argc--; matrix[2][3] = atof(*argv);
        argv++; argc--; matrix[3][0] = atof(*argv);
        argv++; argc--; matrix[3][1] = atof(*argv);
        argv++; argc--; matrix[3][2] = atof(*argv);
        argv++; argc--; matrix[3][3] = atof(*argv);
        if (!Transform(configuration, matrix)) exit(-1);
      }
      else {
        fprintf(stderr, "Invalid program argument: %s", *argv);
        exit(1);
      }
      argv++; argc--;
    }
    else {
      static int nfilenames = 0;
      if (nfilenames < 2) nfilenames++;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }
  
  // Write configuration
  if (!WriteConfiguration(configuration, output_configuration_filename)) exit(-1);
  
  // Return success 
  return 0;
}



