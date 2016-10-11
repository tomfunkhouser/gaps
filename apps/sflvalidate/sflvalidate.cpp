// Source file for the surfel info program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Surfels/R3Surfels.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static char *predicted_scene_name = NULL;
static char *predicted_database_name = NULL;
static char *reference_scene_name = NULL;
static char *reference_database_name = NULL;
static double max_distance = 0.1;
static double min_overlap = 0.1;
static int print_verbose = 0;



////////////////////////////////////////////////////////////////////////
// Surfel database I/O Functions
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
    fprintf(stderr, "Unable to allocate scene\n");
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
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %d\n", scene->Tree()->Database()->NSurfels());
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
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %ds\n", scene->Tree()->Database()->NSurfels());
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
// Validate classification
////////////////////////////////////////////////////////////////////////

static int
ComputeBBoxOverlapStatistics(R3SurfelObject *test_object, R3SurfelObject *reference_object, 
  double& precision, double& recall, double& fmeasure, double& jaccard)
{
  // Initialize results
  precision = 0;
  recall = 0;
  fmeasure = 0;
  jaccard = 0;

  // Get test bounding box
  const R3Box& test_bbox = test_object->BBox();
  RNVolume test_volume = test_bbox.Volume();
  if (test_bbox.IsEmpty() || (test_volume == 0)) return 0;

  // Get reference bounding box
  const R3Box& reference_bbox = reference_object->BBox();
  RNVolume reference_volume = reference_bbox.Volume();
  if (reference_bbox.IsEmpty() || (reference_volume == 0)) return 0;

  // Check bounding box distance
  if (R3Distance(test_bbox, reference_bbox) > 0) return 0;

  // Get/test bbox union
  R3Box union_box(test_bbox);
  union_box.Union(reference_bbox);
  RNScalar union_volume = union_box.Volume();
  if (union_box.IsEmpty() || (union_volume == 0)) return 0;

  // Get/test bbox intersection
  R3Box intersection_box(test_bbox);
  intersection_box.Intersect(reference_bbox);
  RNScalar intersection_volume = intersection_box.Volume();
  if (intersection_box.IsEmpty() || (intersection_volume == 0)) return 0;

  // Compute overlap statistics
  precision = (test_volume > 0) ? intersection_volume / test_volume : 0;
  recall = (reference_volume > 0) ? intersection_volume / reference_volume : 0;
  fmeasure = (precision + recall > 0) ? 2 * precision * recall / (precision + recall) : 0;
  jaccard = (union_volume > 0) ? intersection_volume / union_volume : 0;

  // Return success
  return 1;
}



static int
ComputePointSetOverlapStatistics(R3SurfelObject *test_object, R3SurfelObject *reference_object, 
  int& test_count, int& reference_count, int& intersection_count, int& union_count, 
  double& precision, double& recall, double& fmeasure, double& jaccard)
{
  // Initialize results
  test_count = 0;
  reference_count = 0;
  intersection_count = 0;
  union_count = 0;
  precision = 0;
  recall = 0;
  fmeasure = 0;
  jaccard = 0;

  // Get test bounding box
  const R3Box& test_bbox = test_object->BBox();
  RNVolume test_volume = test_bbox.Volume();
  if (test_bbox.IsEmpty() || (test_volume == 0)) return 0;

  // Get reference bounding box
  const R3Box& reference_bbox = reference_object->BBox();
  RNVolume reference_volume = reference_bbox.Volume();
  if (reference_bbox.IsEmpty() || (reference_volume == 0)) return 0;

  // Check bounding box distance
  if (R3Distance(test_bbox, reference_bbox) > max_distance) return 0;

  // Compute test pointset
  R3SurfelPointSet *test_pointset = test_object->PointSet();
  if (!test_pointset) return 0;
  test_count = test_pointset->NPoints();
  union_count += test_count;
  if (test_count == 0) { 
    delete test_pointset; 
    return 0; 
  }

  // Compute reference pointset
  R3SurfelPointSet *reference_pointset = reference_object->PointSet();
  if (!reference_pointset) { delete test_pointset; return 0; }
  reference_count = reference_pointset->NPoints();
  union_count += reference_count;
  if (reference_count == 0) { 
    delete test_pointset; 
    delete reference_pointset; 
    return 0; 
  }

  // Check pointset bounding boxes
  if (R3Distance(test_pointset->BBox(), reference_pointset->BBox()) > max_distance) {
    delete test_pointset;
    delete reference_pointset;
    return 0;
  }

  // Compute intersection count
  intersection_count = 0;
  for (int i = 0; i < test_pointset->NPoints(); i++) {
    R3SurfelPoint *test_point = test_pointset->Point(i);
    R3Point test_position = test_point->Position();
    if (R3Distance(test_position, reference_pointset->BBox()) <= max_distance) {
      for (int j = 0; j < reference_pointset->NPoints(); j++) {
        R3SurfelPoint *reference_point = reference_pointset->Point(j);
        R3Point reference_position = reference_point->Position();
        if (R3Distance(test_position, reference_position) <= max_distance) {
          intersection_count++;
          break;
        }
      }
    }
  }

  // Compute other statistics
  union_count = test_count + reference_count - intersection_count;
  precision = (double) intersection_count / (double) test_count;
  recall = (double) intersection_count / (double) reference_count;
  fmeasure = (precision + recall > 0) ? 2 * precision * recall / (precision + recall) : 0;
  jaccard = (double) intersection_count / (double) union_count;

  // Delete pointsets
  delete test_pointset;
  delete reference_pointset;

  // Return success
  return 1;
}



