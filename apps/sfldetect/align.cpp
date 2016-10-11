////////////////////////////////////////////////////////////////////////
// Source file for alignment functions
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Correspondence finding functions
////////////////////////////////////////////////////////////////////////

static RNScalar
PointAffinity(ObjectPoint *point1, ObjectPoint *point2)
{
  // Check point values
  if ((point1->Value() <= 0) && (point2->Value() <= 0)) return point1->Value() + point2->Value();
  else if (point1->Value() <= 0) return point1->Value();
  else if (point2->Value() <= 0) return point2->Value();

  // Start with full affinity
  RNScalar affinity = 1.0;

  // Reduce affinity if point values are small (or negative)
  affinity *= point1->Value();
  affinity *= point2->Value();

  // Reduce affinity if normals are not aligned
  // Note that this requires points to be transformed into same coordinate system
  const R3Vector& normal1 = point1->Normal();
  const R3Vector& normal2 = point2->Normal();
  affinity *= 0.5 + 0.5 * fabs(normal1.Dot(normal2));

#if 0
  // Reduce affinity if descriptors are not the same
  const ObjectDescriptor& descriptor1 = point1->Descriptor();
  const ObjectDescriptor& descriptor2 = point2->Descriptor();
  for (int i = 0; i < descriptor1.NValues(); i++) {
    RNScalar value1 = descriptor1.Value(i);
    RNScalar value2 = descriptor2.Value(i);
    RNScalar delta = fabs(value1 - value2);
    if (delta < 0.0) continue;
    if (delta > 1.0) return 0.0;
    affinity *= 1.0 - delta;
  }
#endif

  // Return affinity
  return affinity;
}



static int 
ArePointsCompatible(ObjectPoint *point1, ObjectPoint *point2, void *)
{
  // Check correspondence weight
  if (PointAffinity(point1, point2) < RN_EPSILON) return 0;

  // Passed all tests
  return 1;
}



int 
ClosestPointCorrespondences(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2,
  RNArray<ObjectPoint *>& correspondences1, RNArray<ObjectPoint *>& correspondences2, 
  const R3Affine& affine21, RNLength max_distance, 
  RNBoolean include12, RNBoolean include21)
{
  // Initialize number of correspondences
  int ncorrespondences = 0;

  // Compute correspondences for pointset1 -> pointset2
  if (include12) {
    R3Affine affine12 = affine21.Inverse();
    for (int i = 0; i < pointset1.NPoints(); i++) {
      ObjectPoint *point1 = pointset1.Point(i);
      ObjectPoint transformed_point1 = *point1;
      transformed_point1.Transform(affine12);
      ObjectPoint *point2 = pointset2.FindClosest(&transformed_point1, 0.0, max_distance, ArePointsCompatible, NULL);
      // ObjectPoint *point2 = pointset2.FindClosest(&transformed_point1, 0.0, max_distance);
      if (point2) {
        correspondences1.Insert(point1);
        correspondences2.Insert(point2);
        ncorrespondences++;
      }
    }
  }

  // Compute correspondences for points2 -> points1
  if (include21) {
    for (int i = 0; i < pointset2.NPoints(); i++) {
      ObjectPoint *point2 = pointset2.Point(i);
      ObjectPoint transformed_point2 = *point2;
      transformed_point2.Transform(affine21);
      ObjectPoint *point1 = pointset1.FindClosest(&transformed_point2, 0.0, max_distance, ArePointsCompatible, NULL);
      // ObjectPoint *point1 = pointset1.FindClosest(&transformed_point2, 0.0, max_distance);
      if (point1) {
        correspondences1.Insert(point1);
        correspondences2.Insert(point2);
        ncorrespondences++;
      }
    }
  }

  // Return number of correspondences
  return ncorrespondences;
}



