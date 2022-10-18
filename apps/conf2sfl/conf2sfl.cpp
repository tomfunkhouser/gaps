// Source file for the sfl loader program



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Surfels/R3Surfels.h"
#include "R3Utils/R3Utils.h"
#include "RGBD/RGBD.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

// File input/output options

static const char *input_configuration_name = NULL;
static const char *output_ssa_name = NULL;
static const char *output_ssb_name = NULL;


// Segmentation options

static RNBoolean create_planar_segments = FALSE;
static double max_neighbor_distance_factor = 16;
static double max_neighbor_normal_angle = 0;
static double max_neighbor_color_difference = 0;


// Image/pixel selection options

static int load_images_starting_at_index = 0;
static int load_images_ending_at_index = INT_MAX;
static int load_every_kth_image = 1;
static R3Box load_images_bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
static int max_image_resolution = 0;
static double max_depth = 0;
static int omit_corners = 0;
static int pixel_stride = 1;


// Surfel processing options

static RNBoolean create_multiresolution_hierarchy = FALSE;


// Printing options

static int print_verbose = 0;
static int print_debug = 0;



////////////////////////////////////////////////////////////////////////
// I/O STUFF
////////////////////////////////////////////////////////////////////////

static RGBDConfiguration *
ReadConfiguration(const char *filename) 
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Reading configuration from %s ...\n", filename);
    fflush(stdout);
  }

  // Allocate configuration
  RGBDConfiguration *configuration = new RGBDConfiguration();
  if (!configuration) {
    RNFail("Unable to allocate configuration for %s\n", filename);
    return NULL;
  }

  // Read file
  if (!configuration->ReadFile(filename)) {
    RNFail("Unable to read configuration from %s\n", filename);
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", configuration->NImages());
    printf("  # Surfaces = %d\n", configuration->NSurfaces());
    fflush(stdout);
  }

  // Return configuration
  return configuration;
}



static R3SurfelScene *
OpenSurfelScene(const char *scene_name, const char *database_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate surfel scene
  R3SurfelScene *scene = new R3SurfelScene();
  if (!scene) {
    RNFail("Unable to allocate scene\n");
    return NULL;
  }

  // Open surfel scene files
  if (!scene->OpenFile(scene_name, database_name, "w", "w")) {
    delete scene;
    return NULL;
  }

  // Create surfel features 
  if (!CreateFeatures(scene)) exit(-1);

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
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Return scene
  return scene;
}



