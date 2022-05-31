// Source file for sfllabel input/output functions



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Surfels/R3Surfels.h"
#include "io.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {

  

///////////////////////////////////////////////////////////////////////
// Scene I/O Functions
////////////////////////////////////////////////////////////////////////

R3SurfelScene *
OpenScene(const char *scene_filename, const char *database_filename, int print_verbose)
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
  if (!scene->OpenFile(scene_filename, database_filename, "r+", "r+")) {
    delete scene;
    return NULL;
  }

  // TEMPORARY: Adjust label colors
  R3SurfelLabel *label = NULL;
  label = scene->FindLabelByName("Tree");
  if (label) label->SetColor(RNRgb(80/255.0, 255/255.0, 80/255.0));
  label = scene->FindLabelByName("Person");
  if (label) label->SetColor(RNRgb(255/255.0, 150/255.0, 50/255.0));
  label = scene->FindLabelByName("GuardRail");
  if (label) label->SetColor(RNRgb(80/255.0, 255/255.0, 180/255.0));
  label = scene->FindLabelByName("Animal");
  if (label) label->SetColor(RNRgb(240/255.0, 200/255.0, 240/255.0));

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
    printf("  # Blocks = %d\n", scene->Tree()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->NSurfels());
    fflush(stdout);
  }

  // Return scene
  return scene;
}



static int
CleanScene(R3SurfelScene *scene, int print_verbose = 0)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int start_nobjects = scene->NObjects();

#if 0
  // Find objects to delete
  RNArray<R3SurfelObject *> deletable_objects;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object == scene->RootObject()) continue;
    if (object->NParts() > 0) continue;
    if (object->NNodes() > 0) continue;
    deletable_objects.Insert();
  }

  // Delete objects
  for (int i = 0; i < deletable_objects.NEntries(); i++) {
    R3SurfelObject *object = deletable_objects.Kth(i);
    delete object;
  }
#else
  // Delete null objects
  if (!RemoveEmptyObjects(scene)) return 0;
#endif  
  
  // Print statistics
  if (0 && print_verbose) {
    printf("Cleaned scene ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Deleted objects = %d\n", start_nobjects - scene->NObjects());
    fflush(stdout);
  }

  // Return success
  return 1;
}



int
CloseScene(R3SurfelScene *scene, int print_verbose)
{
  // Clean scene
  CleanScene(scene, print_verbose);

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
    printf("  # Blocks = %d\n", scene->Tree()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->NSurfels());
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

int
ReadImagesFromPixelDatabase(R3SurfelScene *scene, const char *filename,
  double depth_scale, double depth_exponent, int max_images,
  int print_verbose)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Read all image channels
  if (!ReadPixelDatabase(scene, filename, max_images)) return 0;

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
    printf("Read image channels from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Color Reads = %d\n", color_count);
    printf("  # Depth Reads = %d\n", depth_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



int
ReadImagesFromDirectory(R3SurfelScene *scene, const char *directory_name,
  double depth_scale, double depth_exponent, int max_images,
  int print_verbose)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Read all image channels
  if (!ReadImageDirectory(scene, directory_name,
    depth_scale, depth_exponent, max_images)) return 0;

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
    printf("Read image channels from %s ...\n", directory_name);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Color Reads = %d\n", color_count);
    printf("  # Depth Reads = %d\n", depth_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



}; // end namespace
