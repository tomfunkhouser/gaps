// Source file for the surfel scene processing program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Surfels/R3Surfels.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static char *input_scene_filename = NULL;
static char *input_database_filename = NULL;
static char *input_image_directory = NULL;
static char *input_pixel_database_filename = NULL;
static char *output_directory = NULL;
static int write_geometry_grids = 0;
static int write_color_grids = 0;
static int write_ground_grids = 0;
static int write_semantic_grids = 0;
static int write_zoomed_object_images = 0;
static int write_overlaid_object_images = 0;
static int print_verbose = 0;
static int print_debug = 0;

static double pixel_spacing = 0.1; 
static int max_resolution = 32768;
static double min_elevation = -FLT_MAX;
static double max_elevation = FLT_MAX;
static double max_depth = FLT_MAX;
static const char *selected_label_name = NULL;



////////////////////////////////////////////////////////////////////////
// Surfel scene I/O Functions
////////////////////////////////////////////////////////////////////////

static R3SurfelScene *
OpenScene(const char *input_scene_filename, const char *input_database_filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate scene
  R3SurfelScene *scene = new R3SurfelScene();
  if (!scene) {
    RNFail("Unable to allocate scene\n");
    return NULL;
  }

  // Open scene files
  if (!scene->OpenFile(input_scene_filename, input_database_filename, "r", "r")) {
    delete scene;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Opened scene ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Objects = %d\n", scene->NObjects());
    printf("  # Labels = %d\n", scene->NLabels());
    printf("  # Assignments = %d\n", scene->NLabelAssignments());
    printf("  # Features = %d\n", scene->NFeatures());
    printf("  # Scans = %d\n", scene->NScans());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Return scene
  return scene;
}



static int
CloseScene(R3SurfelScene *scene)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Print statistics
  if (print_verbose) {
    printf("Closing scene ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Objects = %d\n", scene->NObjects());
    printf("  # Labels = %d\n", scene->NLabels());
    printf("  # Assignments = %d\n", scene->NLabelAssignments());
    printf("  # Features = %d\n", scene->NFeatures());
    printf("  # Scans = %d\n", scene->NScans());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Close scene files
  if (!scene->CloseFile()) {
    delete scene;
    return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Image writing functions
////////////////////////////////////////////////////////////////////////

static int
WriteGrid(const R2Grid& grid, 
  const char *output_directory, const char *category_name, const char *field_name)
{
  // Create filename
  char filename[4096];
  sprintf(filename, "%s/%s_%s.grd", output_directory, category_name, field_name);

  // Write grid
  return grid.WriteFile(filename);
}



static int
WriteImage(const R2Grid& red, const R2Grid& green, const R2Grid& blue, 
  const char *output_directory, const char *category_name, const char *field_name)
{
  // Create image
  R2Image image(red.XResolution(), red.YResolution());

  // Fill image
  for (int j = 0; j < red.YResolution(); j++) {
    for (int i = 0; i < red.XResolution(); i++) {
      RNScalar r = red.GridValue(i, j);
      RNScalar g = green.GridValue(i, j);
      RNScalar b = blue.GridValue(i, j);
      image.SetPixelRGB(i, j, RNRgb(r, g, b));
    }
  }


  // Create filename
  char filename[1024];
  sprintf(filename, "%s/%s_%s.jpg", output_directory, category_name, field_name);

  // Write image
  return image.Write(filename);
}



////////////////////////////////////////////////////////////////////////
// Geometry image functions
////////////////////////////////////////////////////////////////////////

static int
WriteGeometryGrids(R3SurfelScene *scene, const char *output_directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating geometry grids ...\n");
    fflush(stdout);
  }
    
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Create grids
  const R3Box& scene_bbox = scene->BBox();
  R2Box bbox(scene_bbox[0][0], scene_bbox[0][1], scene_bbox[1][0], scene_bbox[1][1]);
  R2Grid density_grid(bbox, pixel_spacing, 5, max_resolution);
  R2Grid weight_grid(density_grid);
  R2Grid zmin_grid(density_grid);
  R2Grid zmax_grid(density_grid);
  R2Grid nx_grid(density_grid);
  R2Grid ny_grid(density_grid);
  R2Grid nz_grid(density_grid);
  R2Grid horizontal_grid(density_grid);
  zmin_grid.Clear(R2_GRID_UNKNOWN_VALUE);  
  zmax_grid.Clear(R2_GRID_UNKNOWN_VALUE);  

  // Fill grids
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    if (node->NParts() > 0) continue;
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      const R3Point& origin = block->PositionOrigin();

      // Read block
      database->ReadBlock(block);

      // Process surfels
      for (int j = 0; j < block->NSurfels(); j++) {
        const R3Surfel *surfel = block->Surfel(j);

        // Get/check depth
        float depth = surfel->Depth();
        if (depth > max_depth) continue;
        
        // Get/check elevation
        float elevation = surfel->Elevation();
        if (elevation < min_elevation) continue;
        if (elevation > max_elevation) continue;

        // Get weight
        double weight = 1;
        if (depth > 1) weight *= 1.0 / (depth * depth);

        // Get world coordinates
        double px = origin.X() + surfel->X();
        double py = origin.Y() + surfel->Y();
        double pz = origin.Z() + surfel->Z();

        // Get normal
        float nx = surfel->NX();
        float ny = surfel->NY();
        float nz = surfel->NZ();

        // Get grid coordinates
        R2Point grid_position = density_grid.GridPosition(R2Point(px, py));
        int ix = (int) (grid_position.X() + 0.5);
        int iy = (int) (grid_position.Y() + 0.5);
        if ((ix < 0) || (ix >= density_grid.XResolution())) continue;
        if ((iy < 0) || (iy >= density_grid.YResolution())) continue;

        // Update zmin grid
        RNScalar zmin = zmin_grid.GridValue(ix, iy);
        if ((zmin == R2_GRID_UNKNOWN_VALUE) || (pz < zmin)) {
          zmin_grid.SetGridValue(ix, iy, pz);
        }
 
        // Update zmax grid
        RNScalar zmax = zmax_grid.GridValue(ix, iy);
        if ((zmax == R2_GRID_UNKNOWN_VALUE) || (pz > zmax)) {
          zmax_grid.SetGridValue(ix, iy, pz);
        }

        // Update other grids
        density_grid.RasterizeGridPoint(grid_position, 1);
        weight_grid.RasterizeGridPoint(grid_position, weight);
        nx_grid.RasterizeGridPoint(grid_position, weight * nx);
        ny_grid.RasterizeGridPoint(grid_position, weight * ny);
        nz_grid.RasterizeGridPoint(grid_position, weight * nz);
        horizontal_grid.RasterizeGridPoint(grid_position, weight * fabs(nz));
      }

      // Release block
      database->ReleaseBlock(block);
    }
  }

  // Divide by density to get averages
  nx_grid.Divide(weight_grid);
  ny_grid.Divide(weight_grid);
  nz_grid.Divide(weight_grid);
  horizontal_grid.Divide(weight_grid);

  // Write grids
  if (!WriteGrid(density_grid, output_directory, "Geometry", "Density")) return 0;
  if (!WriteGrid(zmin_grid, output_directory, "Geometry", "ZMin")) return 0;
  if (!WriteGrid(zmax_grid, output_directory, "Geometry", "ZMax")) return 0;
  if (!WriteGrid(nx_grid, output_directory, "Geometry", "NX")) return 0;
  if (!WriteGrid(ny_grid, output_directory, "Geometry", "NY")) return 0;
  if (!WriteGrid(nz_grid, output_directory, "Geometry", "NZ")) return 0;
  if (!WriteGrid(horizontal_grid, output_directory, "Geometry", "Horizontal")) return 0;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", density_grid.XResolution(), density_grid.YResolution());
    printf("  Spacing = %g\n", density_grid.GridToWorldScaleFactor());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Color image functions
////////////////////////////////////////////////////////////////////////

static int
WriteColorGrids(R3SurfelScene *scene, const char *output_directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating color grids ...\n");
    fflush(stdout);
  }

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Create grids
  const R3Box& scene_bbox = scene->BBox();
  R2Box bbox(scene_bbox[0][0], scene_bbox[0][1], scene_bbox[1][0], scene_bbox[1][1]);
  R2Grid depth_grid(bbox, pixel_spacing, 5, max_resolution);
  R2Grid red_grid(depth_grid);
  R2Grid green_grid(depth_grid);
  R2Grid blue_grid(depth_grid);
  depth_grid.Clear(R2_GRID_UNKNOWN_VALUE);  

  // Fill grids
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    if (node->NParts() > 0) continue;
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      const R3Point& origin = block->PositionOrigin();

      // Read block
      database->ReadBlock(block);

      // Process surfels
      for (int j = 0; j < block->NSurfels(); j++) {
        const R3Surfel *surfel = block->Surfel(j);

        // Get/check depth
        float depth = surfel->Depth();
        if (depth > max_depth) continue;
        
        // Get/check elevation
        float elevation = surfel->Elevation();
        if (elevation < min_elevation) continue;
        if (elevation > max_elevation) continue;

        // Get world coordinates
        double px = origin.X() + surfel->X();
        double py = origin.Y() + surfel->Y();

        // Get surfel color
        RNRgb rgb = surfel->Rgb();

        // Get grid coordinates
        R2Point grid_position = depth_grid.GridPosition(R2Point(px, py));
        int ix = (int) (grid_position.X() + 0.5);
        int iy = (int) (grid_position.Y() + 0.5);
        if ((ix < 0) || (ix >= depth_grid.XResolution())) continue;
        if ((iy < 0) || (iy >= depth_grid.YResolution())) continue;

        // Update depth grid (and rgb)
        RNScalar d = depth_grid.GridValue(ix, iy);
        if ((d == R2_GRID_UNKNOWN_VALUE) || (depth < d)) {
          depth_grid.SetGridValue(ix, iy, d);
          red_grid.SetGridValue(ix, iy, rgb.R());
          green_grid.SetGridValue(ix, iy, rgb.G());
          blue_grid.SetGridValue(ix, iy, rgb.B());
        }
      }

      // Release block
      database->ReleaseBlock(block);
    }
  }

  // Write grids
  if (!WriteGrid(red_grid, output_directory, "Color", "Red")) return 0;
  if (!WriteGrid(green_grid, output_directory, "Color", "Green")) return 0;
  if (!WriteGrid(blue_grid, output_directory, "Color", "Blue")) return 0;
  if (!WriteImage(red_grid, green_grid, blue_grid, output_directory, "Color", "Rgb")) return 0;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", depth_grid.XResolution(), depth_grid.YResolution());
    printf("  Spacing = %g\n", depth_grid.GridToWorldScaleFactor());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Semantic image functions
