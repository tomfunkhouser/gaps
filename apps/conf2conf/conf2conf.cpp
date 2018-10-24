// Source file for the rgbd loader program



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "RGBD/RGBD.h"



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
// TEXTURING FUNCTIONS
////////////////////////////////////////////////////////////////////////

static RNScalar
TextureScore(RGBDImage *image, RGBDSurface *surface,
  const R2Point& image_position, const R2Point& texture_position,
  const RNRgb& color)
{
  // Don't allow purely black pixels ???
  if (color == RNblack_rgb) return 0.0;

  // Get convenient variables
  R3Point E = image->WorldViewpoint();
  R3Point P = surface->TexelWorldPosition(texture_position);
  R3Vector N = surface->TexelWorldNormal(texture_position);
  R3Vector T = -(image->WorldTowards());
  R3Vector V = E - P; 
  RNLength d = V.Length();
  if (RNIsZero(d)) return 0.0;
  else V /= d;

  // Consider viewing angles
  RNScalar ndotv = N.Dot(V);
  if (RNIsNegativeOrZero(ndotv)) return 0;
  RNScalar tdotv = T.Dot(V);
  if (RNIsNegativeOrZero(tdotv)) return 0;
  RNScalar invdd = (d > 1.0) ? 1.0 / (d*d) : 1.0;
  
  // Compute score
  RNScalar score = 1.0;
  score *= ndotv;
  // score *= tdotv;
  score *= invdd;

  // Return score
  return score * score * score * score;
}



static int
RGBDComputeSurfaceTexture(RGBDSurface *surface)
{
  // Get/check variables
  RGBDConfiguration *configuration = surface->Configuration();
  if (!configuration) return 0;
  int width = surface->NTexels(RN_X);
  if (width == 0) return 0;
  int height = surface->NTexels(RN_Y);
  if (height == 0) return 0;

  // Allocate images for texture RGB channels (initialized with zeroes)
  RNLength world_texel_spacing = surface->WorldTexelSpacing();
  R2Box surface_bbox(0.0, 0.0, world_texel_spacing*width, world_texel_spacing*height);
  R2Grid red_channel(width, height, surface_bbox);
  R2Grid green_channel(width, height, surface_bbox);
  R2Grid blue_channel(width, height, surface_bbox);

  // Allocate storage for accumulating statistics
  if (configuration->NImages() == 0) return 1;
  RNRgb *colors = new RNRgb [ configuration->NImages() ];
  if (!colors) { fprintf(stderr, "Unable to allocate memory\n"); return 0; }
  RNScalar *weights = new RNScalar [ configuration->NImages() ];
  if (!weights) { fprintf(stderr, "Unable to allocate memory\n"); delete [] colors; return 0; }

  // Fill the texture RGB channels
  for (int ix = 0; ix < width; ix++) {
    for (int iy = 0; iy < height; iy++) {
      // Check if (ix,iy) maps to a surface
      if (surface->mesh_face_index) {
        RNScalar face_index_value = surface->mesh_face_index->GridValue(ix, iy);
        if (face_index_value == R2_GRID_UNKNOWN_VALUE) continue;
      }

      // Get position in texture
      R2Point texture_position(ix+0.5, iy+0.5);

      // Initialize statistics
      int count = 0;
      RNScalar weight_sum = 0;
      RNRgb weighted_color_sum(0,0,0);

      // Gather sample from every image
      for (int i = 0; i < configuration->NImages(); i++) {
        // Get image
        RGBDImage *image = configuration->Image(i);

        // Get position in image
        R2Point image_position;
        if (!RGBDTransformTextureToImage(texture_position, image_position, surface, image)) continue;

        // Get rgb from image
        RNScalar r = image->PixelChannelValue(image_position, RGBD_RED_CHANNEL);
        RNScalar g = image->PixelChannelValue(image_position, RGBD_GREEN_CHANNEL);
        RNScalar b = image->PixelChannelValue(image_position, RGBD_BLUE_CHANNEL);

        // Check if valid image pixel
        if (r == R2_GRID_UNKNOWN_VALUE) continue;
        if (g == R2_GRID_UNKNOWN_VALUE) continue;
        if (b == R2_GRID_UNKNOWN_VALUE) continue;

        // Compute weight
        RNRgb color(r, g, b);
        RNScalar weight = TextureScore(image, surface, image_position, texture_position, color);
        if (weight <= 0) continue;
        
        // Update statistics
        weight_sum += weight;
        weighted_color_sum += weight * color;
        weights[count] = weight;
        colors[count] = color;
        count++;
      }

      // Check statistics
      if (count == 0) continue;
      if (weight_sum == 0) continue;

#if 1
      // Compute weighted mean
      RNRgb color = weighted_color_sum / weight_sum;
#else
      // Compute weighted median
      RNRgb color = RNblack_rgb;
      RNScalar best_dd = FLT_MAX;
      for (int i = 0; i < count; i++) {
        RNScalar dd = 0;
        for (int j = 0; j < count; j++) {
          if (i != j) {
            RNScalar dr = colors[i].R() - colors[j].R();
            RNScalar dg = colors[i].G() - colors[j].G();
            RNScalar db = colors[i].B() - colors[j].B();
            dd += weights[j]*weights[j] * (dr*dr + dg*dg + db*db);
          }
        }
        if (dd < best_dd) {
          color = colors[i];
          best_dd = dd;
        }
      }
#endif
      
      // Update the texture channels
      red_channel.SetGridValue(ix, iy, color.R());
      green_channel.SetGridValue(ix, iy, color.G());
      blue_channel.SetGridValue(ix, iy, color.B());
    }
  }

  // Delete temporary memory
  delete [] colors;
  delete [] weights;

  // Create color channels
  R2Image tmp(width, height);
  surface->CreateColorChannels(tmp);

  // Update the texture RGB channels
  surface->SetChannel(RGBD_RED_CHANNEL, red_channel);
  surface->SetChannel(RGBD_GREEN_CHANNEL, green_channel);
  surface->SetChannel(RGBD_BLUE_CHANNEL, blue_channel);

  // Write color channels
  surface->WriteColorChannels();

  // Return success
  return 1;
}



static int
RGBDComputeSurfaceTextures(RGBDConfiguration *configuration)
{
  // Check if should bother
  if (configuration->NSurfaces() == 0) return 1;

  // Read all channels
  configuration->ReadChannels();

  // Compute color textures for all surfaces
  for (int i = 0; i < configuration->NSurfaces(); i++) {
    RGBDSurface *surface = configuration->Surface(i);
    if (!RGBDComputeSurfaceTexture(surface)) return 0;
  }

  // Release all channels
  configuration->ReleaseChannels();

  // Return success
  return 1;
}



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



