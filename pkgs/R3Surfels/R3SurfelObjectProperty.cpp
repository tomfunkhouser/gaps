/* Source file for the R3 surfel object property class */



////////////////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////////////////

#include "R3Surfels.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS/DESTRUCTORS
////////////////////////////////////////////////////////////////////////

R3SurfelObjectProperty::
R3SurfelObjectProperty(int type, R3SurfelObject *object, RNScalar *operands, int noperands)
  : scene(NULL),
    scene_index(-1),
    object(object),
    operands(NULL),
    noperands(0),
    type(type)
{
  // Check if operands were provided
  if ((noperands > 0) && (operands)) {
    // Copy operands
    this->noperands = noperands;
    this->operands = new RNScalar [ this->noperands ];
    for (int i = 0; i < this->noperands; i++) {
      this->operands[i] = operands[i];
    }
  }
  else {
    // Compute operands
    UpdateOperands();
  }
}



R3SurfelObjectProperty::
R3SurfelObjectProperty(const R3SurfelObjectProperty& property)
  : scene(NULL),
    scene_index(-1),
    object(property.object),
    operands(NULL),
    noperands(0),
    type(property.type)
{
  // Initialize operands
  if ((property.noperands > 0) && (property.operands)) {
    this->noperands = property.noperands;
    this->operands = new RNScalar [ this->noperands ];
    for (int i = 0; i < this->noperands; i++) {
      this->operands[i] = property.operands[i];
    }
  }
}



R3SurfelObjectProperty::
~R3SurfelObjectProperty(void)
{
  // Remove from scene (removes from everything)
  if (scene) scene->RemoveObjectProperty(this);

  // Delete operands
  if (operands) delete [] operands;
}



