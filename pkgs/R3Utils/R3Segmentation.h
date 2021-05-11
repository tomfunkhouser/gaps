//////////////////////////////////////////////////////////////////////
// Include file for code that does segmentation of points
////////////////////////////////////////////////////////////////////////
#ifndef __R3__SEGMENTATION__H__
#define __R3__SEGMENTATION__H__



//////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Shapes/R3Shapes.h"



//////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {


  
///////////////////////////////////////////////////////////////////////
// Primitive types
////////////////////////////////////////////////////////////////////////

enum {
  R3_SEGMENTATION_NULL_PRIMITIVE_TYPE,
  R3_SEGMENTATION_POINT_PRIMITIVE_TYPE,
  R3_SEGMENTATION_LINE_PRIMITIVE_TYPE,
  R3_SEGMENTATION_PLANE_PRIMITIVE_TYPE,
  R3_SEGMENTATION_PLANAR_GRID_PRIMITIVE_TYPE,
  R3_SEGMENTATION_NUM_PRIMITIVE_TYPES
};



////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////

class R3SegmentationPoint {
public:
  R3SegmentationPoint(void);
public:
  RNScalar depth;
  R3Point position;
  R3Vector normal;
  R3Vector tangent;
  RNLength radius1;
  RNLength radius2;
  RNScalar elevation;
  RNScalar timestamp;
  RNArea area;
  RNRgb color;
  int category_identifier;
  RNScalar category_confidence;
  unsigned int attribute;
  unsigned int boundary;
  unsigned int identifier;
  RNArray<R3SegmentationPoint *> neighbors;
  class R3SegmentationCluster *cluster;
  RNScalar cluster_affinity;
  int cluster_index;
  int data_index;
  int mark;
};

class R3SegmentationPrimitive {
public:
  R3SegmentationPrimitive(int primitive_type = 0);
  R3SegmentationPrimitive(const R3SegmentationPrimitive& primitive);
  R3SegmentationPrimitive(R3SegmentationPoint *seed_point, const RNArray<R3SegmentationPoint *> *points = NULL);
public:
  RNLength Distance(const R3Point& position) const;
  void Update(const R3Point& point);
  void Update(const R3Line& line);
  void Update(const R3Plane& plane);
  void Update(R3SegmentationPoint *seed_point = NULL, const RNArray<R3SegmentationPoint *> *points = NULL);
  void Update(R3SegmentationPrimitive primitive1, R3SegmentationPrimitive primitive2, RNScalar weight1 = 1.0, RNScalar weight2 = 1.0);
public:
  int primitive_type;
  R3Box bbox;
  R3Point centroid;
  R3Line line;
  R3Plane plane;
};

class R3SegmentationCluster {
public:
  R3SegmentationCluster(R3SegmentationPoint *seed_point = NULL, int primitive_type = 0);
  R3SegmentationCluster(R3SegmentationPoint *seed_point, const R3SegmentationPrimitive& primitive);
  R3SegmentationCluster(R3SegmentationCluster *child1, R3SegmentationCluster *child2);
  ~R3SegmentationCluster(void);
public:
  RNScalar Coverage(void);
  R3Triad PrincipleAxes(R3Point *returned_center = NULL, RNScalar *returned_variances = NULL) const;
  R3Box AxisExtents(void);
  void EmptyPoints(void);
  void InsertPoint(R3SegmentationPoint *point, RNScalar affinity = 1.0);
  void RemovePoint(R3SegmentationPoint *point);
  void InsertChild(R3SegmentationCluster *child);
  void RemoveChild(R3SegmentationCluster *child);
  int UpdatePoints(const R3Kdtree<R3SegmentationPoint *> *kdtree);
  int UpdatePrimitive(void);
  int UpdateColor(void);
  int UpdateTimestamp(void);
  int UpdateArea(void);
  RNScalar Affinity(R3SegmentationPoint *point) const;
  RNScalar Affinity(R3SegmentationCluster *cluster) const;
public:
  R3SegmentationPoint *seed_point;
  RNArray<R3SegmentationPoint *> points;
  R3SegmentationCluster *parent;
  RNArray<R3SegmentationCluster *> children;
  RNArray<class R3SegmentationPair *> pairs;
  R3SegmentationPrimitive primitive;
  RNArea area;
  RNRgb color;
  RNScalar timestamp;
  int category_identifier;
  RNScalar category_confidence;
  RNScalar possible_affinity; 
  RNScalar total_affinity;
  class R3Segmentation *segmentation;
  int segmentation_index;
};

class R3SegmentationPair {
public:
  R3SegmentationPair(R3SegmentationCluster *cluster1 = NULL, R3SegmentationCluster *cluster2 = NULL, RNScalar affinity = 0);
  ~R3SegmentationPair(void);
public:
  R3SegmentationCluster *clusters[2];
  int cluster_index[2];
  RNScalar affinity; 
  R3SegmentationPair **heapentry;
};

class R3Segmentation {
public:
  R3Segmentation(void);
  R3Segmentation(const R3Segmentation& segmentation);
  ~R3Segmentation(void);
public:
  RNScalar Affinity(void) const;
  RNScalar AverageNeighborCount(void) const;
  int NUnclusteredPoints(void) const;
public:
  int CreateNeighbors(
    int max_neighbor_count = 16,
    double max_neighbor_distance = 0,
    double max_neighbor_primitive_distance = 0.01,
    double max_neighbor_normal_angle = RN_PI / 4.0,
    double max_neighbor_color_difference = 0,
    double max_neighbor_distance_factor = 10,
    double max_neighbor_timestamp_difference = 0,
    double max_neighbor_category_difference = 0,
    RNBoolean partition_identifiers = FALSE);
  int UpdatePoints(void);
public:
  int AssignPoints(void);  
  int CreateClusters(int primitive_type);
  int CreateSingletonClusters(int primitive_type);
  int CreateRegionGrowingClusters(int primitive_type);
  int RefineClusters(void);  
  int ReassignClusters(void);  
  int DeleteClusters(void);  
  int MergeClusters(void);  
  int MergeSmallClusters(void);  
  int SplitClusters(void);
  int RefineBoundaries(void);
  int WriteFile(const char *filename) const;
  int PrintParameters(FILE *fp = NULL) const;
public:
  // Data structures
  RNArray<R3SegmentationPoint *> points;
  R3Kdtree<R3SegmentationPoint *> *kdtree;
  RNArray<R3SegmentationCluster *> clusters;
  R3SegmentationPoint *point_buffer;
public:
  // Parameters
  int min_clusters;
  int max_clusters;
  int min_cluster_points;
  double min_cluster_area;
  double min_cluster_coverage;
  double max_cluster_diameter;
  double max_cluster_primitive_distance;
  double max_cluster_normal_angle;
  double max_cluster_color_difference;
  double max_cluster_timestamp_difference;
  double max_cluster_category_difference;
  double max_pair_centroid_distance;
  double max_pair_primitive_distance;
  double max_pair_normal_angle;
  double max_pair_color_difference;
  double max_pair_timestamp_difference;
  double max_pair_category_difference;
  double min_pair_affinity;
  int max_refinement_iterations;  
  int max_reassignment_iterations;  
  RNBoolean equalize_cluster_sizes;
  RNBoolean balance_cluster_sizes;
  RNBoolean favor_convex_clusters;
  RNBoolean scale_tolerances_with_depth;
  RNBoolean initialize_hierarchically;
  RNBoolean allow_outlier_points;
  RNBoolean refine_boundaries;
  int print_progress;
};



} // end namespace

  

#endif
