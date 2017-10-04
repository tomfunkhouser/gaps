// Source file for the rgbd viewer program



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

#include "R3Graphics/R3Graphics.h"
#include "RGBD/RGBD.h"
#include "RGBDSegmentation.cpp"
#ifdef USE_MESA
#  include "GL/osmesa.h"
#else
#  include "fglut/fglut.h" 
#  define USE_GLUT
#endif



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

// Filename program variables

static const char *input_configuration_filename = NULL;
static const char *input_camera_filename = NULL;
static const char *input_mesh_filename = NULL;
static const char *output_image_directory = NULL;


// Image capture program variables

static int create_position_images = 0;
static int create_boundary_images = 0;
static int create_normal_images = 0;
static int create_segmentation_images = 0;
static int capture_color_images = 0;
static int capture_depth_images = 0;
static int capture_mesh_depth_images = 0;
static int capture_mesh_position_images = 0;
static int capture_mesh_wposition_images = 0;
static int capture_mesh_normal_images = 0;
static int capture_mesh_wnormal_images = 0;
static int capture_mesh_ndotv_images = 0;
static int capture_mesh_face_images = 0;
static int capture_mesh_material_images = 0;
static int capture_mesh_segment_images = 0;
static int capture_mesh_category_images = 0;


// Image selection options

static int load_every_kth_image = 1;
static int skip_removed_faces = 0;


// Other parameter program variables

static int width = 0;
static int height = 0;
static RNAngle xfov = 0;
static RNRgb background(0,0,0);
static int glut = 1;
static int mesa = 0;


// Printing program variables

static int print_verbose = 0;
static int print_debug = 0;



////////////////////////////////////////////////////////////////////////
// Internal variables
////////////////////////////////////////////////////////////////////////

// State variables

static RGBDConfiguration configuration;
static RNArray<R3Camera *> cameras;
static R3Mesh mesh;
static int current_image_index = -1;



////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////

// Rendering scheme constants

enum {
  BLACK_RENDERING,
  FACE_INDEX_RENDERING,
  FACE_MATERIAL_RENDERING,
  FACE_SEGMENT_RENDERING,
  FACE_CATEGORY_RENDERING,
  CAMERA_PX_RENDERING,
  CAMERA_PY_RENDERING,
  CAMERA_PZ_RENDERING,
  WORLD_PX_RENDERING,
  WORLD_PY_RENDERING,
  WORLD_PZ_RENDERING,
  CAMERA_NX_RENDERING,
  CAMERA_NY_RENDERING,
  CAMERA_NZ_RENDERING,
  CAMERA_ND_RENDERING,
  WORLD_NX_RENDERING,
  WORLD_NY_RENDERING,
  WORLD_NZ_RENDERING,
  WORLD_ND_RENDERING,
  NDOTV_RENDERING,
  NUM_RENDERING_SCHEMES
};



////////////////////////////////////////////////////////////////////////
// Input/output functions
////////////////////////////////////////////////////////////////////////

static int
ReadConfiguration(const char *filename) 
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Reading configuration from %s ...\n", filename);
    fflush(stdout);
  }

  // Read file
  if (!configuration.ReadFile(filename, load_every_kth_image)) {
    fprintf(stderr, "Unable to read configuration from %s\n", filename);
    return 0;
  }

#if 0
  // Read all channels ... for now
  if (!configuration.ReadChannels()) {
    fprintf(stderr, "Unable to read channels for %s\n", filename);
    return 0;
  }
#endif

  // Set width and height
  if ((width == 0) || (height == 0)) {
    if (configuration.NImages() > 0) {
      RGBDImage *image0 = configuration.Image(0);
      if ((image0->NPixels(RN_X) == 0) || (image0->NPixels(RN_Y) == 0)) {
        image0->ReadColorChannels();
        image0->ReleaseColorChannels();
      }
      width = image0->NPixels(RN_X);
      height = image0->NPixels(RN_Y);
    }
  }

  // Double-check to make sure width and height are not zero
  if (width == 0) width = 1280;
  if (height == 0) height = 1024;;
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration.NImages());
    printf("  # Surfaces = %d\n", configuration.NSurfaces());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