////////////////////////////////////////////////////////////////////////
// MANIPULATION FUNCDTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelObjectProperty::
ResetOperands(RNScalar *operands, int noperands)
{
  // Delete previous operands
  if (this->operands) delete this->operands;
  this->operands = NULL;
  this->noperands = 0;
  
  // Check if operands were provided
  if ((noperands > 0) && (operands)) {
    // Copy operands
    this->noperands = noperands;
    this->operands = new RNScalar [ this->noperands ];
    for (int i = 0; i < this->noperands; i++) {
      this->operands[i] = operands[i];
    }
    
    // Mark scene as dirty
    if (scene) scene->SetDirty();
  }
  else {
    // Compute operands
    UpdateOperands();
  }
}


  
////////////////////////////////////////////////////////////////////////
// DISPLAY FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelObjectProperty::
Draw(RNFlags flags) const
{
  // Draw property based on type
  switch (type) {
  case R3_SURFEL_OBJECT_PCA_PROPERTY: 
    if (noperands == 21) {
      R3Point centroid(operands[0], operands[1], operands[2]);
      R3Vector axis1(operands[3], operands[4], operands[5]);
      R3Vector axis2(operands[6], operands[7], operands[8]);
      R3Vector axis3(operands[9], operands[10], operands[11]);
      RNScalar stddev1 = sqrt(operands[12]);
      RNScalar stddev2 = sqrt(operands[13]);
      RNScalar stddev3 = sqrt(operands[14]);
      RNScalar extent01 = operands[15];
      RNScalar extent02 = operands[16];
      RNScalar extent03 = operands[17];
      RNScalar extent11 = operands[18];
      RNScalar extent12 = operands[19];
      RNScalar extent13 = operands[20];

      // Draw normal
      glLineWidth(5);
      RNGrfxBegin(RN_GRFX_LINES);
      if (flags[R3_SURFEL_COLOR_DRAW_FLAG]) RNLoadRgb(0, 0, 1);
      R3LoadPoint(centroid);
      R3LoadPoint(centroid + extent13*axis3);
      RNGrfxEnd();

      // Draw stddevs
      glLineWidth(2);
      RNGrfxBegin(RN_GRFX_LINES);
      if (flags[R3_SURFEL_COLOR_DRAW_FLAG]) RNLoadRgb(1, 0, 0);
      R3LoadPoint(centroid - stddev1 * axis1);
      R3LoadPoint(centroid + stddev1 * axis1);
      if (flags[R3_SURFEL_COLOR_DRAW_FLAG]) RNLoadRgb(0, 1, 0);
      R3LoadPoint(centroid - stddev2 * axis2);
      R3LoadPoint(centroid + stddev2 * axis2);
      if (flags[R3_SURFEL_COLOR_DRAW_FLAG]) RNLoadRgb(0, 0, 1);
      R3LoadPoint(centroid - stddev3 * axis3);
      R3LoadPoint(centroid + stddev3 * axis3);
      RNGrfxEnd();

      // Draw extents
      glLineWidth(1);
      RNGrfxBegin(RN_GRFX_LINES);
      if (flags[R3_SURFEL_COLOR_DRAW_FLAG]) RNLoadRgb(1, 0, 0);
      R3LoadPoint(centroid + extent01 * axis1);
      R3LoadPoint(centroid + extent11 * axis1);
      if (flags[R3_SURFEL_COLOR_DRAW_FLAG]) RNLoadRgb(0, 1, 0);
      R3LoadPoint(centroid + extent02 * axis2);
      R3LoadPoint(centroid + extent12 * axis2);
      if (flags[R3_SURFEL_COLOR_DRAW_FLAG]) RNLoadRgb(0, 0, 1);
      R3LoadPoint(centroid + extent03 * axis3);
      R3LoadPoint(centroid + extent13 * axis3);
      RNGrfxEnd();
    }
    break; 

  case R3_SURFEL_OBJECT_AMODAL_OBB_PROPERTY: 
    if (noperands == 20) {
      // Parse operands
      R3Point centroid(operands[0], operands[1], operands[2]);
      R3Vector axis1(operands[3], operands[4], operands[5]);
      R3Vector axis2(operands[6], operands[7], operands[8]);
      R3Vector axis3(operands[9], operands[10], operands[11]);
      RNScalar radius1 = operands[12];
      RNScalar radius2 = operands[13];
      RNScalar radius3 = operands[14];
      // RNScalar confidence = operands[15];
      // int originator = (int) (operands[16] + 0.5);

      // Check if should draw nose
      RNBoolean draw_nose = TRUE;
      R3SurfelLabel *label = (object) ? object->CurrentLabel() : NULL;
      if (label && label->Flags()[R3_SURFEL_LABEL_UNORIENTABLE_FLAG]) draw_nose = FALSE;

      // Draw obb
      R3OrientedBox obb(centroid, axis1, axis2, radius1, radius2, radius3);
      obb.Outline();

      // Draw nose
      if (draw_nose) {
        double min_nose_vector_length = 0.5;
        double nose_vector_length = 0.25 * radius1;
        if (nose_vector_length < min_nose_vector_length)
          nose_vector_length = min_nose_vector_length;
        R3Point start = centroid + radius1 * axis1;
        R3Point end = start + nose_vector_length * axis1;
        R3Span(start, end).Draw();
      }
    }
    break; 
  }
}