RNScalar *
PointCorrespondenceWeights(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2,
  RNArray<ObjectPoint *>& correspondences1, RNArray<ObjectPoint *>& correspondences2, 
  const R3Affine& affine21, RNLength max_distance)
{
  // Just checking
  assert(correspondences1.NEntries() == correspondences2.NEntries());
  if (correspondences1.NEntries() == 0) return NULL;
  if (correspondences2.NEntries() == 0) return NULL;

  // Allocate weights
  RNScalar *weights = new RNScalar [ correspondences1.NEntries() ];
  if (!weights) {
    fprintf(stderr, "Unable to allocate correspondence weights\n");
    return NULL;
  }

  // Compute weights
  for (int i = 0; i < correspondences1.NEntries(); i++) {
    ObjectPoint *point1 = correspondences1.Kth(i);    
    ObjectPoint *point2 = correspondences2.Kth(i);
    ObjectPoint transformed_point2(*point2);
    transformed_point2.Transform(affine21);
    weights[i] = PointAffinity(point1, &transformed_point2);
    if (weights[i] < 0) weights[i] = 0;
  }

  // Return weights
  return weights;
}



////////////////////////////////////////////////////////////////////////
// Alignment functions for 2D transformations
////////////////////////////////////////////////////////////////////////

static R3Point
Centroid(const RNArray<ObjectPoint *>& points, const RNScalar *weights = NULL)
{
  // Compute average point
  R3Point sum(0,0,0);
  RNScalar total_weight = 0.0;
  for (int i = 0; i < points.NEntries(); i++) {
    ObjectPoint *point = points.Kth(i);
    RNScalar weight = (weights) ? weights[i] : 1.0;
    sum += weight * point->Position();
    total_weight += weight;
  }
  return (total_weight > 0) ? sum / total_weight : R3zero_point;
}



static RNLength
AverageDistance(const R3Point& center, const RNArray<ObjectPoint *>& points, const RNScalar *weights = NULL)
{
  // Compute average distance between the points and a center point
  RNLength sum = 0.0;
  RNScalar total_weight = 0.0;
  for (int i = 0; i < points.NEntries(); i++) {
    ObjectPoint *point = points[i];
    RNScalar weight = (weights) ? weights[i] : 1.0;
    sum += weight * R3Distance(point->Position(), center);
    total_weight += weight;
  }

  // Return average distance
  return (total_weight > 0) ? sum / total_weight : 0.0;
}



R3Affine 
PCATransformation2D(const ObjectPointSet& pointset)
{
  // Just checking
  if (pointset.NPoints() == 0) return R3identity_affine;

  // Get center
  R3Point centroid = pointset.Centroid();

  // Compute 2D covariance matrix
  RNScalar m[4] = { 0 };
  for (int i = 0; i < pointset.NPoints(); i++) {
    ObjectPoint *point = pointset.Point(i);
    const R3Point& p = point->Position();
    RNScalar x = p[0] - centroid[0];
    RNScalar y = p[1] - centroid[1];
    m[0] += x*x;
    m[1] += x*y;
    m[2] += x*y;
    m[3] += y*y;
  }

  // Normalize covariance matrix
  for (int i = 0; i < 4; i++) {
    m[i] /= pointset.NPoints();
  }

  // Compute eigenvalues and eigenvectors
  RNScalar U[4];
  RNScalar W[2];
  RNScalar Vt[4];
  RNSvdDecompose(2, 2, m, U, W, Vt);  // m == U . DiagonalMatrix(W) . Vt

  // Extract principle axis direction from first eigenvector
  R3Vector axis(Vt[0], Vt[1], 0);

  // Flip principle axis so that "heavier" on positive side
  int positive_count = 0;
  int negative_count = 0;
  for (int i = 0; i < pointset.NPoints(); i++) {
    ObjectPoint *point = pointset.Point(i);
    const R3Point& p = point->Position();
    RNScalar x = p[0] - centroid[0];
    RNScalar y = p[1] - centroid[1];
    R3Vector vertex_vector(x, y, 0);
    RNScalar dot = axis.Dot(vertex_vector);
    if (dot > 0.0) positive_count++;
    else negative_count++;
  }
  if (positive_count < negative_count) {
    axis.Flip();
  }

  // Create transformation
  R3Affine affine = R3identity_affine;
  affine.Translate(-(pointset.Origin().Vector()));
  affine.Translate(centroid.Vector());
  affine.Rotate(axis, R3posx_vector);
  affine.Translate(-(centroid.Vector()));

  // Return transformation
  return affine;
}



