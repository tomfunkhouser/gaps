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
  R2Grid zmin_grid(density_grid);
  R2Grid zmax_grid(density_grid);
  R2Grid zmean_grid(density_grid);
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
        zmean_grid.RasterizeGridPoint(grid_position, pz);
        nx_grid.RasterizeGridPoint(grid_position, nx);
        ny_grid.RasterizeGridPoint(grid_position, ny);
        nz_grid.RasterizeGridPoint(grid_position, nz);
        horizontal_grid.RasterizeGridPoint(grid_position, fabs(nz));
      }

      // Release block
      database->ReleaseBlock(block);
    }
  }

  // Divide by density to get averages
  zmean_grid.Divide(density_grid);
  nx_grid.Divide(density_grid);
  ny_grid.Divide(density_grid);
  nz_grid.Divide(density_grid);
  horizontal_grid.Divide(density_grid);

  // Write grids
  if (!WriteGrid(density_grid, directory_name, "Geometry", "Density")) return 0;
  if (!WriteGrid(zmin_grid, directory_name, "Geometry", "ZMin")) return 0;
  if (!WriteGrid(zmax_grid, directory_name, "Geometry", "ZMax")) return 0;
  if (!WriteGrid(zmean_grid, directory_name, "Geometry", "ZMean")) return 0;
  if (!WriteGrid(nx_grid, directory_name, "Geometry", "NX")) return 0;
  if (!WriteGrid(ny_grid, directory_name, "Geometry", "NY")) return 0;
  if (!WriteGrid(nz_grid, directory_name, "Geometry", "NZ")) return 0;
  if (!WriteGrid(horizontal_grid, directory_name, "Geometry", "Horizontal")) return 0;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", density_grid.XResolution(), density_grid.YResolution());
    printf("  Spacing = %g\n", density_grid.WorldToGridScaleFactor());
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
  R2Grid elevation_grid(bbox, pixel_spacing, 5, max_resolution);
  R2Grid red_grid(elevation_grid);
  R2Grid green_grid(elevation_grid);
  R2Grid blue_grid(elevation_grid);
  elevation_grid.Clear(R2_GRID_UNKNOWN_VALUE);  

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
        R2Point grid_position = elevation_grid.GridPosition(R2Point(px, py));
        int ix = (int) (grid_position.X() + 0.5);
        int iy = (int) (grid_position.Y() + 0.5);
        if ((ix < 0) || (ix >= elevation_grid.XResolution())) continue;
        if ((iy < 0) || (iy >= elevation_grid.YResolution())) continue;

        // Update elevation grid (and rgb)
        RNScalar e = fabs(elevation);
        RNScalar e_old = elevation_grid.GridValue(ix, iy);
        if ((e_old == R2_GRID_UNKNOWN_VALUE) || (e < e_old)) {
          elevation_grid.SetGridValue(ix, iy, e);
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
    printf("  Resolution = %d %d\n", elevation_grid.XResolution(), elevation_grid.YResolution());
    printf("  Spacing = %g\n", elevation_grid.WorldToGridScaleFactor());
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
  R2Grid elevation_grid(bbox, pixel_spacing, 5, max_resolution);
  R2Grid category_grid(elevation_grid);
  R2Grid instance_grid(elevation_grid);
  elevation_grid.Clear(R2_GRID_UNKNOWN_VALUE);  

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
        R2Point grid_position = elevation_grid.GridPosition(R2Point(px, py));
        int ix = (int) (grid_position.X() + 0.5);
        int iy = (int) (grid_position.Y() + 0.5);
        if ((ix < 0) || (ix >= elevation_grid.XResolution())) continue;
        if ((iy < 0) || (iy >= elevation_grid.YResolution())) continue;

        // Update elevation grid (and rgb)
        RNScalar e = fabs(elevation);
        RNScalar e_old = elevation_grid.GridValue(ix, iy);
        if ((e_old == R2_GRID_UNKNOWN_VALUE) || (e < e_old)) {
          elevation_grid.SetGridValue(ix, iy, e);
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
    printf("  Resolution = %d %d\n", elevation_grid.XResolution(), elevation_grid.YResolution());
    printf("  Spacing = %g\n", elevation_grid.WorldToGridScaleFactor());
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
      else if (!strcmp(*argv, "-pixel_spacing")) { argc--; argv++; pixel_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-max_resolution")) { argc--; argv++; max_resolution = atoi(*argv); }
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



