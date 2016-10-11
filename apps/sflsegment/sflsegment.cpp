// Source file for the surfel segmentation program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Surfels/R3Surfels.h"
#include "segmentation.cpp"



////////////////////////////////////////////////////////////////////////
// Shape types
////////////////////////////////////////////////////////////////////////

enum {
  NULL_SHAPE_TYPE,
  POINT_SHAPE_TYPE,
  LINE_SHAPE_TYPE,
  PLANE_SHAPE_TYPE,
  PLANAR_GRID_SHAPE_TYPE,
  NUM_SHAPE_TYPES
};



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static char *scene_name = NULL;
static char *database_name = NULL;
static char *parent_object_name = NULL;
static char *parent_node_name = NULL;
static char *source_node_name = NULL;
static char *mask_grid_name = NULL;
static char *support_grid_name = NULL;
static int initialize_hierarchically = TRUE;
static int min_cluster_points = 100;
static int min_clusters = 0;
static int max_clusters = 0;
static double min_cluster_spacing = 0.5;
static double min_cluster_diameter = 1;
static double min_cluster_coverage = 0.1;
static double max_cluster_diameter = 0;
static double max_cluster_shape_distance = 2;
static double max_cluster_normal_angle = 0;
static int max_neighbors = 16;
static double max_neighbor_distance = 0.5;
static double max_pair_centroid_distance = 20;
static double max_pair_shape_distance = 2;
static double max_pair_normal_angle = RN_PI / 2.0;
static double min_pair_affinity = 1.0E-6;
static double min_pair_affinity_ratio = 0;
static int max_ransac_iterations = 4;
static int max_refinement_iterations = 2;
static int silhouette_points_only = FALSE;
static int shape_type = PLANE_SHAPE_TYPE;
static int print_verbose = 0;
static int print_debug = 0;




////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////

struct Point {
public:
  Point(void);
public:
  R3SurfelPoint *point;
  int neighbors_index;
  Point *parent;
  Point *child;
  struct Cluster *cluster;
  RNScalar cluster_affinity;
  int cluster_index;
  int mark;
};

struct Shape {
  Shape(int shape_type = 0);
  Shape(const Shape& shape);
  Shape(Point *seed_point, const RNArray<Point *> *points = NULL);
  RNLength Distance(const R3Point& position) const;
  void Update(const R3Point& point);
  void Update(const R3Line& line);
  void Update(const R3Plane& plane);
  void Update(Point *seed_point = NULL, const RNArray<Point *> *points = NULL);
  void Update(const Shape& shape1, const Shape& shape2, RNScalar weight1 = 1.0, RNScalar weight2 = 1.0);
public:
  int shape_type;
  R3Box bbox;
  R3Point centroid;
  R3Line line;
  R3Plane plane;
};

struct Cluster {
public:
  Cluster(Point *seed_point = NULL, int shape_type = 0);
  Cluster(Point *seed_point, const Shape& shape);
  Cluster(Cluster *child1, Cluster *child2);
  ~Cluster(void);
  RNScalar Coverage(void);
  void EmptyPoints(void);
  void InsertPoint(Point *point, RNScalar affinity = 1.0);
  void RemovePoint(Point *point);
  int UpdatePoints(const R3Kdtree<Point *> *kdtree, const RNArray<Point *> *neighbors = NULL);
  int UpdateShape(void);
  RNScalar Affinity(Point *point) const;
  RNScalar Affinity(Cluster *cluster) const;
public:
  Point *seed_point;
  RNArray<Point *> points;
  Cluster *parent;
  RNArray<Cluster *> children;
  RNArray<struct Pair *> pairs;
  Shape shape;
  RNScalar possible_affinity; 
  RNScalar total_affinity; 
};

struct Pair {
public:
  Pair(Cluster *cluster1 = NULL, Cluster *cluster2 = NULL, RNScalar affinity = 0);
  ~Pair(void);
public:
  Cluster *clusters[2];
  int cluster_index[2];
  RNScalar affinity; 
  Pair **heapentry;
};



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

#if 0
static void
FloodCopy(R2Grid& input, R2Grid& output, int ix, int iy)
{
  // Check position
  if ((ix < 0) || (ix >= input.XResolution())) return;
  if ((iy < 0) || (iy >= input.YResolution())) return;

  // Get/check output value
  RNScalar output_value = output.GridValue(ix, iy);
  if (output_value != 0) return;

  // Get/check input value
  RNScalar input_value = input.GridValue(ix, iy);
  if (input_value == 0) return;

  // Update output grid
  output.SetGridValue(ix, iy, input_value);

  // Visit eight neighbors
  FloodCopy(input, output, ix-1, iy-1);
  FloodCopy(input, output, ix-1, iy);
  FloodCopy(input, output, ix-1, iy+1);
  FloodCopy(input, output, ix, iy-1);
  FloodCopy(input, output, ix, iy+1);
  FloodCopy(input, output, ix+1, iy-1);
  FloodCopy(input, output, ix+1, iy);
  FloodCopy(input, output, ix+1, iy+1);
}
#endif



static Pair *
FindPair(Cluster *cluster1, Cluster *cluster2) 
{
  // Swap clusters so that cluster1 has fewer pairs
  if (cluster1->pairs.NEntries() > cluster2->pairs.NEntries()) {
    Cluster *swap = cluster1; 
    cluster1 = cluster2; 
    cluster2 = swap;
  }

  // Search for pair
  for (int i = 0; i < cluster1->pairs.NEntries(); i++) {
    Pair *pair = cluster1->pairs.Kth(i);
    if (pair->clusters[0] == cluster2) return pair;
    if (pair->clusters[1] == cluster2) return pair;
  }

  // Pair not found
  return NULL;
}



static RNScalar
TotalAffinity(const RNArray<Cluster *>& clusters) 
{
  // Initialize sum
  RNScalar sum = 0;

  // Compute sum
  for (int i = 0; i < clusters.NEntries(); i++) {
    Cluster *cluster = clusters.Kth(i);
    sum += cluster->total_affinity;
  }

  // Return sum
  return sum;
}



static R3Point
PointPosition(Point *point, void *data)
{
  // Return point position
  return point->point->Position();
}



static int
CompareClusters(const void *data1, const void *data2)
{
  // Return point position
  const Cluster *cluster1 = (const Cluster *) data1;
  const Cluster *cluster2 = (const Cluster *) data2;
  if (cluster1->total_affinity > cluster2->total_affinity) return -1;
  else if (cluster1->total_affinity < cluster2->total_affinity) return 1;
  else return 0;
}



////////////////////////////////////////////////////////////////////////
// Surfel scene I/O Functions
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
// Object creation functions
////////////////////////////////////////////////////////////////////////

