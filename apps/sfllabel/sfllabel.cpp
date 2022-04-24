// Source file for the surfel labeler program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Graphics/R3Graphics.h"
#include "R3Surfels/R3Surfels.h"
#include "io.h"
#include "ui.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static const char *scene_filename = NULL;
static const char *database_filename = NULL;
static const char *input_pixel_database_filename = NULL;
static const char *input_image_directory = NULL;
static const char *output_history_filename = NULL;
static const char *output_snapshot_directory = NULL;
static double depth_scale = 2000;
static double depth_exponent = 0.5;
static int multiresolution = 0;
static int dynamic_cache = 0;
static int max_images = 0;
static int print_verbose = 0;



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
      else if (!strcmp(*argv, "-multiresolution")) { 
        multiresolution = 1;
      }
      else if (!strcmp(*argv, "-dynamic_cache")) { 
        dynamic_cache = 1;
      }      
      else if (!strcmp(*argv, "-pixel_database")) { 
        argv++; argc--; input_pixel_database_filename = *argv;
      }
      else if (!strcmp(*argv, "-image_directory")) { 
        argv++; argc--; input_image_directory = *argv;
      }
      else if (!strcmp(*argv, "-history")) { 
        argc--; argv++; output_history_filename = *argv; 
      }
      else if (!strcmp(*argv, "-snapshot_directory")) { 
        argc--; argv++; output_snapshot_directory = *argv; 
      }
      else if (!strcmp(*argv, "-depth_scale")) { 
        argv++; argc--; depth_scale = atof(*argv);
      }
      else if (!strcmp(*argv, "-depth_exponent")) { 
        argv++; argc--; depth_exponent = atof(*argv);
      }
      else if (!strcmp(*argv, "-max_images")) { 
        argv++; argc--; max_images = atoi(*argv);
      }
      else { 
        RNFail("Invalid program argument: %s", *argv); 
        exit(1); 
      }
      argv++; argc--;
    }
    else {
      if (!scene_filename) scene_filename = *argv;
      else if (!database_filename) database_filename = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check surfels name
  if (!scene_filename || !database_filename) {
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
  // Initialize packages
  if (!R3InitGraphics()) exit(-1);
  if (!R3InitSurfels()) exit(-1);

  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);

  // Open scene
  R3SurfelScene *scene = OpenScene(scene_filename, database_filename, print_verbose);
  if (!scene) exit(-1);

  // Read images
  if (input_pixel_database_filename) {
    if (!ReadImagesFromPixelDatabase(scene, input_pixel_database_filename,
      depth_scale, depth_exponent, max_images, print_verbose)) exit(-1);
  }
  else if (input_image_directory) {
    if (!ReadImagesFromDirectory(scene, input_image_directory,
      depth_scale, depth_exponent, max_images, print_verbose)) exit(-1);
  }

  // Create labeler
  R3SurfelLabeler *labeler = new R3SurfelLabeler(scene, output_history_filename);
  if (!labeler) exit(-1);

  // Check if should read everything into memory always
  if (!dynamic_cache) {
    labeler->ReadCoarsestBlocks(FLT_MAX);
  }

  // Check if should draw everything at full res always
  if (!multiresolution) {
    labeler->SetFocusRadius(RN_INFINITY);
    labeler->SetTargetResolution(RN_INFINITY);
  }

  // Check if should output snapshots
  if (output_snapshot_directory) {
    labeler->SetSnapshotDirectory(output_snapshot_directory);
    labeler->Snapshot();
  }

  // Run interface
  UIInterface(labeler);
  
  // Terminate labeler
  if (labeler) labeler->Terminate();

  // Close scene
  if (scene) CloseScene(scene, print_verbose);

  // Return success 
  return 0;
}