////////////////////////////////////////////////////////////////////////

static int
WriteSemanticGrids(R3SurfelScene *scene, const char *output_directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating semantic grids ...\n");
    fflush(stdout);
  }

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Create grids
  const R3Box& scene_bbox = scene->BBox();
  R2Box bbox(scene_bbox[0][0], scene_bbox[0][1], scene_bbox[1][0], scene_bbox[1][1]);
  R2Grid depth_grid(bbox, pixel_spacing, 5, max_resolution);
  R2Grid category_grid(depth_grid);
  R2Grid instance_grid(depth_grid);
  depth_grid.Clear(R2_GRID_UNKNOWN_VALUE);  

  // Fill grids
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    if (node->NParts() > 0) continue;

    // Get object
    R3SurfelObject *object = node->Object(TRUE);
    while (object && object->Parent() && (object->Parent() != scene->RootObject()))
      object = object->Parent();
    if (!object) continue;

    // Get label
    R3SurfelLabel *label = object->CurrentLabel();
    if (!label) continue;

    // Rasterize surfels
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      const R3Point& origin = block->PositionOrigin();

      // Read block
      database->ReadBlock(block);

      // Process surfels
      for (int j = 0; j < block->NSurfels(); j++) {
        const R3Surfel *surfel = block->Surfel(j);

        // Get/check depth
        float depth = surfel->Depth();
        if (depth > max_depth) continue;
        
        // Get/check elevation
        float elevation = surfel->Elevation();
        if (elevation < min_elevation) continue;
        if (elevation > max_elevation) continue;

        // Get world coordinates
        double px = origin.X() + surfel->X();
        double py = origin.Y() + surfel->Y();

        // Get grid coordinates
        R2Point grid_position = depth_grid.GridPosition(R2Point(px, py));
        int ix = (int) (grid_position.X() + 0.5);
        int iy = (int) (grid_position.Y() + 0.5);
        if ((ix < 0) || (ix >= depth_grid.XResolution())) continue;
        if ((iy < 0) || (iy >= depth_grid.YResolution())) continue;

        // Update depth grid (and rgb)
        RNScalar d = depth_grid.GridValue(ix, iy);
        if ((d == R2_GRID_UNKNOWN_VALUE) || (depth < d)) {
          depth_grid.SetGridValue(ix, iy, depth);
          category_grid.SetGridValue(ix, iy, label->Identifier());
          instance_grid.SetGridValue(ix, iy, object->SceneIndex());
        }
      }

      // Release block
      database->ReleaseBlock(block);
    }
  }

  // Write grids
  if (!WriteGrid(category_grid, output_directory, "Semantic", "Category")) return 0;
  if (!WriteGrid(instance_grid, output_directory, "Semantic", "Instance")) return 0;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", depth_grid.XResolution(), depth_grid.YResolution());
    printf("  Spacing = %g\n", depth_grid.GridToWorldScaleFactor());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Ground image functions