static R3SurfelObject *
CreateObject(R3SurfelScene *scene, 
  const RNArray<R3SurfelPoint *>& points, 
  R3SurfelNode *parent_node, R3SurfelObject *parent_object,
  const char *node_name, const char *object_name)
{
  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return NULL;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return NULL;
  if (points.IsEmpty()) return NULL;

  // Compute bounding box
  R3Box bbox = R3null_box;
  for (int j = 0; j < points.NEntries(); j++) {
    R3SurfelPoint *point = points.Kth(j);
    bbox.Union(point->Position());
  }
      
  // Create array of surfels
  R3Point origin = bbox.Centroid();
  R3Surfel *surfels = new R3Surfel [ points.NEntries() ];
  for (int j = 0; j < points.NEntries(); j++) {
    R3SurfelPoint *point = points.Kth(j);
    R3SurfelBlock *block = point->Block();
    const R3Surfel *surfel = point->Surfel();
    RNCoord x = surfel->X() + block->Origin().X() - origin.X();
    RNCoord y = surfel->Y() + block->Origin().Y() - origin.Y();
    RNCoord z = surfel->Z() + block->Origin().Z() - origin.Z();
    surfels[j].SetCoords(x, y, z);
    surfels[j].SetNormal(surfel->NX(), surfel->NY(), surfel->NZ());
    surfels[j].SetColor(surfel->Color());
    surfels[j].SetRadius(surfel->Radius());
    surfels[j].SetFlags(surfel->Flags());
  }

  // Create block
  R3SurfelBlock *block = new R3SurfelBlock(surfels, points.NEntries(), origin);
  if (!block) {
    fprintf(stderr, "Unable to create node\n");
    delete [] surfels;
    return NULL;
  }
  
  // Delete surfels
  delete [] surfels;
      
  // Insert block
  block->UpdateProperties();
  database->InsertBlock(block);

  // Create node
  R3SurfelNode *node = NULL;
  if (block && parent_node) {
    node = new R3SurfelNode(node_name);
    if (!node) {
      fprintf(stderr, "Unable to create node\n");
      delete block;
      return NULL;
    }
    
    // Insert node
    node->InsertBlock(block);
    node->UpdateProperties();
    tree->InsertNode(node, parent_node);
  }

  // Create object
  R3SurfelObject *object = NULL;
  if (node && parent_object) {
    object = new R3SurfelObject(object_name);
    if (!object) {
      fprintf(stderr, "Unable to create object\n");
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
    
  // Return object
  return object;
}



static int
CreateObjects(const RNArray<Cluster *>& clusters,
  R3SurfelScene *scene, R3SurfelPointSet *pointset, 
  R3SurfelObject *parent_object, R3SurfelNode *parent_node)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int object_count = 0;

  // Create objects 
  for (int i = 0; i < clusters.NEntries(); i++) {
    Cluster *cluster = clusters.Kth(i);

    // Create node name
    char node_name[1024];
    if (cluster->shape.shape_type == LINE_SHAPE_TYPE) {
      if (silhouette_points_only) sprintf(node_name, "sflsegment_silhouette_%d", object_count);
      else sprintf(node_name, "sflsegment_line_%d", object_count);
    }
    else if (cluster->shape.shape_type == PLANE_SHAPE_TYPE) {
      sprintf(node_name, "sflsegment_plane_%d", object_count);
    }

    // Print debug message
    if (print_debug) printf("  C %6d : %3d %6d %12.3f %12.3f : %9.3f %9.3f %9.3f %9.3f\n", i, 
        cluster->shape.shape_type, cluster->points.NEntries(), cluster->total_affinity, cluster->possible_affinity, 
        cluster->shape.plane.A(), cluster->shape.plane.B(), cluster->shape.plane.C(), cluster->shape.plane.D());

    // Create array of R3SurfelPoints
    RNArray<R3SurfelPoint *> points;
    for (int j = 0; j < cluster->points.NEntries(); j++) {
      Point *point = cluster->points.Kth(j);
      points.Insert(point->point);
    }

    // Insert points into object
    if (CreateObject(scene, points, parent_node, parent_object, node_name, node_name)) {
      object_count++;
    }
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Cluster creation functions
////////////////////////////////////////////////////////////////////////

static int
CreateSingletonClusters(RNArray<Cluster *>& clusters, const RNArray<Point *>& points,
  const R3Kdtree<Point *> *kdtree, const RNArray<Point *> *neighbors)
{
  // Create cluster for every point
  for (int i = 0; i < points.NEntries(); i++) {
    Point *point = points.Kth(i);

    // Create cluster
    Shape shape(shape_type);
    shape.Update(point);
    Cluster *cluster = new Cluster(point, shape);

    // Insert point
    cluster->InsertPoint(point, 1.0);

    // Insert cluster
    clusters.Insert(cluster);
  }

  // Return success
  return 1;
}



static int
CreateRansacClusters(RNArray<Cluster *>& clusters, const RNArray<Point *>& points, 
  const R3Kdtree<Point *> *kdtree, const RNArray<Point *> *neighbors)
{
  // Determine how many seed points to skip each iteration
  int skip = 1;
  if ((max_clusters > 0) && (points.NEntries()/(4*max_clusters) > skip))
    skip = points.NEntries()/(4*max_clusters);
  if ((min_cluster_points > 0) & ((min_cluster_points/4) > skip))
    skip = min_cluster_points/4;
  if ((min_clusters > 0) && (skip > points.NEntries()/min_clusters))
    skip = points.NEntries()/min_clusters;

  // Search seed points
  int seed_index = 0;
  while (seed_index < points.NEntries()) {
    // Find next seed point
    Point *seed_point = NULL;
    while ((seed_index < points.NEntries()) && !seed_point) {
      Point *point = points.Kth(seed_index);
      if (!point->cluster || (point->cluster_affinity < 0.1)) seed_point = point;
      seed_index += skip; 
    }

    // Check seed point
    if (!seed_point) break;

    // Create cluster
    Shape shape(shape_type);
    shape.Update(seed_point);
    Cluster *cluster = new Cluster(seed_point, shape);
    if (!cluster->UpdatePoints(kdtree, neighbors)) { delete cluster; continue; }

    // Iteratively update everything
    RNBoolean error = FALSE;
    for (int iter = 0; iter < max_ransac_iterations; iter++) {
      if (!cluster->UpdateShape()) { error = TRUE; break; }
      if (!cluster->UpdatePoints(kdtree, neighbors)) { error = TRUE; break; }
    }

    // Check for error
    if (error) { 
      delete cluster; 
      continue; 
    }

    // Insert cluster
    clusters.Insert(cluster);
  } 

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Pair creation functions
////////////////////////////////////////////////////////////////////////

static int
CreatePairs(RNArray<Pair *>& pairs, const RNArray<Cluster *>& clusters, 
  const R3Kdtree<Point *> *kdtree, const RNArray<Point *> *neighbors)
{
  // Just checking
  for (int i = 0; i < clusters.NEntries(); i++) {
    assert(clusters[i]->pairs.IsEmpty());
  }

  // Create pairs between clusters with nearby points
  for (int i = 0; i < clusters.NEntries(); i++) {
    Cluster *cluster0 = clusters.Kth(i);
    if (cluster0->points.IsEmpty()) continue;
    Point *point0 = cluster0->points.Head();
    int index0 = point0->neighbors_index;

    // Create pairs
    for (int j = 0; j < neighbors[index0].NEntries(); j++) {
      Point *point1 = neighbors[index0].Kth(j);
      if (point0 == point1) continue;
      Cluster *cluster1 = point1->cluster;
      if (!cluster1) continue;
      if (cluster0 == cluster1) continue;

      // Check if already have pair
      if (FindPair(cluster0, cluster1)) continue;

      // Compute affinity
      RNScalar affinity = cluster0->Affinity(cluster1);
      if (affinity < min_pair_affinity) continue;

      // Create pair
      Pair *pair = new Pair(cluster0, cluster1, affinity);
      if (!pair) continue;

      // Insert pair
      pairs.Insert(pair);
    }
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Clustering manipulation functions
////////////////////////////////////////////////////////////////////////

static int
RefineClusters(RNArray<Cluster *>& clusters, 
  const R3Kdtree<Point *> *kdtree, const RNArray<Point *> *neighbors)
{
  // Iteratively update everything
  int max_iterations = 1;
  for (int iter = 0; iter < max_iterations; iter++) {
    // Copy list of clusters
    RNArray<Cluster *> tmp = clusters;
    tmp.Sort(CompareClusters);

    // Will rebuild list of clusters
    clusters.Empty();

    // Refine each cluster
    RNBoolean converged = TRUE;
    for (int i = 0; i < tmp.NEntries(); i++) {
      Cluster *cluster = tmp.Kth(i);
      int prev_npoints = cluster->points.NEntries();

      // Refine cluster
      RNBoolean error = FALSE;
      if (!error && !cluster->UpdateShape()) error = TRUE;
      if (!error && !cluster->UpdatePoints(kdtree, neighbors)) error = TRUE; 

      // Insert cluster
      if (!error) clusters.Insert(cluster);
      else delete cluster;

      // Check for convergence
      if (error || (prev_npoints != cluster->points.NEntries())) {
        converged = FALSE;
      }
    }

    // Check if converged
    if (converged) break;
  }

  // Return success
  return 1;
}



static int
DeleteClusters(RNArray<Cluster *>& clusters)
{
  // Separate viable from nonviable ones
  RNArray<Cluster *> viable_clusters;
  RNArray<Cluster *> nonviable_clusters;
  for (int i = 0; i < clusters.NEntries(); i++) {
    Cluster *cluster = clusters.Kth(i);

    // Check cluster points
    if (min_cluster_points > 0) {
      if (cluster->points.NEntries() < min_cluster_points) {
        nonviable_clusters.Insert(cluster);
        continue;
      }
    }

    // Check cluster coverage
    if (min_cluster_coverage > 0) {
      if (cluster->Coverage() < min_cluster_coverage) {
        nonviable_clusters.Insert(cluster);
        continue;
      }
    }

    // Check max_clusters
    if (max_clusters > 0) {
      if (viable_clusters.NEntries() > max_clusters) {
        nonviable_clusters.Insert(cluster);
        continue;
      }
    }

    // Cluster is viable
    viable_clusters.Insert(cluster);
  }

  // Delete nonviable clusters
  for (int i = 0; i < nonviable_clusters.NEntries(); i++) {
    Cluster *cluster = nonviable_clusters.Kth(i);
    delete cluster;
  }

  // Replace clusters with viable ones
  clusters = viable_clusters;

  // Return success
  return 1;
}



static int
MergeClusters(RNArray<Cluster *>& clusters, 
  const R3Kdtree<Point *> *kdtree, const RNArray<Point *> *neighbors)
{
  // Initialize statistics
  int merge_count = 0;
  int push_count = 0;

  //////////

  // Create pairs
  RNArray<Pair *> pairs;
  if (!CreatePairs(pairs, clusters, kdtree, neighbors)) return 0;
  if (pairs.IsEmpty()) return 1;

  //////////

  // Initialize heap
  Pair tmp;
  RNHeap<Pair *> heap(&tmp, &tmp.affinity, &tmp.heapentry, FALSE);
  for (int i = 0; i < pairs.NEntries(); i++) {
    Pair *pair = pairs.Kth(i);
    heap.Push(pair);
  }

  // Merge clusters hierarchically
  while (!heap.IsEmpty()) {
    // Get pair
    Pair *pair = heap.Pop();

    // Check if we are done
    if (pair->affinity < min_pair_affinity) break;

    // Get clusters
    Cluster *cluster0 = pair->clusters[0];
    Cluster *cluster1 = pair->clusters[1];

    // Check if either cluster has already been merged
    if (cluster0->parent || cluster1->parent) {
      // Find ancestors
      Cluster *ancestor0 = cluster0;
      Cluster *ancestor1 = cluster1;
      while (ancestor0->parent) ancestor0 = ancestor0->parent;
      while (ancestor1->parent) ancestor1 = ancestor1->parent;
      if (ancestor0 != ancestor1) {
        if (!FindPair(ancestor0, ancestor1)) {
          RNScalar affinity = ancestor0->Affinity(ancestor1);
          if (affinity > min_pair_affinity) {
            // Create a pair between the ancestors
            Pair *pair = new Pair(ancestor0, ancestor1, affinity);
            heap.Push(pair);
            push_count++;
          }
        }
      }
    }
    else {
      if (print_debug) {
        static unsigned long count = 0;
        if ((count++ % 1000) == 0) {
          printf("  %15.12f : %9d %9d : %15d %15d %15d\n", pair->affinity, 
                 cluster0->points.NEntries(), cluster1->points.NEntries(), 
                 heap.NEntries(), merge_count, push_count);
        }
      }
    
      // Create merged cluster
      Cluster *cluster = new Cluster(cluster0, cluster1);
      clusters.Insert(cluster);
      merge_count++;
    }

    // Delete pair
    delete pair;
  }

  // if (print_debug) printf("M %d %d\n", merge_count, push_count);

  // Delete merged clusters
  RNArray<Cluster *> tmp_clusters = clusters;
  clusters.Empty();
  for (int i = 0; i < tmp_clusters.NEntries(); i++) {
    Cluster *cluster = tmp_clusters.Kth(i);
    if (cluster->parent) delete cluster;
    else clusters.Insert(cluster);
  }

  // Return success
  return 1;
}



#if 0
static int
SplitClusters(RNArray<Cluster *>& clusters)
{
  // Check min clusters spacing
  if (min_cluster_spacing <= 0) return 1;

  // Split connected components
  RNArray<Cluster *> tmp = clusters;
  clusters.Empty();
  for (int i = 0; i < tmp.NEntries(); i++) {
    Cluster *cluster = tmp.Kth(i);

    // Check cluster
    if (cluster->points.NEntries() < min_cluster_points) continue;

    // Rasterize points into planar grid
    R3PlanarGrid grid(cluster->shape.plane, cluster->shape.bbox, min_cluster_spacing);
    for (int j = 0; j < cluster->points.NEntries(); j++) {
      Point *point = cluster->points.Kth(j);
      R3Point position = point->point->Position();
      grid.RasterizeWorldPoint(position.X(), position.Y(), position.Z(), 1.0);
    }

    // Compute connected components
    int max_components = grid.NEntries();
    int *components = new int [ max_components ];
    int ncomponents = grid.ConnectedComponents(RN_EPSILON, max_components, NULL, NULL, components);

    // Check connected components
    if (ncomponents == 1) {
      // One connected component - simply insert cluster
      clusters.Insert(cluster);
    }
    else {
      // Create cluster for each connnected component
      for (int j = 0; j < ncomponents; j++) {
        // Make array of points in component
        RNArray<Point *> component_points;
        for (int k = 0; k < cluster->points.NEntries(); k++) {
          Point *point = cluster->points.Kth(k);
          R3Point world_position = point->point->Position();
          R2Point grid_position = grid.GridPosition(world_position);
          int ix = (int) (grid_position.X() + 0.5);
          int iy = (int) (grid_position.Y() + 0.5);
          int index; grid.Grid().IndicesToIndex(ix, iy, index);
          if (components[index] != j) continue;
          component_points.Insert(point);
        }

        // Check number of points
        if (component_points.NEntries() > min_cluster_points) {

          // Find centroid
          R3Point centroid = R3zero_point;
          for (int k = 0; k < component_points.NEntries(); k++) {
            Point *point = component_points.Kth(k);
            R3Point world_position = point->point->Position();
            centroid += world_position;
          }
          centroid /= component_points.NEntries();

          // Find seed point
          Point *seed_point = NULL;
          RNLength min_dd = FLT_MAX;
          for (int k = 0; k < component_points.NEntries(); k++) {
            Point *point = component_points.Kth(k);
            R3Point world_position = point->point->Position();
            RNLength dd = R3SquaredDistance(centroid, world_position);
            if (dd < min_dd) { seed_point = point; min_dd = dd; }
          }

          // Check seed point
          if (seed_point) {
            // Create cluster
            Cluster *c = new Cluster(seed_point, PLANE_SHAPE_TYPE);
            c->possible_affinity = cluster->possible_affinity;

            // Insert points into cluster
            for (int k = 0; k < component_points.NEntries(); k++) {
              Point *point = component_points.Kth(k);
              c->InsertPoint(point);
            }

            // Update shape
            c->UpdateShape();

            // Update planar grid
            // c->UpdatePlanarGrid();

            // Insert cluster
            clusters.Insert(c);
          }
        }
      }

      // Delete the original cluster
      delete cluster;
    }

    // Delete components
    delete [] components;
  }

  // Return success
  return 1;
}
#endif



static int
InitializeClusters(RNArray<Cluster *>& clusters, 
  const RNArray<Point *>& points, 
  const R3Kdtree<Point *> * kdtree,
  RNArray<Point *> *neighbors)
{
  // Create clusters
  if (initialize_hierarchically) {
    if (!CreateSingletonClusters(clusters, points, kdtree, neighbors)) return 0;
    if (!MergeClusters(clusters, kdtree, neighbors)) return 0;
  }
  else {
    if (!CreateRansacClusters(clusters, points, kdtree, neighbors)) return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Top-level segmentation functions
////////////////////////////////////////////////////////////////////////

static int
SegmentPoints(R3SurfelScene *scene, R3SurfelPointSet *pointset, 
  R3SurfelObject *parent_object, R3SurfelNode *parent_node)
{
  // Get convenient variables
  RNTime step_time;
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;
  if (pointset->NPoints() < min_cluster_points) return 1;
  if (!parent_object) parent_object = scene->RootObject();
  if (!parent_node) parent_node = tree->RootNode();

  // Print debug message
  if (print_debug) {
    printf("HEREA %d\n", pointset->NPoints());
    step_time.Read();
  }

  // Allocate points 
  int npoints = pointset->NPoints();
  Point *points = new Point [ npoints ];
  for (int i = 0; i < npoints; i++) {
    Point *point = &points[i];
    point->point = pointset->Point(i);
  }

  // Print debug message
  if (print_debug) {
    printf("HEREB %.3f\n", step_time.Elapsed());
    step_time.Read();
  }

  // Create array of points to segment
  RNArray<Point *> array;
  for (int i = 0; i < npoints; i++) {
    Point *point = &points[i];
    if (silhouette_points_only && !point->point->IsOnSilhouetteBoundary()) continue;
    array.Insert(point);
  }

  // Create kdtree of points
  R3Kdtree<Point *> kdtree(array, PointPosition);

  // Create arrays of neighbor points
  RNArray<Point *> *neighbors = NULL;
  int neighbor_count = 0;
  if (max_neighbor_distance > 0) {
    neighbors = new RNArray<Point *> [ array.NEntries() ];
    for (int i = 0; i < array.NEntries(); i++) {
      Point *point = array.Kth(i);
      point->neighbors_index = i;
      kdtree.FindClosest(point, 0, max_neighbor_distance, max_neighbors, neighbors[i]);
      neighbor_count += neighbors[i].NEntries();
    }
  }

  // Print debug message
  if (print_debug) {
    printf("HEREC %.3f %d\n", step_time.Elapsed(), neighbor_count);
    step_time.Read();
  }

  // Create clusters
  RNArray<Cluster *> clusters;
  if (!InitializeClusters(clusters, array, &kdtree, neighbors)) return 0;
  if (clusters.IsEmpty()) return 0;

  // Print debug message
  if (print_debug) {
    printf("HERED %.3f %d %g\n", step_time.Elapsed(), clusters.NEntries(), TotalAffinity(clusters));
    step_time.Read();
  }

  // Iteratively update clusters
  for (int i = 0; i < max_refinement_iterations; i++) {
    // Refine clusters
    if (!RefineClusters(clusters, &kdtree, neighbors)) return 0;

    // Print debug message
    if (print_debug) {
      printf("HEREE %d : %.3f %d %g\n", i, step_time.Elapsed(), clusters.NEntries(), TotalAffinity(clusters));
      step_time.Read();
    }

    // Create clusters
    if (!CreateRansacClusters(clusters, array, &kdtree, neighbors)) return 0;

    // Print debug message
    if (print_debug) {
      printf("HEREF %d : %.3f %d %g\n", i, step_time.Elapsed(), clusters.NEntries(), TotalAffinity(clusters));
      step_time.Read();
    }

    // Merge clusters
    if (!MergeClusters(clusters, &kdtree, neighbors)) return 0;

    // Print debug message
    if (print_debug) {
      printf("HEREG %d : %.3f %d %g\n", i, step_time.Elapsed(), clusters.NEntries(), TotalAffinity(clusters));
      step_time.Read();
    }

    // Delete clusters
    if (!DeleteClusters(clusters)) return 0;

    // Print debug message
    if (print_debug) {
      printf("HEREH %d : %.3f %d %g\n", i, step_time.Elapsed(), clusters.NEntries(), TotalAffinity(clusters));
      step_time.Read();
    }
  }

  // Split clusters
  // if (!SplitClusters(clusters)) return 0;

  // Print debug message
  if (print_debug) {
    printf("HEREI %.3f %d %g\n", step_time.Elapsed(), clusters.NEntries(), TotalAffinity(clusters));
    step_time.Read();
  }

  // Delete clusters
  if (!DeleteClusters(clusters)) return 0;

  // Print debug message
  if (print_debug) {
    printf("HEREJ %.3f %d %g\n", step_time.Elapsed(), clusters.NEntries(), TotalAffinity(clusters));
    step_time.Read();
  }

  // Sort clusters
  clusters.Sort(CompareClusters);

  // Create objects for clusters
  if (!CreateObjects(clusters, scene, pointset, parent_object, parent_node)) return 0;

  // Print debug message
  if (print_debug) {
    printf("HEREK %.3f %d %d\n", step_time.Elapsed(), clusters.NEntries(), scene->NObjects());
    step_time.Read();
  }

  // Make array of unclustered points
  RNArray<R3SurfelPoint *> unclustered_points;
  for (int i = 0; i < npoints; i++) {
    Point *point = &points[i];
    if (point->cluster) continue;
    unclustered_points.Insert(point->point);
  }

  // Create objects for unclustered points
  if (unclustered_points.NEntries() > 0) {
    char node_name[1024];
    sprintf(node_name, "Unclustered");
    CreateObject(scene, unclustered_points, parent_node, NULL, node_name, NULL);
  }

  // Print debug message
  if (print_debug) {
    printf("HEREL %.3f %d\n", step_time.Elapsed(), unclustered_points.NEntries());
    step_time.Read();
  }

  // Delete clusters
  for (int i = 0; i < clusters.NEntries(); i++) {
    Cluster *cluster = clusters.Kth(i);
    delete cluster;
  }

  // Delete data
  delete [] neighbors;
  delete [] points;

  // Return success
  return 1;
}



#if 1

static int
SegmentPoints(R3SurfelScene *scene, 
  const char *parent_object_name, 
  const char *parent_node_name, 
  const char *source_node_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating segments ...\n");
    fflush(stdout);
  }

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Find parent object
  R3SurfelObject *parent_object = scene->RootObject();
  if (parent_object_name) {
    parent_object = scene->FindObjectByName(parent_object_name);
    if (!parent_object) {
      fprintf(stderr, "Unable to find parent object with name %s\n", parent_object_name);
      return 0;
    }
  }

  // Find parent node
  R3SurfelNode *parent_node = tree->RootNode();
  if (parent_node_name) {
    parent_node = tree->FindNodeByName(parent_node_name);
    if (!parent_node) {
      fprintf(stderr, "Unable to find parent node with name %s\n", parent_node_name);
      return 0;
    }
  }

  // Find source node
  R3SurfelNode *source_node = tree->RootNode();
  if (source_node_name) {
    source_node = tree->FindNodeByName(source_node_name);
    if (!source_node) {
      fprintf(stderr, "Unable to find source node with name %s\n", source_node_name);
      return 0;
    }
  }

  // Split nodes
  // tree->SplitNodes(source_node);

  // Create a set of blocks in leaf nodes to split
  RNArray<R3SurfelBlock *> blocks;
  RNArray<R3SurfelNode *> stack;
  stack.InsertTail(source_node);
  while (!stack.IsEmpty()) {
    R3SurfelNode *node = stack.Tail();
    stack.RemoveTail();
    if (node->NParts() == 0) {
      for (int i = 0; i < node->NBlocks(); i++) {
        blocks.Insert(node->Block(i));
      }
    }
    else {
      for (int i = 0; i < node->NParts(); i++) {
        stack.InsertTail(node->Part(i));
      }
    }
  }

  // Segment blocks 
  for (int i = 0; i < blocks.NEntries(); i++) {
    R3SurfelBlock *block = blocks.Kth(i);
    if (block->NSurfels() == 0) continue;
    R3SurfelNode *node = block->Node();
    if (!node) continue;
    if (node->NParts() > 0) continue;
    R3SurfelObject *object = node->Object();
    if (object) continue;
    
    // Print debug statement
    if (print_debug) {
      printf("  %6d/%6d : ", i, blocks.NEntries());
      fflush(stdout);
    }

    // Create pointset from block
    R3SurfelPointSet *pointset = new R3SurfelPointSet(block);
    if (!pointset) continue;

    // Print debug statement
    if (print_debug) {
      printf("%s %9d\n", node->Name(), pointset->NPoints());
      fflush(stdout);
    }

    // Segment points
    if (!SegmentPoints(scene, pointset, parent_object, node)) {
      fprintf(stderr, "Error in clustering\n");
      delete pointset; 
      return 0; 
    }

    // Delete pointset
    delete pointset;

    // Delete block
    node->RemoveBlock(block);
    database->RemoveBlock(block);
    delete block;
  }

  // Print message
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    fflush(stdout);
  }

  // Return success
  return 1;
}

#else

static int
SegmentPoints(R3SurfelScene *scene, 
  const char *parent_object_name, 
  const char *parent_node_name, 
  const char *source_node_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Creating segments ...\n");
    fflush(stdout);
  }

  // Get convenient variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Find parent object
  R3SurfelObject *parent_object = scene->RootObject();
  if (parent_object_name) {
    parent_object = scene->FindObjectByName(parent_object_name);
    if (!parent_object) {
      fprintf(stderr, "Unable to find parent object with name %s\n", parent_object_name);
      return 0;
    }
  }

  // Find parent node
  R3SurfelNode *parent_node = tree->RootNode();
  if (parent_node_name) {
    parent_node = tree->FindNodeByName(parent_node_name);
    if (!parent_node) {
      fprintf(stderr, "Unable to find parent node with name %s\n", parent_node_name);
      return 0;
    }
  }

  // Find source node
  R3SurfelNode *source_node = tree->RootNode();
  if (source_node_name) {
    source_node = tree->FindNodeByName(source_node_name);
    if (!source_node) {
      fprintf(stderr, "Unable to find source node with name %s\n", source_node_name);
      return 0;
    }
  }

  // Remember original blocks
  RNArray<R3SurfelBlock *> original_blocks;
  RNArray<R3SurfelNode *> stack;
  stack.InsertTail(source_node);
  while (!stack.IsEmpty()) {
    R3SurfelNode *node = stack.Tail();
    stack.RemoveTail();
    for (int i = 0; i < node->NBlocks(); i++) {
      blocks.Insert(node->Block(i));
    }
    for (int i = 0; i < node->NParts(); i++) {
      stack.InsertTail(node->Part(i));
    }
  }

  // Process chunk-by-chunk
  const R3Box& scene_bbox = scene->BBox();
  double chunk_size = 10;
  if (chunk_size <= 0) chunk_size = scene_bbox.DiagonalLength() / 10;
  if (chunk_size <= 0) return 0;
  int nxchunks = (int) (scene_bbox.XLength() / chunk_size) + 1;
  int nychunks = (int) (scene_bbox.YLength() / chunk_size) + 1;
  RNLength xchunk = scene_bbox.XLength() / nxchunks;
  RNLength ychunk = scene_bbox.YLength() / nychunks;
  for (int j = 0; j < nychunks; j++) {
    for (int i = 0; i < nxchunks; i++) {
      // Compute chunk bounding box
      R3Box chunk_bbox = scene_bbox;
      chunk_bbox[0][0] = scene_bbox.XMin() + i     * xchunk;
      chunk_bbox[1][0] = scene_bbox.XMin() + (i+1) * xchunk;
      chunk_bbox[0][1] = scene_bbox.YMin() + j     * ychunk;
      chunk_bbox[1][1] = scene_bbox.YMin() + (j+1) * ychunk;

      // Compute pointset for chunk (inside box, not in object)
      R3SurfelMultiConstraint multi_constraint;
      R3SurfelBoxConstraint box_constraint(chunk_bbox);
      R3SurfelObjectConstraint object_constraint(NULL, TRUE);
      multi_constraint.InsertConstraint(&box_constraint);
      multi_constraint.InsertConstraint(&object_constraint);
      R3SurfelPointSet *pointset = CreatePointSet(scene, source_node, &multi_constraint);
      if (!pointset) continue;
      if (pointset->NPoints() == 0) { delete pointset; continue; }

      // Print debug statement
      if (print_debug) {
        printf("  %6d/%6d %6d/%6d\n : %d", i, nxchunks, j, nychunks, pointset->NPoints());
        fflush(stdout);
      }

      // Segment points
      if (!SegmentPoints(scene, pointset, parent_object, parent_node)) {
        fprintf(stderr, "Error in clustering\n");
        delete pointset; 
        return 0; 
      }

      // Delete pointset
      delete pointset;
    }
  }
  
  // Delete original blocks
  for (int i = 0; i < blocks.NEntries(); i++) {    
    R3SurfelBlock *block = blocks.Kth(i);
    R3SurfelNode *node = block->Node();
    if (!node) continue;
    node->RemoveBlock(block);
    database->RemoveBlock(block);
    delete block;
  }

  // Print message
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    fflush(stdout);
  }

  // Return success
  return 1;
}

#endif



////////////////////////////////////////////////////////////////////////
// Support surface segmentation functions
////////////////////////////////////////////////////////////////////////

static int
SegmentSupportGrid(R3SurfelScene *scene, 
  const char *parent_object_name, 
  const char *source_node_name,
  const char *parent_node_name,
  const char *support_grid_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int nobjects = 0;
  if (print_verbose) {
    printf("Creating support grid object ...\n");
    fflush(stdout);
  }

  // Get tree
  R3SurfelTree *tree = scene->Tree();
  if (!tree) {
    fprintf(stderr, "Scene has no tree\n");
    return 0;
  }    

  // Find parent object
  R3SurfelObject *parent_object = scene->RootObject();
  if (parent_object_name) {
    parent_object = scene->FindObjectByName(parent_object_name);
    if (!parent_object) {
      fprintf(stderr, "Unable to find parent object with name %s\n", parent_object_name);
      return 0;
    }
  }

  // Find source node
  R3SurfelNode *source_node = tree->RootNode();
  if (source_node_name) {
    source_node = tree->FindNodeByName(source_node_name);
    if (!source_node) {
      fprintf(stderr, "Unable to find source node with name %s\n", source_node_name);
      return 0;
    }
  }

  // Read support grid
  R2Grid *support_grid = new R2Grid();
  if (!support_grid->ReadFile(support_grid_name)) return 0;

  // Create constraint
  R3SurfelMultiConstraint constraint;
  R3SurfelObjectConstraint non_object_constraint(NULL, TRUE);
  R3SurfelOverheadGridConstraint support_constraint(support_grid, R3_SURFEL_CONSTRAINT_EQUAL, 
   R3_SURFEL_CONSTRAINT_Z, R3_SURFEL_CONSTRAINT_VALUE, 0, 0, 0.25);
  constraint.InsertConstraint(&non_object_constraint);
  constraint.InsertConstraint(&support_constraint);

  // Split source node
  RNArray<R3SurfelNode *> support_nodes, not_support_nodes;
  tree->SplitLeafNodes(source_node, constraint, &support_nodes, &not_support_nodes);
  if (support_nodes.NEntries() > 0) {
    // Create object name from grid name
    char object_name[1024];
    const char *grid_namep = strstr(support_grid_name, "/");
    if (grid_namep) grid_namep++;
    else grid_namep = support_grid_name;
    strcpy(object_name, grid_namep);
    char *object_namep = strrchr(object_name, '.');
    if (object_namep) *object_namep = '\0';

    // Create object
    R3SurfelObject *object = new R3SurfelObject(object_name);

    // Set names of nodes and insert them into object
    for (int i = 0; i < support_nodes.NEntries(); i++) {
      R3SurfelNode *node = support_nodes.Kth(i);
      if (node->Object()) continue;
      char node_name[1024];
      sprintf(node_name, "Support_%d", i);
      node->SetName(node_name);
      object->InsertNode(node);
    }

    // Set names of other nodes
    for (int i = 0; i < not_support_nodes.NEntries(); i++) {
      R3SurfelNode *node = not_support_nodes.Kth(i);
      char node_name[1024];
      sprintf(node_name, "Not_Support_%d", i);
      node->SetName(node_name);
    }

    // Insert object into scene
    scene->InsertObject(object, parent_object);

    // Update properties
    object->UpdateProperties();

    // Update statistics
    nobjects++;
  }

  // Print message
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Objects = %d\n", nobjects);
    printf("  # Nodes = %d\n", support_nodes.NEntries());
    fflush(stdout);
  }

  // Delete grid
  if (support_grid) delete support_grid;

  // Return success
  return 1;
}



static int
SegmentMaskGrid(R3SurfelScene *scene, 
  const char *parent_object_name, 
  const char *source_node_name,
  const char *parent_node_name,
  const char *mask_grid_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int nobjects = 0;
  if (print_verbose) {
    printf("Creating mask grid object ...\n");
    fflush(stdout);
  }

  // Get tree
  R3SurfelTree *tree = scene->Tree();
  if (!tree) {
    fprintf(stderr, "Scene has no tree\n");
    return 0;
  }    

  // Find parent object
  R3SurfelObject *parent_object = scene->RootObject();
  if (parent_object_name) {
    parent_object = scene->FindObjectByName(parent_object_name);
    if (!parent_object) {
      fprintf(stderr, "Unable to find parent object with name %s\n", parent_object_name);
      return 0;
    }
  }

  // Find source node
  R3SurfelNode *source_node = tree->RootNode();
  if (source_node_name) {
    source_node = tree->FindNodeByName(source_node_name);
    if (!source_node) {
      fprintf(stderr, "Unable to find source node with name %s\n", source_node_name);
      return 0;
    }
  }

  // Read mask grid
  R2Grid *mask_grid = new R2Grid();
  if (!mask_grid->ReadFile(mask_grid_name)) return 0;

  // Create constraint
  R3SurfelMultiConstraint constraint;
  R3SurfelObjectConstraint non_object_constraint(NULL, TRUE);
  R3SurfelOverheadGridConstraint mask_constraint(mask_grid, R3_SURFEL_CONSTRAINT_GREATER, 
   R3_SURFEL_CONSTRAINT_OPERAND, R3_SURFEL_CONSTRAINT_VALUE, 0.5, 0, 0);
  constraint.InsertConstraint(&non_object_constraint);
  constraint.InsertConstraint(&mask_constraint);

  // Split source node
  RNArray<R3SurfelNode *> mask_nodes, not_mask_nodes;
  tree->SplitLeafNodes(source_node, constraint, &mask_nodes, &not_mask_nodes);
  if (mask_nodes.NEntries() > 0) {
    // Create object name from grid name
    char object_name[1024];
    const char *grid_namep = strstr(mask_grid_name, "/");
    if (grid_namep) grid_namep++;
    else grid_namep = mask_grid_name;
    strcpy(object_name, grid_namep);
    char *object_namep = strrchr(object_name, '.');
    if (object_namep) *object_namep = '\0';

    // Create object
    R3SurfelObject *object = new R3SurfelObject(object_name);

    // Insert nodes into object
    for (int i = 0; i < mask_nodes.NEntries(); i++) {
      R3SurfelNode *node = mask_nodes.Kth(i);
      if (node->Object()) continue;
      char node_name[1024];
      sprintf(node_name, "Mask_%d", i);
      node->SetName(node_name);
      object->InsertNode(node);
    }

    // Set names of other nodes
    for (int i = 0; i < not_mask_nodes.NEntries(); i++) {
      R3SurfelNode *node = not_mask_nodes.Kth(i);
      char node_name[1024];
      sprintf(node_name, "Not_Mask_%d", i);
      node->SetName(node_name);
    }

    // Insert object into scene
    scene->InsertObject(object, parent_object);

    // Update properties
    object->UpdateProperties();

    // Update statistics
    nobjects++;
  }

  // Print message
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Objects = %d\n", nobjects);
    printf("  # Nodes = %d\n", mask_nodes.NEntries());
    fflush(stdout);
  }

  // Delete grid
  if (mask_grid) delete mask_grid;

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
      else if (!strcmp(*argv, "-lines")) shape_type = LINE_SHAPE_TYPE;
      else if (!strcmp(*argv, "-planes")) shape_type = PLANE_SHAPE_TYPE;
      else if (!strcmp(*argv, "-silhouette_points_only")) silhouette_points_only = TRUE;
      else if (!strcmp(*argv, "-parent_object")) { argc--; argv++; parent_object_name = *argv; }
      else if (!strcmp(*argv, "-parent_node")) { argc--; argv++; parent_node_name = *argv; }
      else if (!strcmp(*argv, "-source_node")) { argc--; argv++; source_node_name = *argv; }
      else if (!strcmp(*argv, "-mask_grid")) { argc--; argv++; mask_grid_name = *argv; }
      else if (!strcmp(*argv, "-support_grid")) { argc--; argv++; support_grid_name = *argv; }
      else if (!strcmp(*argv, "-ransac")) initialize_hierarchically = FALSE; 
      else if (!strcmp(*argv, "-hierarchical")) initialize_hierarchically = TRUE; 
      else if (!strcmp(*argv, "-no_refinements")) max_refinement_iterations = 0; 
      else if (!strcmp(*argv, "-max_ransac_iterations")) { argc--; argv++; max_ransac_iterations = atoi(*argv); }
      else if (!strcmp(*argv, "-max_refinement_iterations")) { argc--; argv++; max_refinement_iterations = atoi(*argv); }
      else if (!strcmp(*argv, "-min_cluster_points")) { argc--; argv++; min_cluster_points = atoi(*argv); }
      else if (!strcmp(*argv, "-min_cluster_diameter")) { argc--; argv++; min_cluster_diameter = atof(*argv); }
      else if (!strcmp(*argv, "-min_cluster_coverage")) { argc--; argv++; min_cluster_coverage = atof(*argv); }
      else if (!strcmp(*argv, "-min_cluster_spacing")) { argc--; argv++; min_cluster_spacing = atof(*argv); }
      else if (!strcmp(*argv, "-max_cluster_shape_distance")) { argc--; argv++; max_cluster_shape_distance = atof(*argv); }
      else if (!strcmp(*argv, "-max_cluster_normal_angle")) { argc--; argv++; max_cluster_normal_angle = atof(*argv); }
      else if (!strcmp(*argv, "-max_neighbors")) { argc--; argv++; max_neighbors = atoi(*argv); }
      else if (!strcmp(*argv, "-max_neighbor_distance")) { argc--; argv++; max_neighbor_distance = atof(*argv); }
      else if (!strcmp(*argv, "-max_pair_centroid_distance")) { argc--; argv++; max_pair_centroid_distance = atof(*argv); }
      else if (!strcmp(*argv, "-max_pair_shape_distance")) { argc--; argv++; max_pair_shape_distance = atof(*argv); }
      else if (!strcmp(*argv, "-max_pair_normal_angle")) { argc--; argv++; max_pair_normal_angle = atof(*argv); }
      else if (!strcmp(*argv, "-min_pair_affinity")) { argc--; argv++; min_pair_affinity = atof(*argv); }
      else if (!strcmp(*argv, "-min_pair_affinity_ratio")) { argc--; argv++; min_pair_affinity_ratio = atof(*argv); }
      else if (!strcmp(*argv, "-large_outdoor_planes")) {
        shape_type = PLANE_SHAPE_TYPE;
        min_cluster_points = 10000;
        min_cluster_diameter = 4;
        max_cluster_diameter = 0;
        max_cluster_shape_distance = 1;
        max_cluster_normal_angle = RN_PI / 4.0;
        max_pair_centroid_distance = 50;
        max_pair_shape_distance = 1;
        max_pair_normal_angle = RN_PI / 4.0;
        max_neighbor_distance = 0.5;
        min_pair_affinity = 1.0E-6;
      }
      else if (!strcmp(*argv, "-outdoor_planes")) {
        shape_type = PLANE_SHAPE_TYPE;
        min_cluster_points = 100;
        min_cluster_diameter = 1;
        max_cluster_diameter = 0;
        max_cluster_shape_distance = 1;
        max_cluster_normal_angle = RN_PI / 3.0;
        max_pair_centroid_distance = 50;
        max_pair_shape_distance = 1;
        max_pair_normal_angle = RN_PI / 3.0;
        max_neighbor_distance = 0.5;
        min_pair_affinity = 1.0E-6;
      }
      else if (!strcmp(*argv, "-outdoor_objects") || !strcmp(*argv, "-small_objects")) {
        min_cluster_points = 10;
        min_cluster_diameter = 1;
        max_cluster_diameter = 20;
        max_cluster_shape_distance = 0;
        max_cluster_normal_angle = 0;
        max_pair_centroid_distance = 10;
        max_pair_shape_distance = 0;
        max_pair_normal_angle = 0;
        max_neighbor_distance = 0.5;
        min_pair_affinity_ratio = 0.75;
        min_pair_affinity = 1.0E-6;
      }
      else if (!strcmp(*argv, "-indoor_planes")) {
        shape_type = PLANE_SHAPE_TYPE;
        min_cluster_points = 100;
        min_cluster_diameter = 0.25;
        max_cluster_diameter = 16;
        max_cluster_shape_distance = 0.1;
        max_cluster_normal_angle = RN_PI / 3.0;
        min_cluster_spacing = 0.05;
        max_pair_centroid_distance = 16;
        max_pair_shape_distance = 0.1;
        max_pair_normal_angle = RN_PI / 3.0;
        max_neighbor_distance = 0.1;
        max_neighbors = 6;
        min_pair_affinity = 1.0E-6;
      }
      else if (!strcmp(*argv, "-indoor_superpixels")) {
        min_cluster_points = 10;
        min_cluster_diameter = 0.1;
        max_cluster_diameter = 0;
        max_cluster_shape_distance = 0.1;
        max_cluster_normal_angle = RN_PI / 3.0;
        min_cluster_spacing = 0.05;
        max_pair_centroid_distance = 4;
        max_pair_shape_distance = 0.1;
        max_pair_normal_angle = RN_PI / 3.0;
        max_neighbor_distance = 0.1;
        max_neighbors = 6;
        min_pair_affinity = 1.0E-6;
      }
      else if (!strcmp(*argv, "-indoor_objects")) {
        min_cluster_points = 10;
        min_cluster_diameter = 1;
        max_cluster_diameter = 10;
        max_cluster_shape_distance = 0;
        max_cluster_normal_angle = 0;
        min_cluster_spacing = 0.05;
        max_pair_centroid_distance = 4;
        max_pair_shape_distance = 0;
        max_pair_normal_angle = 0;
        max_neighbor_distance = 0.25;
        min_pair_affinity_ratio = 0.75;
        min_pair_affinity = 1.0E-6;
      }
      else if (!strcmp(*argv, "-sun3d_planes")) {
        shape_type = PLANE_SHAPE_TYPE;
        min_cluster_points = 100;
        min_cluster_diameter = 0.25;
        max_cluster_diameter = 16;
        max_cluster_shape_distance = 0.1;
        max_cluster_normal_angle = RN_PI / 4.0;
        min_cluster_spacing = 0.05;
        max_pair_centroid_distance = 16;
        max_pair_shape_distance = 0.1;
        max_pair_normal_angle = RN_PI / 4.0;
        max_neighbor_distance = 0.1;
        max_neighbors = 6;
        min_pair_affinity_ratio = 0.75;
        min_pair_affinity = 1.0E-6;
      }
      else if (!strcmp(*argv, "-sun3d_silhouettes")) {
        shape_type = LINE_SHAPE_TYPE;
        min_cluster_points = 20;
        min_cluster_diameter = 0.25;
        max_cluster_diameter = 16;
        max_cluster_shape_distance = 0.05;
        max_cluster_normal_angle = RN_PI / 4.0;
        min_cluster_spacing = 0.05;
        max_pair_centroid_distance = 16;
        max_pair_shape_distance = 0.05;
        max_pair_normal_angle = RN_PI / 4.0;
        max_neighbor_distance = 0.1;
        max_neighbors = 6;
        min_pair_affinity_ratio = 0.75;
        min_pair_affinity = 1.0E-6;
        silhouette_points_only = TRUE;
      }
      else if (!strcmp(*argv, "-foo")) {
        shape_type = PLANE_SHAPE_TYPE;
        min_cluster_points = 10;
        min_cluster_diameter = 0.25;
        max_cluster_diameter = 16;
        max_cluster_shape_distance = 0.01;
        max_cluster_normal_angle = RN_PI / 32.0;
        min_cluster_spacing = 0.05;
        max_pair_centroid_distance = 16;
        max_pair_shape_distance = 0.1;
        max_pair_normal_angle = RN_PI / 32.0;
        max_neighbor_distance = 0.1;
        max_neighbors = 6;
        min_pair_affinity_ratio = 0.75;
        min_pair_affinity = 1.0E-6;
      }
      else { 
        fprintf(stderr, "Invalid program argument: %s", *argv); 
        exit(1); 
      }
      argv++; argc--;
    }
    else {
      if (!scene_name) scene_name = *argv;
      else if (!database_name) database_name = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!scene_name || !database_name) {
    fprintf(stderr, "Usage: sflsegment scenefile databasefile [options]\n");
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
  R3SurfelScene *scene = OpenScene(scene_name, database_name);
  if (!scene) exit(-1);

  // Split out surfels selected by mask
  if (mask_grid_name) {
    if (!SegmentMaskGrid(scene, parent_object_name, parent_node_name, source_node_name, mask_grid_name)) return 0;
  }

  // Split out surfels selected by overhead grids
  if (support_grid_name) {
    if (!SegmentSupportGrid(scene, parent_object_name, parent_node_name, source_node_name, support_grid_name)) return 0;
  }

  // Split points into clusters
  if (!SegmentPoints(scene, parent_object_name, parent_node_name, source_node_name)) return 0;

  // Close scene
  if (!CloseScene(scene)) exit(-1);

  // Return success 
  return 0;
}