static void
CountObjects(R3SurfelScene *test_scene, R3SurfelScene *reference_scene, 
  const char *test_label_name, int test_type, int reference_type, 
  int *num_total_objects = NULL, int *num_labeled_objects = NULL, 
  int *num_located_objects = NULL, int *num_missed_objects = NULL)
{
  // Initialize statistics
  int total_count = 0;
  int labeled_count = 0;
  int located_count = 0;

  // Check coverage of test objects
  for (int k = 0; k < test_scene->NObjects(); k++) {
    R3SurfelObject *test_object = test_scene->Object(k);

    // Get/check test label
    R3SurfelLabel *test_label = (test_type == 0) ? test_object->CurrentLabel() : test_object->GroundTruthLabel();
    if (!test_label) continue;
    if (!test_label->Name()) continue;
    if (!strcmp(test_label->Name(), "Unknown")) continue;
    if (test_label_name && strcmp(test_label_name, test_label->Name())) continue;

    // Find match amongst reference objects
    RNBoolean hit_label = FALSE;
    RNBoolean hit_location = FALSE;
    RNScalar best_overlap = min_overlap;
    R3SurfelObject *best_reference_object = NULL;
    for (int i = 0; i < reference_scene->NObjects(); i++) {
      R3SurfelObject *reference_object = reference_scene->Object(i);

      // Get/check reference label
      R3SurfelLabel*reference_label = (reference_type == 0) ? reference_object->CurrentLabel() : reference_object->GroundTruthLabel();
      if (!reference_label) continue;
      if (!reference_label->Name()) continue;
      if (!strcmp(reference_label->Name(), "Unknown")) continue;

#if 0
      // Compute overlap
      double precision, recall, fmeasure, jaccard;
      int test_count, reference_count, intersection_count, union_count;
      ComputePointSetOverlapStatistics(test_object, reference_object, 
        test_count, reference_count, intersection_count, union_count,
        precision, recall, fmeasure, jaccard);
#else
      // Compute overlap
      double precision, recall, fmeasure, jaccard;
      ComputeBBoxOverlapStatistics(test_object, reference_object, 
        precision, recall, fmeasure, jaccard);
#endif
      RNScalar overlap = jaccard;
      if (overlap >= best_overlap) {
        best_reference_object = reference_object;
        best_overlap = overlap;
      }
    }

    // Log hit
    if (best_reference_object) {
      R3SurfelLabel*reference_label = (reference_type == 0) ? best_reference_object->CurrentLabel() : best_reference_object->GroundTruthLabel();
      if (!strcmp(reference_label->Name(), test_label->Name())) hit_label = TRUE;
      hit_location = TRUE;
    }

    // Update statistics
    if (hit_location) located_count++;
    if (hit_label) labeled_count++;
    total_count++;
  }

  // Return results
  if (num_total_objects) *num_total_objects = total_count;
  if (num_labeled_objects) *num_labeled_objects = labeled_count;
  if (num_located_objects) *num_located_objects = located_count;
  if (num_missed_objects) *num_missed_objects = total_count - located_count;
}