ReadCameras(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int camera_count = 0;

  // Get useful variables
  RNScalar neardist = 0.01;
  RNScalar fardist = 100;
  RNScalar aspect = (RNScalar) height / (RNScalar) width;

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open cameras file %s\n", filename);
    return 0;
  }

  // Read file
  RNScalar vx, vy, vz, tx, ty, tz, ux, uy, uz, xf, yf, value;
  while (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &vx, &vy, &vz, &tx, &ty, &tz, &ux, &uy, &uz, &xf, &yf, &value) == (unsigned int) 12) {
    R3Point viewpoint(vx, vy, vz);
    R3Vector towards(tx, ty, tz);
    R3Vector up(ux, uy, uz);
    R3Vector right = towards % up;
    towards.Normalize();
    up = right % towards;
    up.Normalize();
    if (xfov > 0) xf = xfov;
    yf = atan(aspect * tan(xf));
    R3Camera *camera = new R3Camera(viewpoint, towards, up, xf, yf, neardist, fardist);
    camera->SetValue(value);
    cameras.Insert(camera);
    camera_count++;
  }

  // Close file
  fclose(fp);

  // Print statistics
  if (print_verbose) {
    printf("Read cameras from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Cameras = %d\n", camera_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
ReadMesh(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Read mesh from file
  if (!mesh.ReadFile(filename)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("Read mesh ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Faces = %d\n", mesh.NFaces());
    printf("  # Edges = %d\n", mesh.NEdges());
    printf("  # Vertices = %d\n", mesh.NVertices());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Image write functions
////////////////////////////////////////////////////////////////////////

static int
WriteImages(const char *output_directory)
{
  // Check parameters
  if (!create_position_images && !create_boundary_images && !create_normal_images && !create_segmentation_images) return 1;

  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Writing images to %s ...\n", output_directory);
    fflush(stdout);
  }

  // Create output directory
  char cmd[1024];
  sprintf(cmd, "mkdir -p %s", output_image_directory);
  system(cmd);

  // Write images
  for (int i = 0; i < configuration.NImages(); i++) {
    RGBDImage *image = configuration.Image(i);

    // Get image name
    char image_name_buffer[4096];
    const char *filename = image->DepthFilename();
    if (!filename) filename = image->ColorFilename();
    if (!filename) filename = "default";
    strncpy(image_name_buffer, filename, 4096);
    char *image_name = (strrchr(image_name_buffer, '/')) ? strrchr(image_name_buffer, '/')+1 : image_name_buffer;
    char *endp = strrchr(image_name, '.');
    if (endp) *endp = '\0';

    // Check if already done
    RNBoolean done = TRUE;
    if (done && create_boundary_images) {
      char output_image_filename[4096];
      sprintf(output_image_filename, "%s/%s_boundary.png", output_directory, image_name);
      if (!RNFileExists(output_image_filename)) done = FALSE;
    }
    if (done && create_position_images) {
      char output_image_filename[4096];
      sprintf(output_image_filename, "%s/%s_px.png", output_directory, image_name);
      if (!RNFileExists(output_image_filename)) done = FALSE;
    }
    if (done && create_normal_images) {
      char output_image_filename[4096];
      sprintf(output_image_filename, "%s/%s_nx.png", output_directory, image_name);
      if (!RNFileExists(output_image_filename)) done = FALSE;
    }
    if (done && create_segmentation_images) {
      char output_image_filename[4096];
      sprintf(output_image_filename, "%s/%s_segmentation.png", output_directory, image_name);
      if (!RNFileExists(output_image_filename)) done = FALSE;
    }
    if (done) continue;

    // Print timing message
    RNTime step_time;
    if (print_debug) {
      printf("    A %s\n", image_name);
      fflush(stdout);
      step_time.Read();
    }

    // Create depth image
    if (!image->ReadDepthChannel()) return 0;
    R2Grid depth_image = *(image->DepthChannel());
    R3Matrix intrinsics_matrix = image->Intrinsics();
    height = (int) ((double) image->NPixels(RN_Y) * width / (double) image->NPixels(RN_X) + 0.5);
    RGBDResampleDepthImage(depth_image, intrinsics_matrix, width, height);
    if (!image->ReleaseDepthChannel()) return 0;

    // Print timing message
    if (print_debug) {
      printf("    B %g %d %d %g\n", step_time.Elapsed(), depth_image.XResolution(), depth_image.YResolution(), depth_image.Mean());
      fflush(stdout);
      step_time.Read();
    }

    // Create boundary image
    R2Grid boundary_image;
    if (create_boundary_images || create_normal_images || create_segmentation_images) {
      if (!RGBDCreateBoundaryChannel(depth_image, boundary_image)) return 0;
      if (create_boundary_images) {
        // Write boundary images
        char output_image_filename[4096];
        sprintf(output_image_filename, "%s/%s_boundary.png", output_directory, image_name);
        boundary_image.WriteFile(output_image_filename);
      }
    }

    // Print timing message
    if (print_debug) {
      printf("    C %g\n", step_time.Elapsed());
      fflush(stdout);
      step_time.Read();
    }

    // Create position images (in camera coordinates)
    R2Grid px_image, py_image, pz_image;
    if (create_position_images || create_normal_images || create_segmentation_images) {
      if (!RGBDCreatePositionChannels(depth_image, px_image, py_image, pz_image,
          intrinsics_matrix, R4identity_matrix)) return 0;
      if (create_position_images) {
        // Write position images
        char output_image_filename[4096];
        sprintf(output_image_filename, "%s/%s_px.pfm", output_directory, image_name);
        px_image.WriteFile(output_image_filename);
        sprintf(output_image_filename, "%s/%s_py.pfm", output_directory, image_name);
        py_image.WriteFile(output_image_filename);
        sprintf(output_image_filename, "%s/%s_pz.pfm", output_directory, image_name);
        pz_image.WriteFile(output_image_filename);
      }
    }

    // Print timing message
    if (print_debug) {
      printf("    D %g\n", step_time.Elapsed());
      fflush(stdout);
      step_time.Read();
    }

    // Create normal images (in camera coordinates)
    R2Grid nx_image, ny_image, nz_image, radius_image;
    if (create_normal_images || create_segmentation_images) {
      RGBDCreateNormalChannels(depth_image,
        px_image, py_image, pz_image, boundary_image,
        nx_image, ny_image, nz_image, radius_image,
        R3zero_point, R3negz_vector, R3posy_vector);
      if (create_normal_images) {
        // Write normal images
        R2Grid tmp;
        char output_image_filename[4096];
        sprintf(output_image_filename, "%s/%s_nx.png", output_directory, image_name);
        tmp = nx_image;  tmp.Add(1); tmp.Multiply(32768);  tmp.Threshold(0, 0, R2_GRID_KEEP_VALUE); tmp.Threshold(65535, R2_GRID_KEEP_VALUE, 65535);
        tmp.WriteFile(output_image_filename);
        sprintf(output_image_filename, "%s/%s_ny.png", output_directory, image_name);
        tmp = ny_image;  tmp.Add(1); tmp.Multiply(32768);  tmp.Threshold(0, 0, R2_GRID_KEEP_VALUE); tmp.Threshold(65535, R2_GRID_KEEP_VALUE, 65535);
        tmp.WriteFile(output_image_filename);
        sprintf(output_image_filename, "%s/%s_nz.png", output_directory, image_name);
        tmp = nz_image;  tmp.Add(1); tmp.Multiply(32768);  tmp.Threshold(0, 0, R2_GRID_KEEP_VALUE); tmp.Threshold(65535, R2_GRID_KEEP_VALUE, 65535);
        tmp.WriteFile(output_image_filename);
        sprintf(output_image_filename, "%s/%s_radius.png", output_directory, image_name);
        tmp = radius_image;  tmp.Multiply(4000); 
        tmp.WriteFile(output_image_filename);
      }
    }

    // Print timing message
    if (print_debug) {
      printf("    E %g\n", step_time.Elapsed());
      fflush(stdout);
      step_time.Read();
    }

    // Create segmentation image
    if (create_segmentation_images) {
      // Create color image
      if (!image->ReadColorChannels()) return 0;
      R2Image color_image(width, height, 3);
      for (int iy = 0; iy < height; iy++) {
        for (int ix = 0; ix < width; ix++) {
          color_image.SetPixelRGB(ix, iy, image->PixelColor(ix, iy));
        }
      }
      if (!image->ReleaseColorChannels()) return 0;
      R3Matrix color_intrinsics_matrix = image->Intrinsics();
      RGBDResampleColorImage(color_image, color_intrinsics_matrix, width, height);

      // Create segmentation image
      R2Grid segmentation_image;
      char output_ascii_filename[4096];
      sprintf(output_ascii_filename, "%s/%s_segmentation.txt", output_directory, image_name);
      RGBDCreateSegmentationChannel(depth_image,
        px_image, py_image, pz_image, boundary_image,
        nx_image, ny_image, nz_image, radius_image,
        color_image, segmentation_image,
        R3zero_point, R3negz_vector, R3posy_vector,
        output_ascii_filename);

      // Write segmentation image
      if (create_segmentation_images) {
        char output_image_filename[4096];
        sprintf(output_image_filename, "%s/%s_segmentation.png", output_directory, image_name);
        segmentation_image.WriteFile(output_image_filename);
      }
    }

    // Print timing message
    if (print_debug) {
      printf("    F %g\n", step_time.Elapsed());
      fflush(stdout);
      step_time.Read();
    }
}

  // Print statistics
  if (print_verbose) {
    printf("Wrote images to %s\n", output_directory);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration.NImages());
    fflush(stdout);
  }


  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Image capture functions
////////////////////////////////////////////////////////////////////////

static int
CaptureColor(R2Image& image)
{
  // Capture image 
  image.Capture();

  // Return success
  return 1;
}



static void
LoadInteger(int value)
{
  // Set color to represent an integer (24 bits)
  unsigned char color[4];
  color[0] = (value >> 24) & 0xFF;
  color[1] = (value >> 16) & 0xFF;
  color[2] = (value >>  8) & 0xFF;
  color[3] = (value      ) & 0xFF;
  glColor4ubv(color);
}



static int
CaptureInteger(R2Grid& image)
{
  // Allocate pixel buffer
  unsigned char *pixels = new unsigned char [ 4 * width * height ];

  // Read pixels
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // Fill image
  unsigned char *pixelp = pixels;
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      unsigned int red = *pixelp++;
      unsigned int green = *pixelp++;
      unsigned int blue = *pixelp++;
      unsigned int alpha = *pixelp++;
      unsigned int value = 0;
      value |= red << 24;
      value |= green << 16;
      value |= blue <<  8;
      value |= alpha;
      image.SetGridValue(ix, iy, value);
    }
  }

  // Delete pixels
  delete [] pixels;
  
  // Return success
  return 1;
}



static int 
CaptureDepth(R2Grid& image)
{
  // Get viewport dimensions
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  // Get modelview  matrix
  static GLdouble modelview_matrix[16];
  // glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  for (int i = 0; i < 16; i++) modelview_matrix[i] = 0;
  modelview_matrix[0] = 1.0;
  modelview_matrix[5] = 1.0;
  modelview_matrix[10] = 1.0;
  modelview_matrix[15] = 1.0;
  
  // Get projection matrix
  GLdouble projection_matrix[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);

  // Get viewpoint matrix
  GLint viewport_matrix[16];
  glGetIntegerv(GL_VIEWPORT, viewport_matrix);

  // Allocate pixels
  float *pixels = new float [ image.NEntries() ];

  // Read pixels from frame buffer 
  glReadPixels(0, 0, viewport[2], viewport[3], GL_DEPTH_COMPONENT, GL_FLOAT, pixels); 

  // Clear image
  image.Clear(0.0);
  
  // Convert pixels to depths
  int ix, iy;
  double x, y, z;
  for (int i = 0; i < image.NEntries(); i++) {
    if (RNIsEqual(pixels[i], 1.0)) continue;
    if (RNIsNegativeOrZero(pixels[i])) continue;
    image.IndexToIndices(i, ix, iy);
    gluUnProject(ix, iy, pixels[i], modelview_matrix, projection_matrix, viewport_matrix, &x, &y, &z);
    image.SetGridValue(i, -z);
  }

  // Delete pixels
  delete [] pixels;

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Rendering callback functions
////////////////////////////////////////////////////////////////////////

static RNScalar
Affinity(const R3Point& source_viewpoint, const R3Vector& source_towards,
         const R3Point& target_viewpoint, const R3Vector& target_towards)
{
  // Initialize affinity
  RNScalar affinity = 1.0;
  
  // Consider distance
  RNScalar dd = R3SquaredDistance(source_viewpoint, target_viewpoint);
  if (dd > 1.0) affinity /= dd;

  // Consider delta z
  RNScalar dz = fabs(source_viewpoint.Z() - target_viewpoint.Z());
  if (dz > 1.0) return 0;

  // Consider view angle
  RNScalar dot = source_towards.Dot(target_towards);
  if (dot > 0) affinity *= dot*dot;
  else return 0;

  // Return affinity
  return affinity;
}



static void
RenderConfiguration(const RGBDConfiguration& configuration,
  const R3Point& target_viewpoint, const R3Vector& target_towards, const R3Vector& target_up,
  RNScalar min_affinity = 0.01, RNScalar min_affinity_ratio = 0.25)
{
  // Clear window
  glViewport(0, 0, width, height);
  glClearColor(background.R(), background.G(), background.B(), 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glColor3d(1.0, 1.0, 1.0);

  // Draw images
  if (configuration.NImages() > 0) {
    // Create array of affinities
    RNArray<RGBDImage *> images;
    RNScalar *affinities = new RNScalar [ configuration.NImages() ];
    for (int i = 0; i < configuration.NImages(); i++) {
      RGBDImage *image = configuration.Image(i);
      RNScalar affinity = Affinity(image->WorldViewpoint(), image->WorldTowards(), target_viewpoint, target_towards);
      if (affinity < min_affinity) continue;
      affinities[images.NEntries()] = affinity;
      images.Insert(image);
    }

    // Sort images based on affinity
    for (int i = 0; i < images.NEntries(); i++) {
      for (int j = i+1; j < images.NEntries(); j++) {
        if (affinities[j] > affinities[i]) {
          RGBDImage *swap_image = images[j];
          images[j] = images[i];
          images[i] = swap_image;
          RNScalar swap_affinity = affinities[j];
          affinities[j] = affinities[i];
          affinities[i] = swap_affinity;
        }
      }
    }

    // Draw images with best affinity
    RNScalar best_affinity = affinities[0];
    if (best_affinity > 0) {
      for (int i = 0; i < images.NEntries(); i++) {
        RGBDImage *image = images.Kth(i);
        RNScalar affinity = affinities[i];
        if ((min_affinity_ratio > 0) && (affinity < min_affinity_ratio * best_affinity)) continue;
        image->ReadChannels();
        image->DrawSurfels(RGBD_PHOTO_COLOR_SCHEME);
        image->ReleaseChannels();
        printf("%d %s %g\n", i, image->Name(), affinity);
      }
    }

    // Delete array of affinities
    delete [] affinities;
  }
  
  // Draw surfaces
  for (int i = 0; i < configuration.NSurfaces(); i++) {
    RGBDSurface *surface = configuration.Surface(i);
    surface->ReadChannels();
    surface->DrawTexture(RGBD_PHOTO_COLOR_SCHEME);
    surface->ReleaseChannels();
  }
}



static void
RenderMesh(const R3Mesh& mesh, int rendering_scheme,
  const R3Point& viewpoint, const R3Vector& towards, const R3Vector& up)
{
  // Clear window
  glViewport(0, 0, width, height);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glColor3d(1.0, 1.0, 1.0);

  // Get world to camera matrix
  R3CoordSystem cs(viewpoint, R3Triad(towards, up));
  R4Matrix world_to_camera = cs.InverseMatrix();

  // Draw mesh
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < mesh.NFaces(); i++) {
    R3MeshFace *face = mesh.Face(i);
    if (skip_removed_faces) {
      // This is a hack to skip faces marked for removal in the matterport dataset
      int category_index = mesh.FaceCategory(face);
      if (category_index == 9) continue;
      if (category_index == 64) continue;
    }
    if ((rendering_scheme >= CAMERA_PX_RENDERING) && (rendering_scheme <= WORLD_PZ_RENDERING)) {
      // Set color per vertex
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh.VertexOnFace(face, j);
        RNScalar position_coordinate = 0;
        const R3Point& world_position = mesh.VertexPosition(vertex);
        R3Point camera_position = world_to_camera * world_position;
        if (rendering_scheme == CAMERA_PX_RENDERING) position_coordinate = camera_position.X();
        else if (rendering_scheme == CAMERA_PY_RENDERING) position_coordinate = camera_position.Y();
        else if (rendering_scheme == CAMERA_PZ_RENDERING) position_coordinate = camera_position.Z();
        else if (rendering_scheme == WORLD_PX_RENDERING) position_coordinate = world_position.X() - viewpoint.X();
        else if (rendering_scheme == WORLD_PY_RENDERING) position_coordinate = world_position.Y() - viewpoint.Y();
        else if (rendering_scheme == WORLD_PZ_RENDERING) position_coordinate = world_position.Z() - viewpoint.Z();
        int position_value = 2000 * position_coordinate + 32768;
        if (position_value < 0) position_value = 0;
        else if (position_value > 65535) position_value = 65535;
        LoadInteger(position_value);
        R3LoadPoint(world_position);
      }
    }
    else if (((rendering_scheme >= CAMERA_NX_RENDERING) && (rendering_scheme <= CAMERA_NZ_RENDERING)) ||
             ((rendering_scheme >= WORLD_NX_RENDERING) && (rendering_scheme <= WORLD_NZ_RENDERING)) ||
             (rendering_scheme == NDOTV_RENDERING)) {
      // Set color per face based on normal
      RNScalar normal_coordinate = 0;
      R3Vector world_normal = mesh.FaceNormal(face);
      R3Vector camera_normal = world_to_camera * world_normal; camera_normal.Normalize();
      if (rendering_scheme == CAMERA_NX_RENDERING) normal_coordinate = camera_normal.X();
      else if (rendering_scheme == CAMERA_NY_RENDERING) normal_coordinate = camera_normal.Y();
      else if (rendering_scheme == CAMERA_NZ_RENDERING) normal_coordinate = camera_normal.Z();
      else if (rendering_scheme == WORLD_NX_RENDERING) normal_coordinate = world_normal.X();
      else if (rendering_scheme == WORLD_NY_RENDERING) normal_coordinate = world_normal.Y();
      else if (rendering_scheme == WORLD_NZ_RENDERING) normal_coordinate = world_normal.Z();
      else if (rendering_scheme == NDOTV_RENDERING) normal_coordinate = -(towards.Dot(world_normal));
      int normal_value = 32768 * (normal_coordinate + 1.0);
      if (normal_value < 0) normal_value = 0;
      else if (normal_value > 65535) normal_value = 65535;
      LoadInteger(normal_value);
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh.VertexOnFace(face, j);
        const R3Point& position = mesh.VertexPosition(vertex);
        R3LoadPoint(position);
      }
    }
    else if ((rendering_scheme >= CAMERA_NX_RENDERING) && (rendering_scheme <= NDOTV_RENDERING)) {
      RNScalar offset_coordinate = 0;
      R3Vector world_normal = mesh.FaceNormal(face);
      R3Point world_position = mesh.FaceCentroid(face);
      if (rendering_scheme == CAMERA_ND_RENDERING) {
        R3Point camera_position = world_to_camera * world_position;
        R3Vector camera_normal = world_to_camera * world_normal; camera_normal.Normalize();
        offset_coordinate = -camera_position.X()*camera_normal.X() - camera_position.Y()*camera_normal.Y() - camera_position.Z()*camera_normal.Z();
      }
      else if (rendering_scheme == WORLD_ND_RENDERING) {
        offset_coordinate = -world_position.X()*world_normal.X() - world_position.Y()*world_normal.Y() - world_position.Z()*world_normal.Z();
      }
      int offset_value = 2000 * offset_coordinate + 32768;
      if (offset_value < 0) offset_value = 0;
      else if (offset_value > 65535) offset_value = 65535;
      LoadInteger(offset_value);
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh.VertexOnFace(face, j);
        const R3Point& position = mesh.VertexPosition(vertex);
        R3LoadPoint(position);
      }
    }
    else if ((rendering_scheme >= FACE_INDEX_RENDERING) && (rendering_scheme <= FACE_CATEGORY_RENDERING)) {
      // Set color per face based on face attributes
      if (rendering_scheme == FACE_INDEX_RENDERING) LoadInteger(mesh.FaceID(face)+1);
      else if (rendering_scheme == FACE_MATERIAL_RENDERING) LoadInteger((mesh.FaceMaterial(face)+1) % 65536);
      else if (rendering_scheme == FACE_SEGMENT_RENDERING) LoadInteger((mesh.FaceSegment(face)+1) % 65536);
      else if (rendering_scheme == FACE_CATEGORY_RENDERING) LoadInteger((mesh.FaceCategory(face)+1) % 65536);
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh.VertexOnFace(face, j);
        const R3Point& position = mesh.VertexPosition(vertex);
        R3LoadPoint(position);
      }
    }
    else {
      LoadInteger(0);
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh.VertexOnFace(face, j);
        const R3Point& position = mesh.VertexPosition(vertex);
        R3LoadPoint(position);
      }
    }
  }
  glEnd();
}



