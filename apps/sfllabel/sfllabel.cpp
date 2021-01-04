// Source file for the surfel labeler program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Graphics/R3Graphics.h"
#include "R3Surfels/R3Surfels.h"
#include "R3SurfelClassifier.h"
#include "R3SurfelSegmenter.h"
#include "R3SurfelLabeler.h"



////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////

// Program arguments

static const char *scene_name = NULL;
static const char *database_name = NULL;
static const char *history_name = NULL;
static const char *pixel_database = NULL;
static const char *image_directory = NULL;
static double depth_scale = 2000;
static double depth_exponent = 0.5;
static int print_verbose = 0;



// Application variables

static R3SurfelScene *scene = NULL;
static R3SurfelLabeler *labeler = NULL;



///////////////////////////////////////////////////////////////////////
// Scene I/O Functions
////////////////////////////////////////////////////////////////////////

static R3SurfelScene *
OpenScene(const char *scene_name, const char *database_name)
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
  if (!scene->OpenFile(scene_name, database_name, "r+", "r+")) {
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
CleanScene(R3SurfelScene *scene)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Find objects to delete
  RNArray<R3SurfelObject *> deletable_objects;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->NParts() > 0) continue;
    if (object->NNodes() > 0) continue;
    deletable_objects.Insert(object);
  }

  // Delete objects
  for (int i = 0; i < deletable_objects.NEntries(); i++) {
    R3SurfelObject *object = deletable_objects.Kth(i);
    delete object;
  }
  
  // Print statistics
  if (0 && print_verbose) {
    printf("Cleaned scene ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Deleted objects = %d\n", deletable_objects.NEntries());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
CloseScene(R3SurfelScene *scene)
{
  // Clean scene
  CleanScene(scene);

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



///////////////////////////////////////////////////////////////////////
// Image I/O Functions
////////////////////////////////////////////////////////////////////////

static int
ReadImagesFromPixelDatabase(R3SurfelScene *scene, const char *pixel_database)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Read all image channels
  if (!ReadPixelDatabase(scene, pixel_database)) return 0;

  // Print statistics
  if (print_verbose) {
    // Gather statistics
    int color_count = 0;
    int depth_count = 0;
    for (int i = 0; i < scene->NImages(); i++) {
      R3SurfelImage *image = scene->Image(i);
      if (image->RedChannel()) color_count++;
      if (image->DepthChannel()) depth_count++;
    }

    // Print statistics
    printf("Read image channels from %s ...\n", pixel_database);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Color Reads = %d\n", color_count);
    printf("  # Depth Reads = %d\n", depth_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
ReadImagesFromDirectory(R3SurfelScene *scene, const char *image_directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Read all image channels
  if (!ReadImageDirectory(scene, image_directory,
    depth_scale, depth_exponent)) return 0;

  // Print statistics
  if (print_verbose) {
    // Gather statistics
    int color_count = 0;
    int depth_count = 0;
    for (int i = 0; i < scene->NImages(); i++) {
      R3SurfelImage *image = scene->Image(i);
      if (image->RedChannel()) color_count++;
      if (image->DepthChannel()) depth_count++;
    }

    // Print statistics
    printf("Read image channels from %s ...\n", image_directory);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Color Reads = %d\n", color_count);
    printf("  # Depth Reads = %d\n", depth_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



///////////////////////////////////////////////////////////////////////
// Temporary Label processing Functions
////////////////////////////////////////////////////////////////////////

static void
AssignLabelColor(R3SurfelScene *scene, const char *name, int r, int g, int b)
{
  // Find label
  R3SurfelLabel *label = scene->FindLabelByName(name);
  if (!label) return;

  // Set color
  RNRgb color(r/255.0, g/255.0, b/255.0);
  label->SetColor(color);
}



static void
UpdateLabels(R3SurfelScene *scene)
{
  // Create wire label
  if (!scene->FindLabelByName("Wire")) {
    R3SurfelLabel *parent = scene->FindLabelByName("Static");
    if (parent) {
      R3SurfelLabel *label = new R3SurfelLabel("Wire");
      label->SetIdentifier(49);
      label->SetAssignmentKeystroke('L');
      label->SetColor(RNRgb(240/255.0, 180.0/255.0, 180.0/255.0));
      scene->InsertLabel(label, parent);
    }
  }

  // Assign label colors  
  AssignLabelColor(scene, "Sidewalk",             0, 140, 0);
  AssignLabelColor(scene, "Driveway",             0, 120, 240);
  AssignLabelColor(scene, "Terrain",              40, 40, 220);
  AssignLabelColor(scene, "Motorcycle",           0, 0, 240);
  AssignLabelColor(scene, "Rider",                240, 160, 80);
}  



///////////////////////////////////////////////////////////////////////
// User interface stuff
////////////////////////////////////////////////////////////////////////

#ifdef USE_GLFW
#  include "glfw_ui.h"
#else
#  include "glut_ui.h"
#endif



///////////////////////////////////////////////////////////////////////
// Argument Parsing Functions
////////////////////////////////////////////////////////////////////////

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
      else if (!strcmp(*argv, "-history")) { 
        argc--; argv++; history_name = *argv; 
      }
      else if (!strcmp(*argv, "-pixel_database")) { 
        argv++; argc--; pixel_database = *argv;
      }
      else if (!strcmp(*argv, "-image_directory")) { 
        argv++; argc--; image_directory = *argv;
      }
      else if (!strcmp(*argv, "-depth_scale")) { 
        argv++; argc--; depth_scale = atof(*argv);
      }
      else if (!strcmp(*argv, "-depth_exponent")) { 
        argv++; argc--; depth_exponent = atof(*argv);
      }
      else { 
        RNFail("Invalid program argument: %s", *argv); 
        exit(1); 
      }
      argv++; argc--;
    }
    else {
      if (!scene_name) scene_name = *argv;
      else if (!database_name) database_name = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check surfels name
  if (!scene_name || !database_name) {
    RNFail("Usage: sfllabel scenefile databasefile [options]\n");
    return FALSE;
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

  // Open scene
  scene = OpenScene(scene_name, database_name);
  if (!scene) exit(-1);

  // Read images
  if (pixel_database) {
    if (!ReadImagesFromPixelDatabase(scene, pixel_database)) exit(-1);
  }
  else if (image_directory) {
    if (!ReadImagesFromDirectory(scene, image_directory)) exit(-1);
  }
  
  // Create labeler
  labeler = new R3SurfelLabeler(scene, history_name);
  if (!labeler) exit(-1);

  // Temporary (for surfel tests)
  UpdateLabels(scene);
  labeler->SetFocusRadius(RN_INFINITY);
  labeler->SetTargetResolution(RN_INFINITY);

  // Run interface
  UIInterface();
  
  // Terminate labeler
  if (labeler) labeler->Terminate();

  // Close scene
  if (scene) CloseScene(scene);

  // Return success 
  return 0;
}