static int
PrintClassificationStatistics(R3SurfelScene *predicted_scene, R3SurfelScene *reference_scene)
{
  // Print header
  printf("    %48s %48s\n", "Precision", "Recall");
  printf("    %-24s | %6s : %5s (%3s)  %5s (%3s) %5s (%3s) | %6s : %5s (%3s)  %5s (%3s) %5s (%3s)\n", 
    "Type", "PRED", "RT", "%", "RL", "%", "W", "%", "REF", "FT", "%", "FL", "%", "M", "%");

  // Print separating line
  printf("    "); for (int i = 0; i < 112; i++) printf("-"); printf("\n");

  // Print label statistics
  for (int i = 0; i < reference_scene->NLabels(); i++) {
    R3SurfelLabel *label = reference_scene->Label(i);
    if (!label->Name() || !strcmp(label->Name(), "Unknown")) continue;
    int predicted_total_objects, predicted_labeled_objects, predicted_located_objects, predicted_missed_objects;
    CountObjects(predicted_scene, reference_scene, label->Name(), 0, 1, 
      &predicted_total_objects, &predicted_labeled_objects, &predicted_located_objects, &predicted_missed_objects);
    int reference_total_objects, reference_labeled_objects, reference_located_objects, reference_missed_objects;
    CountObjects(reference_scene, predicted_scene, label->Name(), 1, 0, 
      &reference_total_objects, &reference_labeled_objects, &reference_located_objects, &reference_missed_objects);
    if ((predicted_total_objects > 0) || (reference_total_objects > 0)) {
      char short_label_name[32] = { 0 };
      strncpy(short_label_name, label->Name(), 24);
      printf("    %-24s | %6d : %5d (%3.0f)  %5d (%3.0f) %5d (%3.0f) | %6d : %5d (%3.0f)  %5d (%3.0f) %5d (%3.0f)\n", 
        short_label_name,
        predicted_total_objects, 
        predicted_labeled_objects, (predicted_total_objects > 0) ? 100.0 * predicted_labeled_objects / predicted_total_objects : 0, 
        predicted_located_objects, (predicted_total_objects > 0) ? 100.0 * predicted_located_objects / predicted_total_objects : 0, 
        predicted_missed_objects, (predicted_total_objects > 0) ? 100.0 * predicted_missed_objects / predicted_total_objects : 0,
        reference_total_objects, 
        reference_labeled_objects, (reference_total_objects > 0) ? 100.0 * reference_labeled_objects / reference_total_objects : 0, 
        reference_located_objects, (reference_total_objects > 0) ? 100.0 * reference_located_objects / reference_total_objects : 0, 
        reference_missed_objects, (reference_total_objects > 0) ? 100.0 * reference_missed_objects / reference_total_objects : 0);
    }
  }

  // Print separating line
  printf("    "); for (int i = 0; i < 112; i++) printf("-"); printf("\n");

  // Print overall statistics
  if (TRUE) {
    int predicted_total_objects, predicted_labeled_objects, predicted_located_objects, predicted_missed_objects;
    CountObjects(predicted_scene, reference_scene, NULL, 0, 1, 
      &predicted_total_objects, &predicted_labeled_objects, &predicted_located_objects, &predicted_missed_objects);
    int reference_total_objects, reference_labeled_objects, reference_located_objects, reference_missed_objects;
    CountObjects(reference_scene, predicted_scene, NULL, 1, 0, 
      &reference_total_objects, &reference_labeled_objects, &reference_located_objects, &reference_missed_objects);
    if ((predicted_total_objects > 0) || (reference_total_objects > 0)) {
      // Print line
      printf("    %-24s | %6d : %5d (%3.0f)  %5d (%3.0f) %5d (%3.0f) | %6d : %5d (%3.0f)  %5d (%3.0f) %5d (%3.0f)\n", 
       "Overall",
       predicted_total_objects, 
       predicted_labeled_objects, (predicted_total_objects > 0) ? 100.0 * predicted_labeled_objects / predicted_total_objects : 0, 
       predicted_located_objects, (predicted_total_objects > 0) ? 100.0 * predicted_located_objects / predicted_total_objects : 0, 
       predicted_missed_objects, (predicted_total_objects > 0) ? 100.0 * predicted_missed_objects / predicted_total_objects : 0,
       reference_total_objects, 
       reference_labeled_objects, (reference_total_objects > 0) ? 100.0 * reference_labeled_objects / reference_total_objects : 0, 
       reference_located_objects, (reference_total_objects > 0) ? 100.0 * reference_located_objects / reference_total_objects : 0, 
       reference_missed_objects, (reference_total_objects > 0) ? 100.0 * reference_missed_objects / reference_total_objects : 0);
    }
  }

  // Print an extra newline
  printf("\n");

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Argument Parsing Functions
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-max_distance")) { argc--; argv++; max_distance = atof(*argv); }
      else if (!strcmp(*argv, "-min_overlap")) { argc--; argv++; min_overlap = atof(*argv); }
      else { fprintf(stderr, "Invalid program argument: %s", *argv); return 0; }
      argv++; argc--;
    }
    else {
      if (!predicted_scene_name) predicted_scene_name = *argv;
      else if (!predicted_database_name) predicted_database_name = *argv;
      else if (!reference_scene_name) reference_scene_name = *argv;
      else if (!reference_database_name) reference_database_name = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); return 0; }
      argv++; argc--;
    }
  }

  // Check arguments
  if (!predicted_scene_name || !predicted_database_name || !reference_scene_name || !reference_database_name) {
    fprintf(stderr, "Usage: sflvalidate predicted_scene predicted_database reference_scene reference_database [options]\n");
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

  // Open predicted scene file
  R3SurfelScene *predicted_scene = OpenScene(predicted_scene_name, predicted_database_name);
  if (!predicted_scene) exit(-1);

  // Open reference scene file
  R3SurfelScene *reference_scene = OpenScene(reference_scene_name, reference_database_name);
  if (!reference_scene) exit(-1);

  // Print info
  if (!PrintClassificationStatistics(predicted_scene, reference_scene)) exit(-1);

  // Close predicted scene file
  if (!CloseScene(predicted_scene)) exit(-1);;

  // Close reference scene file
  if (!CloseScene(reference_scene)) exit(-1);;

  // Return success 
  return 0;
}


