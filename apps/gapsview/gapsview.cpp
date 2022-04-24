// Source file for the gaps viewer program



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Graphics/R3Graphics.h"
#include "R3Surfels/R3Surfels.h"
#include "GSV/GSV.h"
#include "R3PointSet.h"
#include "fglut/fglut.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

// Input filenames
static RNArray<const char *> pointset_filenames;
static RNArray<const char *> mesh_filenames;
static RNArray<const char *> scene_filenames;
static RNArray<const char *> grid_filenames;
static RNArray<const char *> ssa_filenames;
static RNArray<const char *> ssb_filenames;
static RNArray<const char *> gsv_filenames;
static const char *image_directory = NULL;

// Display parameters
static RNScalar background_color[3] = { 1, 1, 1 };
static R3Point initial_camera_origin = R3Point(0.0, 0.0, 0.0);
static R3Vector initial_camera_towards = R3Vector(0.0, 0.0, -1.0);
static R3Vector initial_camera_up = R3Vector(0.0, 1.0, 0.0);
static RNBoolean initial_camera = FALSE;

// Info variables
static int print_verbose = 0;



////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////

enum {
  NULL_COLOR_SCHEME,
  RGB_COLOR_SCHEME,
  SHADING_COLOR_SCHEME,
  CATEGORY_COLOR_SCHEME,
  INSTANCE_COLOR_SCHEME,
  PICK_COLOR_SCHEME,
}; 
  
enum {
  NULL_ELEMENT_TYPE,
  POINTSET_POINT_ELEMENT_TYPE,
  MESH_VERTEX_ELEMENT_TYPE,
  MESH_EDGE_ELEMENT_TYPE,
  MESH_FACE_ELEMENT_TYPE,
  SCENE_NODE_ELEMENT_TYPE,
  GRID_CELL_ELEMENT_TYPE,
  SFL_POINT_ELEMENT_TYPE,
  SFL_CAMERA_ELEMENT_TYPE,
  GSV_POINT_ELEMENT_TYPE,
  GSV_CAMERA_ELEMENT_TYPE,
};



////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////

// Data variables
static RNArray<R3Scene *> scenes;
static RNArray<R3Mesh *> meshes;
static RNArray<R3PointSet *> pointsets;
static RNArray<R3Grid *> grids;
static RNArray<R3SurfelScene *> sfls;
static RNArray<GSVScene *> gsvs;

// Viewer variables
static R3Viewer *viewer = NULL;
static R3Box world_bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
static R3Point world_origin(0, 0, 0);

// Selection variables
static R3Point selected_position(0, 0, 0);

// Display variables - which data sources to show
static int show_scenes = 1;
static int show_meshes = 1;
static int show_pointsets = 1;
static int show_grids = 1;
static int show_sfls = 1;
static int show_gsvs = 1;

// Display variables - which elements to show
static int show_surfaces = 0;
static int show_edges = 0;
static int show_points = 1;
static int show_normals = 0;
static int show_cameras = 1;
static int show_image_insets = 0;
static int show_image_billboards = 0;
static int show_image_depths = 0;
static int show_image_rays = 0;
static int show_slices = 0;
static int show_text_labels = 0;
static int show_selected_position = 1;
static int show_grid_threshold = 0;
static int show_bbox = 0;
static int show_axes = 0;

// Display variables - how to show them
static int color_scheme = RGB_COLOR_SCHEME;

// Model selection
static int nmodels = 0;
static const int max_models = 64;
static int selected_model_index = 0;

// Image display variables
static RNScalar image_inset_scale = 0.1;
static RNScalar image_billboard_depth = 1;

// Depth image scaling
static RNScalar depth_scale = 2000;
static RNScalar depth_exponent = 0.5;

// Grid display variables
static RNScalar grid_thresholds[max_models] = { 0 };
static RNInterval grid_ranges[max_models] = { RNInterval(0,0) };
static int grid_slice_coords[3] = { 0, 0, 0 };

// GLUT state variables 
static int GLUTwindow = 0;
static int GLUTwindow_width = 800;
static int GLUTwindow_height = 600;
static int GLUTmouse[2] = { 0, 0 };
static int GLUTbutton[3] = { 0, 0, 0 };
static int GLUTmouse_drag_distance_squared = 0;
static int GLUTmodifiers = 0;



////////////////////////////////////////////////////////////////////////
// Image data type definition
////////////////////////////////////////////////////////////////////////

struct ImageData {
  ImageData(void) : image_width(0), image_height(0), texture_id(0) {};
  R2Image color_image;
  R2Grid depth_image;
  R2Grid category_image;
  R2Grid instance_image;
  R2Grid ray_origin_images[3];
  R2Grid ray_direction_images[3];
  int image_width;
  int image_height;
  GLuint texture_id;
};



////////////////////////////////////////////////////////////////////////
// Image I/O functions
////////////////////////////////////////////////////////////////////////

static ImageData *
ReadImageFiles(const char *image_name,
  double depth_scale, double depth_exponent)
{
  // Check image directory
  if (!image_directory) return NULL;
  if (!image_name) return NULL;
  
  /// Get convenient variables
  double ds = (depth_scale >= 0) ? 1.0 / depth_scale : 1.0;
  double de = (depth_exponent != 0) ? 1.0 / depth_exponent : 1.0;
  char filename[2048];
  
  // Allocate GSV image data structure
  ImageData *data = new ImageData();
  if (!data) {
    fprintf(stderr, "Unable to allocate GSV image data\n");
    return NULL;
  }

  // Read color image 
  sprintf(filename, "%s/color_images/%s.png", image_directory, image_name);
  if (RNFileExists(filename)) {
    if (!data->color_image.ReadFile(filename)) { delete data; return NULL; }
  }
  else {
    sprintf(filename, "%s/color_images/%s.jpg", image_directory, image_name);
    if (RNFileExists(filename)) {
      if (!data->color_image.ReadFile(filename)) { delete data; return NULL; }
    }
  }

  // Read depth image
  sprintf(filename, "%s/depth_images/%s.png", image_directory, image_name);
  if (RNFileExists(filename)) {
    if (!data->depth_image.ReadFile(filename)) { delete data; return NULL; }
    data->depth_image.Multiply(ds);
    data->depth_image.Pow(de);
  }

  // Read category image
  sprintf(filename, "%s/category_images/%s.png", image_directory, image_name);
  if (RNFileExists(filename)) {
    if (!data->category_image.ReadFile(filename)) { delete data; return NULL; }
  }

  // Read instance image
  sprintf(filename, "%s/instance_images/%s.png", image_directory, image_name);
  if (RNFileExists(filename)) {
    if (!data->instance_image.ReadFile(filename)) { delete data; return NULL; }
  }

  // Read ray_origin images
  for (int i = 0; i < 3; i++) {
    const char *channel_name = "";
    if (i == 0) channel_name = "ox";
    else if (i == 1) channel_name = "oy";
    else if (i == 2) channel_name = "oz";
    sprintf(filename, "%s/ray_images/%s_%s.pfm", image_directory, image_name, channel_name);
    if (RNFileExists(filename)) {
      if (!data->ray_origin_images[i].ReadFile(filename)) { delete data; return NULL; }
    }
  }

  // Read ray_direction images
  for (int i = 0; i < 3; i++) {
    const char *channel_name = "";
    if (i == 0) channel_name = "dx";
    else if (i == 1) channel_name = "dy";
    else if (i == 2) channel_name = "dz";
    sprintf(filename, "%s/ray_images/%s_%s.pfm", image_directory, image_name, channel_name);
    if (RNFileExists(filename)) {
      if (!data->ray_direction_images[i].ReadFile(filename)) { delete data; return NULL; }
    }
  }

  // Set image dimensions
  data->image_width = data->color_image.Width();
  data->image_height = data->color_image.Height();
  
  // Return data
  return data;
}