////////////////////////////////////////////////////////////////////////

static int
WriteGroundGrids(R3SurfelScene *scene, const char *output_directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating ground grids ...\n");
    fflush(stdout);
  }

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Create grids
  const R3Box& scene_bbox = scene->BBox();
  R2Box bbox(scene_bbox[0][0], scene_bbox[0][1], scene_bbox[1][0], scene_bbox[1][1]);
  R2Grid density_grid(bbox, pixel_spacing, 5, max_resolution);
  R2Grid ground_z_grid(density_grid);

  // Fill grids
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    if (node->NParts() > 0) continue;
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      const R3Point& origin = block->PositionOrigin();

      // Read block
      database->ReadBlock(block);

      // Process surfels
      for (int j = 0; j < block->NSurfels(); j++) {
        const R3Surfel *surfel = block->Surfel(j);

        // Get/check depth
        float depth = surfel->Depth();
        if (depth > max_depth) continue;
        
        // Get/check elevation
        float elevation = surfel->Elevation();
        if (elevation < min_elevation) continue;
        if (elevation > max_elevation) continue;

        // Get world coordinates
        double px = origin.X() + surfel->X();
        double py = origin.Y() + surfel->Y();
        double pz = origin.Z() + surfel->Z();

        // Get ground z
        double ground_z = pz - elevation;

        // Compute weight
        RNScalar weight = 1;
        if (depth > 1) weight = 1.0 / (depth * depth);

        // Update grids
        R2Point grid_position = density_grid.GridPosition(R2Point(px, py));
        density_grid.RasterizeGridPoint(grid_position, weight);
        ground_z_grid.RasterizeGridPoint(grid_position, weight * ground_z);
      }

      // Release block
      database->ReleaseBlock(block);
    }
  }

  // Divide to get weighted average
  ground_z_grid.Divide(density_grid);

  // Write grids
  if (!WriteGrid(ground_z_grid, output_directory, "Ground", "Z")) return 0;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", density_grid.XResolution(), density_grid.YResolution());
    printf("  Spacing = %g\n", density_grid.GridToWorldScaleFactor());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Object image functions