////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCDTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelObjectProperty::
UpdateOperands(void)
{
  // Check if already uptodate
  if (noperands > 0) return;

  // Check object
  if (!object) return;

  // Check property type
  switch(type) {
  case R3_SURFEL_OBJECT_PCA_PROPERTY: {
    // PCA Properties
    // noperands = 21
    // operands[0-2]: centroid
    // operands[3-5]: xyz of axis1
    // operands[6-8]: xyz of axis2
    // operands[9-11]: xyz of axis3
    // operands[12-14]: variances
    // operands[15-20]: extents 

    // Allocate operands
    noperands = 21;
    operands = new RNScalar [ noperands ];
    for (int i = 0; i < noperands; i++) operands[i] = 0;

    // Create pointset
    R3SurfelPointSet *pointset = object->PointSet();
    if (!pointset) return;
    if (pointset->NPoints() < 3) { delete pointset; return; }

    // Compute principle axes
    RNScalar variances[3];
    R3Point centroid = pointset->Centroid();
    R3Triad triad = pointset->PrincipleAxes(&centroid, variances);

    // Check if positive z of triad is facing away from viewpoint(s)
    int npositive = 0, nnegative = 0;
    R3Plane plane(centroid, triad.Axis(2));
    for (int i = 0; i < pointset->NPoints(); i++) {
      R3SurfelPoint *point = pointset->Point(i);
      R3SurfelBlock *block = point->Block();
      if (!block) continue;
      R3SurfelNode *node = block->Node();
      if (!node) continue;
      R3SurfelScan *scan = node->Scan();
      if (!scan) continue;
      RNScalar d = R3SignedDistance(plane, scan->Viewpoint());
      if (d < 0) nnegative++;
      else if (d > 0) npositive++;
    }

    // Rotate triad so that z is not backfacing to viewpoints
    if (nnegative > npositive) triad = R3Triad(triad[0], -triad[1], -triad[2]);

    // Find extents
    R3Box extent = R3null_box;
    for (int i = 0; i < pointset->NPoints(); i++) {
      R3SurfelPoint *point = pointset->Point(i);
      R3Point position = point->Position();
      R3Vector vector = position - centroid;
      RNScalar x = triad.Axis(0).Dot(vector);
      RNScalar y = triad.Axis(1).Dot(vector);
      RNScalar z = triad.Axis(2).Dot(vector);
      extent.Union(R3Point(x, y, z));
    }

    // Fill operands
    operands[0] = centroid.X();
    operands[1] = centroid.Y();
    operands[2] = centroid.Z();
    operands[3] = triad.Axis(0).X();
    operands[4] = triad.Axis(0).Y();
    operands[5] = triad.Axis(0).Z();
    operands[6] = triad.Axis(1).X();
    operands[7] = triad.Axis(1).Y();
    operands[8] = triad.Axis(1).Z();
    operands[9] = triad.Axis(2).X();
    operands[10] = triad.Axis(2).Y();
    operands[11] = triad.Axis(2).Z();
    operands[12] = variances[0];
    operands[13] = variances[1];
    operands[14] = variances[2];
    operands[15] = extent[0][0];
    operands[16] = extent[0][1];
    operands[17] = extent[0][2];
    operands[18] = extent[1][0];
    operands[19] = extent[1][1];
    operands[20] = extent[1][2];

    // Delete pointset
    delete pointset;
    break; }

  case R3_SURFEL_OBJECT_AMODAL_OBB_PROPERTY: {
    // PCA Properties
    // noperands = 16
    // operands[0-2]: centroid
    // operands[3-5]: xyz of axis1
    // operands[6-8]: xyz of axis2
    // operands[9-11]: xyz of axis3
    // operands[12-14]: radii
    // operands[15]: confidence
    // operands[16]: originator
    // operands[17-19]: reserved

    // Allocate operands
    noperands = 20;
    operands = new RNScalar [ noperands ];
    for (int i = 0; i < noperands; i++) operands[i] = 0;

    // Estimate obb
    R3OrientedBox obb = EstimateOrientedBBox(object);
    double confidence = 0.01;
    int originator = R3_SURFEL_MACHINE_ORIGINATOR;
    
    // Fill operands
    operands[0] = obb.Center().X();
    operands[1] = obb.Center().Y();
    operands[2] = obb.Center().Z();
    operands[3] = obb.Axis(0).X();
    operands[4] = obb.Axis(0).Y();
    operands[5] = obb.Axis(0).Z();
    operands[6] = obb.Axis(1).X();
    operands[7] = obb.Axis(1).Y();
    operands[8] = obb.Axis(1).Z();
    operands[9] = obb.Axis(2).X();
    operands[10] = obb.Axis(2).Y();
    operands[11] = obb.Axis(2).Z();
    operands[12] = obb.Radius(0);
    operands[13] = obb.Radius(1);
    operands[14] = obb.Radius(2);
    operands[15] = confidence;
    operands[16] = originator;
    operands[17] = 0;
    operands[18] = 0;
    operands[19] = 0;
    break; }
  }

  // Mark scene as dirty
  if (scene) scene->SetDirty();
}



} // namespace gaps