static int
ReadGSVImageFiles(GSVScene *scene,
  double depth_scale, double depth_exponent)
{
  // Check image directory
  if (!image_directory) return 1;

  // Start statistics
  RNTime start_time;
  start_time.Read();
  int color_image_count = 0;
  int depth_image_count = 0;
  int category_image_count = 0;
  int instance_image_count = 0;
  int ray_image_count = 0;
  
  // Read data for all images
  for (int ir = 0; ir < scene->NRuns(); ir++) {
    GSVRun *run = scene->Run(ir);
    for (int is = 0; is < run->NSegments(); is++) {
      GSVSegment *segment = run->Segment(is);
      for (int ip = 0; ip < segment->NPanoramas(); ip++) {
        GSVPanorama *panorama = segment->Panorama(ip);
        for (int ii = 0; ii < panorama->NImages(); ii++) {
          GSVImage *image = panorama->Image(ii);
          if (!image->Name()) continue;

          // Read images
          ImageData *data = ReadImageFiles(image->Name(), depth_scale, depth_exponent);
          if (!data) {
            RNFail("Unable to read images for %s\n", image->Name());
            return 0;
          }

          // Associate data with image
          image->SetData(data);

          // Update statistics
          if (data->color_image.Width() > 0) color_image_count++;
          if (data->depth_image.NEntries() > 0) depth_image_count++;
          if (data->category_image.NEntries() > 0) category_image_count++;
          if (data->instance_image.NEntries() > 0) instance_image_count++;
          if (data->ray_origin_images[0].NEntries() > 0) ray_image_count++;
          if (data->ray_origin_images[1].NEntries() > 0) ray_image_count++;
          if (data->ray_origin_images[2].NEntries() > 0) ray_image_count++;
          if (data->ray_direction_images[0].NEntries() > 0) ray_image_count++;
          if (data->ray_direction_images[1].NEntries() > 0) ray_image_count++;
          if (data->ray_direction_images[2].NEntries() > 0) ray_image_count++;
        }
      }
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("Read GSV images from %s ...\n", image_directory);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Color Images = %d\n", color_image_count);
    printf("  # Depth Images = %d\n", depth_image_count);
    printf("  # Category Images = %d\n", category_image_count);
    printf("  # Instance Images = %d\n", instance_image_count);
    printf("  # Ray Images = %d\n", ray_image_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
ReadSFLImageFiles(R3SurfelScene *scene,
  double depth_scale, double depth_exponent)
{
  // Check image directory
  if (!image_directory) return 1;

  // Start statistics
  RNTime start_time;
  start_time.Read();
  int color_image_count = 0;
  int depth_image_count = 0;
  int category_image_count = 0;
  int instance_image_count = 0;
  int ray_image_count = 0;

  // Read all images
  for (int i = 0; i < scene->NImages(); i++) {
    R3SurfelImage *image = scene->Image(i);
    if (!image->Name()) continue;

    // Read images
    ImageData *data = ReadImageFiles(image->Name(), depth_scale, depth_exponent);
    if (!data) {
      RNFail("Unable to read images for %s\n", image->Name());
      return 0;
    }

    // Associate data with image
    image->SetData(data);

    // Update statistics
    if (data->color_image.Width() > 0) color_image_count++;
    if (data->depth_image.NEntries() > 0) depth_image_count++;
    if (data->category_image.NEntries() > 0) category_image_count++;
    if (data->instance_image.NEntries() > 0) instance_image_count++;
    if (data->ray_origin_images[0].NEntries() > 0) ray_image_count++;
    if (data->ray_origin_images[1].NEntries() > 0) ray_image_count++;
    if (data->ray_origin_images[2].NEntries() > 0) ray_image_count++;
    if (data->ray_direction_images[0].NEntries() > 0) ray_image_count++;
    if (data->ray_direction_images[1].NEntries() > 0) ray_image_count++;
    if (data->ray_direction_images[2].NEntries() > 0) ray_image_count++;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read GSV images from %s ...\n", image_directory);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Color Images = %d\n", color_image_count);
    printf("  # Depth Images = %d\n", depth_image_count);
    printf("  # Category Images = %d\n", category_image_count);
    printf("  # Instance Images = %d\n", instance_image_count);
    printf("  # Ray Images = %d\n", ray_image_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Scene I/O functions
////////////////////////////////////////////////////////////////////////

static R3PointSet *
ReadPointSetFile(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate pointset
  R3PointSet *pointset = new R3PointSet();
  if (!pointset) {
    RNFail("Unable to allocate pointset for %s\n", filename);
    return NULL;
  }

  // Read pointset from file
  if (!pointset->ReadFile(filename)) {
    delete pointset;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read pointset from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Points = %d\n", pointset->NPoints());
    fflush(stdout);
  }

  // Return success
  return pointset;
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
    printf("Read mesh from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Faces = %d\n", mesh->NFaces());
    printf("  # Edges = %d\n", mesh->NEdges());
    printf("  # Vertices = %d\n", mesh->NVertices());
    fflush(stdout);
  }

  // Return success
  return mesh;
}



static R3Scene *
ReadSceneFile(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate scene
  R3Scene *scene = new R3Scene();
  if (!scene) {
    RNFail("Unable to allocate scene for %s\n", filename);
    return NULL;
  }

  // Read scene from file
  if (!scene->ReadFile(filename)) {
    delete scene;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read scene from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Nodes = %d\n", scene->NNodes());
    printf("  # Lights = %d\n", scene->NLights());
    printf("  # Materials = %d\n", scene->NMaterials());
    printf("  # Brdfs = %d\n", scene->NBrdfs());
    printf("  # Textures = %d\n", scene->NTextures());
    printf("  # Referenced scenes = %d\n", scene->NReferencedScenes());
    fflush(stdout);
  }

  // Return scene
  return scene;
}



static R3Grid *
ReadGridFile(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate grid
  R3Grid *grid = new R3Grid();
  if (!grid) {
    RNFail("Unable to allocated grid");
    return NULL;
  }

  // Read grid 
  if (!grid->ReadFile(filename)) {
    RNFail("Unable to read grid file %s", filename);
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    const R3Box bbox = grid->WorldBox();
    RNInterval grid_range = grid->Range();
    printf("Read grid from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d %d\n", grid->XResolution(), grid->YResolution(), grid->ZResolution());
    printf("  World Box = ( %g %g %g ) ( %g %g %g )\n", bbox[0][0], bbox[0][1], bbox[0][2], bbox[1][0], bbox[1][1], bbox[1][2]);
    printf("  Spacing = %g\n", grid->GridToWorldScaleFactor());
    printf("  Cardinality = %d\n", grid->Cardinality());
    printf("  Volume = %g\n", grid->Volume());
    printf("  Minimum = %g\n", grid_range.Min());
    printf("  Maximum = %g\n", grid_range.Max());
    printf("  L1Norm = %g\n", grid->L1Norm());
    fflush(stdout);
  }

  // Return success
  return grid;
}



static R3SurfelScene *
OpenSFLFiles(const char *scene_filename, const char *database_filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate scene
  R3SurfelScene *scene = new R3SurfelScene();
  if (!scene) {
    RNFail("Unable to allocate SFL scene\n");
    return NULL;
  }

  // Open scene files
  if (!scene->OpenFile(scene_filename, database_filename, "r", "r")) {
    delete scene;
    return NULL;
  }

  // Read all surfels into memory
  scene->ReadBlocks();
  
  // Print statistics
  if (print_verbose) {
    printf("Opened sfl scene from %s ...\n", scene_filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Objects = %d\n", scene->NObjects());
    printf("  # Labels = %d\n", scene->NLabels());
    printf("  # Assignments = %d\n", scene->NLabelAssignments());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Surfels = %lld\n", scene->Tree()->NSurfels());
    fflush(stdout);
  }

  // Return scene
  return scene;
}



static void
CloseSFLFiles(R3SurfelScene *scene)
{
  // Release all surfels from memory
  scene->ReleaseBlocks();
  
  // Close scene
  scene->CloseFile();
}



static GSVScene *
ReadGSVFile(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate gsv scene
  GSVScene *scene = new GSVScene();
  if (!scene) {
    RNFail("Unable to allocated gsv scene");
    return NULL;
  }

  // Read gsv 
  if (!scene->ReadFile(filename)) {
    RNFail("Unable to read gsv file %s", filename);
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read GSV scene from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Runs = %d\n", scene->NRuns());
    printf("  # Segments = %d\n", scene->NSegments());
    printf("  # Cameras = %d\n", scene->NCameras());
    printf("  # Tapestries = %d\n", scene->NTapestries());
    printf("  # Panoramas = %d\n", scene->NPanoramas());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Lasers = %d\n", scene->NLasers());
    printf("  # Surveys = %d\n", scene->NSurveys());
    printf("  # Scans = %d\n", scene->NScans());
    printf("  # Scanlines = %d\n", scene->NScanlines());
    printf("  # Points = %d\n", scene->NPoints());
    fflush(stdout);
  }

  // Return GSV scene
  return scene;
}



static int 
ReadFiles(void)
{
  // Read pointsets
  for (int i = 0; i < pointset_filenames.NEntries(); i++) {
    const char *filename = pointset_filenames[i];
    R3PointSet *pointset = ReadPointSetFile(filename);
    if (!pointset) return 0;
    pointsets.Insert(pointset);
  }

  // Read meshes
  for (int i = 0; i < mesh_filenames.NEntries(); i++) {
    const char *filename = mesh_filenames[i];
    R3Mesh *mesh = ReadMeshFile(filename);
    if (!mesh) return 0;
    meshes.Insert(mesh);
  }

  // Read scenes
  for (int i = 0; i < scene_filenames.NEntries(); i++) {
    const char *filename = scene_filenames[i];
    R3Scene *scene = ReadSceneFile(filename);
    if (!scene) return 0;
    scenes.Insert(scene);
  }

  // Read grids
  for (int i = 0; i < grid_filenames.NEntries(); i++) {
    const char *filename = grid_filenames[i];
    R3Grid *grid = ReadGridFile(filename);
    if (!grid) return 0;
    grids.Insert(grid);
    grid_ranges[i] = grid->Range();
    grid_thresholds[i] = grid->Mean() + 3 * grid->StandardDeviation();
    if (grid_thresholds[i] > grid_ranges[i].Max() - RN_EPSILON) {
      grid_thresholds[i] = grid_ranges[i].Max() - RN_EPSILON;
    }
  }

  // Read sfl scenes
  for (int i = 0; i < ssa_filenames.NEntries(); i++) {
    if (i >= ssb_filenames.NEntries()) break;
    const char *ssa_filename = ssa_filenames[i];
    const char *ssb_filename = ssb_filenames[i];
    R3SurfelScene *sfl = OpenSFLFiles(ssa_filename, ssb_filename);
    if (!sfl) return 0;
    sfls.Insert(sfl);
    if (!ReadSFLImageFiles(sfl, depth_scale, depth_exponent)) {
      delete sfl;
      return 0;
    }
  }
  
  // Read gsv scenes
  for (int i = 0; i < gsv_filenames.NEntries(); i++) {
    const char *filename = gsv_filenames[i];
    GSVScene *gsv = ReadGSVFile(filename);
    if (!gsv) return 0;
    gsvs.Insert(gsv);
    if (!ReadGSVImageFiles(gsv, depth_scale, depth_exponent)) {
      delete gsv;
      return 0;
    }
  }
  
  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Coloring utilities
////////////////////////////////////////////////////////////////////////

static void
LoadColorIndex(int model_index, int element_type, int element_index)
{
  // Set color to represent an integer (24 bits)
  int k = element_index + 1;
  unsigned char color[4];
  color[0] = (k >> 16) & 0xFF;
  color[1] = (k >>  8) & 0xFF;
  color[2] = (k      ) & 0xFF;
  color[3] = ((model_index + 1) & 0x0F) << 4;
  color[3] |= (element_type + 1) & 0x0F;
  RNLoadRgba(color);
}



static void
LoadColorIdentifier(int k)
{
  // Make array of colors
  const int ncolors = 72;
  const RNRgb colors[ncolors] = {
    RNRgb(0.5, 0.5, 0.5), RNRgb(1, 0, 0), RNRgb(0, 0, 1), 
    RNRgb(0, 1, 0), RNRgb(0, 1, 1), RNRgb(1, 0, 1), 
    RNRgb(1, 0.5, 0), RNRgb(0, 1, 0.5), RNRgb(0.5, 0, 1), 
    RNRgb(0.5, 1, 0), RNRgb(0, 0.5, 1), RNRgb(1, 0, 0.5), 
    RNRgb(0.5, 0, 0), RNRgb(0, 0.5, 0), RNRgb(0, 0, 0.5), 
    RNRgb(0.5, 0.5, 0), RNRgb(0, 0.5, 0.5), RNRgb(0.5, 0, 0.5),
    RNRgb(0.7, 0, 0), RNRgb(0, 0.7, 0), RNRgb(0, 0, 0.7), 
    RNRgb(0.7, 0.7, 0), RNRgb(0, 0.7, 0.7), RNRgb(0.7, 0, 0.7), 
    RNRgb(0.7, 0.3, 0), RNRgb(0, 0.7, 0.3), RNRgb(0.3, 0, 0.7), 
    RNRgb(0.3, 0.7, 0), RNRgb(0, 0.3, 0.7), RNRgb(0.7, 0, 0.3), 
    RNRgb(0.3, 0, 0), RNRgb(0, 0.3, 0), RNRgb(0, 0, 0.3), 
    RNRgb(0.3, 0.3, 0), RNRgb(0, 0.3, 0.3), RNRgb(0.3, 0, 0.3),
    RNRgb(1, 0.3, 0.3), RNRgb(0.3, 1, 0.3), RNRgb(0.3, 0.3, 1), 
    RNRgb(1, 1, 0.3), RNRgb(0.3, 1, 1), RNRgb(1, 0.3, 1), 
    RNRgb(1, 0.5, 0.3), RNRgb(0.3, 1, 0.5), RNRgb(0.5, 0.3, 1), 
    RNRgb(0.5, 1, 0.3), RNRgb(0.3, 0.5, 1), RNRgb(1, 0.3, 0.5), 
    RNRgb(0.5, 0.3, 0.3), RNRgb(0.3, 0.5, 0.3), RNRgb(0.3, 0.3, 0.5), 
    RNRgb(0.5, 0.5, 0.3), RNRgb(0.3, 0.5, 0.5), RNRgb(0.5, 0.3, 0.5),
    RNRgb(0.3, 0.5, 0.5), RNRgb(0.5, 0.3, 0.5), RNRgb(0.5, 0.5, 0.3), 
    RNRgb(0.3, 0.3, 0.5), RNRgb(0.5, 0.3, 0.3), RNRgb(0.3, 0.5, 0.3), 
    RNRgb(0.3, 0.8, 0.5), RNRgb(0.5, 0.3, 0.8), RNRgb(0.8, 0.5, 0.3), 
    RNRgb(0.8, 0.3, 0.5), RNRgb(0.5, 0.8, 0.3), RNRgb(0.3, 0.5, 0.8), 
    RNRgb(0.8, 0.5, 0.5), RNRgb(0.5, 0.8, 0.5), RNRgb(0.5, 0.5, 0.8), 
    RNRgb(0.8, 0.8, 0.5), RNRgb(0.5, 0.8, 0.8), RNRgb(0.8, 0.5, 0.8)
  };

  // Load color
  if (k == -1) RNLoadRgb(0.8, 0.8, 0.8);
  else if (k == 0) RNLoadRgb(colors[0]);
  else RNLoadRgb(colors[1 + (k % (ncolors-1))]);
}



static void 
LoadColorValue(double value) 
{
  // Initialize color
  unsigned char color[3];
  
  // Compute rgb based on blue-green-red heatmap
  if (value <= 0) {
    color[0] = 0;
    color[1] = 0;
    color[2] = 255;
  }
  else if (value < 0.1) {
    value *= 10 * 255;
    color[0] = 0;
    color[1] = value;
    color[2] = 255;
  }
  else if (value < 0.5) {
    value = (value - 0.1) * 2.5 * 255;
    color[0] = 0;
    color[1] = 255;
    color[2] = 255 - value;
  }
  else if (value < 0.9) {
    value = (value - 0.5) * 2.5 * 255;
    color[0] = value;
    color[1] = 255;
    color[2] = 0;
  }
  else if (value < 1) {
    value = (value - 0.9) * 10 * 255;
    color[0] = 255;
    color[1] = 255 - value;
    color[2] = 0;
  }
  else {
    color[0] = 255;
    color[1] = 0;
    color[2] = 0;
  }

  // Load color
  RNLoadRgb(color);
}



////////////////////////////////////////////////////////////////////////
// Texturing utilities
////////////////////////////////////////////////////////////////////////

static void
LoadTexture(ImageData *data)
{
  // Check if already loaded
  if (data->texture_id > 0) return;

  // Check color image
  if (data->color_image.Width() <= 0) return;
  if (data->color_image.Height() <= 0) return;
  if (data->color_image.Depth() != 3) return;

  // Allocate texture id
  glGenTextures(1, &data->texture_id);
  assert(data->texture_id > 0);

  // Set texture parameters
  glBindTexture(GL_TEXTURE_2D, data->texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // Set texture pixels
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
    data->color_image.Width(), data->color_image.Height(),
    GL_RGB, GL_UNSIGNED_BYTE, data->color_image.Pixels());
}



////////////////////////////////////////////////////////////////////////
// Drawing utilities
////////////////////////////////////////////////////////////////////////

static void
DrawSelectedPosition(void)
{
  // Check if should draw selected position
  if (!show_selected_position) return;

  // Draw sphere
  glEnable(GL_LIGHTING);
  RNLoadRgb(1, 0, 0);
  R3Sphere(selected_position, 0.05).Draw();
}



static void
DrawBBox(void)
{
  // Check if should draw bbox
  if (!show_bbox) return;

  // Draw bounding box
  glDisable(GL_LIGHTING);
  RNLoadRgb(0.5, 0.5, 0.5);
  world_bbox.Outline();
}



static void
DrawAxes(void)
{
  // Check if should draw axes
  if (!show_axes) return;

  // Draw axes
  RNScalar d = world_bbox.DiagonalRadius();
  glDisable(GL_LIGHTING);
  glLineWidth(3);
  R3BeginLine();
  RNLoadRgb(1, 0, 0);
  R3LoadPoint(R3zero_point + 0.5*d * R3negx_vector);
  R3LoadPoint(R3zero_point + d * R3posx_vector);
  R3EndLine();
  R3BeginLine();
  RNLoadRgb(0, 1, 0);
  R3LoadPoint(R3zero_point + 0.5*d * R3negy_vector);
  R3LoadPoint(R3zero_point + d * R3posy_vector);
  R3EndLine();
  R3BeginLine();
  RNLoadRgb(0, 0, 1);
  R3LoadPoint(R3zero_point + 0.5*d * R3negz_vector);
  R3LoadPoint(R3zero_point + d * R3posz_vector);
  R3EndLine();
  glLineWidth(1);
}



static void
DrawImageInset(ImageData *data,
  double x, double y, double scale)
{
  // Check if drawing image insets
  if (!show_image_insets) return;

  // Get convenient variables
  double w = scale * data->image_width;
  double h = scale * data->image_height;
  if ((w == 0) || (h == 0)) return;

  // Load texture
  LoadTexture(data);
  if (data->texture_id == 0) return;

  // Set viewing transformation
  glMatrixMode(GL_PROJECTION);  
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, GLUTwindow_width, 0, GLUTwindow_height, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Enable texture
  glBindTexture(GL_TEXTURE_2D, data->texture_id);
  glEnable(GL_TEXTURE_2D);

  // Draw textured polygon
  RNLoadRgb(1, 1, 1);
  RNGrfxBegin(RN_GRFX_POLYGON);
  R3LoadTextureCoords(0, 0);
  R3LoadPoint(x, y, -0.5);
  R3LoadTextureCoords(1, 0);
  R3LoadPoint(x+w, y, -0.5);
  R3LoadTextureCoords(1, 1);
  R3LoadPoint(x+w, y+h, -0.5);
  R3LoadTextureCoords(0, 1);
  R3LoadPoint(x, y+h, -0.5);
  RNGrfxEnd();

  // Disable texture
  glDisable(GL_TEXTURE_2D);

  // Restore viewing transformation
  glMatrixMode(GL_PROJECTION);  
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);  
  glPopMatrix();
}



static void
DrawDotOnImageInset(ImageData *data,
  double x, double y, double scale,
  const R2Point& image_position)
{
  // Check if drawing image insets
  if (!show_image_insets) return;

  // Get convenient variables
  double w = scale * data->image_width;
  double h = scale * data->image_height;
  if ((w == 0) || (h == 0)) return;

  // Set viewport
  glViewport(x, y, w, h);

  // Set viewing transformation
  glMatrixMode(GL_PROJECTION);  
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, data->image_width, 0, data->image_height, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Draw dot
  RNLoadRgb(1, 0, 0);
  glPointSize(5);
  RNGrfxBegin(RN_GRFX_POINTS);
  R3LoadPoint(image_position.X(), image_position.Y(), -0.25);
  RNGrfxEnd();
  glPointSize(1);

  // Restore viewing transformation
  glMatrixMode(GL_PROJECTION);  
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);  
  glPopMatrix();

  // Resore viewport
  viewer->Viewport().Load();
}



static void
DrawImageBillboard(ImageData *data, const R3Point& viewpoint,
  const R3Vector& towards, const R3Vector& right, const R3Vector& up,
  RNAngle xfov, RNAngle yfov, RNLength image_billboard_depth)
{
  // Check if drawing image billboards
  if (!show_image_billboards) return;

  // Load texture
  LoadTexture(data);
  if (data->texture_id == 0) return;

  // Enable texture
  glBindTexture(GL_TEXTURE_2D, data->texture_id);
  glEnable(GL_TEXTURE_2D);

  // Draw textured polygon
  R3Point origin = viewpoint + towards * image_billboard_depth;
  R3Vector dx = right * image_billboard_depth * tan(xfov);
  R3Vector dy = up * image_billboard_depth * tan(yfov);
  R3Point ur = origin + dx + dy;
  R3Point lr = origin + dx - dy;
  R3Point ul = origin - dx + dy;
  R3Point ll = origin - dx - dy;
  RNLoadRgb(1, 1, 1);
  RNGrfxBegin(RN_GRFX_POLYGON);
  R3LoadTextureCoords(0,0);
  R3LoadPoint(ll);
  R3LoadTextureCoords(1,0);
  R3LoadPoint(lr);
  R3LoadTextureCoords(1, 1);
  R3LoadPoint(ur);
  R3LoadTextureCoords(0,1);
  R3LoadPoint(ul);
  RNGrfxEnd();
  
  // Disable texture
  glDisable(GL_TEXTURE_2D);
}



static void
DrawDotOnImageBillboard(ImageData *data, const R3Point& viewpoint,
  const R3Vector& towards, const R3Vector& right, const R3Vector& up,
  RNAngle xfov, RNAngle yfov, RNLength image_billboard_depth,
  const R2Point& image_position, RNLength radius)
{
  // Check if drawing image billboards
  if (!show_image_billboards) return;

  // Get convenient variables
  int w = data->image_width;
  int h = data->image_height;
  if ((w == 0) || (h == 0)) return;
  
  // Check 2D image_position
  if (image_position.X() < 0) return;
  if (image_position.Y() < 0) return;
  if (image_position.X() >= w) return;
  if (image_position.Y() >= h) return;

  // Compute 3D position
  RNScalar dx = 2.0 * (image_position.X() - 0.5 * w) / w;
  RNScalar dy = 2.0 * (image_position.Y() - 0.5 * h) / h;
  R3Point world_position = viewpoint + image_billboard_depth * towards;
  world_position += image_billboard_depth * dx * right * tan(xfov);
  world_position += image_billboard_depth * dy * up * tan(yfov);

  // Draw sphere
  RNLoadRgb(1, 0, 0);
  R3Sphere(world_position, radius).Draw();
}



static void
DrawImageDepths(ImageData *data, const R3Point& viewpoint,
  const R3Vector& towards, const R3Vector& right, const R3Vector& up,
  RNAngle xfov, RNAngle yfov, RNLength image_billboard_depth)
{
  // Check if drawing image depths
  if (!show_image_depths) return;

  // Check image data
  int w = data->image_width;
  int h = data->image_height;
  if (w == 0) return;
  if (h == 0) return;
  if (data->depth_image.XResolution() != w) return;
  if (data->depth_image.YResolution() != h) return;
  
  // Load texture
  LoadTexture(data);
  if (data->texture_id == 0) return;

  // Enable texture
  glBindTexture(GL_TEXTURE_2D, data->texture_id);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  RNLoadRgb(1, 1, 1);

  // Check if have ray images
  if ((data->ray_origin_images[0].XResolution() == w) &&
      (data->ray_origin_images[1].XResolution() == w) &&
      (data->ray_origin_images[2].XResolution() == w) && 
      (data->ray_origin_images[0].YResolution() == h) &&
      (data->ray_origin_images[1].YResolution() == h) &&
      (data->ray_origin_images[2].YResolution() == h) && 
      (data->ray_direction_images[0].XResolution() == w) &&
      (data->ray_direction_images[1].XResolution() == w) &&
      (data->ray_direction_images[2].XResolution() == w) && 
      (data->ray_direction_images[0].YResolution() == h) &&
      (data->ray_direction_images[1].YResolution() == h) &&
      (data->ray_direction_images[2].YResolution() == h)) {
    RNGrfxBegin(RN_GRFX_POINTS);
    for (int ix = 0; ix < w; ix++) {
      for (int iy = 0; iy < h; iy++) {
        // Get/check depth
        double depth = data->depth_image.GridValue(ix, iy);
        if ((depth <= 0) || (depth == R2_GRID_UNKNOWN_VALUE)) continue;

        // Get/check ray direction
        double dx = data->ray_direction_images[0].GridValue(ix, iy);    
        double dy = data->ray_direction_images[1].GridValue(ix, iy);    
        double dz = data->ray_direction_images[2].GridValue(ix, iy);
        R3Vector direction(dx, dy, dz);
        if (direction.IsZero()) continue;

        // Get ray origin
        double ox = data->ray_origin_images[0].GridValue(ix, iy);    
        double oy = data->ray_origin_images[1].GridValue(ix, iy);    
        double oz = data->ray_origin_images[2].GridValue(ix, iy);    
        R3Point origin(ox, oy, oz);

        // Get point position
        RNScalar t = depth * direction.Dot(towards);
        R3Point p = origin + t * direction;

        // Get texture coordinates
        double u = (double) ix / (double) w;
        double v = (double) iy / (double) h;

        // Draw point
        R3LoadTextureCoords(u, v);
        R3LoadPoint(p);
      }
    }
    RNGrfxEnd();
  }
  else {
    // Compute view plane rectangle in world coordinates
    R3Vector dx = 2.0 * right * image_billboard_depth * tan(xfov);
    R3Vector dy = 2.0 * up * image_billboard_depth * tan(yfov);
    R3Point origin = viewpoint + towards * image_billboard_depth;
    R3Point lower_left = origin - 0.5*dx - 0.5*dy;

    // Backproject pixels from the view plane rectangle
    RNGrfxBegin(RN_GRFX_POINTS);
    for (int ix = 0; ix < w; ix++) {
      for (int iy = 0; iy < h; iy++) {
        double depth = data->depth_image.GridValue(ix, iy);
        if ((depth <= 0) || (depth == R2_GRID_UNKNOWN_VALUE)) continue;
        double u = (double) ix / (double) w;
        double v = (double) iy / (double) h;
        R3Point pA = lower_left + u*dx + v*dy;
        R3Vector vA = pA - viewpoint;
        double dA = vA.Dot(towards);
        if (dA <= 0) continue;
        R3Point p = viewpoint + vA * depth / dA;
        R3LoadTextureCoords(u, v);
        R3LoadPoint(p);
      }
    }
    RNGrfxEnd();
  }
  
  // Disable texture
  glDisable(GL_TEXTURE_2D);
}



static void
DrawImageRay(ImageData *data,
  const R2Point& image_position)
{               
  // Check image position
  R2Box b = data->ray_origin_images[0].GridBox();
  if (!R2Contains(b, image_position)) return;

  // Get ray parameter from images
  RNScalar ox = data->ray_origin_images[0].GridValue(image_position);
  RNScalar oy = data->ray_origin_images[1].GridValue(image_position);
  RNScalar oz = data->ray_origin_images[2].GridValue(image_position);
  RNScalar dx = data->ray_direction_images[0].GridValue(image_position);
  RNScalar dy = data->ray_direction_images[1].GridValue(image_position);
  RNScalar dz = data->ray_direction_images[2].GridValue(image_position);

  // Create ray
  R3Point ray_origin(ox, oy, oz);
  R3Vector ray_direction(dx, dy, dz);
  R3Ray ray(ray_origin, ray_direction);

  // Find projection of selected position onto ray
  RNScalar t = ray.T(selected_position);
  if (t <= 0) return;
  R3Point p = ray.Point(t);

  // Draw ray
  glDisable(GL_LIGHTING);
  RNLoadRgb(0, 1, 1);
  R3Span(ray_origin, p).Draw();
}



////////////////////////////////////////////////////////////////////////
// Data drawing functions
////////////////////////////////////////////////////////////////////////

static void
DrawPointSet(R3PointSet *pointset,
  int model_index, int color_scheme)
{
  // Check if should draw point set
  if (!show_pointsets) return;

  // Draw points
  if (show_points) {
    glDisable(GL_LIGHTING);    
    RNLoadRgb(0.0, 0.0, 1.0);
    RNGrfxBegin(RN_GRFX_POINTS);
    for (int i = 0; i < pointset->NPoints(); i++) {
      const R3Point& position = pointset->PointPosition(i);
      RNScalar value = pointset->PointValue(i);
      if (color_scheme == PICK_COLOR_SCHEME)
        LoadColorIndex(model_index, POINTSET_POINT_ELEMENT_TYPE, i);
      else if (color_scheme == RGB_COLOR_SCHEME)
        RNLoadRgb(-10*value, 0.0, 10*value);
      R3LoadPoint(position);
    }
    RNGrfxEnd();
    glPointSize(1);
  }
  
  // Draw normals
  if (show_normals) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(0.0, 1.0, 0.0);
    RNGrfxBegin(RN_GRFX_LINES);
    RNScalar d = 0.0025 * world_bbox.DiagonalRadius();
    for (int i = 0; i < pointset->NPoints(); i++) {
      const R3Point& position = pointset->PointPosition(i);
      const R3Vector& normal = pointset->PointNormal(i);
      R3LoadPoint(position);
      R3LoadPoint(position + d * normal);
    }
    RNGrfxEnd();
  }
}



static void
DrawMesh(R3Mesh *mesh,
  int model_index, int color_scheme)
{
  // Check if should draw mesh
  if (!show_meshes) return;

  // Draw faces
  if (show_surfaces) {
    if (color_scheme == SHADING_COLOR_SCHEME) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);
    RNLoadRgb(0.8, 0.8, 0.8);
    RNGrfxBegin(RN_GRFX_TRIANGLES);
    for (int i = 0; i < mesh->NFaces(); i++) {
      R3MeshFace *face = mesh->Face(i);
      if (color_scheme == PICK_COLOR_SCHEME)
        LoadColorIndex(model_index, MESH_FACE_ELEMENT_TYPE, i);
      else if (color_scheme == CATEGORY_COLOR_SCHEME)
        LoadColorIdentifier(1 + mesh->FaceCategory(face));
      else if (color_scheme == INSTANCE_COLOR_SCHEME)
        LoadColorIdentifier(1 + mesh->FaceSegment(face)); 
      if (color_scheme == SHADING_COLOR_SCHEME)
        R3LoadNormal(mesh->FaceNormal(face));
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh->VertexOnFace(face, j);
        if (color_scheme == RGB_COLOR_SCHEME)
          R3LoadRgb(mesh->VertexColor(vertex));
        R3LoadPoint(mesh->VertexPosition(vertex));
      }
    }
    RNGrfxEnd();
  }

  // Draw edges
  if (show_edges) {
    if (color_scheme != PICK_COLOR_SCHEME) {
      glDisable(GL_LIGHTING);
      RNLoadRgb(1.0, 0.0, 0.0);
      mesh->DrawEdges();
    }
  }

  // Draw vertices
  if (show_points) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(0.0, 0.0, 1.0);
    RNGrfxBegin(RN_GRFX_POINTS);
    for (int i = 0; i < mesh->NVertices(); i++) {
      R3MeshVertex *vertex = mesh->Vertex(i);
      if (color_scheme == PICK_COLOR_SCHEME)
        LoadColorIndex(model_index, MESH_VERTEX_ELEMENT_TYPE, i);
      else if (color_scheme == RGB_COLOR_SCHEME)
        R3LoadRgb(mesh->VertexColor(vertex));
      R3LoadPoint(mesh->VertexPosition(vertex));
    }
    RNGrfxEnd();
  }
  
  // Draw normals
  if (show_normals) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(0.0, 1.0, 0.0); 
    RNGrfxBegin(RN_GRFX_LINES);
    RNScalar d = 0.0025 * world_bbox.DiagonalRadius();
    for (int i = 0; i < mesh->NVertices(); i++) {
      R3MeshVertex *vertex = mesh->Vertex(i);
      const R3Point& position = mesh->VertexPosition(vertex);
      const R3Vector& normal = mesh->VertexNormal(vertex);
      R3LoadPoint(position);
      R3LoadPoint(position + d * normal);
    }
    RNGrfxEnd();
  }
}