////////////////////////////////////////////////////////////////////////

static R3SurfelImage
CreateObjectCamera(R3SurfelObject *object)
{
  // Set some default parameters (hardwired for now)
  int width = 512;
  int height = 512;

  // Determine orientation
  R3OrientedBox obb = object->CurrentOrientedBBox();
  R3Vector towards = -obb.Axes()[0];
  towards.Rotate(obb.Axes()[1], -RN_PI/4.0);
  towards.Normalize();
  R3Vector right = towards % R3posz_vector;
  right.Normalize();
  R3Vector up = right % towards;
  up.Normalize();

  // Determine viewpoint
  R3Point lookat = obb.Center();
  double r = 5.0 * obb.DiagonalRadius();
  if (r == 0) r = 10;
  R3Point viewpoint= lookat - r * towards;

  // Create camera
  R3SurfelImage camera;
  camera.SetViewpoint(viewpoint);
  camera.SetOrientation(towards, up);
  camera.SetImageDimensions(width, height);
  camera.SetFocalLengths(width);
  camera.SetImageCenter(R2Point(0.5*width, 0.5*height));

  // Return camera
  return camera;
}



static R2Box
ComputeImageBBox(R3SurfelImage *image, R3SurfelObject *object, double inflation_factor = 1, int min_length = 0)
{
  // Initialize extent
  R2Box image_bbox = R2null_box;

  // Union projections of object bbox corners
  R3Box object_bbox = object->BBox();
  for (int i = 0; i < 8; i++) {
    R3Point world_position = object_bbox.Corner(i);
    R2Point image_position = image->TransformFromWorldToImage(world_position);
    if (image_position == R2unknown_point) continue;
    image_bbox.Union(image_position);
  }

  // Check if empty
  if (image_bbox.IsEmpty()) return image_bbox;
  if (RNIsZero(image_bbox.Area())) return image_bbox;
  
  // Inflate bbox
  image_bbox.Inflate(inflation_factor);

  // Fix min image resolution in each dimension
  if (min_length > 0) {
    for (int i = 0; i < 2; i++) {
      int delta_length = 0.5 * (min_length - image_bbox.AxisLength(i));
      if (delta_length <= 0) continue;
      image_bbox[0][i] -= delta_length;
      image_bbox[1][i] += delta_length;
    }
  }
  
  // Keep within image extent
  image_bbox.Intersect(R2Box(0, 0, image->ImageWidth()-1, image->ImageHeight()-1));
    
  // Return image bbox
  return image_bbox;
}