R3Affine 
PCATransformation2D(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2)
{
  // Compute transformation
  R3Affine affine21 = R3identity_affine;
  affine21.InverseTransform(PCATransformation2D(pointset1));
  affine21.Transform(PCATransformation2D(pointset2));
  return affine21;
}



R3Affine 
CorrespondenceTransformation2D(
  const RNArray<ObjectPoint *>& correspondences1, const RNArray<ObjectPoint *>& correspondences2,
  const R3Point& origin1, const R3Point& origin2, 
  RNScalar min_scale, RNScalar max_scale,
  const RNScalar *weights)
{
  // Just checking
  assert(correspondences1.NEntries() == correspondences2.NEntries());
  if (correspondences1.NEntries() == 0) return R3identity_affine;

  // Compute centroids
  R3Point centroid1 = Centroid(correspondences1, weights);
  R3Point centroid2 = Centroid(correspondences2, weights);

  // Compute scale
  RNScalar scale = 0.5 * (min_scale + max_scale);
  if (min_scale < max_scale) {
    RNLength s2 = AverageDistance(centroid2, correspondences2, weights);
    if (RNIsPositive(s2)) {
      RNLength s1 = AverageDistance(centroid1, correspondences1, weights);
      scale = s1/s2;
      if (scale < min_scale) scale = min_scale;
      if (scale > max_scale) scale = max_scale;
    }
  }

  // Compute covariance matrix
  RNScalar m[4] = { 0 };
  RNScalar total_weight = 0;
  for (int i = 0; i < correspondences1.NEntries(); i++){
    ObjectPoint *point1 = correspondences1.Kth(i);
    ObjectPoint *point2 = correspondences2.Kth(i);
    R3Vector p1 = point1->Position() - centroid1;
    R3Vector p2 = point2->Position() - centroid2;
    RNScalar w = (weights) ? weights[i] : 1.0;
    m[0] += w * p1[0]*p2[0];
    m[1] += w * p1[0]*p2[1];
    m[2] += w * p1[1]*p2[0];
    m[3] += w * p1[1]*p2[1];
    total_weight += w;
  }

  // Check total weight
  if (total_weight <= 0) return R3identity_affine;

  // Normalize covariance matrix
  for (int j = 0; j < 4; j++) m[j] /= total_weight;

  // Calculate SVD of covariance matrix
  RNScalar Um[4];
  RNScalar Wm[2];
  RNScalar Vmt[4];
  RNSvdDecompose(2, 2, m, Um, Wm, Vmt);

  // https://sakai.rutgers.edu/access/content/group/7bee3f05-9013-4fc2-8743-3c5078742791/material/svd_ls_rotation.pdf
  R3Matrix Ut(Um[0], Um[2], 0, Um[1], Um[3], 0, 0, 0, 1); 
  R3Matrix V(Vmt[0], Vmt[2], 0, Vmt[1], Vmt[3], 0, 0, 0, 1); 
  R3Matrix VUt = V * Ut;
  R3Matrix D = R3identity_matrix;
  D[1][1] = R3MatrixDet2(VUt[0][0], VUt[0][1], VUt[1][0], VUt[1][1]);
  R3Matrix R = V * D * Ut;
  R4Matrix rotation = R4identity_matrix;
  rotation[0][0] = R[0][0];
  rotation[0][1] = R[0][1];
  rotation[1][0] = R[1][0];
  rotation[1][1] = R[1][1];

  // Compute matrix21
  // XXX Why rotation.Inverse()? XXX
  R3Affine affine21 = R3identity_affine;
  affine21.Translate(origin1.Vector());
  if (scale != 1.0) affine21.Scale(scale);
  affine21.Transform(rotation.Inverse());
  affine21.Translate(-(origin2.Vector()));

  // Return resulting affine that takes points2 to points1
  return affine21;
}