static void
DrawScene(R3Scene *scene,
  int model_index, int color_scheme)
{
  // Check if should draw scene
  if (!show_scenes) return;

  // Draw faces
  if (show_surfaces) {
    glEnable(GL_LIGHTING);
    scene->LoadLights(2);
    scene->Draw();
  }

  // Draw edges
  if (show_edges) {
    if (color_scheme != PICK_COLOR_SCHEME) {
      glDisable(GL_LIGHTING);
      RNLoadRgb(0.0, 1.0, 0.0);
      scene->Draw(R3_EDGES_DRAW_FLAG);
    }
  }
}



static void
DrawGrid(R3Grid *grid,
  int model_index, int color_scheme)
{
  // Check if should draw grid
  if (!show_grids) return;

  // Push transformation
  grid->GridToWorldTransformation().Push();

  // Draw grid isosurface faces
  if (show_surfaces) {
    glEnable(GL_LIGHTING);
    RNLoadRgb(0.8, 0.8, 0.8);
    grid->DrawIsoSurface(grid_thresholds[model_index]);
  }

  // Draw grid isosurface edges
  if (show_edges) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(1.0, 0.0, 0.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    grid->DrawIsoSurface(grid_thresholds[model_index]);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Draw grid slices
  if (show_slices) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(1.0, 1.0, 1.0);
    grid->DrawSlice(RN_X, grid_slice_coords[RN_X]);
    grid->DrawSlice(RN_Y, grid_slice_coords[RN_Y]);
    grid->DrawSlice(RN_Z, grid_slice_coords[RN_Z]);
  }

  // Draw grid text
  if (show_text_labels) {
    char buffer[64];
    glDisable(GL_LIGHTING);
    RNLoadRgb(0.0, 0.0, 1.0);
    for (int i = 0; i < grid->XResolution(); i += 10) {
      for (int j = 0; j < grid->YResolution(); j += 10) {
        for (int k = 0; k < grid->ZResolution(); k += 10) {
          RNScalar value = grid->GridValue(i, j, k);
          if (value >= grid_thresholds[model_index]) continue;
          sprintf(buffer, "%.2g", value);
          R3DrawText(i, j, k, buffer);
        }
      }
    }
  }

  // Draw grid threshold
  if (show_grid_threshold) {
    char buffer[128];
    sprintf(buffer, "%f", grid_thresholds[selected_model_index]);
    viewer->DrawText(10, 10, buffer);
  }

  // Pop transformation
  grid->GridToWorldTransformation().Pop();
}