static int
CloseSurfelScene(R3SurfelScene *scene)
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
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Close surfel scene files
  if (!scene->CloseFile()) {
    delete scene;
    return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Segmentation Processing
////////////////////////////////////////////////////////////////////////

static int 
CreatePoints(R3Segmentation *segmentation,
  const R2Grid& px_image, const R2Grid& py_image, const R2Grid& pz_image, 
  const R2Grid& nx_image, const R2Grid& ny_image, const R2Grid& nz_image,
  const R2Grid& tx_image, const R2Grid& ty_image, const R2Grid& tz_image,
  const R2Grid& depth_image, const R2Grid& r1_image, const R2Grid& r2_image,
  const R2Grid& boundary_image, const R2Image& color_image)
{
  // Allocate points 
  segmentation->point_buffer = new R3SegmentationPoint [ depth_image.NEntries() ];
  if (!segmentation->point_buffer) {
    RNFail("Unable to allocate points\n");
    return 0;
  }

  // Fill points
  for (int ix = 0; ix < depth_image.XResolution(); ix++) {
    for (int iy = 0; iy < depth_image.YResolution(); iy++) {
      int i;
      depth_image.IndicesToIndex(ix, iy, i);
      R3SegmentationPoint *point = &segmentation->point_buffer[i];

      // Check depth
      RNScalar depth = depth_image.GridValue(i);
      if (RNIsNegativeOrZero(depth)) continue;
      if ((max_depth > 0) && (depth > max_depth)) continue;
      point->depth = depth;

      // Get position
      RNScalar px = px_image.GridValue(i);
      RNScalar py = py_image.GridValue(i);
      RNScalar pz = pz_image.GridValue(i);
      point->position.Reset(px, py, pz);

      // Get normal
      RNScalar nx = nx_image.GridValue(i);
      RNScalar ny = ny_image.GridValue(i);
      RNScalar nz = nz_image.GridValue(i);
      point->normal.Reset(nx, ny, nz);

      // Get tangent
      RNScalar tx = tx_image.GridValue(i);
      RNScalar ty = ty_image.GridValue(i);
      RNScalar tz = tz_image.GridValue(i);
      point->tangent.Reset(tx, ty, tz);

      // Get radius
      point->radius1 = r1_image.GridValue(i);
      point->radius2 = r2_image.GridValue(i);
      point->area = RN_PI * point->radius1 * point->radius2;

      // Get color
      point->color = color_image.PixelRGB(ix, iy);
    
      // Get flags
      point->boundary = (unsigned int) (boundary_image.GridValue(i) + 0.5);

      // Set grid index
      point->data_index = i;

      // Insert point
      segmentation->points.Insert(point);
    }
  }

  // Create kdtree of points
  R3SegmentationPoint tmp; int position_offset = (unsigned char *) &(tmp.position) - (unsigned char *) &tmp;
  segmentation->kdtree = new R3Kdtree< R3SegmentationPoint *>(segmentation->points, position_offset);
  if (!segmentation->kdtree) {
    RNFail("Unable to create kdtree\n");
    return 0;
  }
  
  // Create arrays of neighbor points
  for (int i = 0; i < segmentation->points.NEntries(); i++) {
    R3SegmentationPoint *point = segmentation->points.Kth(i);
    int ix, iy, neighbor_index;
    depth_image.IndexToIndices(point->data_index, ix, iy);
    for (int s = -1; s <= 1; s++) {
      if ((ix+s < 0) || (ix+s >= depth_image.XResolution())) continue;
      for (int t = -1; t <= 1; t++) {
        if ((s == 0) && (t == 0)) continue;
        if ((iy+t < 0) || (iy+t >= depth_image.YResolution())) continue;
        depth_image.IndicesToIndex(ix+s, iy+t, neighbor_index);
        R3SegmentationPoint *neighbor = &segmentation->point_buffer[neighbor_index];

        // Check if across boundary
        if ((point->boundary & RGBD_SHADOW_BOUNDARY) && (neighbor->boundary & RGBD_SILHOUETTE_BOUNDARY)) continue;
        if ((point->boundary & RGBD_SILHOUETTE_BOUNDARY) && (neighbor->boundary & RGBD_SHADOW_BOUNDARY)) continue;

        // Check if too far away
        if (max_neighbor_distance_factor > 0) {
          RNScalar radius = (point->radius1 > neighbor->radius1) ? neighbor->radius1 : point->radius1;
          RNScalar max_neighbor_distance = (radius > 0) ? max_neighbor_distance_factor * radius : 0.25;
          RNScalar dd = R3SquaredDistance(point->position, neighbor->position);
          if (dd > max_neighbor_distance * max_neighbor_distance) continue;
        }

        // Check if too much color difference
        if (max_neighbor_color_difference > 0) {
          RNScalar dr = fabs(point->color.R() - neighbor->color.R());
          if (dr > max_neighbor_color_difference) continue;
          RNScalar dg = fabs(point->color.G() - neighbor->color.G());
          if (dg > max_neighbor_color_difference) continue;
          RNScalar db = fabs(point->color.B() - neighbor->color.B());
          if (db > max_neighbor_color_difference) continue;
        }

        // Check if too much normal angle
        if (max_neighbor_normal_angle > 0) {
          RNScalar dot = fabs(point->normal.Dot(neighbor->normal));
          RNAngle normal_angle = (dot < 1) ? RN_PI_OVER_TWO - acos(dot) : RN_PI_OVER_TWO;
          if (normal_angle > max_neighbor_normal_angle) continue;
        }
        
        // Insert neighbor
        point->neighbors.Insert(neighbor);
      }
    }
  }

  // Return success
  return 1;
}
               


static R3Segmentation *
CreateSegmentation(const R2Grid& px_image, const R2Grid& py_image, const R2Grid& pz_image,
  const R2Grid& nx_image, const R2Grid& ny_image, const R2Grid& nz_image,
  const R2Grid& tx_image, const R2Grid& ty_image, const R2Grid& tz_image,
  const R2Grid& depth_image, const R2Grid& r1_image, const R2Grid& r2_image,
  const R2Grid& boundary_image, const R2Image& color_image,
  const R3Point& viewpoint, const R3Vector& towards, const R3Vector& up)
{
  // Allocate segmentation
  R3Segmentation *segmentation = new R3Segmentation();
  if (!segmentation) {
    RNFail("Unable to allocate segmentation.\n");
    return NULL;
  }

  // Adjust segmentation parameters ???
  segmentation->min_cluster_points = 10 * depth_image.NEntries() / (640 * 480);
  segmentation->scale_tolerances_with_depth = TRUE;
  // segmentation->print_progress = TRUE;
  // segmentation->PrintParameters();
  
  // Create points
  if (!CreatePoints(segmentation, px_image, py_image, pz_image,
    nx_image, ny_image, nz_image, tx_image, ty_image, tz_image,
    depth_image, r1_image, r2_image, boundary_image, color_image)) {
    RNFail("Unable to create points for segmentation.\n");
    delete segmentation;
    return 0;
  }

  // Check points
  if (segmentation->points.NEntries() == 0) {
    delete segmentation;
    return 0;
  }

  // Create clusters
  if (!segmentation->CreateClusters(R3_SEGMENTATION_PLANE_PRIMITIVE_TYPE)) {
    delete segmentation;
    return 0;
  }

  // Return segmentation
  return segmentation;
}



////////////////////////////////////////////////////////////////////////
// Surfel Processing
////////////////////////////////////////////////////////////////////////

static R3SurfelBlock *
LoadSurfels(R3SurfelScene *scene, const RNArray<R3SegmentationPoint *>& points,
  R3SurfelObject *parent_object, R3SurfelNode *parent_node,
  const char *node_name, const R2Grid& depth_image, const R3Point& origin)
{
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  R3SurfelDatabase *database = tree->Database();
  int cx = depth_image.XResolution()/2;
  int cy = depth_image.YResolution()/2;
  double rx = cx;
  double ry = cy;
  
  // Allocate array of surfels
  int nsurfels = 0;
  R3Surfel *surfels = new R3Surfel [ points.NEntries() ];
  if (!surfels) {
    RNFail("Unable to allocate surfels for %s\n", node_name);
    return NULL;
  }

  // Load surfels into array
  for (int i = 0; i < points.NEntries(); i++) {
    R3SegmentationPoint *point = points.Kth(i);
        
    // Check depth
    if (RNIsNegativeOrZero(point->depth)) continue;
    if ((max_depth > 0) && (point->depth > max_depth)) continue;

    // Get/check grid index
    int ix, iy;
    if (point->data_index < 0) continue;
    depth_image.IndexToIndices(point->data_index, ix, iy);

    // Check pixel stride
    if (ix % pixel_stride != 0) continue;
    if (iy % pixel_stride != 0) continue;

    // Check if outside elipse centered in image
    if (omit_corners) {
      int dx = (ix - cx) / rx;
      int dy = (iy - cy) / ry;
      if (dx*dx + dy*dy > 1) continue;
    }

    // Set position
    float px = (float) (point->position.X() - origin.X());
    float py = (float) (point->position.Y() - origin.Y());
    float pz = (float) (point->position.Z() - origin.Z());
    surfels[nsurfels].SetPosition(px, py, pz);

    // Set normal
    float nx = (float) point->normal.X();
    float ny = (float) point->normal.Y();
    float nz = (float) point->normal.Z();
    surfels[nsurfels].SetNormal(nx, ny, nz);

    // Set tangent
    float tx = (float) point->tangent.X();
    float ty = (float) point->tangent.Y();
    float tz = (float) point->tangent.Z();
    surfels[nsurfels].SetTangent(tx, ty, tz);

    // Set radius
    surfels[nsurfels].SetRadius(0, (float) point->radius1);
    surfels[nsurfels].SetRadius(1, (float) point->radius2);

    // Set depth
    surfels[nsurfels].SetDepth(point->depth);
    
    // Set color
    surfels[nsurfels].SetColor(point->color);

    // Set identifier
    unsigned int identifier = database->NSurfels() + nsurfels + 1;    
    surfels[nsurfels].SetIdentifier(identifier);

    // Set boundary flags
    if (point->boundary == R3_SURFEL_BORDER_BOUNDARY_FLAG) surfels[nsurfels].SetBorderBoundary(TRUE);
    else if (point->boundary == R3_SURFEL_SILHOUETTE_BOUNDARY_FLAG) surfels[nsurfels].SetSilhouetteBoundary(TRUE);
    else if (point->boundary == R3_SURFEL_SHADOW_BOUNDARY_FLAG) surfels[nsurfels].SetShadowBoundary(TRUE);

    // Increment number of surfels
    nsurfels++;
  }

  // Create block from array of surfels
  R3SurfelBlock *block = new R3SurfelBlock(surfels, nsurfels, origin);
  if (!block) {
    RNFail("Unable to allocate block\n");
    delete [] surfels;
    return NULL;
  }

  // Delete array of surfels
  delete [] surfels;

  // Insert block into database
  block->UpdateProperties();
  database->InsertBlock(block);

  // Create node
  R3SurfelNode *node = NULL;
  if (block && parent_node) {
    // Create node
    node = new R3SurfelNode(node_name);
    if (!node) {
      RNFail("Unable to allocate node for %s\n", node_name);
      delete block;
      return NULL;
    }
            
    // Insert node into tree
    tree->InsertNode(node, parent_node);
    node->InsertBlock(block);
    node->UpdateProperties();
  }

  // Creat object
  R3SurfelObject *object = NULL;
  if (node && parent_object) {
    // Create object
    object = new R3SurfelObject(node_name);
    if (!object) {
      RNFail("Unable to allocate object for %s\n", node_name);
      delete block;
      delete node;
      return NULL;
    }

    // Insert object
    scene->InsertObject(object, parent_object);
    object->InsertNode(node);
    object->UpdateProperties();

    // Create PCA object property
    R3SurfelObjectProperty *pca = new R3SurfelObjectProperty(R3_SURFEL_OBJECT_PCA_PROPERTY, object);
    scene->InsertObjectProperty(pca);
  }

  // Release block
  database->ReleaseBlock(block);

  // Return block
  return block;
}



static int
LoadSurfels(R3SurfelScene *scene, 
  const char *scan_name, R3Segmentation *segmentation, const R2Grid& depth_image, 
  const R3Point& viewpoint, const R3Vector& towards, const R3Vector& up,
  const R3Matrix& intrinsics)
{
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  R3SurfelObject *parent_object = scene->RootObject();
  R3SurfelNode *parent_node = tree->RootNode();
  R2Point center(intrinsics[0][2], intrinsics[1][2]);
  RNLength xfocal = intrinsics[0][0];
  RNLength yfocal = intrinsics[1][1];
  int width = depth_image.XResolution();
  int height = depth_image.YResolution();
  
  // Create array of points outside any cluster
  RNArray<R3SegmentationPoint *> unclustered_points;
  for (int i = 0; i < segmentation->points.NEntries(); i++) {
    R3SegmentationPoint *point = segmentation->points.Kth(i);
    if (!point->cluster) unclustered_points.Insert(point);
  }

  // Create node
  char node_name[1024];
  sprintf(node_name, "SCAN:%s", scan_name);
  R3SurfelNode *scan_node = new R3SurfelNode(node_name);
  if (!scan_node) {
    RNFail("Unable to allocate node for %s\n", scan_name);
    return 0;
  }
            
  // Insert node into tree
  tree->InsertNode(scan_node, parent_node);

  // Load surfels for points inside every cluster
  for (int i = 0; i < segmentation->clusters.NEntries(); i++) {
    R3SegmentationCluster *cluster = segmentation->clusters.Kth(i);
    if (cluster->points.NEntries() == 0) continue;
    
    // Create node name
    char node_name[1024];
    sprintf(node_name, "SCAN:%s:%d", scan_name, i);

    // Load surfels
    LoadSurfels(scene, cluster->points, parent_object, scan_node, node_name, depth_image, viewpoint);
  }

  // Load surfels for points outside any cluster
  if (unclustered_points.NEntries() > 0) {
    // Create node name
    char node_name[1024];
    sprintf(node_name, "SCAN:%s:unclustered", scan_name);

    // Load surfels
    LoadSurfels(scene, unclustered_points, NULL, scan_node, node_name, depth_image, viewpoint);
  }
            
  // Update node properties
  scan_node->UpdateProperties();

  // Create scan
  R3SurfelScan *scan = new R3SurfelScan(scan_name);
  if (!scan) {
    RNFail("Unable to allocate scan\n");
    return 0;
  }

  // Assign scan properties
  scan->SetViewpoint(viewpoint);
  scan->SetOrientation(towards, up);
  scan->SetImageDimensions(width, height);
  scan->SetImageCenter(center);
  scan->SetXFocal(xfocal);
  scan->SetYFocal(yfocal);
  scan->SetNode(scan_node);
          
  // Insert scan
  scene->InsertScan(scan);

  // Create image
  R3SurfelImage *image = new R3SurfelImage(scan_name);
  if (!image) {
    RNFail("Unable to allocate image\n");
    return 0;
  }

  // Assign image properties
  image->SetViewpoint(viewpoint);
  image->SetOrientation(towards, up);
  image->SetImageDimensions(width, height);
  image->SetImageCenter(center);
  image->SetXFocal(xfocal);
  image->SetYFocal(yfocal);
  image->SetScan(scan);
          
  // Insert image
  scene->InsertImage(image);

  // Return success
  return 1;
}



static int
LoadSurfels(R3SurfelScene *scene,
  const char *scan_name, R3Segmentation *segmentation, const R2Grid& depth_image, 
  const R2Grid& px_image, const R2Grid& py_image, const R2Grid& pz_image,
  const R2Grid& nx_image, const R2Grid& ny_image, const R2Grid& nz_image,
  const R2Grid& tx_image, const R2Grid& ty_image, const R2Grid& tz_image,
  const R2Grid& r1_image, const R2Grid& r2_image,
  const R2Grid& boundary_image, const R2Image& color_image,
  const R3Point& viewpoint, const R3Vector& towards, const R3Vector& up,
  const R3Matrix& intrinsics)
{
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  R3SurfelDatabase *database = tree->Database();
  R3SurfelNode *parent_node = tree->RootNode();
  int width = depth_image.XResolution();
  int height = depth_image.YResolution();
  R2Point center(intrinsics[0][2], intrinsics[1][2]);
  RNLength xfocal = intrinsics[0][0];
  RNLength yfocal = intrinsics[1][1];
  int cx = depth_image.XResolution()/2;
  int cy = depth_image.YResolution()/2;
  double rx = cx;
  double ry = cy;
  
  // Allocate array of surfels
  int max_surfels = depth_image.NEntries();
  R3Surfel *surfels = new R3Surfel [ max_surfels ];
  if (!surfels) {
    RNFail("Unable to allocate surfels for %s\n", scan_name);
    return 0;
  }

  // Load surfels into array
  int nsurfels = 0;
  for (int i = 0; i < depth_image.XResolution(); i += pixel_stride) {
    for (int j = 0; j < depth_image.YResolution(); j += pixel_stride) {
      // Check depth
      RNScalar depth = depth_image.GridValue(i, j);
      if (RNIsNegativeOrZero(depth)) continue;
      if ((max_depth > 0) && (depth > max_depth)) continue;

      // Check if outside elipse centered in image
      if (omit_corners) {
        int dx = (i - cx) / rx;
        int dy = (j - cy) / ry;
        if (dx*dx + dy*dy > 1) continue;
      }

      // Set position
      float px = (float) (px_image.GridValue(i, j) - viewpoint.X());
      float py = (float) (py_image.GridValue(i, j) - viewpoint.Y());
      float pz = (float) (pz_image.GridValue(i, j) - viewpoint.Z());
      surfels[nsurfels].SetPosition(px, py, pz);

      // Set normal
      float nx = (float) (nx_image.GridValue(i, j));
      float ny = (float) (ny_image.GridValue(i, j));
      float nz = (float) (nz_image.GridValue(i, j));
      surfels[nsurfels].SetNormal(nx, ny, nz);

      // Set tangent
      float tx = (float) (tx_image.GridValue(i, j));
      float ty = (float) (ty_image.GridValue(i, j));
      float tz = (float) (tz_image.GridValue(i, j));
      surfels[nsurfels].SetTangent(tx, ty, tz);

      // Set radius
      float r1 = (float) (r1_image.GridValue(i, j));
      float r2 = (float) (r2_image.GridValue(i, j));
      surfels[nsurfels].SetRadius(0, r1);
      surfels[nsurfels].SetRadius(1, r2);

      // Set depth
      surfels[nsurfels].SetDepth(depth);

      // Set color
      RNRgb color(0.0, 0.8, 0.0);
      if ((i < color_image.Width()) && (j < color_image.Height())) {
        color = color_image.PixelRGB(i, j);
        surfels[nsurfels].SetColor(color);
      }

      // Set identifier
      unsigned int identifier = database->NSurfels() + nsurfels + 1;    
      surfels[nsurfels].SetIdentifier(identifier);

      // Set boundary flags
      int boundary = (int) (boundary_image.GridValue(i, j) + 0.5);
      if (boundary == R3_SURFEL_BORDER_BOUNDARY_FLAG) surfels[nsurfels].SetBorderBoundary(TRUE);
      else if (boundary == R3_SURFEL_SILHOUETTE_BOUNDARY_FLAG) surfels[nsurfels].SetSilhouetteBoundary(TRUE);
      else if (boundary == R3_SURFEL_SHADOW_BOUNDARY_FLAG) surfels[nsurfels].SetShadowBoundary(TRUE);

      // Increment number of surfels
      nsurfels++;
    }
  }

  // Create block from array of surfels
  R3SurfelBlock *block = new R3SurfelBlock(surfels, nsurfels, viewpoint);
  if (!block) {
    RNFail("Unable to allocate block\n");
    delete [] surfels;
    return 0;
  }

  // Delete array of surfels
  delete [] surfels;

  // Update block properties
  block->UpdateProperties();
  
  // Insert block into database
  database->InsertBlock(block);

  // Create node name
  char node_name[1024];
  sprintf(node_name, "SCAN:%s", scan_name);
  
  // Create node
  R3SurfelNode *node = new R3SurfelNode(node_name);
  if (!node) {
    RNFail("Unable to allocate node\n");
    delete block;
    return 0;
  }
            
  // Insert node into tree
  tree->InsertNode(node, parent_node);

  // Insert block into node
  node->InsertBlock(block);
  
  // Update node properties
  node->UpdateProperties();

  // Release block
  database->ReleaseBlock(block);

  // Create scan
  R3SurfelScan *scan = new R3SurfelScan(node_name);
  if (!scan) {
    RNFail("Unable to allocate scan\n");
    return 0;
  }

  // Assign scan properties
  scan->SetViewpoint(viewpoint);
  scan->SetOrientation(towards, up);
  scan->SetImageDimensions(width, height);
  scan->SetImageCenter(center);
  scan->SetXFocal(xfocal);
  scan->SetYFocal(yfocal);
  scan->SetNode(node);
          
  // Insert scan
  scene->InsertScan(scan);

  // Create image
  R3SurfelImage *image = new R3SurfelImage(scan_name);
  if (!image) {
    RNFail("Unable to allocate image\n");
    return 0;
  }

  // Assign image properties
  image->SetViewpoint(viewpoint);
  image->SetOrientation(towards, up);
  image->SetImageDimensions(width, height);
  image->SetImageCenter(center);
  image->SetXFocal(xfocal);
  image->SetYFocal(yfocal);
  image->SetScan(scan);
          
  // Insert image
  scene->InsertImage(image);

  // Return success
  return 1;
}



static int
LoadSurfels(R3SurfelScene *scene, RGBDImage *image)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("  Processing %s\n", image->Name());
    fflush(stdout);
  }

  // Get useful variables
  R3Point viewpoint = image->WorldViewpoint();
  R3Vector towards = image->WorldTowards();
  R3Vector up = image->WorldUp();
  R3Matrix intrinsics = image->Intrinsics();
  R4Matrix camera_to_world = image->CameraToWorld().Matrix();
  const char *scan_name = image->Name();
  
  // Print timing message
  RNTime step_time;
  if (print_debug) {
    printf("    A\n");
    fflush(stdout);
    step_time.Read();
  }

  // Get color and depth channels
  R2Grid *red_channel = image->RedChannel();
  R2Grid *green_channel = image->GreenChannel();
  R2Grid *blue_channel = image->BlueChannel();
  R2Grid *depth_channel = image->DepthChannel();
  if (!red_channel || !green_channel || !blue_channel || !depth_channel) {
    RNFail("Unable to read color/depth channels for image %s\n", image->Name());
    return 0;
  }

  // Resample images
  R2Grid depth_image(*depth_channel);
  R2Image color_image(*red_channel, *green_channel, *blue_channel);
  if ((max_image_resolution > 0) && (max_image_resolution != image->NPixels(RN_X))) {
    R3Matrix tmp = intrinsics;
    int xresolution = max_image_resolution;
    int yresolution = (int) (image->NPixels(RN_Y) * xresolution / (double) image->NPixels(RN_X) + 0.5);
    if (!RGBDResampleDepthImage(depth_image, intrinsics, xresolution, yresolution)) return 0;
    if (!RGBDResampleColorImage(color_image, tmp, xresolution, yresolution)) return 0;
  }
  
  // Print timing message
  if (print_debug) {
    printf("    B %g %d %d %g\n", step_time.Elapsed(), depth_image.XResolution(), depth_image.YResolution(), depth_image.Mean());
    fflush(stdout);
    step_time.Read();
  }

  // Create boundary image
  R2Grid boundary_image;
  if (!RGBDCreateBoundaryChannel(depth_image, boundary_image)) return 0;

  // Print timing message
  if (print_debug) {
    printf("    E %g\n", step_time.Elapsed());
    fflush(stdout);
    step_time.Read();
  }

  // Create position images
  R2Grid px_image, py_image, pz_image;
  if (!RGBDCreatePositionChannels(depth_image, px_image, py_image, pz_image, intrinsics, camera_to_world)) return 0;

  // Print timing message
  if (print_debug) {
    printf("    F %g\n", step_time.Elapsed());
    fflush(stdout);
    step_time.Read();
  }

  // Create normal, tangent, and radius images
  R2Grid nx_image, ny_image, nz_image;
  R2Grid tx_image, ty_image, tz_image;
  R2Grid r1_image, r2_image;
  if (!RGBDCreateTangentChannels(depth_image, 
    px_image, py_image, pz_image, boundary_image, 
    nx_image, ny_image, nz_image,
    tx_image, ty_image, tz_image,
    r1_image, r2_image,
    viewpoint, towards, up)) return 0;
  
  // Print timing message
  if (print_debug) {
    printf("    G %g\n", step_time.Elapsed());
    fflush(stdout);
    step_time.Read();
  }

  // Create segmentation
  R3Segmentation *segmentation = NULL;
  if (create_planar_segments) {
    segmentation = CreateSegmentation(px_image, py_image, pz_image,
      nx_image, ny_image, nz_image, tx_image, ty_image, tz_image,
      depth_image, r1_image, r2_image, boundary_image, color_image, 
      viewpoint, towards, up);
  }

  // Print timing message
  if (print_debug) {
    printf("    H %g\n", step_time.Elapsed());
    fflush(stdout);
    step_time.Read();
  }

  // Load surfels
  if (scene) {
    if (segmentation) {
      // Load surfels with segmentation
      LoadSurfels(scene, scan_name, segmentation, depth_image, 
        viewpoint, towards, up, intrinsics);
    }
    else {
      // Load surfels without segmentation
      LoadSurfels(scene, scan_name, segmentation, depth_image, 
        px_image, py_image, pz_image, nx_image, ny_image, nz_image,
        tx_image, ty_image, tz_image, r1_image, r2_image, boundary_image, color_image, 
        viewpoint, towards, up, intrinsics);
    }
  }

  // Print timing message
  if (print_debug) {
    printf("    J %g\n", step_time.Elapsed());
    fflush(stdout);
    step_time.Read();
  }

  // Delete segmentation
  if (segmentation) delete segmentation;
  
  // Print statistics
  if (print_debug) {
    printf("    Total = %.2f\n", start_time.Elapsed());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
LoadSurfels(R3SurfelScene *scene, RGBDConfiguration *configuration)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating surfels ...\n");
    fflush(stdout);
  }

  // Load images
  for (int i = 0; i < configuration->NImages(); i++) {
    RGBDImage *image = configuration->Image(i);
   
    // Check if should load image
    if ((load_every_kth_image > 1) && ((i % load_every_kth_image) != 0)) continue; 
    if (i < load_images_starting_at_index) continue;
    if (i > load_images_ending_at_index) continue;
    if (!load_images_bbox.IsEmpty() && !R3Contains(load_images_bbox, image->WorldViewpoint())) continue;

    // Read image
    if (!image->ReadChannels()) continue;

    // Load image
    if (!LoadSurfels(scene, image)) continue;

    // Release image
    if (!image->ReleaseChannels()) continue;
  }
  
  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Scans = %d\n", configuration->NImages());
    printf("  # Objects = %d\n", scene->NObjects());
    printf("  # Scans = %d\n", scene->NScans());
    printf("  # Images = %d\n", scene->NImages());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
