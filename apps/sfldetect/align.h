////////////////////////////////////////////////////////////////////////
// Include file for alignment functions
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Closest point queries
////////////////////////////////////////////////////////////////////////

int ClosestPointCorrespondences(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2,
  RNArray<ObjectPoint *>& correspondences1, RNArray<ObjectPoint *>& correspondences2,
  const R3Affine& affine21 = R3identity_affine, RNLength max_distance = FLT_MAX,
  RNBoolean include12 = TRUE, RNBoolean include21 = TRUE);



////////////////////////////////////////////////////////////////////////
// 2D alignment functions (translate in xy, rotate around z)
////////////////////////////////////////////////////////////////////////

R3Affine PCATransformation2D(
  const ObjectPointSet& pointset);
R3Affine PCATransformation2D(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2);
R3Affine CorrespondenceTransformation2D(
  const RNArray<ObjectPoint *>& correspondences1, const RNArray<ObjectPoint *>& correspondences2,
  const R3Point& origin1, const R3Point& origin2, 
  RNScalar min_scale, RNScalar max_scale,
  const RNScalar *weights = NULL);
R3Affine ClosestPointTransformation2D(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2, 
  const R3Affine& initial_affine21,
  RNLength max_distance, 
  RNScalar min_scale, RNScalar max_scale,
  RNBoolean include12 = TRUE, RNBoolean include21 = TRUE);
R3Affine IterativeClosestPointTransformation2D(
  const ObjectPointSet& pointset1, const ObjectPointSet& pointset2, const R3Affine& initial_affine21, 
  RNLength start_max_distance, RNLength end_max_distance, RNScalar max_distance_update_factor,
  RNScalar min_scale, RNScalar max_scale,
  RNBoolean include12 = TRUE, RNBoolean include21 = TRUE);



////////////////////////////////////////////////////////////////////////
// Alignment evaluation functions
////////////////////////////////////////////////////////////////////////

RNScalar RMSD(const ObjectPointSet& correspondences1, const ObjectPointSet& correspondences2, const R3Affine& affine21);
RNScalar Coverage(const ObjectPointSet& pointset1, const ObjectPointSet& pointset2, const R3Affine& affine21, RNLength sigma);