static void
DrawSFL(R3SurfelScene *scene,
  int model_index, int color_scheme)
{
  // Check if should draw sfl scene
  if (!show_sfls) return;

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return;

  // Draw points
  if (show_points) {
    glDisable(GL_LIGHTING);
    RNGrfxBegin(RN_GRFX_POINTS);
    int sfl_count = 0;
    for (int i = 0; i < tree->NNodes(); i++) {
      R3SurfelNode *node = tree->Node(i);
      if (node->NParts() > 0) continue;
      R3SurfelObject *object = node->Object(TRUE);
      while (object && object->Parent() && (object->Parent() != scene->RootObject()))
        object = object->Parent();
      int instance_identifier = (object) ? object->SceneIndex()  : -1;
      R3SurfelLabel *label = (object) ? object->CurrentLabel() : NULL;
      int category_identifier = (label) ? label->Identifier() : -1;
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        for (int k = 0; k < block->NSurfels(); k++) {
          if (color_scheme == PICK_COLOR_SCHEME)
            LoadColorIndex(model_index, SFL_POINT_ELEMENT_TYPE, sfl_count);
          else if (color_scheme == RGB_COLOR_SCHEME)
            RNLoadRgb(block->SurfelColor(k));
          else if (color_scheme == CATEGORY_COLOR_SCHEME)
            LoadColorIdentifier(category_identifier);
          else if (color_scheme == INSTANCE_COLOR_SCHEME)
            LoadColorIdentifier(instance_identifier);
          else LoadColorValue(block->SurfelElevation(k) / 2.0);
          R3LoadPoint(block->SurfelPosition(k));
          sfl_count++;
        }
      }
    }
    RNGrfxEnd();
  }

  // Draw normals
  if (show_normals && (color_scheme != PICK_COLOR_SCHEME)) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(0.0, 1.0, 0.0); 
    RNGrfxBegin(RN_GRFX_LINES);
    RNScalar d = 0.0025 * world_bbox.DiagonalRadius();
    for (int i = 0; i < tree->NNodes(); i++) {
      R3SurfelNode *node = tree->Node(i);
      if (node->NParts() > 0) continue;
      for (int j = 0; j < node->NBlocks(); j++) {
        R3SurfelBlock *block = node->Block(j);
        for (int k = 0; k < block->NSurfels(); k++) {
          R3Point position = block->SurfelPosition(k);
          R3Vector normal = block->SurfelNormal(k);
          R3LoadPoint(position);
          R3LoadPoint(position + d * normal);
        }
      }
    }
    RNGrfxEnd();
  }

  // Draw cameras
  if (show_cameras) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(0, 1, 0);
    for (int i = 0; i < scene->NImages(); i++) {
      R3SurfelImage *image = scene->Image(i);
      if (color_scheme == PICK_COLOR_SCHEME)
        LoadColorIndex(model_index, SFL_CAMERA_ELEMENT_TYPE, i);
      R3Sphere(image->Viewpoint(), 0.1).Draw();
      R3Span(image->Viewpoint(), image->Viewpoint() + image->Towards()).Draw();
      R3Span(image->Viewpoint(), image->Viewpoint() + 0.5 * image->Up()).Draw();
      R3Span(image->Viewpoint(), image->Viewpoint() + 0.25 * image->Right()).Draw();
    }
  }

  // Draw images
  if (show_image_insets || show_image_billboards || show_image_depths || show_image_rays) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(1, 1, 1);
    int image_count = 0;
    for (int i = 0; i < scene->NImages(); i++) {
      R3SurfelImage *image = scene->Image(i);
      ImageData *data = (ImageData *) image->Data();
      if (!data) continue;

      // Set color
      if (color_scheme == PICK_COLOR_SCHEME)
        LoadColorIndex(model_index, SFL_CAMERA_ELEMENT_TYPE, i);

      // Compute projection of selected 3D position
      R2Point image_position = image->TransformFromWorldToImage(selected_position);
      if (image_position.X() < 0) continue;
      if (image_position.Y() < 0) continue;
      if (image_position.X() >= image->ImageWidth()) continue;
      if (image_position.Y() >= image->ImageHeight()) continue;

      // Draw image inset
      if (show_image_insets) {
        double x = image_count * image_inset_scale * data->image_width;
        DrawImageInset(data, x, 0, image_inset_scale);
        DrawDotOnImageInset(data, x, 0, image_inset_scale, image_position);
      }
      
      // Draw image billboard
      if (show_image_billboards) {
        DrawImageBillboard(data, image->Viewpoint(),
          image->Towards(), image->Right(), image->Up(),
          image->XFOV(), image->YFOV(), image_billboard_depth);
        DrawDotOnImageBillboard(data, image->Viewpoint(),
          image->Towards(), image->Right(), image->Up(),
          image->XFOV(), image->YFOV(), image_billboard_depth,
          image_position, 0.025);
      }

      // Draw image depths
      if (show_image_depths) {
        DrawImageDepths(data, image->Viewpoint(),
          image->Towards(), image->Right(), image->Up(),
          image->XFOV(), image->YFOV(), image_billboard_depth);
      }

      // Draw image ray
      if (show_image_rays) {
        DrawImageRay(data, image_position);
      }

      // Increment image counter
      image_count++;
    }
  }
}



