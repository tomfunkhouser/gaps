////////////////////////////////////////////////////////////////////////
// Source file for object instance label assignment class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Constructor/desctructor functions
////////////////////////////////////////////////////////////////////////

ObjectAssignment::
ObjectAssignment(ObjectSegmentation *segmentation, ObjectModel *model, 
  const R3Affine& model_to_segmentation_transformation, RNScalar score, RNBoolean ground_truth)
  : parse(NULL),
    parse_index(-1),
    segmentation(segmentation),
    segmentation_index(-1),
    model(model),
    model_index(-1),
    model_to_segmentation_transformation(model_to_segmentation_transformation),
    score(score),
    ground_truth(ground_truth)
{
}



ObjectAssignment::
~ObjectAssignment(void)
{
  // Remove from segmentation
  if (segmentation_index >= 0) segmentation->RemoveAssignment(this);

  // Remove from model
  if (model_index >= 0) model->RemoveAssignment(this);

  // Remove from parse
  if (parse_index >= 0) parse->RemoveAssignment(this);
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectAssignment::
SetModelToSegmentationTransformation(const R3Affine& transformation) 
{
  // Set transformation
  this->model_to_segmentation_transformation = transformation;

  // Reset score
  this->score = RN_UNKNOWN;
}



void ObjectAssignment::
SetSegmentationToModelTransformation(const R3Affine& transformation) 
{
  // Set inverse transformation
  SetModelToSegmentationTransformation(transformation.Inverse());
}



void ObjectAssignment::
SetScore(RNScalar score) 
{
  // Set score
  this->score = score;
}



void ObjectAssignment::
SetGroundTruth(RNBoolean ground_truth)
{
  // Set ground truth
  this->ground_truth = ground_truth;
}



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

int CompareObjectAssignments(const void *data1, const void *data2)
{
  ObjectAssignment *assignment1 = *((ObjectAssignment **) data1);
  ObjectAssignment *assignment2 = *((ObjectAssignment **) data2);
  if (assignment1->score > assignment2->score) return -1;
  if (assignment1->score < assignment2->score) return 1;
  return 0;
}

