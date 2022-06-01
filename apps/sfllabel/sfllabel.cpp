// Source file for the surfel labeler program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Utils/R3Utils.h"
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
static int apply_may2022_scene_updates = 0;
static int print_verbose = 0;



///////////////////////////////////////////////////////////////////////
// Apply temporary scene modifications
////////////////////////////////////////////////////////////////////////

static int
AddLabel(R3SurfelScene *scene, const char *name,
  int identifier, int assignment_key,
  int r, int g, int b, const char *parent_name = NULL)
{
  // Check stuff
  if (!scene) return 0;
  if (!name) return 0;

  // Check if label already exists
  if (scene->FindLabelByName(name)) return 1;

  // Find parent label
  R3SurfelLabel *parent = scene->RootLabel();
  if (parent_name) parent = scene->FindLabelByName(parent_name);
  if (!parent) return 0;

  // Create label
  R3SurfelLabel *label =  new R3SurfelLabel(name);
  label->SetColor(gaps::RNRgb(r / 255.0, g / 255.0, b / 255.0));
  label->SetIdentifier(identifier);
  label->SetAssignmentKeystroke(assignment_key);
  scene->InsertLabel(label, parent);

  // Return success
  return 1;
}



static int
AddLabelFlags(R3SurfelScene *scene, const char *label_name, RNFlags flags)
{
  // Find label
  R3SurfelLabel *label = scene->FindLabelByName(label_name);
  if (!label) return 0;

  // Update label flags
  RNFlags label_flags = label->Flags();
  label_flags.Add(flags);
  label->SetFlags(label_flags);

  // Return success
  return 1;
}



static int
SetLabelAssignmentKey(R3SurfelScene *scene, const char *label_name, int assignment_key)
{
  // Find label
  R3SurfelLabel *label = scene->FindLabelByName(label_name);
  if (!label) return 0;

  // Update assignment key
  label->SetAssignmentKeystroke(assignment_key);

  // Return success
  return 1;
}



static void
ApplyMay2022SceneUpdates(R3SurfelScene *scene)
{
  // Reset label assignment keys
  SetLabelAssignmentKey(scene, "OtherVehicle", 'C');
  
  // Add new labels
  const int PEDESTRIAN_ENTRANCE_LABEL = 47;
  const int VEHICLE_ENTRANCE_LABEL = 48;
  AddLabel(scene, "PedestrianEntrance", PEDESTRIAN_ENTRANCE_LABEL, 'n', 240, 140, 140);
  AddLabel(scene, "VehicleEntrance", VEHICLE_ENTRANCE_LABEL, 'v', 240, 0, 240);
  
  // Add label axis flags
  AddLabelFlags(scene, "Billboard", R3_SURFEL_LABEL_SHORT_AXIS_TOWARDS_FRONT_FLAG);
  AddLabelFlags(scene, "BusinessSign", R3_SURFEL_LABEL_SHORT_AXIS_TOWARDS_FRONT_FLAG);
  AddLabelFlags(scene, "PedestrianEntrance", R3_SURFEL_LABEL_SHORT_AXIS_TOWARDS_FRONT_FLAG);
  AddLabelFlags(scene, "TempTrafficSign", R3_SURFEL_LABEL_SHORT_AXIS_TOWARDS_FRONT_FLAG);
  AddLabelFlags(scene, "TrafficSign", R3_SURFEL_LABEL_SHORT_AXIS_TOWARDS_FRONT_FLAG);
  AddLabelFlags(scene, "VehicleEntrance", R3_SURFEL_LABEL_SHORT_AXIS_TOWARDS_FRONT_FLAG);

  // Add label unorientable flags
  AddLabelFlags(scene, "Bridge", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "BuildingInterior", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "BuildingExterior", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "BusStop", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Crosswalk", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Driveway", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Fence", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "FireHydrant", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "GuardRail", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "LidarArtifact", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Mountain", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "OtherGround", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "OtherPermObject", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "OtherStructure", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "OtherTempObject", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "ParkingMeter", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "PavedRoad", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Pole", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Self", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Sidewalk", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Sky", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "StreetLight", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "TempCone", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Terrain", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Tree", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Tunnel", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Unknown", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "UnpavedRoad", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Wall", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Water", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
  AddLabelFlags(scene, "Wire", R3_SURFEL_LABEL_UNORIENTABLE_FLAG);
}



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
      else if (!strcmp(*argv, "-apply_may2022_scene_updates")) { 
        apply_may2022_scene_updates = 1; 
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

  // TEMPORARY
  if (apply_may2022_scene_updates) {
    ApplyMay2022SceneUpdates(scene);
  }

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


