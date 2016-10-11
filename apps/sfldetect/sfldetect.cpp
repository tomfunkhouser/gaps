// Source file for the surfel segmentation program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Surfels/R3Surfels.h"
#include "GSV/GSV.h"
#include "object.h"
#include "markers.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

// Files and operations
static char *input_scene_name = NULL;
static char *input_database_name = NULL;
static char *input_parse_name = NULL;
static char *input_ground_truth_name = NULL;
static char *input_detections_name = NULL;
static char *input_labels_name = NULL;
static char *input_gsv_name = NULL;
static char *input_markers_name = NULL;
static char *input_centers_name = NULL;
static char *input_search_grid_name = NULL;
static char *input_zsupport_grid_name = NULL;
static char *output_parse_name = NULL;
static char *output_gsv_grid_name = NULL;
static char *output_search_grid_name = NULL;
static char *output_zsupport_grid_name = NULL;
static char *output_rectangles_name = NULL;
static char *output_segmentation_directory = NULL;
static int create_detection_for_each_surfel_object = 0;
static int create_segmentation_for_each_assignment = 0;
static int create_best_assignment_for_each_detection_label_pair = 0;
static int create_best_assignment_for_each_detection = 0;
static int create_best_assignment_for_each_surfel_object = 0;
static int create_assignments_using_search_grid = 0;
static int create_assignments_using_convolution = 0;
static int remove_weak_assignments = 0;
static int remove_equivalent_assignments = 0;
static int remove_overlapping_assignments = 0;
static int create_uniform_search_grid = 0;
static int create_density_search_grid = 0;
static int create_gsv_hough_search_grid = 0;
static int mask_nonmaxima_in_search_grid = 1;
static int max_assignments = 0;
static int print_verbose = 0;
static int print_debug = 0;
static int sort = 0;

// Surfel extraction parameters 
static R2Grid *zsupport_grid = NULL;
static RNCoord zsupport_coordinate = RN_UNKNOWN;
static int superpixels = 0;
static RNLength min_radius = 0.25;
static RNLength max_radius = 4;
static RNLength min_height = 0.25;
static RNLength max_height = 16;
static RNLength max_gap = 0.25;
static RNVolume min_volume = 0.1;
static RNVolume max_volume = 1000;
static int min_points = 16;
static int max_points = 1024;

// Alignment parameters
static RNScalar min_scale = 0.9;
static RNScalar max_scale = 1.1;
static RNBoolean use_icp = TRUE; 
static int nrotations = 8; 

// Detection parameters
static R3Box detections_bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
static RNScalar min_score = 0;

// Grid sampling parameters
static int max_samples_per_grid_cell = 1;
static RNScalar min_points_per_square_meter = 1;

// Grid matching parameters
static RNLength rasterization_spacing = 0.25;
static RNLength alignment_spacing = 0.25;

// Non-maximum suppression parameters
static RNLength min_assignment_spacing = 1.0;
static RNLength min_rotation_spacing = RN_PI / 4.0;
static RNScalar max_overlap = 0.1;



////////////////////////////////////////////////////////////////////////
// PointSet utility functions
////////////////////////////////////////////////////////////////////////

static int 
LoadSurfels(ObjectPointSet& pointset, 
  R3SurfelScene *scene, R3SurfelPointSet *surfels,
  int max_points, RNScalar value = 1.0)
{ 
  // Check surfels
  if (!surfels) return 1;
  if (surfels->NPoints() == 0) return 1;

  // Load points
  if (surfels->NPoints() > max_points) {
    // Load random sampling of points
    int count = 0;
    while (count < max_points) {
      int index = (int) (RNRandomScalar() * surfels->NPoints());
      const R3SurfelPoint *surfel_point = surfels->Point(index);
      ObjectPoint object_point;
      object_point.SetPosition(surfel_point->Position());
      object_point.SetColor(surfel_point->Rgb());
      object_point.SetValue(value);
      pointset.InsertPoint(object_point);
      count++;
    }
  }
  else {
    // Load all points
    for (int index = 0; index < surfels->NPoints(); index++) {
      const R3SurfelPoint *surfel_point = surfels->Point(index);
      ObjectPoint object_point;
      object_point.SetPosition(surfel_point->Position());
      object_point.SetColor(surfel_point->Rgb());
      object_point.SetValue(value);
      pointset.InsertPoint(object_point);
    }
  }
    
  // Return success
  return 1;
}



static int 
LoadSurfels(ObjectPointSet& pointset, 
  R3SurfelScene *scene, const R3Point& origin, RNLength max_radius, 
  RNLength min_height, RNLength max_height, 
  RNLength min_volume, RNLength max_volume,
  RNLength max_gap,
  int min_points, int max_points)
{ 
  // Extract foreground surfels 
  R3Point center = origin; 
  R3SurfelPointSet *foreground = CreatePointSet(scene, center, max_radius, 
    min_height, max_height, 0.0, min_volume, max_volume, min_points);
  if (!foreground) return 0;

  // Extract background surfels
  R3SurfelPointSet *background = new R3SurfelPointSet();
  if (superpixels) {
    R3SurfelPointSet copy(*foreground);
    foreground->Empty();
    for (int i = 0; i < copy.NPoints(); i++) {
      R3SurfelPoint *surfel = copy.Point(i);
      R3SurfelBlock *block = surfel->Block();
      R3SurfelNode *node = (block) ? block->Node() : NULL;
      R3SurfelObject *object = (node) ? node->Object() : NULL;
      RNBoolean f = TRUE;
      if (object) {
        // Check object label
        // if (object->HumanLabel()) f = FALSE;
        // if (object->PredictedLabel()) {
        //   R3SurfelLabel *label = object->PredictedLabel();
        //   if (strcmp(label->Name(), "Null") && strcmp(label->Name(), "Unknown")) f = FALSE;
        // }

        // Check object extent
        R3Point furthest_point = object->BBox().FurthestPoint(origin);
        RNLength dz = fabs(furthest_point[2] - origin[2]);
        if (dz > max_height) f = FALSE;
        else {
          RNLength dx = furthest_point[0] - origin[0];
          RNLength dy = furthest_point[1] - origin[1];
          if (dx*dx + dy*dy > max_radius*max_radius) f = FALSE;
        }
      }

      // Insert point
      if (f) foreground->InsertPoint(*surfel);
      else background->InsertPoint(*surfel);
    }
  }

  // Keep connected component
  if (max_gap > 0) {
    R3SurfelPointSet *connected = CreateConnectedPointSet(foreground, origin, 
      0.5 * max_gap, min_height, max_height, min_volume, max_volume, max_gap, 1024);
    if (connected) {
      delete foreground;
      foreground = connected;
    }
  }

  // Load surfels
  int status = 1;
  status &= LoadSurfels(pointset, scene, foreground, max_points, 1.0);
  status &= LoadSurfels(pointset, scene, background, max_points, -1.0);

  // Delete surfels
  delete foreground;
  delete background;

  // Set origin
  pointset.SetOrigin(origin);

  // Return status
  return status;
}