static void
DrawGSV(GSVScene *scene,
  int model_index, int color_scheme)
{
  // Check if should draw gsv scene
  if (!show_gsvs) return;

  // Draw points
  if (show_points) {
    glDisable(GL_LIGHTING);
    RNGrfxBegin(RN_GRFX_POINTS);
    int point_count = 0;
    for (int ir = 0; ir < scene->NRuns(); ir++) {
      GSVRun *run = scene->Run(ir);
      for (int is = 0; is < run->NSegments(); is++) {
        GSVSegment *segment = run->Segment(is);
        for (int iu = 0; iu < segment->NSurveys(); iu++) {
          GSVSurvey *survey = segment->Survey(iu);
          for (int ia = 0; ia < survey->NScans(); ia++) {
            GSVScan *scan = survey->Scan(ia);
            for (int ie = 0; ie < scan->NScanlines(); ie++) {
              GSVScanline *scanline = scan->Scanline(ie);
              for (int ik = 0; ik < scanline->NPoints(); ik++) {
                const GSVPoint& point = scanline->Point(ik);
                if (color_scheme == PICK_COLOR_SCHEME)
                  LoadColorIndex(model_index, GSV_POINT_ELEMENT_TYPE, point_count);
                else if (color_scheme == RGB_COLOR_SCHEME)
                  RNLoadRgb(point.Color());
                else if (color_scheme == CATEGORY_COLOR_SCHEME)
                  LoadColorIdentifier(point.CategoryIdentifier());
                else if (color_scheme == INSTANCE_COLOR_SCHEME)
                  LoadColorIdentifier(point.ClusterIdentifier());
                else
                  LoadColorValue(point.Elevation() / 2.0);
                R3LoadPoint(point.Position());
                point_count++;
              }
            }
          }
        }
      }
    }
    RNGrfxEnd();
  }

  // Draw normals
  if (show_normals && (color_scheme != PICK_COLOR_SCHEME)) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(0.0, 1.0, 0.0); 
    RNGrfxBegin(RN_GRFX_LINES);
    RNScalar d = 0.0025 * world_bbox.DiagonalRadius();
    for (int ir = 0; ir < scene->NRuns(); ir++) {
      GSVRun *run = scene->Run(ir);
      for (int is = 0; is < run->NSegments(); is++) {
        GSVSegment *segment = run->Segment(is);
        for (int iu = 0; iu < segment->NSurveys(); iu++) {
          GSVSurvey *survey = segment->Survey(iu);
          for (int ia = 0; ia < survey->NScans(); ia++) {
            GSVScan *scan = survey->Scan(ia);
            for (int ie = 0; ie < scan->NScanlines(); ie++) {
              GSVScanline *scanline = scan->Scanline(ie);
              for (int ik = 0; ik < scanline->NPoints(); ik++) {
                const GSVPoint& point = scanline->Point(ik);
                R3Point position = point.Position();
                R3Vector normal = point.Normal();
                R3LoadPoint(position);
                R3LoadPoint(position + d * normal);
              }
            }
          }
        }
      }
    }
    RNGrfxEnd();
  }

  // Draw cameras
  if (show_cameras) {
    glDisable(GL_LIGHTING);
    RNLoadRgb(0, 1, 0);
    int image_count = 0;
    for (int ir = 0; ir < scene->NRuns(); ir++) {
      GSVRun *run = scene->Run(ir);
      for (int is = 0; is < run->NSegments(); is++) {
        GSVSegment *segment = run->Segment(is);
        for (int ip = 0; ip < segment->NPanoramas(); ip++) {
          GSVPanorama *panorama = segment->Panorama(ip);
          for (int ii = 0; ii < panorama->NImages(); ii++) {
            GSVImage *image = panorama->Image(ii);
            GSVPose pose = image->Pose();
            if (color_scheme == PICK_COLOR_SCHEME)
              LoadColorIndex(model_index, GSV_CAMERA_ELEMENT_TYPE, image_count);
            R3Sphere(pose.Viewpoint(), 0.1).Draw();
            R3Span(pose.Viewpoint(), pose.Viewpoint() + pose.Towards()).Draw();
            R3Span(pose.Viewpoint(), pose.Viewpoint() + 0.5 * pose.Up()).Draw();
            R3Span(pose.Viewpoint(), pose.Viewpoint() + 0.25 * pose.Right()).Draw();
            image_count++;
          }
        }
      }
    }
  }

  // Draw images
  if (show_image_insets || show_image_billboards || show_image_depths || show_image_rays) {
    glDisable(GL_LIGHTING);
    int image_count = 0;
    for (int ir = 0; ir < scene->NRuns(); ir++) {
      GSVRun *run = scene->Run(ir);
      for (int is = 0; is < run->NSegments(); is++) {
        GSVSegment *segment = run->Segment(is);
        for (int ip = 0; ip < segment->NPanoramas(); ip++) {
          GSVPanorama *panorama = segment->Panorama(ip);
          for (int ii = 0; ii < panorama->NImages(); ii++) {
            GSVImage *image = panorama->Image(ii);
            ImageData *data = (ImageData *) image->Data();
            if (!data) continue;

            // Compute projection of selected 3D position
            R2Point image_position = image->DistortedPosition(selected_position);
            if (image_position.X() < 0) continue;
            if (image_position.Y() < 0) continue;
            if (image_position.X() >= image->Width()) continue;
            if (image_position.Y() >= image->Height()) continue;

            // Get pose info
            GSVPose pose = image->Pose();
            R3Point viewpoint = pose.Viewpoint();
            R3Vector towards = pose.Towards();
            R3Vector right = pose.Right();
            R3Vector up = pose.Up();

            // Draw image inset
            if (show_image_insets) {
              double x = image_count * image_inset_scale * data->image_width;
              DrawImageInset(data, x, 0, image_inset_scale);
              DrawDotOnImageInset(data, x, 0, image_inset_scale, image_position);
            }
      
            // Draw image billboard
            if (show_image_billboards) {
              DrawImageBillboard(data, pose.Viewpoint(),
                pose.Towards(), pose.Right(), pose.Up(),
                0.5 * image->XFov(), 0.5 * image->YFov(),
                image_billboard_depth);
              DrawDotOnImageBillboard(data, viewpoint, towards, right, up,
                0.5 * image->XFov(), 0.5 * image->YFov(), image_billboard_depth,
                image_position, 0.025);
            }

            // Draw image depths
            if (show_image_depths) {
              DrawImageDepths(data, pose.Viewpoint(),
                pose.Towards(), pose.Right(), pose.Up(),
                0.5 * image->XFov(), 0.5 * image->YFov(),
                image_billboard_depth);
            }

            // Draw image ray
            if (show_image_rays) {
              DrawImageRay(data, image_position);
            }

            // Increment image counter
            image_count++;
          }
        }
      }
    }
  }
}