CreateMultiresolutionHierarchy(R3SurfelScene *scene)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating hierarchy ...\n");
    fflush(stdout);
  }

  // Get surfel tree
  R3SurfelTree *tree = scene->Tree();
  if (!tree) {
    RNFail("Surfel scene has no tree\n");
    return 0;
  }    

  // Split nodes
  int max_parts_per_node = 8;
  int max_blocks_per_node = 32;
  RNScalar max_node_complexity = 1024;
  RNScalar max_block_complexity = 1024;
  RNLength max_leaf_extent = 1.0;
  RNLength max_block_extent = 1.0;
  int max_levels = 64;
  tree->SplitNodes(max_parts_per_node, max_blocks_per_node, 
    max_node_complexity, max_block_complexity, 
    max_leaf_extent, max_block_extent, max_levels);

  // Create multiresolution blocks
  RNScalar multiresolution_factor = 0.25;
  tree->CreateMultiresolutionBlocks(multiresolution_factor, max_block_complexity);

  // Print statistics
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
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
      else if (!strcmp(*argv, "-create_planar_segments")) { create_planar_segments = TRUE; }
      else if (!strcmp(*argv, "-create_multiresolution_hierarchy")) create_multiresolution_hierarchy = TRUE;
      else if (!strcmp(*argv, "-max_image_resolution")) { argc--; argv++; max_image_resolution = atoi(*argv); }
      else if (!strcmp(*argv, "-max_depth")) { argc--; argv++; max_depth = atof(*argv); }
      else if (!strcmp(*argv, "-pixel_stride")) { argc--; argv++; pixel_stride = atoi(*argv); }
      else if (!strcmp(*argv, "-omit_corners")) omit_corners = 1;
      else if (!strcmp(*argv, "-load_image_at_index")) { argc--; argv++; load_images_starting_at_index = load_images_ending_at_index = atoi(*argv); }
      else if (!strcmp(*argv, "-load_images_starting_at_index")) { argc--; argv++; load_images_starting_at_index = atoi(*argv); }
      else if (!strcmp(*argv, "-load_images_ending_at_index")) { argc--; argv++; load_images_ending_at_index = atoi(*argv); }
      else if (!strcmp(*argv, "-load_every_kth_image")) { argc--; argv++; load_every_kth_image = atoi(*argv); }
      else if (!strcmp(*argv, "-load_images_bbox")) {
        argc--; argv++; load_images_bbox[0][0] = atof(*argv);
        argc--; argv++; load_images_bbox[0][1] = atof(*argv);
        argc--; argv++; load_images_bbox[0][2] = atof(*argv);
        argc--; argv++; load_images_bbox[1][0] = atof(*argv);
        argc--; argv++; load_images_bbox[1][1] = atof(*argv);
        argc--; argv++; load_images_bbox[1][2] = atof(*argv);
      }      
      else {
        RNFail("Invalid program argument: %s", *argv);
        exit(1);
      }
      argv++; argc--;
    }
    else {
      if (!input_configuration_name) input_configuration_name = *argv;
      else if (!output_ssa_name) output_ssa_name = *argv;
      else if (!output_ssb_name) output_ssb_name = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!input_configuration_name || !output_ssa_name || !output_ssb_name) {
    RNFail("Usage: conf2sfl inputconfigurationfile outputssafile outputssbfile [options]\n");
    return 0;
  }

  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  // Check number of arguments
  if (!ParseArgs(argc, argv)) exit(1);

  // Read configuration
  RGBDConfiguration *configuration = ReadConfiguration(input_configuration_name);
  if (!configuration) exit(-1);

  // Open surfel scene
  R3SurfelScene *scene = OpenSurfelScene(output_ssa_name, output_ssb_name);
  if (!scene) exit(-1);

  // Create surfels
  if (!LoadSurfels(scene, configuration)) exit(-1);

  // Process surfels
  if (create_multiresolution_hierarchy) {
    if (!CreateMultiresolutionHierarchy(scene)) exit(-1);
  }

  // Close surfel scene
  if (!CloseSurfelScene(scene)) exit(-1);

  // Delete scene
  delete scene;

  // Return success 
  return 0;
}