R3Affine 
ClosestPointTransformation2D(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2, 
  const R3Affine& initial_affine21, 
  RNLength max_distance, 
  RNScalar min_scale, RNScalar max_scale,
  RNBoolean include12, RNBoolean include21)
{
  // Compute correspondences
  RNArray<ObjectPoint *> correspondences1, correspondences2;
  int ncorrespondences = ClosestPointCorrespondences(pointset1, pointset2, 
    correspondences1, correspondences2, initial_affine21, max_distance, include12, include21);
  if (ncorrespondences == 0) return initial_affine21;

  // Compute weights
  RNScalar *weights = PointCorrespondenceWeights(pointset1, pointset2, 
    correspondences1, correspondences2, initial_affine21, max_distance);

  // Return correspondence transformation
  R3Affine xform = CorrespondenceTransformation2D(
    correspondences1, correspondences2, 
    pointset1.Origin(), pointset2.Origin(), 
    min_scale, max_scale, weights);

  // Delete weights
  if (weights) delete [] weights;

  // Return transformation
  return xform;
}



R3Affine 
IterativeClosestPointTransformation2D(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2, const R3Affine& initial_affine21,
  RNLength start_max_distance, RNLength end_max_distance, RNScalar max_distance_update_factor,
  RNScalar min_scale, RNScalar max_scale, 
  RNBoolean include12, RNBoolean include21)
{
  // Just checking
  if (end_max_distance > start_max_distance) end_max_distance = start_max_distance;
  if (max_distance_update_factor < 0.0) max_distance_update_factor = 0.5;
  if (max_distance_update_factor >= 1.0) max_distance_update_factor = 0.5;
  int max_iterations = 16;

  // Initialize everything
  R3Affine affine21 = initial_affine21;
  RNLength max_distance = start_max_distance;
  RNArray<ObjectPoint *> correspondences_buffer1[2];
  RNArray<ObjectPoint *> correspondences_buffer2[2];
  static int counter = 0;  counter++;

  // Iterate until max_iterations or converged
  for (int iteration = 0; iteration < max_iterations; iteration++) {
    // Initialize correspondence buffers
    RNArray<ObjectPoint *>& correspondences1 = correspondences_buffer1[iteration%2];
    RNArray<ObjectPoint *>& correspondences2 = correspondences_buffer2[iteration%2];
    correspondences1.Empty(); correspondences2.Empty();

    // Compute correspondences
    int ncorrespondences = ClosestPointCorrespondences(pointset1, pointset2, 
      correspondences1, correspondences2, affine21, max_distance, include12, include21);
    if (ncorrespondences == 0) return affine21;

#if 0
    // Write point files to visualize correspondences
    if (1) {
      char name1[1024], name2[1024];
      sprintf(name1, "foo/c_%d_%d_1.xyzn", counter, iteration);
      sprintf(name2, "foo/c_%d_%d_2.xyzn", counter, iteration);
      FILE *fp1 = fopen(name1, "w");
      FILE *fp2 = fopen(name2, "w");
      for (int i = 0; i < correspondences1.NEntries(); i++) {
        ObjectPoint *point = correspondences1.Kth(i);
        R3Point position = point->Position();
        R3Vector normal = point->Normal();
        fprintf(fp1, "%g %g %g   ", position.X(), position.Y(), position.Z());
        fprintf(fp1, "%g %g %g\n", normal.X(), normal.Y(), normal.Z());
      }
      for (int i = 0; i < correspondences2.NEntries(); i++) {
        ObjectPoint *point = correspondences2.Kth(i);
        R3Point position = point->Position();
        R3Vector normal = point->Normal();
        position.Transform(affine21);
        normal.Transform(affine21);
        fprintf(fp2, "%g %g %g   ", position.X(), position.Y(), position.Z());
        fprintf(fp2, "%g %g %g\n", normal.X(), normal.Y(), normal.Z());
      }
      fclose(fp1);
      fclose(fp2);
    }
#endif

    // Compute weights
    RNScalar *weights = PointCorrespondenceWeights(pointset1, pointset2, 
      correspondences1, correspondences2, initial_affine21, max_distance);

    // Get the origins
    R3Point origin1 = Centroid(correspondences1, weights);
    R3Point origin2 = Centroid(correspondences2, weights);
    origin1[2] = pointset1.Origin().Z();
    origin2[2] = pointset2.Origin().Z();

    // Compute correspondence transformation
    affine21 = CorrespondenceTransformation2D(
      correspondences1, correspondences2, 
      origin1, origin2, 
      min_scale, max_scale, 
      weights);

    // Delete weights
    if (weights) delete [] weights;

#if 0
    // Print debug statement
    R4Matrix m = affine21.Matrix();
    RNScalar coverage1 = Coverage(pointset1, pointset2, affine21, 0.5 * end_max_distance);
    RNScalar coverage2 = Coverage(pointset2, pointset1, affine21.Inverse(), 0.5 * end_max_distance);
    printf("I %6.3f : %6d / %6d %6d : %6.3f %6.3f %6.3f : %6.3f : %6.3f %6.3f : %6.3f\n", max_distance,
      ncorrespondences, pointset1.NPoints(), pointset2.NPoints(),   
      m[0][3], m[1][3], m[2][3], acos(m[0][0] / affine21.ScaleFactor()),
      coverage1, coverage2, coverage1 * coverage2);
#endif

#if 1
    // Check for convergence (stop when get to end distance)
    if (max_distance <= end_max_distance) break;
#else
    // Check for convergence (stop when correspondences are stable)
    if ((RNIsEqual(max_distance, end_max_distance)) && (iteration > 0) && (iteration < max_iterations-1)) {
      RNArray<ObjectPoint *>& prev_correspondences1 = correspondences_buffer1[(1-(iteration%2))];
      RNArray<ObjectPoint *>& prev_correspondences2 = correspondences_buffer2[(1-(iteration%2))];
      if (correspondences1.NEntries() == prev_correspondences1.NEntries()) {
        RNBoolean done = TRUE;
        for (int i = 0; i < correspondences1.NEntries(); i++) {
          if (correspondences1.Kth(i) != prev_correspondences1.Kth(i)) { done = FALSE; break; }
          if (correspondences2.Kth(i) != prev_correspondences2.Kth(i)) { done = FALSE; break; }
        }
        if (done) break;
      }
    }
#endif

    // Reduce max_distance
    max_distance *= max_distance_update_factor;
    if (RNIsLessOrEqual(max_distance, end_max_distance)) max_distance = end_max_distance;
  }

  // Return transformation
  return affine21;
}