static void
DrawData(int color_scheme)
{
  // Draw pointsets
  for (int m = 0; m < pointsets.NEntries(); m++) {
    if ((selected_model_index >= 0) && (m != selected_model_index)) continue;
    R3PointSet *pointset = pointsets[m];
    if (!pointset) continue;
    DrawPointSet(pointset, m, color_scheme);
  }
  
  // Draw scenes
  for (int m = 0; m < scenes.NEntries(); m++) {
    if ((selected_model_index >= 0) && (m != selected_model_index)) continue;
    R3Scene *scene = scenes[m];
    if (!scene) continue;
    DrawScene(scene, m, color_scheme);
  }
  
  // Draw meshes
  for (int m = 0; m < meshes.NEntries(); m++) {
    if ((selected_model_index >= 0) && (m != selected_model_index)) continue;
    R3Mesh *mesh = meshes[m];
    if (!mesh) continue;
    DrawMesh(mesh, m, color_scheme);
  }
  
  // Draw grids
  for (int m = 0; m < grids.NEntries(); m++) {
    if ((selected_model_index >= 0) && (m != selected_model_index)) continue;
    R3Grid *grid = grids[m];
    if (!grid) continue;
    DrawGrid(grid, m, color_scheme);
  }
  
  // Draw SFL scenes
  for (int m = 0; m < sfls.NEntries(); m++) {
    if ((selected_model_index >= 0) && (m != selected_model_index)) continue;
    R3SurfelScene *sfl = sfls[m];
    if (!sfl) continue;
    DrawSFL(sfl, m, color_scheme);
  }

  // Draw GSV scenes
  for (int m = 0; m < gsvs.NEntries(); m++) {
    if ((selected_model_index >= 0) && (m != selected_model_index)) continue;
    GSVScene *gsv = gsvs[m];
    if (!gsv) continue;
    DrawGSV(gsv, m, color_scheme);
  }
}



////////////////////////////////////////////////////////////////////////
// Pick function
////////////////////////////////////////////////////////////////////////

