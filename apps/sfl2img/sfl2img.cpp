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

static char *input_scene_name = NULL;
static char *input_database_name = NULL;
static char *output_directory_name = NULL;
static int write_geometry_grids = 0;
static int write_color_grids = 0;
static int write_ground_grids = 0;
static int write_semantic_grids = 0;
static int print_verbose = 0;
static int print_debug = 0;

static double pixel_spacing = 0.1; 
static int max_resolution = 32768;
static double min_elevation = -FLT_MAX;
static double max_elevation = FLT_MAX;
static double max_depth = FLT_MAX;



////////////////////////////////////////////////////////////////////////
// Surfel scene I/O Functions
////////////////////////////////////////////////////////////////////////

static R3SurfelScene *
OpenScene(const char *input_scene_name, const char *input_database_name)
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
  if (!scene->OpenFile(input_scene_name, input_database_name, "r", "r")) {
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
  const char *directory_name, const char *category_name, const char *field_name)
{
  // Create filename
  char filename[4096];
  sprintf(filename, "%s/%s_%s.grd", directory_name, category_name, field_name);

  // Write grid
  return grid.WriteFile(filename);
}



static int
WriteImage(const R2Grid& red, const R2Grid& green, const R2Grid& blue, 
  const char *directory_name, const char *category_name, const char *field_name)
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
  sprintf(filename, "%s/%s_%s.jpg", directory_name, category_name, field_name);

  // Write image
  return image.Write(filename);
}



////////////////////////////////////////////////////////////////////////
// Geometry image functions
////////////////////////////////////////////////////////////////////////

static int
WriteGeometryGrids(R3SurfelScene *scene, const char *directory_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating geometry images ...\n");
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
  if (!WriteGrid(density_grid, directory_name, "Geometry", "Density")) return 0;
  if (!WriteGrid(zmin_grid, directory_name, "Geometry", "ZMin")) return 0;
  if (!WriteGrid(zmax_grid, directory_name, "Geometry", "ZMax")) return 0;
  if (!WriteGrid(nx_grid, directory_name, "Geometry", "NX")) return 0;
  if (!WriteGrid(ny_grid, directory_name, "Geometry", "NY")) return 0;
  if (!WriteGrid(nz_grid, directory_name, "Geometry", "NZ")) return 0;
  if (!WriteGrid(horizontal_grid, directory_name, "Geometry", "Horizontal")) return 0;

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
WriteColorGrids(R3SurfelScene *scene, const char *directory_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating color images ...\n");
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
  if (!WriteGrid(red_grid, directory_name, "Color", "Red")) return 0;
  if (!WriteGrid(green_grid, directory_name, "Color", "Green")) return 0;
  if (!WriteGrid(blue_grid, directory_name, "Color", "Blue")) return 0;
  if (!WriteImage(red_grid, green_grid, blue_grid, directory_name, "Color", "Rgb")) return 0;

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
WriteSemanticGrids(R3SurfelScene *scene, const char *directory_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating semantic images ...\n");
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
  if (!WriteGrid(category_grid, directory_name, "Semantic", "Category")) return 0;
  if (!WriteGrid(instance_grid, directory_name, "Semantic", "Instance")) return 0;

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
WriteGroundGrids(R3SurfelScene *scene, const char *directory_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating ground images ...\n");
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
  if (!WriteGrid(ground_z_grid, directory_name, "Ground", "Z")) return 0;

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
// GRID WRITING
////////////////////////////////////////////////////////////////////////

static int
WriteGrids(R3SurfelScene *scene, const char *directory_name)
{
  // Create directory
  char buffer[1024];
  sprintf(buffer, "mkdir -p %s", directory_name);
  system(buffer);

  // Write grids
  if (write_geometry_grids && !WriteGeometryGrids(scene, directory_name)) return 0;
  if (write_color_grids && !WriteColorGrids(scene, directory_name)) return 0;
  if (write_semantic_grids && !WriteSemanticGrids(scene, directory_name)) return 0;
  if (write_ground_grids && !WriteGroundGrids(scene, directory_name)) return 0;

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
      else if (!strcmp(*argv, "-pixel_spacing")) { argc--; argv++; pixel_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-max_resolution")) { argc--; argv++; max_resolution = atoi(*argv); }
      else if (!strcmp(*argv, "-min_elevation")) { argc--; argv++; min_elevation = atof(*argv); }
      else if (!strcmp(*argv, "-max_elevation")) { argc--; argv++; max_elevation = atof(*argv); }
      else if (!strcmp(*argv, "-max_depth")) { argc--; argv++; max_depth = atof(*argv); }
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
    else {
      if (!input_scene_name) input_scene_name = *argv;
      else if (!input_database_name) input_database_name = *argv;
      else if (!output_directory_name) output_directory_name = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check file names
  if (!input_scene_name || !input_database_name || !output_directory_name) {
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
  R3SurfelScene *scene = OpenScene(input_scene_name, input_database_name);
  if (!scene) exit(-1);

  // Write grids
  if (!WriteGrids(scene, output_directory_name)) exit(-1);

  // Close scene
  if (!CloseScene(scene)) exit(-1);

  // Return success 
  return 0;
}



