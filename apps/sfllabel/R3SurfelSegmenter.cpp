/* Source file for the surfel segmenter class */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Surfels/R3Surfels.h"
#include "R3SurfelSegmenter.h"

#ifdef RN_USE_GCO
#include "gco/gco.h"
#endif



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



////////////////////////////////////////////////////////////////////////
// GCO graph cut segmentation
////////////////////////////////////////////////////////////////////////

#if RN_USE_GCO

static int 
SetDataCosts(R3SurfelSegmenter *segmenter, GCoptimizationGeneralGraph *gc)
{
  // Just checking
  if (segmenter->candidate_objects.IsEmpty()) return 0;
  if (segmenter->anchor_objects.IsEmpty()) return 0;

  // Get useful variables
  int ncandidates = segmenter->candidate_objects.NEntries();
  int nanchors = segmenter->anchor_objects.NEntries();

  // Allocate cost matrix
  int *costs = new int [ncandidates * nanchors];
  if (!costs) {
    RNFail("Unable to allocate data cost matrix\n");
    return 0;
  }

  // Initialize all data costs
  for (int i = 0; i < ncandidates; i++) {
    R3SurfelObject *candidate = segmenter->candidate_objects.Kth(i);
    for (int j = 0; j < nanchors; j++) {
      R3SurfelObject *anchor = segmenter->anchor_objects.Kth(j);
      if (anchor == candidate) costs[i*nanchors+j] = 0;
      else costs[i*nanchors+j] = 1;
    }
  }

  // Set the data costs
  gc->setDataCost(costs);

  // Return success
  return 1;
}



static int
SetNeighborCosts(R3SurfelSegmenter *segmenter, GCoptimizationGeneralGraph *gc)
{
  // Get useful variables
  R3SurfelScene *scene = segmenter->Scene();
  if (segmenter->candidate_objects.IsEmpty()) return 0;
  int ncandidates = segmenter->candidate_objects.NEntries();

  // Allocate array of neighbor overlaps
  RNScalar *overlaps = new RNScalar [ncandidates * ncandidates];
  int *counts = new int [ncandidates * ncandidates];
  if (!overlaps || !counts) {
    RNFail("Unable to allocate data for segmenter\n");
    return 0;
  }

  // Initialize array of neighbor overlaps
  for (int i = 0; i < ncandidates * ncandidates; i++) {
    overlaps[i] = 0;
    counts[i] = 0;
  }

  // Fill matrix of neighbor overlaps
  for (int i = 0; i < scene->NObjectRelationships(); i++) {
    R3SurfelObjectRelationship *relationship = scene->ObjectRelationship(i);
    R3SurfelObject *object0 = relationship->Object(0);
    R3SurfelObject *object1 = relationship->Object(1);
    while (object0->Parent() && (object0->Parent() != scene->RootObject())) object0 = object0->Parent();
    while (object1->Parent() && (object1->Parent() != scene->RootObject())) object1 = object1->Parent();
    int index0 = segmenter->candidate_object_index[object0->SceneIndex()];
    int index1 = segmenter->candidate_object_index[object1->SceneIndex()];
    if ((index0 < 0) || (index1 < 0)) continue;
    RNScalar overlap = relationship->Operand(0);
    overlaps[index0*ncandidates + index1] += overlap;
    overlaps[index1*ncandidates + index0] += overlap;
    counts[index0*ncandidates + index1]++;
    counts[index1*ncandidates + index0]++;
  }

  // Create neighbor relationships 
  for (int i = 0; i < ncandidates; i++) {
    for (int j = i; j < ncandidates; j++) {
      int count = counts[i*ncandidates+j];
      if (count == 0) continue;
      RNScalar overlap = overlaps[i*ncandidates+j]/count;
      if (overlap <= 0.1) overlap = 0.1;
      int neighbor_cost = 1.0/(overlap * overlap);
      gc->setNeighbors(i, j, neighbor_cost);
    }
  }

  // Delete array of neighbor overlaps
  delete [] overlaps;
  delete [] counts;

  // Return success
  return 1;
}



int R3SurfelSegmenter::
PredictObjectSegmentations(const RNArray<R3SurfelObject *>& objects)
{
  // Empty data structures
  if (candidate_object_index) { delete [] candidate_object_index; candidate_object_index = NULL; }
  if (anchor_object_index) { delete [] anchor_object_index; anchor_object_index = NULL; }
  candidate_objects.Empty();
  anchor_objects.Empty();
  
  // Allocate data structures
  if (scene->NObjects() == 0) return 1;
  candidate_object_index = new int [ scene->NObjects() ];
  anchor_object_index = new int [ scene->NObjects() ];
  for (int i = 0; i < scene->NObjects(); i++) candidate_object_index[i] = -1;
  for (int i = 0; i < scene->NObjects(); i++) anchor_object_index[i] = -1;

  // Fill candidate objects
  for (int i = 0; i < objects.NEntries(); i++) {
    R3SurfelObject *object = objects.Kth(i);
    if (!object->Parent() || (object->Parent() != scene->RootObject())) continue;
    candidate_object_index[object->SceneIndex()] = candidate_objects.NEntries();
    candidate_objects.Insert(object);
  }

  // Fill anchor objects
  for (int i = 0; i < candidate_objects.NEntries(); i++) {
    R3SurfelObject *object = candidate_objects.Kth(i);
    anchor_object_index[object->SceneIndex()] = anchor_objects.NEntries();
    anchor_objects.Insert(object);
  }

  // Check data structures
  if (candidate_objects.NEntries() == 0) return 0;
  if (anchor_objects.NEntries() == 0) return 0;
  if (anchor_objects.NEntries() == 1) {
    return 1;
  }

  // Allocate GC structure
  GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(candidate_objects.NEntries(), anchor_objects.NEntries());
  if (!gc) {
    RNFail("Unable to allocate graph cut structure\n");
    return 0;
  }

  // Set up cost structure
  if (!SetDataCosts(this, gc)) return 0;
  if (!SetNeighborCosts(this, gc)) return 0;

  // Set initial labels
  for (int i = 0; i < candidate_objects.NEntries(); i++) {
    R3SurfelObject *object = candidate_objects.Kth(i);
    if (anchor_object_index[object->SceneIndex()] >= 0) gc->setLabel(i, anchor_object_index[object->SceneIndex()]);
    else gc->setLabel(i, 0);
  }

  // Solve graph cut
  printf("A %d %d %g\n", candidate_objects.NEntries(), anchor_objects.NEntries(), (double) gc->compute_energy());
  gc->expansion(1);
  // gc->swap(1);
  printf("B %g\n", (double) gc->compute_energy());

  // Get final labels
  for (int i = 0; i < candidate_objects.NEntries(); i++) {
    R3SurfelObject *object = candidate_objects.Kth(i);
    int index = gc->whatLabel(i);
    R3SurfelObject *anchor_object = anchor_objects.Kth(index);
  }

  // Delete GC structure
  delete gc;

  // Return success
  return 1;
}

#else

int R3SurfelSegmenter::
PredictObjectSegmentations(const RNArray<R3SurfelObject *>& objects)
{
  // Temporary
  return 1;
}

#endif



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