static int
Pick(double x, double y, R3Point *hit_position = NULL,
  int *hit_model_index = NULL, int *hit_element_type = NULL, int *hit_element_index = NULL)
{
  // Initialize result
  if (hit_position) *hit_position = R3unknown_point;
  if (hit_model_index) *hit_model_index = -1;
  if (hit_element_type) *hit_element_type = -1;
  if (hit_element_index) *hit_element_index = -1;

  // Clear window 
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set viewing transformation
  viewer->Camera().Load();

  // Draw everything
  int pick_tolerance = 10;
  glDisable(GL_LIGHTING);
  glDisable(GL_MULTISAMPLE);
  glPointSize(2 * pick_tolerance);    
  glLineWidth(2 * pick_tolerance);    
  DrawData(PICK_COLOR_SCHEME);
  glEnable(GL_MULTISAMPLE);
  glPointSize(1);
  glLineWidth(1);
  glFinish();

  // Read color buffer at cursor position
  unsigned char rgba[4];
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  if (rgba[3] == 0) return 0;
  int r = rgba[0] & 0xFF;
  int g = rgba[1] & 0xFF;
  int b = rgba[2] & 0xFF;
  int a = rgba[3] & 0xFF;
  if (a == 0) return 0;
  if (a == 0xFF) return 0;

  // Get model index
  int model_index = (a >> 4) & 0x0F;
  if (model_index == 0) return 0;
  model_index -= 1;

  // Get element type
  int element_type = a & 0x0F;
  if (element_type == 0) return 0;
  element_type -= 1;

  // Get element index
  int element_index = (r << 16) | (g << 8) | b;
  if (element_index < 1) return 0;
  element_index -= 1;

  // Return hit info
  if (hit_model_index) *hit_model_index = model_index;
  if (hit_element_type) *hit_element_type = element_type;
  if (hit_element_index) *hit_element_index = element_index;

  // Return hit position
  if (hit_position) {
    // Find hit position
    GLfloat depth;
    GLdouble p[3];
    GLint viewport[4];
    GLdouble modelview_matrix[16];
    GLdouble projection_matrix[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    gluUnProject(x, y, depth, modelview_matrix, projection_matrix, viewport, &(p[0]), &(p[1]), &(p[2]));
    R3Point position(p[0], p[1], p[2]);
    *hit_position = position;
  }
  
  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// GLUT interface functions
////////////////////////////////////////////////////////////////////////

static void
GLUTStop(void)
{
  // Close sfl scenes
  for (int i = 0; i < sfls.NEntries(); i++) {
    if (sfls[i]) CloseSFLFiles(sfls[i]);
  }
  
  // Destroy window 
  glutDestroyWindow(GLUTwindow);

  // Exit
  exit(0);
}



static void
GLUTRedraw(void)
{
  // Clear window 
  glClearColor(background_color[0], background_color[1], background_color[2], 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set viewing transformation
  viewer->Camera().Load();

  // Set lights
  static GLfloat light0_position[] = { 3.0, 4.0, 5.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  static GLfloat light1_position[] = { -3.0, -2.0, -3.0, 0.0 };
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

  // Draw everything
  DrawData(color_scheme);
  DrawSelectedPosition();
  DrawBBox();
  DrawAxes();

  // Swap buffers 
  glutSwapBuffers();
}    



static void
GLUTResize(int w, int h)
{
  // Resize window
  glViewport(0, 0, w, h);

  // Resize viewer viewport
  viewer->ResizeViewport(0, 0, w, h);

  // Remember window size 
  GLUTwindow_width = w;
  GLUTwindow_height = h;

  // Redraw
  glutPostRedisplay();
}



static void
GLUTMotion(int x, int y)
{
  // Invert y coordinate
  y = GLUTwindow_height - y;

  // Compute mouse movement
  int dx = x - GLUTmouse[0];
  int dy = y - GLUTmouse[1];
  
  // World in hand navigation 
  if (GLUTbutton[0]) viewer->RotateWorld(1.0, world_origin, x, y, dx, dy);
  else if (GLUTbutton[1]) viewer->ScaleWorld(1.0, world_origin, x, y, dx, dy);
  else if (GLUTbutton[2]) viewer->TranslateWorld(1.0, world_origin, x, y, dx, dy);
  if (GLUTbutton[0] || GLUTbutton[1] || GLUTbutton[2]) glutPostRedisplay();

  // Remember mouse position 
  GLUTmouse[0] = x;
  GLUTmouse[1] = y;

  // Update mouse drag movement
  GLUTmouse_drag_distance_squared += dx*dx + dy*dy;
}



static void
GLUTMouse(int button, int state, int x, int y)
{
  // Invert y coordinate
  y = GLUTwindow_height - y;
  
  // Process mouse button event
  if (state == GLUT_DOWN) {
    // Button is going down
    GLUTmouse_drag_distance_squared = 0;

    // Process thumbwheel
    if ((button == 3) || (button == 4)) {
      R3Point viewing_center_point = world_origin;
      const R3Camera& camera = viewer->Camera();
      R3Plane camera_plane(camera.Origin(), camera.Towards());
      RNScalar signed_distance = R3SignedDistance(camera_plane, viewing_center_point);
      if (signed_distance < 0) viewing_center_point -= (signed_distance - 1) * camera.Towards();
      if (button == 3) viewer->ScaleWorld(viewing_center_point, 1.1);
      else if (button == 4) viewer->ScaleWorld(viewing_center_point, 0.9);
    }
  }
  else { // Button is going up
    // Check for drag
    RNBoolean drag = (GLUTmouse_drag_distance_squared > 10 * 10);
    GLUTmouse_drag_distance_squared = 0;

    // Process button 
    if (button == GLUT_LEFT) {
      // Check for double click  
      // static RNBoolean double_click = FALSE;
      // static RNTime last_mouse_down_time;
      // double_click = (!double_click) && (last_mouse_down_time.Elapsed() < 0.4);
      // last_mouse_down_time.Read();

      // Select position
      if (!drag) {
        int e;
        R3Point p;
        if (Pick(x, y, &p, NULL, &e)) {
          world_origin = p;
          if ((e != GSV_CAMERA_ELEMENT_TYPE) && (e != SFL_CAMERA_ELEMENT_TYPE)) {
            selected_position = p;
          }
        }
      }
    }
  }
  
  // Remember button state 
  int b = (button == GLUT_LEFT_BUTTON) ? 0 : ((button == GLUT_MIDDLE_BUTTON) ? 1 : 2);
  GLUTbutton[b] = (state == GLUT_DOWN) ? 1 : 0;

  // Remember modifiers 
  GLUTmodifiers = glutGetModifiers();

  // Remember mouse position 
  GLUTmouse[0] = x;
  GLUTmouse[1] = y;

  // Redraw
  glutPostRedisplay();
}



static void
GLUTSpecial(int key, int x, int y)
{
  // Invert y coordinate
  y = GLUTwindow_height - y;

  // Process keyboard button event 
  switch (key) {
  case GLUT_KEY_DOWN:
    grid_thresholds[selected_model_index] -= 0.025 * grid_ranges[selected_model_index].Diameter();
    if (grid_thresholds[selected_model_index] < grid_ranges[selected_model_index].Min() + 1E-20) 
      grid_thresholds[selected_model_index] = grid_ranges[selected_model_index].Min() + 1E-20;
    break;

  case GLUT_KEY_UP:
    grid_thresholds[selected_model_index] += 0.025 * grid_ranges[selected_model_index].Diameter();
    if (grid_thresholds[selected_model_index] > grid_ranges[selected_model_index].Max() - 1E-20)
      grid_thresholds[selected_model_index] = grid_ranges[selected_model_index].Max() - 1E-20;
    break;
      
  case GLUT_KEY_PAGE_UP:
    if (++selected_model_index > nmodels-1) selected_model_index = nmodels - 1;
    break;

  case GLUT_KEY_PAGE_DOWN:
    if (--selected_model_index < 0) selected_model_index = 0;
    break;
  }

  // Remember mouse position 
  GLUTmouse[0] = x;
  GLUTmouse[1] = y;

  // Remember modifiers 
  GLUTmodifiers = glutGetModifiers();

  // Redraw
  glutPostRedisplay();
}



static void
GLUTKeyboard(unsigned char key, int x, int y)
{
  // Initialize redraw
  RNBoolean redraw = TRUE;

  // Determine modifiers
  int modifiers = glutGetModifiers();
  int ctrl = (modifiers & GLUT_ACTIVE_CTRL);
  int alt = (modifiers & GLUT_ACTIVE_ALT);

  // Process keyboard button event
  if (ctrl) {
    // Control which data sources are drawn
    switch(key) {
    case 7: // ctrl-G
      show_grids = !show_grids;
      break;

    case 12: // ctrl-L
      show_gsvs = !show_gsvs;
      break;

    case 13: // ctrl-M
      show_meshes = !show_meshes;
      break;

    case 15: // ctrl-O
      show_scenes = !show_scenes;
      break;

    case 16: // ctrl-P
      show_pointsets = !show_pointsets;
      break;

    case 19: // ctrl-S
      show_sfls = !show_sfls;
      break;

    default:
      redraw = FALSE;
      break;
    }
  }
  else if (alt) {
    // Control how things are drawn
    switch (key) {
    case 'C':
    case 'c':
      color_scheme = RGB_COLOR_SCHEME;
      break;

    case 'E':
    case 'e':
      color_scheme = NULL_COLOR_SCHEME;
      break;

    case 'I':
    case 'i':
      color_scheme = INSTANCE_COLOR_SCHEME;
      break;

    case 'L':
    case 'l':
      color_scheme = CATEGORY_COLOR_SCHEME;
      break;

    case 'S':
    case 's':
      color_scheme = SHADING_COLOR_SCHEME;
      break;

    case 'X':
      if (selected_model_index < grids.NEntries()) {
        grid_slice_coords[RN_X]++;
        if (grid_slice_coords[RN_X] >= grids[selected_model_index]->XResolution()) 
          grid_slice_coords[RN_X] = grids[selected_model_index]->XResolution() - 1;
      }
      break;

    case 'x':
      if (selected_model_index < grids.NEntries()) {
        grid_slice_coords[RN_X]--;
        if (grid_slice_coords[RN_X] < 0)
          grid_slice_coords[RN_X] = 0;
      }
      break;

    case 'Y':
      if (selected_model_index < grids.NEntries()) {
        grid_slice_coords[RN_Y]++;
        if (grid_slice_coords[RN_Y] >= grids[selected_model_index]->YResolution()) 
          grid_slice_coords[RN_Y] = grids[selected_model_index]->YResolution() - 1;
      }
      break;

    case 'y':
      if (selected_model_index < grids.NEntries()) {
        grid_slice_coords[RN_Y]--;
        if (grid_slice_coords[RN_Y] < 0)
          grid_slice_coords[RN_Y] = 0;
      }
      break;

    case 'Z':
      if (selected_model_index < grids.NEntries()) {
        grid_slice_coords[RN_Z]++;
        if (grid_slice_coords[RN_Z] >= grids[selected_model_index]->ZResolution()) 
          grid_slice_coords[RN_Z] = grids[selected_model_index]->ZResolution() - 1;
      }
      break;

    case 'z':
      if (selected_model_index < grids.NEntries()) {
        grid_slice_coords[RN_Z]--;
        if (grid_slice_coords[RN_Z] < 0)
          grid_slice_coords[RN_Z] = 0;
      }
      break;

    default:
      redraw = FALSE;
      break;
    }
  }
  else {
    // Control which data elements are drawn
    switch (key) {
    case '1':
    case '2':
    case '3':
    case '4':
      selected_model_index = key - '1';
      if (selected_model_index > nmodels - 1) {
        selected_model_index = nmodels - 1;
      }
      break;

    case 'A':
    case 'a':
      show_axes = !show_axes;
      break;

    case 'B':
    case 'b':
      show_bbox = !show_bbox;
      break;

    case 'C':
    case 'c':
      show_cameras = !show_cameras;
      break;

    case 'D':
    case 'd':
      show_image_depths = !show_image_depths;
      break;
      
    case 'E':
    case 'e':
      show_edges = !show_edges;
      break;

    case 'F':
    case 'f':
      show_surfaces = !show_surfaces;
      break;

    case 'G':
    case 'g':
      show_slices = !show_slices;
      break;

    case 'I':
      show_image_billboards = !show_image_billboards;
      break;

    case 'i':
      show_image_insets = !show_image_insets;
      break;

    case 'L':
    case 'l':
      show_text_labels = !show_text_labels;
      break;

    case 'N':
    case 'n':
      show_normals = !show_normals;
      break;

    case 'P':
    case 'p':
      show_points = !show_points;
      break;

    case 'R':
    case 'r':
      show_image_rays = !show_image_rays;
      break;

    case 'T':
    case 't':
      show_grid_threshold = !show_grid_threshold;
      break;

    case ' ': {
      // Print camera
      const R3Camera& camera = viewer->Camera();
      printf("#camera  %g %g %g  %g %g %g  %g %g %g  %g \n",
             camera.Origin().X(), camera.Origin().Y(), camera.Origin().Z(),
             camera.Towards().X(), camera.Towards().Y(), camera.Towards().Z(),
             camera.Up().X(), camera.Up().Y(), camera.Up().Z(),
             camera.YFOV());
      break; }
      
    case 27: // ESCAPE
      GLUTStop();
      break;

    default:
      redraw = FALSE;
      break;
    }
  }
  
  // Remember mouse position 
  GLUTmouse[0] = x;
  GLUTmouse[1] = GLUTwindow_height - y;

  // Remember modifiers 
  GLUTmodifiers = modifiers;

  // Redraw
  if (redraw) glutPostRedisplay();  
}




static void
GLUTInit(int *argc, char **argv)
{
  // Open window 
  glutInit(argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(GLUTwindow_width, GLUTwindow_height);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA | GLUT_MULTISAMPLE);
  GLUTwindow = glutCreateWindow("GAPS Viewer");
  
  // Initialize grfx (after create context because calls glewInit)
  RNInitGrfx();

  // Initialize multisampling
  glEnable(GL_MULTISAMPLE);
  
  // Initialize background color 
  glClearColor(200.0/255.0, 200.0/255.0, 200.0/255.0, 1.0);

  // Initialize lights
  static GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  static GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glEnable(GL_LIGHT0);
  static GLfloat light1_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glEnable(GL_LIGHT1);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);

  // Initialize color settings
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  // Initialize graphics modes 
  glEnable(GL_DEPTH_TEST);

  // Initialize GLUT callback functions 
  glutDisplayFunc(GLUTRedraw);
  glutReshapeFunc(GLUTResize);
  glutKeyboardFunc(GLUTKeyboard);
  glutSpecialFunc(GLUTSpecial);
  glutMouseFunc(GLUTMouse);
  glutMotionFunc(GLUTMotion);

  // Initialize font
#if (RN_OS == RN_WINDOWSNT)
  int font = glGenLists(256);
  wglUseFontBitmaps(wglGetCurrentDC(), 0, 256, font); 
  glListBase(font);
#endif
}



static R3Viewer *
CreateViewer(void)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Get bounding box radius
  assert(!world_bbox.IsEmpty());
  RNLength r = world_bbox.DiagonalRadius();
  assert((r > 0.0) && RNIsFinite(r));

  // Setup camera view looking down the Z axis
  if (!initial_camera) initial_camera_origin = world_bbox.Centroid() - initial_camera_towards * (2.5 * r);;
  R3Camera camera(initial_camera_origin, initial_camera_towards, initial_camera_up, 0.4, 0.4, 0.01 * r, 100.0 * r);
  R2Viewport viewport(0, 0, GLUTwindow_width, GLUTwindow_height);
  R3Viewer *viewer = new R3Viewer(camera, viewport);

  // Print statistics
  if (print_verbose) {
    printf("Created viewer ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Origin = %g %g %g\n", camera.Origin().X(), camera.Origin().Y(), camera.Origin().Z());
    printf("  Towards = %g %g %g\n", camera.Towards().X(), camera.Towards().Y(), camera.Towards().Z());
    printf("  Up = %g %g %g\n", camera.Up().X(), camera.Up().Y(), camera.Up().Z());
    printf("  Fov = %g %g\n", camera.XFOV(), camera.YFOV());
    printf("  Near = %g\n", camera.Near());
    printf("  Far = %g\n", camera.Far());
    fflush(stdout);
  }

  // Return viewer
  return viewer;
}



void GLUTMainLoop(void)
{
 // Set world bbbox
  world_bbox = R3null_box;
  for (int i = 0; i < scenes.NEntries(); i++) world_bbox.Union(scenes[i]->BBox());
  for (int i = 0; i < meshes.NEntries(); i++) world_bbox.Union(meshes[i]->BBox());
  for (int i = 0; i < pointsets.NEntries(); i++) world_bbox.Union(pointsets[i]->BBox());
  for (int i = 0; i < grids.NEntries(); i++) world_bbox.Union(grids[i]->WorldBox());
  for (int i = 0; i < sfls.NEntries(); i++) world_bbox.Union(sfls[i]->BBox());
  for (int i = 0; i < gsvs.NEntries(); i++) world_bbox.Union(gsvs[i]->BBox());

  // Set world origin
  world_origin = world_bbox.Centroid();

  // Set slice coords to middle of grid
  if (grids.NEntries() > 0) {
    grid_slice_coords[RN_X] = grids[0]->XResolution()/2;
    grid_slice_coords[RN_Y] = grids[0]->YResolution()/2;
    grid_slice_coords[RN_Z] = grids[0]->ZResolution()/2;
  }

  // Create viewer
  viewer = CreateViewer();
  if (!viewer) exit(-1);

   // Run main loop -- never returns 
  glutMainLoop();
}



static int 
ParseArgs(int argc, char **argv)
{
  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) {
        print_verbose = 1;
      }
      else if (!strcmp(*argv, "-pointset")) {
        argc--; argv++; pointset_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-mesh")) {
        argc--; argv++; mesh_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-scene")) {
        argc--; argv++; scene_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-grid")) {
        argc--; argv++; grid_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-sfl")) {
        argc--; argv++; ssa_filenames.Insert(*argv);
        argc--; argv++; ssb_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-gsv")) {
        argc--; argv++; gsv_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-image_directory")) {
        argc--; argv++; image_directory = *argv;
      }
      else if (!strcmp(*argv, "-depth_scale")) {
        argc--; argv++; depth_scale = atof(*argv);
      }
      else if (!strcmp(*argv, "-depth_exponent")) {
        argc--; argv++; depth_exponent = atof(*argv);
      }
      else if (!strcmp(*argv, "-camera")) { 
        RNCoord x, y, z, tx, ty, tz, ux, uy, uz;
        argv++; argc--; x = atof(*argv); 
        argv++; argc--; y = atof(*argv); 
        argv++; argc--; z = atof(*argv); 
        argv++; argc--; tx = atof(*argv); 
        argv++; argc--; ty = atof(*argv); 
        argv++; argc--; tz = atof(*argv); 
        argv++; argc--; ux = atof(*argv); 
        argv++; argc--; uy = atof(*argv); 
        argv++; argc--; uz = atof(*argv); 
        initial_camera_origin = R3Point(x, y, z);
        initial_camera_towards.Reset(tx, ty, tz);
        initial_camera_up.Reset(ux, uy, uz);
        initial_camera = TRUE;
      }
      else if (!strcmp(*argv, "-background")) { 
        argc--; argv++; background_color[0] = atof(*argv); 
        argc--; argv++; background_color[1] = atof(*argv); 
        argc--; argv++; background_color[2] = atof(*argv); 
      }
      else { 
        RNFail("Invalid program argument: %s", *argv); 
        exit(1); 
      }
      argv++; argc--;
    }
    else {
      // Parse filename
      char *ext = strrchr(*argv, '.');
      if (ext && (!strcmp(ext, ".obj") || !strcmp(ext, ".scn") || !strcmp(ext, ".json"))) 
        scene_filenames.Insert(*argv);
      else if (ext && (!strcmp(ext, ".off") || !strcmp(ext, ".ply") || !strcmp(ext, ".wrl"))) 
        mesh_filenames.Insert(*argv);
      else if (ext && (!strcmp(ext, ".xyzn") || !strcmp(ext, ".pts") || !strcmp(ext, ".sdf"))) 
        pointset_filenames.Insert(*argv);
      else if (ext && (!strcmp(ext, ".grd"))) 
        grid_filenames.Insert(*argv);
      else if (ext && (!strcmp(ext, ".ssa") || !strcmp(ext, ".ssx"))) 
        ssa_filenames.Insert(*argv);
      else if (ext && (!strcmp(ext, ".ssb")))
        ssb_filenames.Insert(*argv);
      else if (ext && (!strcmp(ext, ".gsv"))) 
        gsv_filenames.Insert(*argv);
      else { 
        RNFail("Invalid program argument: %s", *argv); 
        exit(1); 
      }
      argv++; argc--;
    }
  }

  // Get number of models
  nmodels = 0;
  if (pointset_filenames.NEntries() > nmodels) nmodels = pointset_filenames.NEntries();
  if (mesh_filenames.NEntries() > nmodels) nmodels = mesh_filenames.NEntries();
  if (scene_filenames.NEntries() > nmodels) nmodels = scene_filenames.NEntries();
  if (grid_filenames.NEntries() > nmodels) nmodels = grid_filenames.NEntries();
  if (ssa_filenames.NEntries() > nmodels) nmodels = ssa_filenames.NEntries();
  if (gsv_filenames.NEntries() > nmodels) nmodels = gsv_filenames.NEntries();

  // Check inputs
  if (nmodels == 0) {
    RNFail("Usage: gapsview <scenes> <meshes> <pointsets> <grids> <sfls> <gsvs> [options]\n");
    return 0;
  }

  // Return OK status 
  return 1;
}



int 
main(int argc, char **argv)
{
  // Initialize GLUT
  GLUTInit(&argc, argv);

  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);

  // Read data
  if (!ReadFiles()) exit(-1);

  // Run GLUT interface
  GLUTMainLoop();

  // Return success 
  return 0;
}