////////////////////////////////////////////////////////////////////////
// Alignment evaluation functions
////////////////////////////////////////////////////////////////////////

RNScalar
RMSD(const RNArray<ObjectPoint *>& correspondences1, const RNArray<ObjectPoint *>& correspondences2, 
  const R3Affine& affine21)
{
  // Check number of correspondences
  assert(correspondences1.NEntries() == correspondences2.NEntries());
  if (correspondences1.NEntries() == 0) return 0;

  // Compute SSD
  RNScalar ssd = 0;
  for (int i = 0; i < correspondences1.NEntries(); i++) {
    ObjectPoint *point1 = correspondences1.Kth(i);
    ObjectPoint *point2 = correspondences2.Kth(i);
    const R3Point& position1 = point1->Position();
    R3Point position2 = point2->Position();
    position2.Transform(affine21);
    ssd += R3SquaredDistance(position1, position2);
  }
  
  // Return RMSD 
  return sqrt(ssd / correspondences1.NEntries());
}



RNScalar
Coverage(const ObjectPointSet& pointset1, const ObjectPointSet& pointset2, 
  const R3Affine& affine21, RNLength sigma)
{
  // Check number of points
  if (pointset1.NPoints() == 0) return 0;
  if (pointset2.NPoints() == 0) return 0;

  // Compute support
  RNScalar support = 0;
  RNLength coverage_factor = -1.0 / (2.0 * sigma * sigma);
  RNLength max_distance = 3 * sigma;
  for (int i = 0; i < pointset2.NPoints(); i++) {
    RNLength closest_distance = 0;
    ObjectPoint *point2 = pointset2.Point(i);
    ObjectPoint transformed_point2 = *point2;
    transformed_point2.Transform(affine21);
    ObjectPoint *point1 = pointset1.FindClosest(&transformed_point2, 0, max_distance, NULL, NULL, &closest_distance);
    if (!point1) continue;
    RNScalar affinity = PointAffinity(point1, &transformed_point2);
    affinity *= exp(closest_distance * closest_distance * coverage_factor);
    support += affinity;
  }
  
  // Return coverage
  return support / pointset2.NPoints();
}