static R2Image
CropImage(const R2Image& input_image, const R2Box& bbox)
{
  int input_width = input_image.Width();
  int input_height = input_image.Height();
  int output_width = bbox.XLength();
  int output_height = bbox.YLength();
  R2Image output_image(output_width, output_height, 3);

  // Fill output image
  for (int output_iy = 0; output_iy < output_height; output_iy++) {
    int input_iy = output_iy + bbox.YMin();
    if (input_iy >= input_height) continue;
    for (int output_ix = 0; output_ix < output_width; output_ix++) {
      int input_ix = output_ix + bbox.XMin();
      if (input_ix >= input_width) continue;
      RNRgb input_rgb = input_image.PixelRGB(input_ix, input_iy);
      output_image.SetPixelRGB(output_ix, output_iy, input_rgb);
    }
  }

  // Return output image
  return output_image;
}



static R2Image
RenderImage(R2Image& image,
  const R3SurfelPointSet& pointset,
  const R3SurfelImage& camera,
  int pointradius, int color_scheme)
{
  // Get convenient variables
  int width = image.Width();
  int height = image.Height();
  R3Point viewpoint = camera.Viewpoint();
  R3Vector towards = camera.Towards();
  
  // Render pointset
  for (int i = 0; i < pointset.NPoints(); i++) {
    R3SurfelPoint *point = pointset.Point(i);
    R3Point position = point->Position();
    R3Vector vector = position - viewpoint;
    double depth = towards.Dot(vector);
    if (RNIsNegativeOrZero(depth)) continue;
    R2Point image_position = camera.TransformFromWorldToImage(position);
    if (!camera.ContainsImagePosition(image_position)) continue;
    int cx = image_position.X() + 0.5;
    int cy = image_position.Y() + 0.5;
    for (int dx = -pointradius; dx <= pointradius; dx++) {
      int ix = cx + dx;
      if ((ix < 0) || (ix >= width)) continue;
      for (int dy = -pointradius; dy <= pointradius; dy++) {
        int iy = cy + dy;
        if ((iy < 0) || (iy >= height)) continue;
        RNRgb rgb = (color_scheme == 0) ? point->Rgb() : RNyellow_rgb;
        image.SetPixelRGB(ix, iy, rgb);
      }
    }
  }

  // Return the image
  return image;
}


    
static int
WriteZoomedObjectImages(R3SurfelScene *scene, const char *output_directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Creating zoomed object images ...\n");
    fflush(stdout);
  }

  // Default parameters
  int width = 512;
  int height = 512;
  
  // Create pointset for whole scene
  R3SurfelPointSet *scene_pointset = CreatePointSet(scene);
  if (!scene_pointset) return 0;
  if (scene_pointset->NPoints() == 0) { delete scene_pointset; return 0; }

  // Write image for each object
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (!object->Name()) continue;

    // Get/check label
    R3SurfelLabel *label = object->CurrentLabel();
    if (!label) continue;
    if (!label->Name()) continue;
    if (selected_label_name && strcmp(label->Name(), selected_label_name)) continue;

    // Get/check pointset
    R3SurfelPointSet *object_pointset = object->PointSet(TRUE);
    if (!object_pointset) continue;
    if (object_pointset->NPoints() == 0) { delete object_pointset; continue; }

    // Render image
    R2Image image(width, height, 3);
    R3SurfelImage camera = CreateObjectCamera(object);
    RenderImage(image, *scene_pointset, camera, 1, 0);
    RenderImage(image, *object_pointset, camera, 2, 1);

    // Write image
    char image_filename[1024];
    sprintf(image_filename, "%s/%s_%d_%d_zoomed.png", output_directory,
      label->Name(), object_pointset->NPoints(), object->SceneIndex());
    image.Write(image_filename);

    // Delete object_pointset
    delete object_pointset;

    // Write debug message
    if (print_debug) {
      printf("  %s\n", image_filename);
      fflush(stdout);
    }
    
    // Increment count
    count++;
  }

  // Delete scene pointset
  delete scene_pointset;
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
WriteOverlaidObjectImages(R3SurfelScene *scene, const char *output_directory)
{
  // Check inputs
  if (!input_image_directory && !input_pixel_database_filename) return 1;
  
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Creating overlaid object images ...\n");
    fflush(stdout);
  }

  // Open pixel database
  R2PixelDatabase pixel_database;
  if (input_pixel_database_filename) {
    if (!pixel_database.OpenFile(input_pixel_database_filename, "r")) return 0;
  }

  // Write image for each object
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->Parent() != scene->RootObject()) continue;
    if (!object->Name()) continue;

    // Get/check label
    R3SurfelLabel *label = object->CurrentLabel();
    if (!label) continue;
    if (!label->Name()) continue;
    if (selected_label_name && strcmp(label->Name(), selected_label_name)) continue;

    // Find best sfl image
    R3SurfelImage *sfl_image = scene->FindImageByBestView(object->BBox().Centroid(), R3zero_vector);
    if (!sfl_image) continue;
    
    // Get/read input color image
    R2Image image;
    char input_image_filename[1024];
    if (input_pixel_database_filename) {
      // Read image from pixel database
      sprintf(input_image_filename, "color_images/%s.jpg", sfl_image->Name());
      if (!pixel_database.FindImage(input_image_filename, &image))
        sprintf(input_image_filename, "color_images/%s.png", sfl_image->Name());
      if (!pixel_database.FindImage(input_image_filename, &image)) continue;
    }
    else if (input_image_directory) {
      // Read image from directory of images
      sprintf(input_image_filename, "%s/color_images/%s.jpg", input_image_directory, sfl_image->Name());
      if (!RNFileExists(input_image_filename)) sprintf(input_image_filename, "%s/color_images/%s.png", input_image_directory, sfl_image->Name());
      if (!RNFileExists(input_image_filename)) continue;
      if (!image.Read(input_image_filename)) continue;
    }

    // Get/check pointset
    R3SurfelPointSet *object_pointset = object->PointSet(TRUE);
    if (!object_pointset) continue;
    if (object_pointset->NPoints() == 0) { delete object_pointset; continue; }

    // Create overlay image
    RenderImage(image, *object_pointset, *sfl_image, 2, 1);

    // Crop overlay image
    R2Box crop_box = ComputeImageBBox(sfl_image, object, 4, 100);
    if (crop_box.IsEmpty()) { delete object_pointset; continue; }
    R2Image crop_image = CropImage(image, crop_box);
    image = crop_image;

    // Check image
    if ((image.Width() == 0) || (image.Height() == 0)) { delete object_pointset; continue; }
    
    // Write overlay image
    char output_image_filename[1024];
    sprintf(output_image_filename, "%s/%s_%d_%d_overlaid.png", output_directory,
      label->Name(), object_pointset->NPoints(), object->SceneIndex());
    image.Write(output_image_filename);

    // Delete object_pointset
    delete object_pointset;

    // Write debug message
    if (print_debug) {
      printf("  %s\n", output_image_filename);
      fflush(stdout);
    }
    
    // Increment count
    count++;
  }

  // Close pixel database
  if (input_pixel_database_filename) {
    if (!pixel_database.CloseFile()) return 0;
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// GRID WRITING
////////////////////////////////////////////////////////////////////////

static int
WriteGrids(R3SurfelScene *scene, const char *output_directory)
{
  // Create directory
  char buffer[1024];
  sprintf(buffer, "mkdir -p %s", output_directory);
  system(buffer);

  // Write grids
  if (write_geometry_grids && !WriteGeometryGrids(scene, output_directory)) return 0;
  if (write_color_grids && !WriteColorGrids(scene, output_directory)) return 0;
  if (write_semantic_grids && !WriteSemanticGrids(scene, output_directory)) return 0;
  if (write_ground_grids && !WriteGroundGrids(scene, output_directory)) return 0;
  if (write_overlaid_object_images && !WriteOverlaidObjectImages(scene, output_directory)) return 0;
  if (write_zoomed_object_images && !WriteZoomedObjectImages(scene, output_directory)) return 0;

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Detects if default set of grids should be computed
  int default_grids = 1;

  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-debug")) print_debug = 1;
      else if (!strcmp(*argv, "-geometry")) { write_geometry_grids = 1; default_grids = 0; }
      else if (!strcmp(*argv, "-color")) { write_color_grids = 1; default_grids = 0; }
      else if (!strcmp(*argv, "-semantic")) { write_semantic_grids = 1; default_grids = 0; }
      else if (!strcmp(*argv, "-ground")) { write_ground_grids = 1; default_grids = 0; }
      else if (!strcmp(*argv, "-zoomed_object_images")) { write_zoomed_object_images = 1; default_grids = 0; }
      else if (!strcmp(*argv, "-overlaid_object_images")) { write_overlaid_object_images = 1; default_grids = 0; }
      else if (!strcmp(*argv, "-pixel_spacing")) { argc--; argv++; pixel_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-max_resolution")) { argc--; argv++; max_resolution = atoi(*argv); }
      else if (!strcmp(*argv, "-min_elevation")) { argc--; argv++; min_elevation = atof(*argv); }
      else if (!strcmp(*argv, "-max_elevation")) { argc--; argv++; max_elevation = atof(*argv); }
      else if (!strcmp(*argv, "-max_depth")) { argc--; argv++; max_depth = atof(*argv); }
      else if (!strcmp(*argv, "-selected_label")) { argc--; argv++; selected_label_name = *argv; }
      else if (!strcmp(*argv, "-image_directory")) { argc--; argv++; input_image_directory = *argv; }
      else if (!strcmp(*argv, "-pixel_database")) { argc--; argv++; input_pixel_database_filename = *argv; }
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
    else {
      if (!input_scene_filename) input_scene_filename = *argv;
      else if (!input_database_filename) input_database_filename = *argv;
      else if (!output_directory) output_directory = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check file names
  if (!input_scene_filename || !input_database_filename || !output_directory) {
    RNFail("Usage: sfl2img scenefile databasefile [options]\n");
    return FALSE;
  }

  // Set grid selection if nothing else specified
  if (default_grids) {
    write_geometry_grids = 1;
    write_color_grids = 1;
    write_semantic_grids = 1;
    write_ground_grids = 1;
  }

  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);

  // Open tree
  R3SurfelScene *scene = OpenScene(input_scene_filename, input_database_filename);
  if (!scene) exit(-1);

  // Write grids
  if (!WriteGrids(scene, output_directory)) exit(-1);

  // Close scene
  if (!CloseScene(scene)) exit(-1);

  // Return success 
  return 0;
}



