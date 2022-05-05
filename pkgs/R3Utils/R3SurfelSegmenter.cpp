/* Source file for the surfel segmenter class */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// Surfel segmenter constructor/destructor
////////////////////////////////////////////////////////////////////////

R3SurfelSegmenter::
R3SurfelSegmenter(R3SurfelScene *scene)
  : scene(scene),
    candidate_objects(),
    anchor_objects(),
    candidate_object_index(NULL),
    anchor_object_index(NULL)
{
}



R3SurfelSegmenter::
~R3SurfelSegmenter(void)
{
  // Delete object indices
  if (candidate_object_index) delete [] candidate_object_index;
  if (anchor_object_index) delete [] anchor_object_index;
}



int R3SurfelSegmenter::
PredictObjectSegmentations(const RNArray<R3SurfelObject *>& objects)
{
  // Temporary
  return 1;
}



int R3SurfelSegmenter::
PredictObjectSegmentations(void)
{
  // Predict segmentations for all objects
  RNArray<R3SurfelObject *> objects;
  for (int i = 0; i < scene->NObjects(); i++) objects.Insert(scene->Object(i));
  return PredictObjectSegmentations(objects);
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void R3SurfelSegmenter::
UpdateAfterInsertObject(R3SurfelObject *object)
{
}



void R3SurfelSegmenter::
UpdateBeforeRemoveObject(R3SurfelObject *object)
{
}



void R3SurfelSegmenter::
UpdateAfterInsertLabelAssignment(R3SurfelLabelAssignment *assignment)
{
}



void R3SurfelSegmenter::
UpdateBeforeRemoveLabelAssignment(R3SurfelLabelAssignment *assignment)
{
}



}; // end namespace