static int
LoadNextCamera(R3Point& viewpoint, R3Vector& towards, R3Vector& up)
{
  // Update next image index
  current_image_index++;

  // Check input cameras
  if (cameras.NEntries() > 0) {
    if (current_image_index < cameras.NEntries()) {
      // Load camera from input file
      R3Camera *camera = cameras.Kth(current_image_index);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      camera->Load();
      viewpoint = camera->Origin();
      towards = camera->Towards();
      up = camera->Up();
      return 1;
    }
  }
  else if (configuration.NImages() > 0) {
    if (current_image_index < configuration.NImages()) {
      // Load camera for image from configuration
      RGBDImage *image = configuration.Image(current_image_index);
      if (image->NPixels(RN_X) == 0) {
        image->SetNPixels(width, height);
        // image->ReadChannels();  // Temporary, just to read width/height :(
        // image->ReleaseChannels();
      }
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      image->ProjectionMatrix().Load();
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      image->CameraToWorld().InverseMatrix().Load();
      viewpoint = image->WorldViewpoint();
      towards = image->WorldTowards();
      up = image->WorldUp();
      return 1;
    }
  }

  // Return failure - no more cameras
  return 0;
}



static void
Redraw(void)
{
  // Statistics variables
  static RNTime start_time;
  if (current_image_index == -1) start_time.Read(); 

  // Get camera and name for next image
  R3Point target_viewpoint;
  R3Vector target_towards, target_up;
  if (!LoadNextCamera(target_viewpoint, target_towards, target_up)) {
    if (print_verbose) {
      printf("  Time = %.2f seconds\n", start_time.Elapsed());
      printf("  # Images = %d\n", current_image_index);
      fflush(stdout);
    }
    exit(0);
  }

  // Get image name
  char image_name_buffer[4096];
  char *name = image_name_buffer;
  if (configuration.NImages() > 0) {
    RGBDImage *image = configuration.Image(current_image_index);
    const char *filename = image->DepthFilename();
    if (!filename) filename = image->ColorFilename();
    if (!filename) filename = "default";
    strncpy(image_name_buffer, filename, 4096);
    if (strrchr(image_name_buffer, '/')) name = strrchr(image_name_buffer, '/')+1;
    char *endp = strrchr(name, '.');
    if (endp) *endp = '\0';
  }
  else {
    sprintf(image_name_buffer, "%06d", current_image_index);
  }

  // Print debug statement
  if (print_debug) {
    printf("  Rendering %s ...\n", name);
    fflush(stdout);
  }

  // Capture and write depth image 
  if (capture_depth_images || capture_color_images) {
    // Draw configuration
    RenderConfiguration(configuration, target_viewpoint, target_towards, target_up);

    // Capture and write depth image
    if (capture_depth_images) {
      R2Grid depth_image(width, height);
      if (CaptureDepth(depth_image)) {
        depth_image.Multiply(4000);
        depth_image.Threshold(65535, R2_GRID_KEEP_VALUE, 0);
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_depth.png", output_image_directory, name);
        depth_image.WriteFile(output_image_filename);
      }
    }

    // Capture and write color image 
    if (capture_color_images) {
      R2Image color_image(width, height, 3);
      if (CaptureColor(color_image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_color.jpg", output_image_directory, name);
        color_image.Write(output_image_filename);
      }
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_depth_images) {
    RenderMesh(mesh, BLACK_RENDERING, target_viewpoint, target_towards, target_up);
    R2Grid depth_image(width, height);
    if (CaptureDepth(depth_image)) {
      depth_image.Multiply(4000);
      depth_image.Threshold(65535, R2_GRID_KEEP_VALUE, 0);
      char output_image_filename[1024];
      sprintf(output_image_filename, "%s/%s_mesh_depth.png", output_image_directory, name);
      depth_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_position_images) {
    R2Grid position_image(width, height);
    char output_image_filename[1024];
    RenderMesh(mesh, CAMERA_PX_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(position_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_px.png", output_image_directory, name);
      position_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, CAMERA_PY_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(position_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_py.png", output_image_directory, name);
      position_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, CAMERA_PZ_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(position_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_pz.png", output_image_directory, name);
      position_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_wposition_images) {
    R2Grid position_image(width, height);
    char output_image_filename[1024];
    RenderMesh(mesh, WORLD_PX_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(position_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_wpx.png", output_image_directory, name);
      position_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, WORLD_PY_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(position_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_wpy.png", output_image_directory, name);
      position_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, WORLD_PZ_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(position_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_wpz.png", output_image_directory, name);
      position_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_normal_images) {
    R2Grid normal_image(width, height);
    char output_image_filename[1024];
    RenderMesh(mesh, CAMERA_NX_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_nx.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, CAMERA_NY_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_ny.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, CAMERA_NZ_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_nz.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, CAMERA_ND_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_nd.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_wnormal_images) {
    R2Grid normal_image(width, height);
    char output_image_filename[1024];
    RenderMesh(mesh, WORLD_NX_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_wnx.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, WORLD_NY_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_wny.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, WORLD_NZ_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_wnz.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
    RenderMesh(mesh, WORLD_ND_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(normal_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_wnd.png", output_image_directory, name);
      normal_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_ndotv_images) {
    R2Grid ndotv_image(width, height);
    char output_image_filename[1024];
    RenderMesh(mesh, NDOTV_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(ndotv_image)) {
      sprintf(output_image_filename, "%s/%s_mesh_ndotv.png", output_image_directory, name);
      ndotv_image.WriteFile(output_image_filename);
    }
  }
  
  if (!mesh.IsEmpty() && capture_mesh_face_images) {
    R2Grid face_image(width, height);
    RenderMesh(mesh, FACE_INDEX_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(face_image)) {
      char output_image_filename[1024];
      sprintf(output_image_filename, "%s/%s_mesh_face.pfm", output_image_directory, name);
      face_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_material_images) {
    R2Grid material_image(width, height);
    RenderMesh(mesh, FACE_MATERIAL_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(material_image)) {
      char output_image_filename[1024];
      sprintf(output_image_filename, "%s/%s_mesh_material.png", output_image_directory, name);
      material_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_segment_images) {
    R2Grid segment_image(width, height);
    RenderMesh(mesh, FACE_SEGMENT_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(segment_image)) {
      char output_image_filename[1024];
      sprintf(output_image_filename, "%s/%s_mesh_segment.png", output_image_directory, name);
      segment_image.WriteFile(output_image_filename);
    }
  }

  if (!mesh.IsEmpty() && capture_mesh_category_images) {
    R2Grid category_image(width, height);
    RenderMesh(mesh, FACE_CATEGORY_RENDERING, target_viewpoint, target_towards, target_up);
    if (CaptureInteger(category_image)) {
      char output_image_filename[1024];
      sprintf(output_image_filename, "%s/%s_mesh_category.png", output_image_directory, name);
      category_image.WriteFile(output_image_filename);
    }
  }
    
#ifdef USE_GLUT
  // Redraw
  glutPostRedisplay();
#endif
}



static int
RenderImagesWithGlut(const char *output_image_directory)
{
#ifdef USE_GLUT
  // Print message
  if (print_verbose) {
    printf("Rendering images with glut to %s\n", output_image_directory);
    fflush(stdout);
  }

  // Open window
  int argc = 1;
  char *argv[1];
  argv[0] = strdup("scn2img");
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA); 
  glutCreateWindow("Configuration Image Capture");

  // Initialize GLUT callback functions 
  glutDisplayFunc(Redraw);

  // Run main loop  -- never returns 
  glutMainLoop();

  // Return success -- actually never gets here
  return 1;
#else
  // Not supported
  RNAbort("Program was not compiled with glut.  Recompile with make.\n");
  return 0;
#endif
}



static int
RenderImagesWithMesa(const char *output_image_directory)
{
#ifdef USE_MESA
  // Print message
  if (print_verbose) {
    printf("Rendering images with mesa to %s\n", output_image_directory);
    fflush(stdout);
  }

  // Create mesa context
  OSMesaContext ctx = OSMesaCreateContextExt(OSMESA_RGBA, 32, 0, 0, NULL);
  if (!ctx) {
    fprintf(stderr, "Unable to create mesa context\n");
    return 0;
  }

  // Create frame buffer
  void *frame_buffer = malloc(width * height * 4 * sizeof(GLubyte) );
  if (!frame_buffer) {
    fprintf(stderr, "Unable to allocate mesa frame buffer\n");
    return 0;
  }

  // Assign mesa context
  if (!OSMesaMakeCurrent(ctx, frame_buffer, GL_UNSIGNED_BYTE, width, height)) {
    fprintf(stderr, "Unable to make mesa context current\n");
    return 0;
  }

  // Draw images
  while (TRUE) Redraw();
  
  // Delete mesa context
  OSMesaDestroyContext(ctx);

  // Delete frame buffer
  free(frame_buffer);

  // Return success
  return 1;
#else
  // Not supported
  RNAbort("Program was not compiled with mesa.  Recompile with make mesa.\n");
  return 0;
#endif
}



static int
RenderImages(const char *output_image_directory)
{
  // Check parameters
  if (!capture_color_images && !capture_depth_images &&
      !capture_mesh_depth_images &&
      !capture_mesh_position_images && !capture_mesh_wposition_images &&
      !capture_mesh_normal_images && !capture_mesh_wnormal_images && !capture_mesh_ndotv_images &&
      !capture_mesh_face_images && !capture_mesh_material_images &&
      !capture_mesh_segment_images && !capture_mesh_category_images) return 1;

  // Create output directory
  char cmd[1024];
  sprintf(cmd, "mkdir -p %s", output_image_directory);
  system(cmd);

  // Render images
  if (glut) { if (!RenderImagesWithGlut(output_image_directory)) return 0; }
  else if (mesa) { if (!RenderImagesWithMesa(output_image_directory)) return 0; }
  else { RNAbort("Not implemented"); }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Initialize variables to track whether to assign defaults
  int output = 0;

  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-debug")) print_debug = 1;
      else if (!strcmp(*argv, "-glut")) { mesa = 0; glut = 1; }
      else if (!strcmp(*argv, "-mesa")) { mesa = 1; glut = 0; }
      else if (!strcmp(*argv, "-skip_removed_faces")) skip_removed_faces = 1;
      else if (!strcmp(*argv, "-cameras")) { argc--; argv++; input_camera_filename = *argv; }
      else if (!strcmp(*argv, "-mesh")) { argc--; argv++; input_mesh_filename = *argv; }
      else if (!strcmp(*argv, "-width")) { argc--; argv++; width = atoi(*argv); }
      else if (!strcmp(*argv, "-height")) { argc--; argv++; height = atoi(*argv); }
      else if (!strcmp(*argv, "-xfov")) { argc--; argv++; xfov = atof(*argv); }
      else if (!strcmp(*argv, "-load_every_kth_image")) { argc--; argv++; load_every_kth_image = atoi(*argv); }
      else if (!strcmp(*argv, "-create_position_images")) { output = create_position_images = 1; }
      else if (!strcmp(*argv, "-create_normal_images")) { output = create_normal_images = 1; }
      else if (!strcmp(*argv, "-create_boundary_images")) { output = create_boundary_images = 1; }
      else if (!strcmp(*argv, "-create_segmentation_images")) { output = create_segmentation_images = 1; }
      else if (!strcmp(*argv, "-capture_color_images")) { output = capture_color_images = 1; }
      else if (!strcmp(*argv, "-capture_depth_images")) { output = capture_depth_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_depth_images")) { output = capture_mesh_depth_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_position_images")) { output = capture_mesh_position_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_wposition_images")) { output = capture_mesh_wposition_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_normal_images")) { output = capture_mesh_normal_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_wnormal_images")) { output = capture_mesh_wnormal_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_ndotv_images")) { output = capture_mesh_ndotv_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_face_images")) { output = capture_mesh_face_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_material_images")) { output = capture_mesh_material_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_segment_images")) { output = capture_mesh_segment_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_category_images")) { output = capture_mesh_category_images = 1; }
      else if (!strcmp(*argv, "-capture_mesh_images")) { output = 1;
        capture_mesh_depth_images = 1;
        capture_mesh_position_images = 1;
        capture_mesh_wposition_images = 1; 
        capture_mesh_normal_images = 1;
        capture_mesh_wnormal_images = 1; 
        capture_mesh_ndotv_images = 1; 
        capture_mesh_face_images = 1; 
        capture_mesh_material_images = 1; 
        capture_mesh_segment_images = 1; 
        capture_mesh_category_images = 1; 
      }
      else if (!strcmp(*argv, "-background")) {
        argc--; argv++; background[0] = atof(*argv);
        argc--; argv++; background[1] = atof(*argv);
        argc--; argv++; background[2] = atof(*argv);
      }
      else {
        fprintf(stderr, "Invalid program argument: %s", *argv);
        exit(1);
      }
      argv++; argc--;
    }
    else {
      if (!input_configuration_filename) input_configuration_filename = *argv;
      else if (!output_image_directory) output_image_directory = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Set default capture options
  if (!output) {
    capture_color_images = 1;
    capture_depth_images = 1;
  }
  
  // Check filenames
  if (!input_configuration_filename || !output_image_directory) {
    fprintf(stderr, "Usage: conf2img inputconfigurationfile outputimagedirectory [options]\n");
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
  if (!ReadConfiguration(input_configuration_filename)) exit(-1);

  // Write images for configuration cameras
  if (!WriteImages(output_image_directory)) return 0;

  // Read cameras
  if (input_camera_filename) {
    if (!ReadCameras(input_camera_filename)) exit(-1);
  }

  // Read mesh
  if (input_mesh_filename) {
    if (!ReadMesh(input_mesh_filename)) exit(-1);
  }

  // Render images
  if (!RenderImages(output_image_directory)) exit(-1);
  
  // Return success 
  return 0;
}