static int
LoadSurfels(ObjectPointSet& pointset, R3SurfelScene *scene, 
  ObjectModel *model, const R3Affine& model_to_pointset_transformation, 
  RNLength max_distance, int max_points)
{
  // Compute maximum extent 
  R3Point origin = model->Origin();
  origin.Transform(model_to_pointset_transformation);
  RNLength radius = model->Radius() + max_distance;
  RNLength height = model->Height() + max_distance;
  RNScalar cz = origin.Z();

  // Extract surfels
  R3SurfelPointSet *surfels = CreatePointSet(scene, origin, radius, 
    min_height, height, max_gap, min_volume, max_volume, min_points);
  if (!surfels) return 0;

  // Retain surfels within max distance of transformed model
  R3SurfelPointSet close_surfels;
  const ObjectPointSet& model_pointset = model->PointSet();
  R3Affine affine = model_to_pointset_transformation.Inverse();
  for (int i = 0; i < surfels->NPoints(); i++) {
    const R3SurfelPoint *surfel = surfels->Point(i);
    R3Point position = surfel->Position();
    position.Transform(affine);
    ObjectPoint point(position);
    if (!model_pointset.FindClosest(&point, 0, max_distance)) continue;
    close_surfels.InsertPoint(*surfel);
  }

  // Load surfels
  LoadSurfels(pointset, scene, &close_surfels, max_points);

  // Set origin
  origin = pointset.Centroid(); 
  origin[2] = cz;
  pointset.SetOrigin(origin);

  // Delete surfels
  delete surfels;

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Alignment functions
////////////////////////////////////////////////////////////////////////

static int 
RefineTransformation(
  ObjectSegmentation *segmentation, ObjectModel *model, const R3Affine& initial_transformation, 
  R3Affine& result_transformation, RNScalar& result_score) 
{
  // Get parameters
  RNLength start_max_distance = 1.0 * model->Radius();
  RNLength end_max_distance = 0.125 * model->Radius();
  RNLength coverage_sigma = 0.5 * end_max_distance;
  RNScalar max_distance_update_factor = 0.75;

  // Get/check pointsets
  const ObjectPointSet& segmentation_pointset = segmentation->PointSet();
  if (segmentation_pointset.NPoints() == 0) return 0;
  const ObjectPointSet& model_pointset = model->PointSet();
  if (model_pointset.NPoints() == 0) return 0;

  // Refine transformation
  if (use_icp) {
    // Refine transformation with ICP
    result_transformation = IterativeClosestPointTransformation2D(segmentation_pointset, model_pointset, initial_transformation, 
      start_max_distance, end_max_distance, max_distance_update_factor, min_scale, max_scale, TRUE, TRUE);
  }
  else {
    // Copy initial transformation
    result_transformation = initial_transformation;
  }

  // Compute score
  RNScalar coverage1 = Coverage(segmentation_pointset, model_pointset, result_transformation, coverage_sigma);
  RNScalar coverage2 = Coverage(model_pointset, segmentation_pointset, result_transformation.Inverse(), coverage_sigma);
  if ((coverage1 < 0) && (coverage2 < 0)) result_score = -coverage1 * coverage2;
  else result_score = coverage1 * coverage2;

  // Return success
  return 1;
}



static int 
ComputeBestTransformation(
  ObjectSegmentation *segmentation, ObjectModel *model, 
  R3Affine& result_transformation, RNScalar& result_score) 
{
  // Initialize result
  result_transformation = R3identity_affine;
  result_score = 0;

  // Get/check pointsets
  const ObjectPointSet& segmentation_pointset = segmentation->PointSet();
  if (segmentation_pointset.NPoints() == 0) return 0;
  const ObjectPointSet& model_pointset = model->PointSet();
  if (model_pointset.NPoints() == 0) return 0;

  // Compute PCA alignment 
  R3Affine segmentation_pca(PCATransformation2D(segmentation_pointset));
  R3Affine model_pca(PCATransformation2D(model_pointset));

  // Perform ICP from various initial alignments
  for (int i = 0; i < nrotations; i++) {
    RNAngle rotation_angle = i * RN_TWO_PI / nrotations;

    // Get initial transformation
    R3Affine transformationA = R3identity_affine; 
    transformationA.Transform(segmentation_pca.Inverse());
    transformationA.ZRotate(rotation_angle);
    transformationA.Transform(model_pca);
    
    // Get refined transformation
    RNScalar scoreA = 0;
    if (!RefineTransformation(segmentation, model, transformationA, transformationA, scoreA)) return 0;

    // Remember best
    if (scoreA > result_score) {
      result_transformation = transformationA;
      result_score = scoreA;
    }
  }

  // Print debug message
  if (print_debug) {
    ObjectDetection *detection = segmentation->Detection();
    printf("T %d %d (%d) -> %d %d (%d) : %g\n", 
     (detection) ? segmentation->Detection()->ParseIndex() : -1, 
     segmentation->DetectionIndex(), segmentation_pointset.NPoints(),
     model->Label()->ParseIndex(), model->LabelIndex(), model_pointset.NPoints(),
     result_score);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Assignment creation functions
////////////////////////////////////////////////////////////////////////

static int 
CreateBestAssignment(ObjectParse *parse,
  ObjectDetection *detection, ObjectLabel *label, RNScalar min_score)
{
  // Initialize result
  ObjectSegmentation *best_segmentation = NULL;
  ObjectModel *best_model = NULL;
  R3Affine best_transformation = R3identity_affine;
  RNScalar best_score = min_score;

  // Check all combinations of segmentation and model
  for (int i = 0; i < detection->NSegmentations(); i++) {
    ObjectSegmentation *segmentation = detection->Segmentation(i);
    for (int j = 0; j < label->NModels(); j++) {
      ObjectModel *model = label->Model(j);
      ObjectAssignment *previous_assignment = segmentation->BestAssignment(model);
      if (previous_assignment && !previous_assignment->IsGroundTruth()) {
        if (previous_assignment->Score() > best_score) {
          best_segmentation = previous_assignment->Segmentation();
          best_model = previous_assignment->Model();
          best_transformation = previous_assignment->ModelToSegmentationTransformation();
          best_score = previous_assignment->Score();
        }
      }
      else {
        RNScalar score = 0;
        R3Affine transformation = R3identity_affine;
        if (!ComputeBestTransformation(segmentation, model, transformation, score)) continue;
        if (score > best_score) {
          best_segmentation = segmentation;
          best_model = model;
          best_transformation = transformation;
          best_score = score;
        }
      }
    }
  }

  // Check if found suitable combination
  if (!best_segmentation || !best_model) return 1;
  if (best_segmentation->BestAssignment(best_model)) return 1;

  // Create assignment
  ObjectAssignment *assignment = new ObjectAssignment(best_segmentation, best_model, best_transformation, best_score);
  best_segmentation->InsertAssignment(assignment);
  best_model->InsertAssignment(assignment);

  // Return success
  return 1;
}



static int 
CreateBestAssignment(ObjectParse *parse,
  ObjectDetection *detection, RNScalar min_score)
{
  // Initialize result
  ObjectSegmentation *best_segmentation = NULL;
  ObjectLabel *best_label = NULL;
  ObjectModel *best_model = NULL;
  R3Affine best_transformation = R3identity_affine;
  RNScalar best_score = min_score;

  // Check all combinations of segmentation and model
  for (int i = 0; i < detection->NSegmentations(); i++) {
    ObjectSegmentation *segmentation = detection->Segmentation(i);
    for (int j = 0; j < parse->NLabels(); j++) {
      ObjectLabel *label = parse->Label(j);
      for (int k = 0; k < label->NModels(); k++) {
        ObjectModel *model = label->Model(k);
        ObjectAssignment *previous_assignment = segmentation->BestAssignment(model);
        if (previous_assignment && !previous_assignment->IsGroundTruth()) {
          if (previous_assignment->Score() > best_score) {
            best_segmentation = previous_assignment->Segmentation();
            best_label = label;
            best_model = previous_assignment->Model();
            best_transformation = previous_assignment->ModelToSegmentationTransformation();
            best_score = previous_assignment->Score();
          }
        }
        else {
          RNScalar score = 0;
          R3Affine transformation = R3identity_affine;
          if (!ComputeBestTransformation(segmentation, model, transformation, score)) continue;
          if (score > best_score) {
            best_segmentation = segmentation;
            best_label = label;
            best_model = model;
            best_transformation = transformation;
            best_score = score;
          }
        }
      }
    }
  }

  // Check if found suitable combination
  if (!best_segmentation || !best_label || !best_model) return 1;
  if (best_segmentation->BestAssignment(best_model)) return 1;

  // Create assignment
  ObjectAssignment *assignment = new ObjectAssignment(best_segmentation, best_model, best_transformation, best_score);
  best_segmentation->InsertAssignment(assignment);
  best_model->InsertAssignment(assignment);

  // Return success
  return 1;
}



static int
CreateBestAssignmentForEachDetection(ObjectParse *parse, 
  R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating best assignment for each detection-label pair ...\n");
    fflush(stdout);
  }

  // Create point sets for all segmentations
  for (int i = 0; i < parse->NSegmentations(); i++) {
    ObjectSegmentation *segmentation = parse->Segmentation(i);
    if (segmentation->PointSet().NPoints() == 0) {
      R3Point origin = segmentation->Origin();
      ObjectPointSet pointset;
      if (!LoadSurfels(pointset, scene, origin, max_radius, 
        min_height, max_height, min_volume, max_volume, max_gap, min_points, max_points )) continue;
      segmentation->SetPointSet(pointset);
    }
  }

  // Create all assignments for all detections
  for (int i = 0; i < parse->NDetections(); i++) {
    ObjectDetection *detection = parse->Detection(i);
    if (!CreateBestAssignment(parse, detection, min_score)) return 0;
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Labels = %d\n", parse->NLabels());
    printf("  # Models = %d\n", parse->NModels());
    printf("  # Detections = %d\n", parse->NDetections());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    printf("  # Assignments = %d\n", parse->NAssignments());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
CreateBestAssignmentForEachDetectionLabelPair(ObjectParse *parse, 
  R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating best assignment for each detection-label pair ...\n");
    fflush(stdout);
  }

  // Create point sets for all segmentations
  for (int i = 0; i < parse->NSegmentations(); i++) {
    ObjectSegmentation *segmentation = parse->Segmentation(i);
    if (segmentation->PointSet().NPoints() == 0) {
      R3Point origin = segmentation->Origin();
      ObjectPointSet pointset;
      if (!LoadSurfels(pointset, scene, origin, max_radius, 
        min_height, max_height, min_volume, max_volume, max_gap, min_points, max_points )) continue;
      segmentation->SetPointSet(pointset);
    }
  }

  // Create all assignments for all detections
  for (int i = 0; i < parse->NDetections(); i++) {
    ObjectDetection *detection = parse->Detection(i);
    for (int j = 0; j < parse->NLabels(); j++) {
      ObjectLabel *label = parse->Label(j);
      if (!CreateBestAssignment(parse, detection, label, min_score)) return 0;
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Labels = %d\n", parse->NLabels());
    printf("  # Models = %d\n", parse->NModels());
    printf("  # Detections = %d\n", parse->NDetections());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    printf("  # Assignments = %d\n", parse->NAssignments());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
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
    fprintf(stderr, "Unable to allocate scene\n");
    return NULL;
  }

  // Open scene files
  if (!scene->OpenFile(scene_name, database_name, "r", "r")) {
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
    printf("  # Surfels = %d\n", scene->Tree()->Database()->NSurfels());
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
// GSV I/O Functions
////////////////////////////////////////////////////////////////////////

static GSVScene *
ReadGSV(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate GSV scene
  GSVScene *gsv = new GSVScene();
  if (!gsv) {
    fprintf(stderr, "Unable to allocate GSV scene\n");
    return NULL;
  }

  // Read GSV scene 
  if (!gsv->ReadFile(filename)) {
    delete gsv;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read GSV from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Runs = %d\n", gsv->NRuns());
    printf("  # Cameras = %d\n", gsv->NCameras());
    printf("  # Lasers = %d\n", gsv->NLasers());
    printf("  # Segments = %d\n", gsv->NSegments());
    printf("  # Scans = %d\n", gsv->NScans());
    printf("  # Tapestries = %d\n", gsv->NTapestries());
    printf("  # Panoramas = %d\n", gsv->NPanoramas());
    printf("  # Images = %d\n", gsv->NImages());
    printf("  # Scanlines = %d\n", gsv->NScanlines());
    fflush(stdout);
  }

  // Return GSV scene
  return gsv;
}



////////////////////////////////////////////////////////////////////////
// Centers I/O Functions
////////////////////////////////////////////////////////////////////////

static int
ReadCenters(ObjectParse *parse, R3SurfelScene *scene, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open object parse file %s\n", filename);
    return 0;
  }

  // Read file
  char buffer[4096];
  int line_number = 0;
  RNArray<ObjectModel *> models;
  while (fgets(buffer, 4096, fp)) {
    // Check line
    line_number++;
    char *bufferp = buffer;
    while (*bufferp && isspace(*bufferp)) bufferp++;
    if (!bufferp) continue;
    if (*bufferp == '#') continue;

    // Parse line
    double cx, cy, cz, ax, ay, az;
    double confidence, predicted_label_value, object_identifier_value;
    double distance_to_ground_truth, ground_truth_label_value;
    if (sscanf(bufferp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &cx, &cy, &cz, 
      &confidence, &predicted_label_value, &object_identifier_value,
      &ax, &ay, &az, &distance_to_ground_truth, &ground_truth_label_value) != (unsigned int) 11) {
      fprintf(stderr, "Unable to read center on line %d of %s\n", line_number, filename);
      return 0;
    }

    // Get integer identifiers
    int predicted_label_index = (int) (predicted_label_value + 0.5);
    int ground_truth_label_index = (int) (ground_truth_label_value + 0.5);
    // int object_identifier = (int) (object_identifier_value + 0.5);

    // Get z coordinate
    cz = RN_UNKNOWN; // Input Z coordinates are not on ground in Honza's files
    if (cz == RN_UNKNOWN) cz = zsupport_coordinate;
    if ((cz == RN_UNKNOWN) && zsupport_grid) cz = zsupport_grid->WorldValue(cx, cy);
    if (cz == R2_GRID_UNKNOWN_VALUE) cz = RN_UNKNOWN;
    if (cz == RN_UNKNOWN) cz = EstimateSupportZ(scene, R3Point(cx, cy, 0));
    if (cz == RN_UNKNOWN) cz = scene->BBox().ZMin();

    // Create pointset
    ObjectPointSet pointset;
    if (!LoadSurfels(pointset, scene, R3Point(cx, cy, cz), max_radius, 
     min_height, max_height, min_volume, max_volume, max_gap, min_points, max_points )) continue;

    // Compute pointset origin
    R3Point origin = pointset.Centroid(); 
    origin[2] = cz;
    pointset.SetOrigin(origin);

    // Create detection
    char name[4096];
    sprintf(name, "D%d", parse->NDetections());
    ObjectDetection *detection = new ObjectDetection(name, origin, pointset.Radius(), pointset.Height());
    parse->InsertDetection(detection);

    // Create segmentation
    ObjectSegmentation *segmentation = new ObjectSegmentation(pointset, origin, pointset.Radius(), pointset.Height(), confidence);
    detection->InsertSegmentation(segmentation);
    
    // Create assignment for predicted label
    if (predicted_label_index > 0) {
      if (parse->NLabels() > predicted_label_index) {
        ObjectLabel *label = parse->Label(predicted_label_index);
        if (label->NModels() > 0) {
          ObjectModel *model = label->Model(0);
          
          // Compute transformation
          R3Vector xaxis(ax, ay, 0); xaxis.Normalize();
          R3Affine affine = R3identity_affine;
          affine.Translate(segmentation->Origin().Vector());
          affine.Rotate(R3posx_vector, xaxis);
          affine.Translate(-(model->Origin().Vector()));
          
          // Create ground truth assignment
          ObjectAssignment *assignment = new ObjectAssignment(segmentation, model, affine, 0.0, FALSE);
          segmentation->InsertAssignment(assignment);
          model->InsertAssignment(assignment);
        }
      }
    }

    // Create assignment for ground truth label
    if (ground_truth_label_index > 0) {
      if (parse->NLabels() > ground_truth_label_index) {
        ObjectLabel *label = parse->Label(ground_truth_label_index);
        if (label->NModels() > 0) {
          ObjectModel *model = label->Model(0);
          
          // Compute transformation
          R3Vector xaxis(ax, ay, 0); xaxis.Normalize();
          R3Affine affine = R3identity_affine;
          affine.Translate(segmentation->Origin().Vector());
          affine.Rotate(R3posx_vector, xaxis);
          affine.Translate(-(model->Origin().Vector()));
          
          // Create ground truth assignment
          ObjectAssignment *assignment = new ObjectAssignment(segmentation, model, affine, 1.0, TRUE);
          segmentation->InsertAssignment(assignment);
          model->InsertAssignment(assignment);
        }
      }
    }

    // Update statistics
    count++;
  }

  // Close file 
  fclose(fp);

  // Print statistics
  if (print_verbose) {
    printf("Read centers from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Centers = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Markers I/O Functions
////////////////////////////////////////////////////////////////////////

static int
ReadMarkers(ObjectParse *parse, R3SurfelScene *scene, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open object parse file %s\n", filename);
    return 0;
  }

  // Read file
  char buffer[4096];
  int line_number = 0;
  RNArray<ObjectModel *> models;
  while (fgets(buffer, 4096, fp)) {
    // Check line
    line_number++;
    char *bufferp = buffer;
    while (*bufferp && isspace(*bufferp)) bufferp++;
    if (!bufferp) continue;
    if (*bufferp == '#') continue;

    // Parse line
    int label_identifier;
    double cx, cy, cz, r, h;
    if (sscanf(bufferp, "%d%lf%lf%lf%lf%lf", &label_identifier, &cx, &cy, &cz, &r, &h) != (unsigned int) 6) {
      fprintf(stderr, "Unable to read marker on line %d of %s\n", line_number, filename);
      return 0;
    }

    // Get z coordinate
    if (cz == RN_UNKNOWN) cz = zsupport_coordinate;
    if ((cz == RN_UNKNOWN) && zsupport_grid) cz = zsupport_grid->WorldValue(cx, cy);
    if (cz == R2_GRID_UNKNOWN_VALUE) cz = RN_UNKNOWN;
    if (cz == RN_UNKNOWN) cz = EstimateSupportZ(scene, R3Point(cx, cy, 0));
    if (cz == RN_UNKNOWN) cz = scene->BBox().ZMin();

    // Create pointset
    ObjectPointSet pointset;
    if (!LoadSurfels(pointset, scene, R3Point(cx, cy, cz), r, 
     min_height, h, min_volume, max_volume, max_gap, min_points, max_points)) continue;

    // Compute pointset origin
    R3Point origin = pointset.Centroid(); 
    origin[2] = cz;
    pointset.SetOrigin(origin);

    // Create detection
    char name[4096];
    sprintf(name, "D%d", parse->NDetections());
    ObjectDetection *detection = new ObjectDetection(name, origin, pointset.Radius(), pointset.Height());
    parse->InsertDetection(detection);

    // Create segmentation
    ObjectSegmentation *segmentation = new ObjectSegmentation(pointset, origin, pointset.Radius(), pointset.Height(), 1.0);
    detection->InsertSegmentation(segmentation);

    // Create assignment
    if (label_identifier > 0) {
      const char *label_name = LabelNameFromMarkerIdentifier(label_identifier);
      if (label_name) {
        ObjectLabel *label = parse->Label(label_name);
        if (label) { 
          CreateBestAssignment(parse, detection, label, min_score);
        }
      }
    }

    // Update counter
    count++;
  }

  // Close file 
  fclose(fp);

  // Print statistics
  if (print_verbose) {
    printf("Read markers from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Markers = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Parse I/O functions
////////////////////////////////////////////////////////////////////////

static int
ReadParse(ObjectParse *parse, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Read parse
  if (!parse->ReadFile(filename)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("Read parse from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Labels = %d\n", parse->NLabels());
    printf("  # Models = %d\n", parse->NModels());
    printf("  # Detections = %d\n", parse->NDetections());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    printf("  # Assignments = %d\n", parse->NAssignments());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
WriteParse(ObjectParse *parse, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Write parse
  if (!parse->WriteFile(filename)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("Wrote parse to %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Labels = %d\n", parse->NLabels());
    printf("  # Models = %d\n", parse->NModels());
    printf("  # Detections = %d\n", parse->NDetections());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    printf("  # Assignments = %d\n", parse->NAssignments());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static ObjectParse *
CreateParse(R3SurfelScene *scene)
{
  // Allocate object parse
  ObjectParse *parse = new ObjectParse();
  if (!parse) {
    fprintf(stderr, "Unable to allocate object parse\n");
    return NULL;
  }

  // Read labels
  if (input_labels_name) {
    if (!ReadParse(parse, input_labels_name)) exit(-1);
  }

  // Read ground truth
  if (input_ground_truth_name) {
    if (!ReadParse(parse, input_ground_truth_name)) exit(-1);
  }

  // Read parse
  if (input_parse_name) {
    if (!ReadParse(parse, input_parse_name)) exit(-1);
  }

  // Read detections
  if (input_detections_name) {
    if (!ReadParse(parse, input_detections_name)) exit(-1);
  }

  // Read markers
  if (input_markers_name) {
    if (!ReadMarkers(parse, scene, input_markers_name)) exit(-1);
  }

  // Read centers
  if (input_centers_name) {
    if (!ReadCenters(parse, scene, input_centers_name)) exit(-1);
  }

  // Return parse
  return parse;
}



////////////////////////////////////////////////////////////////////////
// Rectangle I/O Functions
////////////////////////////////////////////////////////////////////////

static int
WriteRectangles(ObjectParse *parse, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open rectangles file %s\n", filename);
    return 0;
  }

  // Get base filename
  char basename[1024];
  const char *filenamep = strrchr(filename, '/');
  if (!filenamep) filenamep = filename;
  else filenamep++;
  strncpy(basename, filenamep, 1024);
  char *basenamep = strrchr(basename, '.');
  if (basenamep) *basenamep = '\0';

  // Write rectangles
  for (int i = 0; i < parse->NAssignments(); i++) {
    ObjectAssignment *assignment = parse->Assignment(i);
    ObjectModel *model = assignment->Model();
    ObjectSegmentation *segmentation = assignment->Segmentation();
    const R3Affine& transformation = assignment->ModelToSegmentationTransformation();
    RNScalar score = assignment->Score();
    
    // Write rectangle coordinates
    const ObjectPointSet& model_pointset = model->pointset;
    R3Box model_bbox = model_pointset.BBox();
    const ObjectPointSet& segmentation_pointset = segmentation->pointset;
    R3Box segmentation_bbox = segmentation_pointset.BBox();
    RNScalar zmin = segmentation->Origin().Z();
    RNScalar zmax = zmin + model->Height();
    if ((!segmentation_bbox.IsEmpty()) && (zmax > zmin + segmentation_bbox.ZLength()))
      zmax = zmin + segmentation_bbox.ZLength();
    if ((!model_bbox.IsEmpty()) && (zmax > zmin + model_bbox.ZLength()))
      zmax = zmin + model_bbox.ZLength();
    R3Point p1 = model_bbox.Corner(RN_NNN_OCTANT);
    R3Point p2 = model_bbox.Corner(RN_PNN_OCTANT);
    R3Point p3 = model_bbox.Corner(RN_PPN_OCTANT);
    R3Point p4 = model_bbox.Corner(RN_NPN_OCTANT);
    p1.Transform(transformation);
    p2.Transform(transformation);
    p3.Transform(transformation);
    p4.Transform(transformation);
    fprintf(fp, "%9.3f %9.3f   %9.3f %9.3f   %9.3f %9.3f   %9.3f %9.3f   %9.3f %9.3f  %6.3f   %s\n",
      p1.X(), p1.Y(), p2.X(), p2.Y(), p3.X(), p3.Y(), p4.X(), p4.Y(), zmin, zmax, score, basename);
  }

  // Close file
  fclose(fp);

  // Print statistics
  if (print_verbose) {
    printf("Wrote rectangles to %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Rectangles = %d\n", parse->NAssignments());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Segmentation I/O Functions
////////////////////////////////////////////////////////////////////////

static int
WriteSegmentations(ObjectParse *parse, const char *directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  char pointset_name[4096];

  // Create directory
  char cmd[4096];
  sprintf(cmd, "mkdir -p %s", directory);
  system(cmd);

  // Write all segmentations
  for (int i = 0; i < parse->NSegmentations(); i++) {
    ObjectSegmentation *segmentation = parse->Segmentation(i);
    const ObjectPointSet& pointset = segmentation->PointSet();
    if (pointset.NPoints() == 0) continue;
    // R3Point origin = segmentation->Origin();
    sprintf(pointset_name, "%s/S%d.xyzn", directory, i);
    pointset.WriteFile(pointset_name);
  }

  // Print statistics
  if (print_verbose) {
    printf("Wrote segmentations to %s ...\n", directory);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Grid I/O Functions
////////////////////////////////////////////////////////////////////////

#if 0
static int 
WriteGrid(R3Grid *grid, const char *grid_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Write grid
  int status = grid->WriteFile(grid_name);

  // Print statistics
  if (print_verbose) {
    printf("Wrote grid ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d %d\n", grid->XResolution(), grid->YResolution(), grid->ZResolution());
    printf("  Spacing = %g\n", grid->GridToWorldScaleFactor());
    printf("  Cardinality = %d\n", grid->Cardinality());
    printf("  Volume = %g\n", grid->Volume());
    RNInterval grid_range = grid->Range();
    printf("  Minimum = %g\n", grid_range.Min());
    printf("  Maximum = %g\n", grid_range.Max());
    printf("  L1Norm = %g\n", grid->L1Norm());
    printf("  L2Norm = %g\n", grid->L2Norm());
    fflush(stdout);
  }

  // Return status
  return status;
}
#endif



////////////////////////////////////////////////////////////////////////
// Image I/O Functions
////////////////////////////////////////////////////////////////////////

static R2Grid *
ReadImage(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate a grid
  R2Grid *grid = new R2Grid();
  if (!grid) {
    RNFail("Unable to allocate grid");
    return NULL;
  }

  // Read grid
  if (!grid->Read(filename)) {
    RNFail("Unable to read grid file %s", filename);
    delete grid;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read grid from %s\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", grid->XResolution(), grid->YResolution());
    printf("  Spacing = %g\n", grid->GridToWorldScaleFactor());
    printf("  Cardinality = %d\n", grid->Cardinality());
    RNInterval grid_range = grid->Range();
    printf("  Minimum = %g\n", grid_range.Min());
    printf("  Maximum = %g\n", grid_range.Max());
    printf("  L1Norm = %g\n", grid->L1Norm());
    printf("  L2Norm = %g\n", grid->L2Norm());
    fflush(stdout);
  }

  // Return success
  return grid;
}



static int
WriteImage(const R2Grid *grid, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Write grid
  if (!grid->Write(filename)) return 0;

  // Print statistics
  if (print_verbose) {
    printf("Wrote grid to %s\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", grid->XResolution(), grid->YResolution());
    printf("  Spacing = %g\n", grid->GridToWorldScaleFactor());
    printf("  Cardinality = %d\n", grid->Cardinality());
    RNInterval grid_range = grid->Range();
    printf("  Minimum = %g\n", grid_range.Min());
    printf("  Maximum = %g\n", grid_range.Max());
    printf("  L1Norm = %g\n", grid->L1Norm());
    printf("  L2Norm = %g\n", grid->L2Norm());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Z support grid creation functions
////////////////////////////////////////////////////////////////////////

static R2Grid *
CreateZSupportGrid(R3SurfelScene *scene)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating zsupport grid ...\n");
    fflush(stdout);
  }

  // Initialize grid
  R2Grid *grid = NULL;

  // Read zsupport grid
  if (input_zsupport_grid_name) {
    // Allocate a grid
    grid = new R2Grid();
    if (!grid) {
      fprintf(stderr, "Unable to allocate zsupport grid\n");
      return NULL;
    }

    // Read grid
    if (!grid->Read(input_zsupport_grid_name)) {
      fprintf(stderr, "Unable to read zsupport grid file %s\n", input_zsupport_grid_name);
      delete grid;
      return NULL;
    }
  }
  else {
    // Allocate grid
    R3Box bbox = scene->BBox();
    R2Box grid_box(bbox[0][0], bbox[0][1], bbox[1][0], bbox[1][1]);
    grid_box.Inflate(2);
    int xres = (int) (grid_box.XLength() / min_assignment_spacing) + 1;
    int yres = (int) (grid_box.YLength() / min_assignment_spacing) + 1;
    if (xres < 3) xres = 3;
    if (yres < 3) yres = 3;
    grid = new R2Grid(xres, yres, grid_box);
    if (!grid) {
      fprintf(stderr, "Unable to allocate zsupport grid\n");
      return NULL;
    }

    // Compute zsupport coordinate
    if (zsupport_coordinate == RN_UNKNOWN) {
      zsupport_coordinate = FLT_MAX;

      // This is a bit of a hack, based on sflsegment results
      if (scene->NObjects() > 0) {
        for (int i = 0; i < scene->NObjects(); i++) {
          R3SurfelObject *object = scene->Object(i);
          const R3Box &object_bbox = object->BBox();
          if (object_bbox.ZMin() - bbox.ZMin() > 0.25) continue;
          RNScalar zlength = object_bbox.ZLength();
          if (zlength > 0.25) continue;
          RNScalar xlength = object_bbox.XLength();
          if (xlength < 0.5) continue;
          RNScalar ylength = object_bbox.YLength();
          if (ylength < 0.5) continue;
          RNScalar npoints = 0;
          R3SurfelPointSet *pointset = object->PointSet();
          R3Plane plane = EstimateSupportPlane(pointset, 0.01, &npoints);
          delete pointset;
          if (npoints < 16) continue;
          if (RNIsNotEqual(fabs(plane.C()), 1.0, 0.1)) continue;
          R3Point centroid = object_bbox.Centroid();
          centroid.Project(plane);
          RNScalar z = centroid.Z();
          if (z < zsupport_coordinate) {
            zsupport_coordinate = z;
          }
        }
      }
      if (zsupport_coordinate == FLT_MAX) {
        zsupport_coordinate = bbox.ZMin();
      }
    }

    // Fill grid with constant zsupport coordinate
    grid->Clear(zsupport_coordinate);
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", grid->XResolution(), grid->YResolution());
    printf("  Spacing = %g\n", grid->GridToWorldScaleFactor());
    printf("  Cardinality = %d\n", grid->Cardinality());
    RNInterval grid_range = grid->Range();
    printf("  Minimum = %g\n", grid_range.Min());
    printf("  Maximum = %g\n", grid_range.Max());
    printf("  L1Norm = %g\n", grid->L1Norm());
    printf("  L2Norm = %g\n", grid->L2Norm());
    fflush(stdout);
  }

  // Write zsupport grid 
  if (output_zsupport_grid_name) {
    WriteImage(grid, output_zsupport_grid_name);
  }

  // Return grid
  return grid;
}



////////////////////////////////////////////////////////////////////////
// Search grid creation functions
////////////////////////////////////////////////////////////////////////

static int
AddVotesToGSVHoughGrid(R2Grid *max_grid, 
  GSVImage *image, const R2Grid *response_image, const R2Grid *depth_image,
  RNCoord z_coordinate)
{
  // Parameters
  int response_window_npixels = 8; 
  RNLength target_object_height = 2.0; // meters

  // Get frustum info in world coordinates
  GSVCamera *camera = image->Camera();
  const GSVPose& pose = image->Pose();
  RNAngle xfov = camera->XFov();
  R2Point viewpoint(pose.Viewpoint().X(), pose.Viewpoint().Y());
  R2Vector towards(pose.Towards().X(), pose.Towards().Y());

  // Get appropriate world distance for response image
  int response_image_npixels = response_image->YResolution();
  RNScalar response_window_fraction = (RNScalar) response_window_npixels / (RNScalar) response_image_npixels;
  RNScalar target_image_height = target_object_height / response_window_fraction;
  RNScalar target_depth = target_image_height / tan(camera->YFov());

  // Get box containing votes in grid coordinates
  R2Box votes_box = R2null_box;
  RNScalar one_over_cos_xfov = 1.0 / cos(camera->XFov());
  RNScalar min_depth = target_depth - 1.0;
  RNScalar max_depth = target_depth + 1.0;
  R2Vector left_vector = towards; left_vector.Rotate(xfov);
  R2Vector right_vector = towards; right_vector.Rotate(-xfov);
  votes_box.Union(max_grid->GridPosition(viewpoint + min_depth * towards));
  votes_box.Union(max_grid->GridPosition(viewpoint + min_depth * one_over_cos_xfov * left_vector));
  votes_box.Union(max_grid->GridPosition(viewpoint + min_depth * one_over_cos_xfov * right_vector));
  votes_box.Union(max_grid->GridPosition(viewpoint + max_depth * towards));
  votes_box.Union(max_grid->GridPosition(viewpoint + max_depth * one_over_cos_xfov * left_vector));
  votes_box.Union(max_grid->GridPosition(viewpoint + max_depth * one_over_cos_xfov * right_vector));
  votes_box.Intersect(max_grid->GridBox());

  // Add votes
  for (int ix = (int) votes_box.XMin(); ix < (int) votes_box.XMax(); ix++) {
    for (int iy = (int) votes_box.YMin(); iy < (int) votes_box.YMax(); iy++) {
      // Just checking
      assert((ix >= 0) && (ix < max_grid->XResolution()));
      assert((iy >= 0) && (iy < max_grid->YResolution()));

      // Get world position
      R2Point grid_position(ix, iy);
      R2Point slice_position = max_grid->WorldPosition(grid_position);
      R3Point world_position(slice_position.X(), slice_position.Y(), z_coordinate);

      // Get/check depth range
      R2Vector vector = slice_position - viewpoint;
      RNScalar depth = towards.Dot(vector);
      if (depth < min_depth) continue;
      if (depth > max_depth) continue;

      // Get/check image position
      R2Point image_position = image->UndistortedPosition(world_position);
      if ((image_position.X() < 0) || (image_position.X() >= image->Width())) continue;
      if ((image_position.Y() < 0) || (image_position.Y() >= image->Height())) continue;

      // Get/check occlusion
      if (depth_image) {
        RNScalar depth_value = pose.Towards().Dot(world_position - pose.Viewpoint());
        if (depth_value < 0) continue;
        RNScalar max_depth_value = depth_image->GridValue(image_position);
        if (max_depth_value != R2_GRID_UNKNOWN_VALUE) {
          if (depth_value > max_depth_value) continue;
        }
      }

      // Update grid
      RNScalar response_value = response_image->WorldValue(image_position);
      RNScalar old_response_value = max_grid->GridValue(ix, iy);
      if (response_value <= old_response_value) continue;
      max_grid->SetGridValue(ix, iy, response_value);
    }
  }

  // Return success
  return 1;
}



static R2Grid *
CreateGSVHoughGrid(R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Parameters
  const RNScalar threshold_value = 40000.0;
  const int start_im = 0;
  const int end_im = 45;

  // Check GSV filename
  if (!input_gsv_name) {
    fprintf(stderr, "Must provide GSV filename if create GSV hough grid\n");
    return NULL;
  }

  // Read GSV file
  GSVScene *gsv = ReadGSV(input_gsv_name);
  if (!gsv) {
    fprintf(stderr, "Unable to read GSV file %s\n", input_gsv_name);
    return NULL;
  }

  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating GSV grid ...\n");
    fflush(stdout);
  }

  // Allocate grids
  R3Box scene_bbox = scene->BBox();
  R2Box grid_bbox(scene_bbox[0][0], scene_bbox[0][1], scene_bbox[1][0], scene_bbox[1][1]);
  int xres = (int) (grid_bbox.XLength() / min_assignment_spacing + 0.5);
  int yres = (int) (grid_bbox.YLength() / min_assignment_spacing + 0.5);
  if ((xres == 0) || (yres == 0)) return 0;
  R2Grid sum_grid(xres, yres, grid_bbox);
  R2Grid count_grid(xres, yres, grid_bbox);
  R2Grid viewpoint_mask(xres, yres, grid_bbox);

  // Fill grids
  for (int ir = 0; ir < gsv->NRuns(); ir++) {
    GSVRun *run = gsv->Run(ir);
    for (int is = 0; is < run->NSegments(); is++) {
      GSVSegment *segment = run->Segment(is);
      for (int ip = 0; ip < segment->NPanoramas(); ip++) {
        GSVPanorama *panorama = segment->Panorama(ip);
        for (int ii = 0; ii < panorama->NImages(); ii++) {
          GSVImage *image = panorama->Image(ii);

          // TEMPORARY
          if (ir != 0) continue;
          if (is != 0) continue;
          if (ip > 20) continue;
          if (ii > 4) continue;

          // Update viewpoint mask
          viewpoint_mask.RasterizeWorldPoint(image->Pose().Viewpoint().X(), image->Pose().Viewpoint().Y(), 1.0);

          // Inialize grid
          R2Grid max_grid(xres, yres, grid_bbox);

          // Fill grid
          int im_count = 0;
          for (int im = start_im; im <= end_im; im++) {
            // Read response image
            R2Grid response_image;
            char response_name[4096];
            sprintf(response_name, "gsv_data/dpm_response/%s/segment%02d/%06d/%06d_%02d_%03d.png", run->Name(), is, ip, ip, ii, im);
            if (!RNFileExists(response_name)) break; // XXX TEMPORARY XXX
            if (!response_image.ReadFile(response_name)) continue;

            // Set response image transformation
            RNScalar scale = (RNScalar) response_image.XResolution() / (RNScalar) image->Width();
            R2Affine image_to_response = R2identity_affine;
            image_to_response.Scale(scale);
            response_image.SetWorldToGridTransformation(image_to_response);

            // Process response image
            response_image.Blur(1);
            response_image.MaskNonMaxima(1);
            response_image.Subtract(threshold_value);
            response_image.Threshold(0, 0, R2_GRID_KEEP_VALUE);

            // Read depth image
            // R2Grid depth_image;
            // char depth_name[4096];
            // sprintf(depth_name, "gsv_data/dpm_depth/%s/segment%02d/%06d/%06d_%02d_%03d.png", run->Name(), is, ip, ip, ii, im);
            // if (!depth_image.ReadFile(depth_name)) continue;

            // Add votes to grid
            for (double delta_z = 0.5; delta_z < 2.5; delta_z += 0.25) {
              RNScalar z = image->Pose().Viewpoint().Z() - delta_z;
              AddVotesToGSVHoughGrid(&max_grid, image, &response_image, NULL, z);
            }

            // Count number of scales actually considered
            im_count++;
          }

          // Check if computed any grids
          if (im_count == 0) continue;

          // Print debug statement
          if (print_debug) {
            printf("%02d %02d %06d %02d : %g %d\n", ir, is, ip, ii, max_grid.L1Norm(), max_grid.Cardinality());
          }

          // Blur max grid
          max_grid.Blur(1);
          
          // TEMPORARY
          // char tmp_name[4096];
          // sprintf(tmp_name, "foo%06d_%02d.grd", ip, ii);
          // max_grid.WriteFile(tmp_name);

          // Sum votes
          sum_grid.Add(max_grid);
          max_grid.Threshold(RN_EPSILON, 0, 1);
          count_grid.Add(max_grid);
        }
      }
    }
  }

  // Create output grid
  R2Grid *grid = new R2Grid(sum_grid);
  grid->Divide(count_grid);
  viewpoint_mask.Threshold(RN_EPSILON, 1, R2_GRID_UNKNOWN_VALUE);
  grid->Mask(viewpoint_mask);

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", grid->XResolution(), grid->YResolution());
    printf("  Spacing = %g\n", grid->GridToWorldScaleFactor());
    printf("  Cardinality = %d\n", grid->Cardinality());
    RNInterval grid_range = grid->Range();
    printf("  Minimum = %g\n", grid_range.Min());
    printf("  Maximum = %g\n", grid_range.Max());
    printf("  L1Norm = %g\n", grid->L1Norm());
    printf("  L2Norm = %g\n", grid->L2Norm());
    fflush(stdout);
  }

  // Write gsv grid 
  if (output_gsv_grid_name) {
    WriteImage(grid, output_gsv_grid_name);
  }

  // Delete gsv stuff
  delete gsv;

  // Return grid
  return grid;
}



static R2Grid *
CreateDensityGrid(R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return NULL;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return NULL;

  // Allocate grid
  R3Box bbox = scene->BBox();
  R2Box grid_box(bbox[0][0], bbox[0][1], bbox[1][0], bbox[1][1]);
  int xres = (int) (grid_box.XLength() / min_assignment_spacing) + 1;
  int yres = (int) (grid_box.YLength() / min_assignment_spacing) + 1;
  if (xres < 3) xres = 3;
  if (yres < 3) yres = 3;
  R2Grid *grid = new R2Grid(xres, yres, grid_box);
  if (!grid) {
    fprintf(stderr, "Unable to allocate density grid\n");
    return NULL;
  }

  // Compute groundZ
  RNScalar groundZ = zsupport_grid.Mean();

  // Fill grid
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    if (node->NParts() > 0) continue;
    for (int j = 0; j < node->NBlocks(); j++) {
      R3SurfelBlock *block = node->Block(j);
      R3SurfelPointSet pointset(block);

      // Check if large, vertical surface (avoid walls)
      RNScalar value = 1.0;
      if (superpixels) {
        RNScalar variances[3];
        R3Point centroid = pointset.Centroid();
        R3Triad axes = pointset.PrincipleAxes(&centroid, variances);
        value = fabs(axes[2].Z());
        if (RNIsZero(axes[2].Z(), 0.1)) {
          if (block->BBox().ZMax() - groundZ > 1.5 * max_height) { 
            if ((variances[0] > 0.1) && (variances[2] < 0.1)) {
              value = -1.0;
            }
          }
        }
      }

      // Rasterize points
      for (int j = 0; j < pointset.NPoints(); j++) {
        R3SurfelPoint *point = pointset.Point(j);
        R3Point position = point->Position();

        // Check height
        double z = position.Z();
        if ((min_height != RN_UNKNOWN) || (max_height != RN_UNKNOWN)) {
          RNScalar zsupport = zsupport_grid.WorldValue(position.X(), position.Y());
          if (zsupport != R2_GRID_UNKNOWN_VALUE) {
            double height = z - zsupport;
            if ((min_height != RN_UNKNOWN) && (height < min_height)) continue;
            if ((max_height != RN_UNKNOWN) && (height > max_height)) continue;
          }
        }

        // Add point
        grid->RasterizeWorldPoint(position.X(), position.Y(), value);
      }
    }
  }

  // Blur to avoid aliasing
  grid->Blur(1);

  // Mask sparse areas
  RNScalar scale = grid->GridToWorldScaleFactor();
  RNScalar min_points_per_grid_cell = scale * scale * min_points_per_square_meter;
  grid->Threshold(min_points_per_grid_cell, 0, R2_GRID_KEEP_VALUE);

  // Return grid
  return grid;
}



static R2Grid *
CreateUniformGrid(R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Allocate grid
  R3Box bbox = scene->BBox();
  R2Box grid_box(bbox[0][0], bbox[0][1], bbox[1][0], bbox[1][1]);
  int xres = (int) (grid_box.XLength() / min_assignment_spacing) + 1;
  int yres = (int) (grid_box.YLength() / min_assignment_spacing) + 1;
  if (xres < 3) xres = 3;
  if (yres < 3) yres = 3;
  R2Grid *grid = new R2Grid(xres, yres, grid_box);
  if (!grid) {
    fprintf(stderr, "Unable to allocate density grid\n");
    return NULL;
  }

  // Fill grid
  grid->Clear(1);

  // Return grid
  return grid;
}



static R2Grid *
CreateSearchGrid(R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating search grid ...\n");
    fflush(stdout);
  }

  // Initialize result
  R2Grid *grid = NULL;

  // Create search grid
  if (input_search_grid_name) {
    // Read search grid
    int saved_print_verbose = print_verbose;
    print_verbose = 0;
    grid = ReadImage(input_search_grid_name);
    print_verbose = saved_print_verbose;
    if (!grid) return NULL;
  }
  else if (create_gsv_hough_search_grid) {
    // Create GSV Hough grid
    grid = CreateGSVHoughGrid(scene, zsupport_grid);
    if (!grid) return NULL;
  }
  else if (create_uniform_search_grid) {
    // Create uniform grid
    grid = CreateUniformGrid(scene, zsupport_grid);
    if (!grid) return NULL;
  }
  else {
    // Create density grid
    grid = CreateDensityGrid(scene, zsupport_grid);
    if (!grid) return NULL;
  }

  // Mask non-maxima
  if (mask_nonmaxima_in_search_grid) {
    grid->MaskNonMaxima(1);
    grid->Substitute(R2_GRID_UNKNOWN_VALUE, 0);
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Resolution = %d %d\n", grid->XResolution(), grid->YResolution());
    printf("  Spacing = %g\n", grid->GridToWorldScaleFactor());
    printf("  Cardinality = %d\n", grid->Cardinality());
    RNInterval grid_range = grid->Range();
    printf("  Minimum = %g\n", grid_range.Min());
    printf("  Maximum = %g\n", grid_range.Max());
    printf("  L1Norm = %g\n", grid->L1Norm());
    printf("  L2Norm = %g\n", grid->L2Norm());
    fflush(stdout);
  }

  // Write search grid 
  if (output_search_grid_name) {
    WriteImage(grid, output_search_grid_name);
  }

  // Return search grid
  return grid;
}



////////////////////////////////////////////////////////////////////////
// Surfel object functions
////////////////////////////////////////////////////////////////////////

static ObjectDetection *
CreateDetectionForSurfelObject(ObjectParse *parse, R3SurfelScene *scene, R3SurfelObject *object, const R2Grid& zsupport_grid)
{
  // Get object info
  R3Box object_bbox = object->BBox();
  RNScalar height = object_bbox.ZLength();
  RNScalar dx = object_bbox.XRadius();
  RNScalar dy = object_bbox.YRadius();
  RNScalar radius = sqrt(dx*dx + dy*dy);

  // Create surfels
  R3SurfelPointSet *surfels = object->PointSet();
  if (surfels->NPoints() < min_points) { delete surfels; return 0; }

  // Create object pointset
  ObjectPointSet object_pointset;
  LoadSurfels(object_pointset, scene, surfels, max_points);
  if (object_pointset.NPoints() < min_points) { delete surfels; return 0; }
  if (object_pointset.NPoints() == 0) { delete surfels; return 0; }

  // Compute detection origin
  R3Point origin = object_pointset.Centroid(); 
  RNScalar cz = zsupport_grid.WorldValue(origin.X(), origin.Y());
  if (cz != R2_GRID_UNKNOWN_VALUE) origin[2] = cz;
  else if (zsupport_coordinate != RN_UNKNOWN) origin[2] = zsupport_coordinate;
  else origin[2] = object_bbox.ZMin();
  object_pointset.SetOrigin(origin);

#if 1
  // Create scene pointset
  ObjectPointSet neighborhood_pointset;
  if (!LoadSurfels(neighborhood_pointset, scene, origin, max_radius, 
    min_height, max_height, min_volume, max_volume, 
    max_gap, min_points, max_points )) return 0;

  // Create combined pointset
  for (int i = 0; i < neighborhood_pointset.NPoints(); i++) {
    ObjectPoint *point = neighborhood_pointset.Point(i);
    object_pointset.InsertPoint(*point);
  }
#endif

  // Create detection
  ObjectDetection *detection = new ObjectDetection(object->Name(), origin, radius, height); 
  parse->InsertDetection(detection);

  // Create segmentation
  ObjectSegmentation *segmentation = new ObjectSegmentation(object_pointset, origin, radius, height); 
  detection->InsertSegmentation(segmentation);

  // Delete surfels
  delete surfels;

  // Return detection
  return detection;
}



static int
CreateDetectionForEachSurfelObject(ObjectParse *parse, R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Creating object detection for each surfel object ...\n");
    fflush(stdout);
  }

  // Create detections for all objects
  for (int i = 1; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);

    // Check if object already has a label
    if (object->HumanLabel()) continue;
    if (object->PredictedLabel()) {
      R3SurfelLabel *label = object->PredictedLabel();
      if (strcmp(label->Name(), "Null") && strcmp(label->Name(), "Unknown")) continue;
    }

    // Check bounding box
    if (!detections_bbox.IsEmpty() && !R3Intersects(object->BBox(), detections_bbox)) return 0;

    // Create detection
    if (!CreateDetectionForSurfelObject(parse, scene, object, zsupport_grid)) return 0;

    // Update statistics
    count++;
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # New Detections = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
CreateBestAssignmentForEachSurfelObject(ObjectParse *parse, 
  R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Creating best assignment for each surfel object ...\n");
    fflush(stdout);
  }

  // Create detection, segment, and assignment for each object
  for (int i = 1; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);

    // Check bounding box
    if (!detections_bbox.IsEmpty() && !R3Intersects(object->BBox(), detections_bbox)) return 0;

    // Create detection
    ObjectDetection *detection = CreateDetectionForSurfelObject(parse, scene, object, zsupport_grid);
    if (!detection) continue;

    // Create best assignment
    R3SurfelLabel *surfel_label = object->HumanLabel();
    // R3SurfelLabel *surfel_label = object->CurrentLabel();
    const char *label_name = (surfel_label) ? surfel_label->Name() : NULL;
    ObjectLabel *label = (label_name) ? parse->Label(label_name) : NULL;
    // if (!label) continue;
    if (label) CreateBestAssignment(parse, detection, label, min_score);
    else CreateBestAssignment(parse, detection, min_score);

    // Update statistics
    count++;
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # New Assignments = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Labels = %d\n", parse->NLabels());
    printf("  # Models = %d\n", parse->NModels());
    printf("  # Detections = %d\n", parse->NDetections());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    printf("  # Assignments = %d\n", parse->NAssignments());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
CreateAssignmentsUsingSearchGrid(ObjectParse *parse, R3SurfelScene *scene, const R2Grid& zsupport_grid, const R2Grid& search_grid)
{
  // Create detections
  for (int ix = 0; ix < search_grid.XResolution(); ix++) {
    for (int iy = 0; iy < search_grid.YResolution(); iy++) {
      // Get/check grid value
      RNScalar value = search_grid.GridValue(ix, iy);
      if (value == R2_GRID_UNKNOWN_VALUE) continue;
      if (value <= 0) continue;

      // Initialize everything (create best assignment for grid cell)
      RNScalar best_score = min_score;
      ObjectSegmentation *best_segmentation = NULL;
      ObjectAssignment *best_assignment = NULL;

      // Compute center of grid cell
      R2Point center = search_grid.WorldPosition(R2Point(ix, iy));

      // Check bounding box
      if (!detections_bbox.IsEmpty()) {
        R3Point tmp(center.X(), center.Y(), detections_bbox.ZCenter());
        if (!R3Intersects(tmp, detections_bbox)) continue;
      }

      // Get z coordinate
      RNScalar sz = zsupport_grid.WorldValue(center.X(), center.Y());
      if (sz == R2_GRID_UNKNOWN_VALUE) sz = scene->BBox().ZMin();
      
      // Consider different segmentations centered at points within grid cell
      for (int iter = 0; iter < max_samples_per_grid_cell; iter++) { 
        // Get seed center point
        RNScalar cell_radius = 0.5 * search_grid.GridToWorldScaleFactor();
        RNScalar sx = center.X() + (2.0 * RNRandomScalar() - 1.0) * cell_radius;
        RNScalar sy = center.Y() + (2.0 * RNRandomScalar() - 1.0) * cell_radius;

        // Create pointset
        ObjectPointSet pointset;
        if (!LoadSurfels(pointset, scene, R3Point(sx, sy, sz), max_radius, 
          min_height, max_height, min_volume, max_volume, max_gap, min_points, max_points )) continue;

        // Compute pointset origin
        R3Point origin = pointset.Centroid(); 
        origin[2] = sz;
        pointset.SetOrigin(origin);
        
        // Create point set properties
        RNLength radius = pointset.Radius();
        if (radius < min_radius) radius = min_radius;
        if (radius > max_radius) radius = max_radius;
        RNLength height = pointset.Height();
        if (height < min_height) height = min_height;
        if (height > max_height) height = max_height;

        // Create candidate segmentation
        ObjectSegmentation *segmentation = new ObjectSegmentation(pointset, origin, radius, height);

        // Consider assignment to each model
        for (int i = 0; i < parse->NModels(); i++) {
          ObjectModel *model = parse->Model(i);

          // Check if best model for this grid cell location
          RNScalar score = 0;
          R3Affine transformation = R3identity_transformation;
          if (ComputeBestTransformation(segmentation, model, transformation, score)) {
            if (score > best_score) {
              if (best_assignment) delete best_assignment;
              if (best_segmentation) delete best_segmentation;
              best_segmentation = new ObjectSegmentation(*segmentation);
              best_assignment = new ObjectAssignment(best_segmentation, model, transformation, score);
              best_score = score;
            }
            
            // Print message
            if (print_debug) {
              printf("  %d %d  : %g %g %g %s : %g\n", 
                ix, iy, origin[0], origin[1], origin[2], 
                model->Label()->Name(), score);
            }
          }
        }

        // Delete segmentation
        delete segmentation;
      }

      // Insert into parse
      if (best_segmentation && best_assignment) {
        char name[4096];
        sprintf(name, "D%d", parse->NDetections());
        R3Point origin = best_segmentation->Origin();
        RNLength radius = best_segmentation->Radius();
        RNLength height = best_segmentation->Height();
        ObjectDetection *best_detection = new ObjectDetection(name, origin, radius, height);
        parse->InsertDetection(best_detection);
        best_detection->InsertSegmentation(best_segmentation);
        best_segmentation->InsertAssignment(best_assignment);
        ObjectModel *best_model = best_assignment->Model();
        best_model->InsertAssignment(best_assignment);

        // Write debug statement
        if (print_debug) {
          printf("%d/%d %d/%d : %g %g %g : %d %g %g : %s %g\n", ix, search_grid.XResolution(), iy, search_grid.YResolution(), 
            best_segmentation->Origin().X(), best_segmentation->Origin().Y(), best_segmentation->Origin().Z(), 
            best_segmentation->PointSet().NPoints(), best_segmentation->Radius(), best_segmentation->Height(),
            best_model->Label()->Name(), best_assignment->Score());
        }
      }
    }
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Convolution functions
////////////////////////////////////////////////////////////////////////

#include "fft/fft.h"



int 
R2ForwardFFT(const R2Grid& grid)
{
  // Spatial domain -> Frequency domain
  int n1 = grid.YResolution();
  int n2 = grid.XResolution();
  double *data = (double *) grid.GridValues();
  R2ForwardFFT(n1, n2, data);
  return 1;
}



int 
R2ReverseFFT(const R2Grid& grid)
{
  // Frequency domain -> Spatial domain
  int n1 = grid.YResolution();
  int n2 = grid.XResolution();
  double *data = (double *) grid.GridValues();
  R2ReverseFFT(n1, n2, data);
  return 1;
}



RNBoolean
IsPointCompatible(R3SurfelPoint *point) 
{
  // Returns whether a point can be part of an object
  R3SurfelBlock *block = point->Block();
  R3SurfelNode *node = (block) ? block->Node() : NULL;
  R3SurfelObject *object = (node) ? node->Object() : NULL;
  if (!object) return TRUE;
  const R3Box& bbox = object->BBox();
  RNScalar dz = bbox.ZLength();
  if (dz < min_height) return TRUE;
  if (dz > max_height) return FALSE;
  RNScalar dx = bbox.XRadius();
  RNScalar dy = bbox.YRadius();
  if (dx*dx + dy*dy > max_radius*max_radius) return FALSE;
  return TRUE;
}



int
CreateAssignmentsUsingConvolution(
  R3SurfelScene *scene, ObjectParse *parse, const R2Grid& zsupport_grid)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int assignment_count = 0;
  if (print_verbose) {
    printf("Creating assignments using convolution ...\n");
    fflush(stdout);
  }

  // Get zsupport plane
  RNCoord zsupport_coordinate = zsupport_grid.Median();
  R3Plane zsupport_plane(0, 0, 1, -zsupport_coordinate);
  RNScalar zsupport_min = zsupport_coordinate + min_height;

  // Get scene bounding box (so that grid will be power of two at least twice as large as needed)
  R3Box scene_bbox = scene->BBox();
  scene_bbox[0][0] -= 2 * max_radius;
  scene_bbox[0][1] -= 2 * max_radius;
  scene_bbox[0][2] = zsupport_coordinate + min_height;
  scene_bbox[1][0] += 2 * max_radius;
  scene_bbox[1][1] += 2 * max_radius;
  scene_bbox[1][2] = zsupport_coordinate + max_height;
  int min_xres = (int) (2.0 * (scene_bbox.XLength() / rasterization_spacing)) + 1;
  int min_yres = (int) (2.0 * (scene_bbox.YLength() / rasterization_spacing)) + 1;
  int xres = 4; int yres = 4;
  while (xres < min_xres) { xres *= 2; }
  while (yres < min_yres) { yres *= 2; }
  scene_bbox[1][0] = scene_bbox[0][0] + rasterization_spacing * xres;
  scene_bbox[1][1] = scene_bbox[0][1] + rasterization_spacing * yres;

  // Get scene origin
  R3Point scene_origin = scene_bbox.Min();
  scene_origin[2] = zsupport_coordinate;

  // Create planar grids
  R3PlanarGrid scene_grid(zsupport_plane, scene_bbox, scene_origin, R3posy_vector, rasterization_spacing);
  R3PlanarGrid model_grid(scene_grid);
  R3PlanarGrid slice_grid(scene_grid);
  R3PlanarGrid detection_grid(scene_grid);
  assert(scene_grid.XResolution() == xres);
  assert(scene_grid.YResolution() == yres);

  // Consider every model
  for (int m = 0; m < parse->NModels(); m++) {
    ObjectModel *model = parse->Model(m);

    // Consider every rotation
    for (int r = 0; r < nrotations; r++) {
      RNAngle rotation = r * RN_TWO_PI / nrotations;
      
      // Compute the model_to_scene transformation
      R3Affine model_to_scene = R3identity_affine;
      R3Vector model_offset(model->Radius(), model->Radius(), 0);
      model_to_scene.Translate(model_offset);
      model_to_scene.Translate(scene_origin.Vector());
      model_to_scene.ZRotate(rotation);
      model_to_scene.Translate(-(model->Origin().Vector()));

      // Get useful variables
      RNLength zrange = model->PointSet().BBox().ZLength();
      int zslices =  zrange / rasterization_spacing;
      if (zslices <= 0) zslices = 1;
      RNScalar zstep = zrange / zslices;
      RNLength max_distance = 3 * alignment_spacing;
      RNScalar exp_factor = -1.0 / (2.0 * alignment_spacing * alignment_spacing);
      RNScalar dxxyy_factor = scene_grid.GridToWorldScaleFactor() * scene_grid.GridToWorldScaleFactor();
      RNScalar total_model_value = 0;
      RNScalar noise_threshold = 1.0;
      int rasterization_radius = (int) (2 * alignment_spacing * scene_grid.WorldToGridScaleFactor());

      // printf("HEREA %d %d\n", m, r); 

      // Initialize detection grid
      detection_grid.Clear(0);

      // Create detection grid slice-by-slice
      for (int s = 0; s < zslices; s++) {
        // Compute plane
        RNScalar slice_z = zsupport_coordinate + (s+0.5) * zstep;
        R3Plane slice_plane(0, 0, 1, -slice_z);

        // printf("  HERE1 %d %d %d\n", m, r, s); 

        // Create scene pointset
        RNInterval range(slice_z - alignment_spacing, slice_z + alignment_spacing);
        if (range.Min() < zsupport_min) range.SetMin(zsupport_min);
        R3SurfelCoordinateConstraint constraint(RN_Z, range);
        R3SurfelPointSet *scene_pointset = CreatePointSet(scene, NULL, &constraint);

        // Check pointset
        if (!scene_pointset) continue;
        if (scene_pointset->NPoints() == 0) { 
          delete scene_pointset; 
          continue;
        }

        // printf("  HERE2 %d %d %d : %d\n", m, r, s, scene_pointset->NPoints()); 

        // Compute scene grid for slice
        int scene_count = 0;
        scene_grid.Clear(0);
        for (int i = 0; i < scene_pointset->NPoints(); i++) {
          R3SurfelPoint *point = scene_pointset->Point(i);
          const R3Point& position = point->Position();
          RNScalar dz = R3Distance(slice_plane, position);
          if (dz > max_distance) continue;
          R2Point grid_position = scene_grid.GridPosition(position);
          int cx = (int) (grid_position.X() + 0.5);
          int cy = (int) (grid_position.Y() + 0.5);
          for (int dx = -rasterization_radius; dx <= rasterization_radius; dx++) {
            int ix = cx + dx;
            if (ix < 0) continue;
            if (ix >= scene_grid.XResolution()) continue;
            for (int dy = -rasterization_radius; dy <= rasterization_radius; dy++) {
              int iy = cy + dy;
              if (iy < 0) continue;
              if (iy >= scene_grid.YResolution()) continue;
              RNScalar dxxyy = (dx*dx + dy*dy) * dxxyy_factor;
              RNScalar dd = dxxyy + dz*dz;
              RNScalar value = exp(exp_factor*dd);
              if (RNIsZero(value)) continue;
              RNScalar old_value = fabs(scene_grid.GridValue(ix, iy));
              if (value > old_value) {
                if (!IsPointCompatible(point)) value = -value;
                scene_grid.SetGridValue(ix, iy, value);
                scene_count++;
              }
            }
          }
        }

        // Delete scene pointset
        delete scene_pointset;

        // Check if any scene points were rasterized
        if (scene_count == 0) continue;

        // printf("  HERE3 %d %d %d : %g\n", m, r, s, scene_grid.L1Norm()); 

        // Compute model grid for slice
        int model_count = 0;
        model_grid.Clear(0);
        for (int i = 0; i < model->PointSet().NPoints(); i++) {
          ObjectPoint *point = model->PointSet().Point(i);
          R3Point position = point->Position();
          position.Transform(model_to_scene);
          RNScalar dz = R3Distance(slice_plane, position);
          if (dz > max_distance) continue;
          R2Point grid_position = model_grid.GridPosition(position);
          int cx = (int) (grid_position.X() + 0.5);
          int cy = (int) (grid_position.Y() + 0.5);
          for (int dx = -rasterization_radius; dx <= rasterization_radius; dx++) {
            int ix = cx + dx;
            if (ix < 0) continue;
            if (ix >= model_grid.XResolution()) continue;
            for (int dy = -rasterization_radius; dy <= rasterization_radius; dy++) {
              int iy = cy + dy;
              if (iy < 0) continue;
              if (iy >= model_grid.YResolution()) continue;
              RNScalar dxxyy = (dx*dx + dy*dy) * dxxyy_factor;
              RNScalar dd = dxxyy + dz*dz;
              RNScalar value = exp(exp_factor*dd);
              if (RNIsZero(value)) continue;
              RNScalar old_value = model_grid.GridValue(ix, iy);
              if (value > old_value) {
                model_grid.SetGridValue(ix, iy, value);
                total_model_value += value - old_value;
                model_count++;
              }
            }
          }
        }

        // Check if any model points were rasterized
        if (model_count == 0) continue;

        // printf("  HERE4 %d %d %d : %g\n", m, r, s, model_grid.L1Norm()); 

#if 0
        // Temporary
        char buffer[4096];
        sprintf(buffer, "S_%d_%d_%d.grd", m, r, s);
        scene_grid.WriteFile(buffer);
        sprintf(buffer, "M_%d_%d_%d.grd", m, r, s);
        model_grid.WriteFile(buffer);
#endif

        // printf("  HERE5 %d %d %d : %g\n", m, r, s, model_grid.L1Norm()); 

        // Compute the forward Fourier transforms
        R2ForwardFFT(scene_grid.grid);
        R2ForwardFFT(model_grid.grid);

        // printf("  HERE6 %d %d %d : %g\n", m, r, s, model_grid.L1Norm()); 

        // Compute slice grid
        slice_grid.Clear(0);
        for (int k1 = 0; k1 < yres; k1++) {
          for (int k2 = 0; k2 < xres/2; k2++) {
            double sr = scene_grid.GridValue(2*k2, k1);
            double sc = scene_grid.GridValue(2*k2+1, k1);
            double fr = model_grid.GridValue(2*k2, k1);
            double fc = model_grid.GridValue(2*k2+1, k1);
            double r = sc*fc + sr*fr;
            double c = sc*fr - sr*fc;
            slice_grid.SetGridValue(2*k2, k1, r);
            slice_grid.SetGridValue(2*k2+1, k1, c);
          }
        }

        // printf("  HERE7 %d %d %d : %g\n", m, r, s, slice_grid.L1Norm()); 

        // Compute the inverse Fourier transform
        R2ReverseFFT(slice_grid.grid);

        // printf("  HERE8 %d %d %d : %g\n", m, r, s, slice_grid.L1Norm()); 

#if 0
        // Temporary
        sprintf(buffer, "D_%d_%d_%d.grd", m, r, s);
        slice_grid.WriteFile(buffer);
#endif

        // Remove noise
        slice_grid.Threshold(noise_threshold, 0.0, R2_GRID_KEEP_VALUE);

        // Add to the detection grid
        detection_grid.Add(slice_grid);

        // printf("  HERE9 %d %d %d : %g\n", m, r, s, detection_grid.L1Norm()); 
      }

      // printf("HEREB %d %d : %g\n", m, r, detection_grid.L1Norm()); 

      // Suppress low values 
      detection_grid.Threshold(noise_threshold, 0.0, R2_GRID_KEEP_VALUE);
      
      // Normalize detection grid 
      assert(total_model_value > 0);
      detection_grid.Divide(total_model_value);

#if 0
      // Write detection grid
      char name[1024];
      sprintf(name, "d_%d_%d.grd", model->ParseIndex(), r);
      detection_grid.WriteFile(name);
#endif
  
      // Suppress nonmaxima
      RNScalar grid_radius = 1.0;
      detection_grid.MaskNonMaxima(grid_radius);
      detection_grid.Substitute(R2_GRID_UNKNOWN_VALUE, 0.0);
      
      // printf("HEREC %d %d : %g\n", m, r, detection_grid.L1Norm()); 
      
      // Create assignments
      for (int ix = 0; ix < detection_grid.XResolution(); ix++) {
        for (int iy = 0; iy < detection_grid.YResolution(); iy++) {
          RNScalar score = detection_grid.GridValue(ix, iy);
          if (score == R2_GRID_UNKNOWN_VALUE) continue;
          if (score <= min_score) continue;
          if (score <= 0) continue;

          // Get candidate detection point
          R3Point segmentation_origin = detection_grid.WorldPosition(R2Point(ix + 0.5, iy + 0.5));
          segmentation_origin += model_offset;
          if (!R3Contains(scene->BBox(), segmentation_origin)) continue;
          
          // Create detection
          char detection_name[4096];
          sprintf(detection_name, "D%d", parse->NDetections());
          ObjectDetection *detection = new ObjectDetection(detection_name, segmentation_origin);
          parse->InsertDetection(detection);
          
          // Create segmentation
          ObjectPointSet segmentation_pointset;
          ObjectSegmentation *segmentation = new ObjectSegmentation(segmentation_pointset, segmentation_origin);
          detection->InsertSegmentation(segmentation);
          
          // Compute transformation
          R3Affine assignment_transformation = R3identity_affine;
          assignment_transformation.Translate(segmentation->Origin().Vector());
          assignment_transformation.ZRotate(rotation);
          assignment_transformation.Translate(-(model->Origin().Vector()));
          
          // Create assignment
          ObjectAssignment *assignment = new ObjectAssignment(segmentation, model, assignment_transformation, score);
          segmentation->InsertAssignment(assignment);
          model->InsertAssignment(assignment);
          assignment_count++;

          // Print a message
          if (print_debug) {
            ObjectLabel *label = model->Label();
            printf("%d %d : %g %g %g : %g %g\n", label->ParseIndex(), model->LabelIndex(), 
              segmentation_origin.X(), segmentation_origin.Y(), segmentation_origin.Z(), 
              rotation, score);
          }
        }
      }
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  Grid Resolution = %d %d\n", detection_grid.XResolution(), detection_grid.YResolution());
    printf("  Grid Spacing = %g\n", detection_grid.GridToWorldScaleFactor());
    printf("  # Rotations = %d\n", nrotations);
    printf("  # Labels = %d\n", parse->NLabels());
    printf("  # Models = %d\n", parse->NModels());
    printf("  # Detections = %d\n", parse->NDetections());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    printf("  # Assignments = %d\n", parse->NAssignments());
    printf("  # New Assignments = %d\n", assignment_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Assignment creation functions
////////////////////////////////////////////////////////////////////////

static int
CreateAssignments(ObjectParse *parse, R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Create assignments
  if (create_best_assignment_for_each_surfel_object) {
    if (!CreateBestAssignmentForEachSurfelObject(parse, scene, zsupport_grid)) return 0;
  }
  if (create_best_assignment_for_each_detection_label_pair) {
    if (!CreateBestAssignmentForEachDetectionLabelPair(parse, scene, zsupport_grid)) return 0;
  }
  if (create_best_assignment_for_each_detection) {
    if (!CreateBestAssignmentForEachDetection(parse, scene, zsupport_grid)) return 0;
  }
  if (create_assignments_using_search_grid) {
    // Create search grid
    R2Grid *search_grid = CreateSearchGrid(scene, zsupport_grid);
    if (!search_grid) return 0;
    
    // Create assignments
    if (!CreateAssignmentsUsingSearchGrid(parse, scene, zsupport_grid, *search_grid)) return 0;
    
    // Delete search grid
    delete search_grid;
  }
  if (create_assignments_using_convolution) {
    if (!CreateAssignmentsUsingConvolution(scene, parse, zsupport_grid)) return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Detection creation functions
////////////////////////////////////////////////////////////////////////

static int 
CreateDetections(ObjectParse *parse, R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Create detections
  if (create_detection_for_each_surfel_object) {
    if (!CreateDetectionForEachSurfelObject(parse, scene, zsupport_grid)) return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Segmentation creation functions
////////////////////////////////////////////////////////////////////////

static ObjectSegmentation * 
CreateSegmentation(ObjectParse *parse, R3SurfelScene *scene, const R2Grid& zsupport_grid,
  ObjectModel *model, const R3Affine& model_to_scene)
{
  // Compute point set
  ObjectPointSet pointset;
  if (!LoadSurfels(pointset, scene, model, model_to_scene, 0.25 * model->Radius(), max_points)) return NULL;

  // Create segmentation
  R3Point origin = model->Origin();
  origin.Transform(model_to_scene);
  ObjectSegmentation *segmentation = new ObjectSegmentation(pointset, origin, model->Radius(), model->Height(), 1.0);

  // Return success
  return segmentation;
}



static int 
CreateSegmentationForEachAssignment(ObjectParse *parse, R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Creating segmentation for each assignment ...\n");
    fflush(stdout);
  }

  // Create segmentations
  RNArray<ObjectAssignment *> assignments = parse->assignments;
  for (int i = 0; i < assignments.NEntries(); i++) {
    ObjectAssignment *assignment1 = assignments.Kth(i);
    RNScalar score1 = assignment1->Score();
    const R3Affine& model_to_segmentation1 = assignment1->ModelToSegmentationTransformation();
    ObjectModel *model = assignment1->Model();
    ObjectSegmentation *segmentation1 = assignment1->Segmentation();
    ObjectDetection *detection = (segmentation1) ? segmentation1->Detection() : NULL;
    ObjectSegmentation *segmentation2 = CreateSegmentation(parse, scene, zsupport_grid, model, model_to_segmentation1);
    if (!segmentation2) continue;
    RNScalar score2 = score1;
    R3Affine model_to_segmentation2 = model_to_segmentation1;
    RefineTransformation(segmentation2, model, model_to_segmentation1, model_to_segmentation2, score2);
    // if (score2 <= score1) { delete segmentation2; continue; }
    ObjectAssignment *assignment2 = new ObjectAssignment(segmentation2, model, model_to_segmentation2, score2);
    if (detection) detection->InsertSegmentation(segmentation2);
    segmentation2->InsertAssignment(assignment2);
    model->InsertAssignment(assignment2);
    count++;
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Labels = %d\n", parse->NLabels());
    printf("  # Models = %d\n", parse->NModels());
    printf("  # Detections = %d\n", parse->NDetections());
    printf("  # Segmentations = %d\n", parse->NSegmentations());
    printf("  # Assignments = %d\n", parse->NAssignments());
    printf("  # New Segmentations = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int 
CreateSegmentations(ObjectParse *parse, R3SurfelScene *scene, const R2Grid& zsupport_grid)
{
  // Create segmentations
  if (create_segmentation_for_each_assignment) {
    if (!CreateSegmentationForEachAssignment(parse, scene, zsupport_grid)) return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Sorting functions
////////////////////////////////////////////////////////////////////////

static int
Sort(ObjectParse *parse)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Sorting ...\n");
    fflush(stdout);
  }

  // Sort everything
  parse->Sort();

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Detections = %d\n", parse->NDetections());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Equivalence functions
////////////////////////////////////////////////////////////////////////

static int
IsAffineEquivalent(const R3Affine& affine0, const R3Affine& affine1,
  RNAngle rotation_equivalence_tolerance, RNScalar translation_equivalence_tolerance)
{
  // Get matrices
  const R4Matrix& m0 = affine0.Matrix();
  const R4Matrix& m1 = affine1.Matrix();

  // Check rotation angle
  RNAngle ca0 = acos(m0[0][0]);
  RNAngle ca1 = acos(m1[0][0]);
  RNAngle sa0 = asin(m0[0][1]);
  RNAngle sa1 = asin(m1[0][1]);
  if (fabs(ca0 - ca1) > rotation_equivalence_tolerance) return FALSE;
  if (fabs(sa0 - sa1) > rotation_equivalence_tolerance) return FALSE;

  // Check translation
  RNScalar x0 = m0[0][3];
  RNScalar y0 = m0[1][3];
  RNScalar x1 = m1[0][3];
  RNScalar y1 = m1[1][3];
  RNScalar dx = x0 - x1;
  RNScalar dy = y0 - y1;
  RNScalar dsq = dx*dx + dy*dy;
  if (dsq > translation_equivalence_tolerance * translation_equivalence_tolerance) return FALSE;

  // Passed all tests
  return TRUE;
}



static int
RemoveEquivalentAssignments(ObjectParse *parse, 
  RNLength translation_tolerance, RNAngle rotation_tolerance)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Removing equivalent assignments ...\n");
    fflush(stdout);
  }

  // Check every assignment against ones earlier in sort
  RNArray<ObjectAssignment *> assignments = parse->assignments;
  for (int i = 0; i < assignments.NEntries(); i++) {
    ObjectAssignment *assignment1 = assignments.Kth(i);
    ObjectModel *model1 = assignment1->Model();
    const R3Affine& transformation1 = assignment1->ModelToSegmentationTransformation();
    for (int j = 0; j < i; j++) {
      ObjectAssignment *assignment2 = assignments.Kth(j);
      ObjectModel *model2 = assignment2->Model();
      if (model1 != model2) continue;
      const R3Affine& transformation2 = assignment2->ModelToSegmentationTransformation();
      if (!IsAffineEquivalent(transformation1, transformation2, translation_tolerance, rotation_tolerance)) continue;
      parse->RemoveAssignment(assignment1);
      count++;
      break;
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Assignments Remaining = %d\n", parse->NAssignments());
    printf("  # Assignments Removed = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



#if 0

static int
RemoveOverlappingAssignments(ObjectParse *parse, RNScalar max_jaccard)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Removing overlapping assignments ...\n");
    fflush(stdout);
  }

  // Check every assignment against ones earlier in sort
  RNArray<ObjectAssignment *> end_assignments;
  RNArray<ObjectAssignment *> start_assignments = parse->assignments;
  for (int i = 0; i < start_assignments.NEntries(); i++) {
    RNBoolean retain1 = TRUE;
    ObjectAssignment *assignment1 = start_assignments.Kth(i);
    ObjectModel *model1 = assignment1->Model();
    const R3Affine& transformation1 = assignment1->ModelToSegmentationTransformation();
    RNScalar scale1 = transformation1.ScaleFactor();
    RNLength radius1 = scale1 * model1->Radius();
    RNLength height1 = scale1 * model1->Height();
    R3Point origin1 = model1->Origin();
    origin1.Transform(transformation1);
// #define CIRCLE
#ifdef CIRCLE
    R2Circle circle1(R2Point(origin1.X(), origin1.Y()), radius1);
    RNInterval zrange1(origin1.Z(), origin1.Z()+height1);
    RNVolume volume1 = circle1.Area() * zrange1.Diameter();
    if (volume1 == 0) retain1 = FALSE;
#else
    R3Point corner1A(origin1.X()-radius1, origin1.Y()-radius1, origin1.Z());
    R3Point corner1B(origin1.X()+radius1, origin1.Y()+radius1, origin1.Z()+height1);
    R3Box box1(corner1A, corner1B);
    RNVolume volume1 = box1.Volume();
    if (volume1 == 0) retain1 = FALSE;
#endif

    // Check for overlap with every earlier assignment
    if (retain1) {
      for (int j = 0; j < end_assignments.NEntries(); j++) {
        ObjectAssignment *assignment2 = end_assignments.Kth(j);
        ObjectModel *model2 = assignment2->Model();
        const R3Affine& transformation2 = assignment2->ModelToSegmentationTransformation();
        RNScalar scale2 = transformation2.ScaleFactor();
        RNLength radius2 = scale2 * model2->Radius();
        RNLength height2 = scale2 * model2->Height();
        R3Point origin2 = model2->Origin();
        origin2.Transform(transformation2);
#ifdef CIRCLE        
        R2Circle circle2(R2Point(origin2.X(), origin2.Y()), radius2);
        RNInterval zrange2(origin2.Z(), origin2.Z()+height2);
        RNVolume volume2 = circle2.Area() * zrange2.Diameter();
        if (volume2 == 0.0) continue;
#else
        R3Point corner2A(origin2.X()-radius2, origin2.Y()-radius2, origin2.Z());
        R3Point corner2B(origin2.X()+radius2, origin2.Y()+radius2, origin2.Z()+height2);
        R3Box box2(corner2A, corner2B);
        RNVolume volume2 = box2.Volume();
        if (volume2 == 0.0) continue;
#endif

#ifdef CIRCLE
        // Compute intersection volume
        RNArea intersection_area = R2IntersectionArea(circle1, circle2);
        if (intersection_area == 0) continue;
        RNInterval intersection_zrange = zrange1; 
        intersection_zrange.Intersect(zrange2);
        RNScalar intersection_height = intersection_zrange.Diameter();
        if (intersection_height == 0) continue;
        RNVolume intersection_volume = intersection_area * intersection_height;
        if (intersection_volume == 0) continue;
#else
        // Check bbox intersection
        R3Box intersection = box1;
        intersection.Intersect(box2);
        if (intersection.IsEmpty()) continue;
        RNVolume intersection_volume = intersection.Volume();
        if (intersection_volume == 0.0) continue;
#endif
        
        // Compute union volume
        RNVolume union_volume = volume1 + volume2 - intersection_volume;

        // Compute jaccard score (intersection / union)
        RNScalar jaccard = intersection_volume / union_volume;
        if (jaccard < max_jaccard) continue;
        retain1 = FALSE;
        break;
      }
    }
      
    // Check if should retain assignment
    if (retain1) {
      // Add assignment1 to list of end assignments
      end_assignments.Insert(assignment1);
    }
    else {
      // Remove assignment1
      parse->RemoveAssignment(assignment1);
      // delete assignment1;
      count++;
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Assignments Remaining = %d\n", parse->NAssignments());
    printf("  # Assignments Removed = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}

#else

static int
RemoveOverlappingAssignments(ObjectParse *parse, RNScalar max_jaccard)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Removing overlapping assignments ...\n");
    fflush(stdout);
  }

  // Compute bounding box of assignments
  R3Box bbox = R3null_box;
  for (int i = 0; i < parse->NAssignments(); i++) {
    ObjectAssignment *assignment = parse->Assignment(i);
    ObjectModel *model = assignment->Model();
    const R3Affine& transformation = assignment->ModelToSegmentationTransformation();
    RNScalar scale = transformation.ScaleFactor();
    RNScalar radius = scale * model->Radius();
    RNScalar height = scale * model->Height();
    R3Point origin = model->Origin();
    origin.Transform(transformation);
    R3Point corner0(origin.X() - radius, origin.Y() - radius, origin.Z());
    R3Point corner1(origin.X() + radius, origin.Y() + radius, origin.Z() + height);
    bbox.Union(corner0);
    bbox.Union(corner1);
  }
    
  // Create an grid representing occupancy of assignments
  int xres = (bbox.XLength() / rasterization_spacing) + 1;
  int yres = (bbox.YLength() / rasterization_spacing) + 1;
  int zres = (bbox.ZLength() / rasterization_spacing) + 1;
  if (xres > 1024) xres = 1024;
  if (yres > 1024) xres = 1024;
  if (zres > 128) zres = 128;
  R3Grid occupancy_grid(xres, yres, zres, bbox);

  // Check every assignment against occupancy grid
  RNArray<ObjectAssignment *> assignments = parse->assignments;
  for (int i = 0; i < assignments.NEntries(); i++) {
    ObjectAssignment *assignment = assignments.Kth(i);
    ObjectModel *model = assignment->Model();
    const R3Affine& transformation = assignment->ModelToSegmentationTransformation();
    const ObjectPointSet& pointset = model->PointSet();

    // Check minimum number of points
    if (pointset.NPoints() < min_points) {
      parse->RemoveAssignment(assignment);
      // delete assignment;
      count++;
      continue;
    }

    // Sum occupancy of points
    RNScalar sum = 0;
    for (int i = 0; i < pointset.NPoints(); i++) {
      ObjectPoint* point = pointset.Point(i);
      R3Point position = point->Position();
      position.Transform(transformation);
      RNScalar occupancy = occupancy_grid.WorldValue(position);
      if (occupancy > 1.0) occupancy = 1.0;
      sum += occupancy;
    }
    
    // Compute average occupancy
    RNScalar mean = sum / pointset.NPoints();
    if (mean > max_overlap) {
      parse->RemoveAssignment(assignment);
      // delete assignment;
      count++;
      continue;
    }

    // Update occupancy grid
    for (int i = 0; i < pointset.NPoints(); i++) {
      ObjectPoint* point = pointset.Point(i);
      R3Point position = point->Position();
      position.Transform(transformation);
      occupancy_grid.RasterizeWorldPoint(position, 1.0);
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Assignments Remaining = %d\n", parse->NAssignments());
    printf("  # Assignments Removed = %d\n", count);
    fflush(stdout);
  }

  // Return success
  return 1;
}

#endif



static int
RemoveWeakAssignments(ObjectParse *parse, RNScalar min_score, int max_assignments)
{
  // Check min score
  if ((min_score <= 0) && (max_assignments <= 0)) return 1;

  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Removing weak assignments ...\n");
    fflush(stdout);
  }

  // Remove assignments at end of list
  while (parse->NAssignments() > 0) {
    ObjectAssignment *assignment = parse->Assignment(parse->NAssignments()-1);
    if ((max_assignments > 0) && (parse->NAssignments() > max_assignments)) {
      parse->RemoveAssignment(assignment);
      count++;
    }
    else if ((min_score > 0) && (assignment->Score() < min_score)) {
      parse->RemoveAssignment(assignment);
      count++;
    }
    else {
      break;
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Assignments Remaining = %d\n", parse->NAssignments());
    printf("  # Assignments Removed = %d\n", count);
    fflush(stdout);
  }

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
      else if (!strcmp(*argv, "-debug")) print_debug = 1;
      else if (!strcmp(*argv, "-input_parse")) { argc--; argv++; input_parse_name = *argv; }
      else if (!strcmp(*argv, "-input_ground_truth")) { argc--; argv++; input_ground_truth_name = *argv; }
      else if (!strcmp(*argv, "-input_labels")) { argc--; argv++; input_labels_name = *argv; }
      else if (!strcmp(*argv, "-input_detections")) { argc--; argv++; input_detections_name = *argv; }
      else if (!strcmp(*argv, "-input_gsv")) { argc--; argv++; input_gsv_name = *argv; }
      else if (!strcmp(*argv, "-input_markers")) { argc--; argv++; input_markers_name = *argv; }
      else if (!strcmp(*argv, "-input_centers")) { argc--; argv++; input_centers_name = *argv; }
      else if (!strcmp(*argv, "-input_search_grid")) { argc--; argv++; input_search_grid_name = *argv; }
      else if (!strcmp(*argv, "-input_zsupport_grid")) { argc--; argv++; input_zsupport_grid_name = *argv; }
      else if (!strcmp(*argv, "-input_zsupport_coordinate")) { argc--; argv++; zsupport_coordinate = atof(*argv); }
      else if (!strcmp(*argv, "-output_rectangles")) { argc--; argv++; output_rectangles_name = *argv; }
      else if (!strcmp(*argv, "-output_gsv_grid")) { argc--; argv++; output_gsv_grid_name = *argv; }
      else if (!strcmp(*argv, "-output_search_grid")) { argc--; argv++; output_search_grid_name = *argv; }
      else if (!strcmp(*argv, "-output_zsupport_grid")) { argc--; argv++; output_zsupport_grid_name = *argv; }
      else if (!strcmp(*argv, "-output_segmentations")) { argc--; argv++; output_segmentation_directory = *argv; }
      else if (!strcmp(*argv, "-superpixels")) superpixels = 1;
      else if (!strcmp(*argv, "-min_assignment_spacing")) { argc--; argv++; min_assignment_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-min_rotation_spacing")) { argc--; argv++; min_rotation_spacing = RN_PI * atof(*argv) / 180.0; }
      else if (!strcmp(*argv, "-min_volume")) { argc--; argv++; min_volume = atof(*argv); }
      else if (!strcmp(*argv, "-max_volume")) { argc--; argv++; max_volume = atof(*argv); }
      else if (!strcmp(*argv, "-min_radius")) { argc--; argv++; min_radius = atof(*argv); }
      else if (!strcmp(*argv, "-max_radius")) { argc--; argv++; max_radius = atof(*argv); }
      else if (!strcmp(*argv, "-min_height")) { argc--; argv++; min_height = atof(*argv); }
      else if (!strcmp(*argv, "-max_height")) { argc--; argv++; max_height = atof(*argv); }
      else if (!strcmp(*argv, "-min_scale")) { argc--; argv++; min_scale = atof(*argv); }
      else if (!strcmp(*argv, "-max_scale")) { argc--; argv++; max_scale = atof(*argv); }
      else if (!strcmp(*argv, "-max_gap")) { argc--; argv++; max_gap = atof(*argv); }
      else if (!strcmp(*argv, "-min_points")) { argc--; argv++; min_points = atoi(*argv); }
      else if (!strcmp(*argv, "-max_points")) { argc--; argv++; max_points = atoi(*argv); }
      else if (!strcmp(*argv, "-rasterization_spacing")) { argc--; argv++; rasterization_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-alignment_spacing")) { argc--; argv++; alignment_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-max_assignments")) { argc--; argv++; max_assignments = atoi(*argv); }
      else if (!strcmp(*argv, "-max_overlap")) { argc--; argv++; max_overlap = atof(*argv); }
      else if (!strcmp(*argv, "-min_score")) { argc--; argv++; min_score = atof(*argv); }
      else if (!strcmp(*argv, "-nrotations")) { argc--; argv++; nrotations = atoi(*argv); }
      else if (!strcmp(*argv, "-no_icp")) { use_icp = FALSE; }
      else if (!strcmp(*argv, "-sort")) sort = 1;
      else if (!strcmp(*argv, "-create_uniform_search_grid")) create_uniform_search_grid = 1;
      else if (!strcmp(*argv, "-create_density_search_grid")) create_density_search_grid = 1;
      else if (!strcmp(*argv, "-create_gsv_hough_search_grid")) create_gsv_hough_search_grid = 1;
      else if (!strcmp(*argv, "-remove_weak_assignments")) remove_weak_assignments = 1;
      else if (!strcmp(*argv, "-remove_equivalent_assignments")) remove_equivalent_assignments = 1;
      else if (!strcmp(*argv, "-remove_overlapping_assignments")) remove_overlapping_assignments = 1;
      else if (!strcmp(*argv, "-create_detection_for_each_surfel_object")) { 
        create_detection_for_each_surfel_object = 1;
      }
      else if (!strcmp(*argv, "-create_segmentation_for_each_assignment")) { 
        create_segmentation_for_each_assignment = 1;
      }
      else if (!strcmp(*argv, "-create_best_assignment_for_each_surfel_object")) {
        create_best_assignment_for_each_surfel_object = 1;
      }
      else if (!strcmp(*argv, "-create_best_assignment_for_each_detection_label_pair")) {
        create_best_assignment_for_each_detection_label_pair = 1;
      }
      else if (!strcmp(*argv, "-create_best_assignment_for_each_detection")) { 
        create_best_assignment_for_each_detection = 1;
      }
      else if (!strcmp(*argv, "-create_assignments_using_search_grid")) { 
        create_assignments_using_search_grid = 1;
      }
      else if (!strcmp(*argv, "-create_assignments_using_convolution")) {
        create_assignments_using_convolution = 1;
      }
      else if (!strcmp(*argv, "-bbox")) { 
        argc--; argv++; detections_bbox[0][0] = atof(*argv); 
        argc--; argv++; detections_bbox[0][1] = atof(*argv); 
        argc--; argv++; detections_bbox[0][2] = atof(*argv); 
        argc--; argv++; detections_bbox[1][0] = atof(*argv); 
        argc--; argv++; detections_bbox[1][1] = atof(*argv); 
        argc--; argv++; detections_bbox[1][2] = atof(*argv); 
      }
      else if (!strcmp(*argv, "-indoor_objects")) { 
        min_radius = 0.25;
        max_radius = 1.0;
        min_height = 0.1;
        max_height = 1.5;
        // max_gap = 0.25;
        min_volume = 0.1;
        max_volume = 100;
        alignment_spacing = 0.05;
        rasterization_spacing = 0.05;
        min_assignment_spacing = 0.25;
        min_points_per_square_meter = 10;
        sort = 1;
        remove_overlapping_assignments = 1;
      }
      else if (!strcmp(*argv, "-outdoor_objects")) { 
        min_radius = 0.25;
        max_radius = 4.0;
        min_height = 0.25;
        max_height = 16;
        max_gap = 0.25;
        min_volume = 0.1;
        max_volume = 1000;
        alignment_spacing = 0.25;
        rasterization_spacing = 0.25;
        min_assignment_spacing = 0.5;
        min_points_per_square_meter = 1;
      }
      else { 
        fprintf(stderr, "Invalid program argument: %s", *argv); 
        exit(1); 
      }
      argv++; argc--;
    }
    else {
      if (!input_scene_name) input_scene_name = *argv;
      else if (!input_database_name) input_database_name = *argv;
      else if (!output_parse_name) output_parse_name = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!input_scene_name || !input_database_name || !output_parse_name) {
    fprintf(stderr, "Usage: sfldetect input_scene input_database output_parse [options]\n");
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
  R3SurfelScene *scene = OpenScene(input_scene_name, input_database_name);
  if (!scene) exit(-1);

  // Create zsupport grid
  R2Grid *zsupport_grid = CreateZSupportGrid(scene);
  if (!zsupport_grid) exit(-1);

  // Create parse
  ObjectParse *parse = CreateParse(scene);
  if (!parse) exit(-1);

  // Create detections
  if (!CreateDetections(parse, scene, *zsupport_grid)) exit(-1);

  // Create segmentations
  if (!CreateSegmentations(parse, scene, *zsupport_grid)) exit(-1);

  // Create assignments
  if (!CreateAssignments(parse, scene, *zsupport_grid)) exit(-1);

  // Remove equivalent assignments
  if (remove_equivalent_assignments) {
    if (!Sort(parse)) exit(-1);
    if (!RemoveEquivalentAssignments(parse, min_assignment_spacing, min_rotation_spacing)) exit(-1);
  }

  // Remove overlapping assignments
  if (remove_overlapping_assignments) {
    if (!Sort(parse)) exit(-1);
    if (!RemoveOverlappingAssignments(parse, max_overlap)) exit(-1);
  }

  // Remove weak assignments
  if ((min_score > 0) || (max_assignments > 0)) {
    if (!Sort(parse)) exit(-1);
    if (!RemoveWeakAssignments(parse, min_score, max_assignments)) exit(-1);
  }

  // Sort everything
  if (sort) {
    if (!Sort(parse)) exit(-1);
  }

  // Write parse
  if (output_parse_name) {
    if (!WriteParse(parse, output_parse_name)) exit(-1);
  }

  // Write rectangles
  if (output_rectangles_name) {
    if (!WriteRectangles(parse, output_rectangles_name)) exit(-1);
  }

  // Write segmentations
  if (output_segmentation_directory) {
    if (!WriteSegmentations(parse, output_segmentation_directory)) exit(-1);
  }

  // Close scene
  if (!CloseScene(scene)) exit(-1);

  // Delete zsupport grid
  delete zsupport_grid;

  // Return success 
  return 0;
}


