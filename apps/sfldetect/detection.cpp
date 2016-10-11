////////////////////////////////////////////////////////////////////////
// Source file for detection class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Constructor/desctructor functions
////////////////////////////////////////////////////////////////////////

ObjectDetection::
ObjectDetection(const char *name,
  const R3Point& origin, RNLength max_radius, RNLength max_height)
  : parse(NULL),
    parse_index(-1),
    segmentations(),
    ground_truth_segmentation(NULL),
    name((name) ? strdup(name) : NULL),
    origin(origin),
    max_radius(max_radius),
    max_height(max_height)
    
{ 
}  



ObjectDetection::
~ObjectDetection(void) 
{ 
  // Delete segmentations
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    delete segmentation;
  }

  // Delete ground truth segmentation
  if (ground_truth_segmentation) {
    delete ground_truth_segmentation;
  }

  // Remove from parse
  if (parse_index >= 0) parse->RemoveDetection(this);

  // Delete name
  if (name) free(name); 
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

ObjectSegmentation *ObjectDetection::
BestSegmentation(void) const
{
  // Find best segmentation
  RNScalar best_score = 0;
  ObjectSegmentation *best_segmentation = NULL;
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    RNScalar score = segmentation->Score();
    if (score > best_score) {
      best_segmentation = segmentation;
      best_score = score;
    }
  }

  // Return best segmentation
  return best_segmentation;
}



ObjectLabel *ObjectDetection::
BestLabel(void) const
{
  // Find best label
  ObjectModel *best_model = BestModel();
  if (!best_model) return NULL;
  return best_model->Label();
}



ObjectLabel *ObjectDetection::
GroundTruthLabel(void) const
{
  // Find ground truth label
  ObjectModel *ground_truth_model = GroundTruthModel();
  if (!ground_truth_model) return NULL;
  return ground_truth_model->Label();
}



ObjectModel *ObjectDetection::
BestModel(void) const
{
  // Find best model
  ObjectAssignment *best_assignment = BestAssignment();
  if (!best_assignment) return NULL;
  return best_assignment->Model();
}



ObjectModel *ObjectDetection::
GroundTruthModel(void) const
{
  // Find ground truth model
  ObjectAssignment *ground_truth_assignment = GroundTruthAssignment();
  if (!ground_truth_assignment) return NULL;
  return ground_truth_assignment->Model();
}



ObjectAssignment *ObjectDetection::
BestAssignment(void) const
{
  // Find best assignment
  RNScalar best_score = 0;
  ObjectAssignment *best_assignment = NULL;
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    ObjectAssignment *assignment = segmentation->BestAssignment();
    RNScalar score = assignment->Score();
    if (score > best_score) {
      best_assignment = assignment;
      best_score = score;
    }
  }

  // Return best assignment
  return best_assignment;
}



ObjectAssignment *ObjectDetection::
BestAssignment(ObjectLabel *label) const
{
  // Find best assignment
  RNScalar best_score = 0;
  ObjectAssignment *best_assignment = NULL;
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    ObjectAssignment *assignment = segmentation->BestAssignment(label);
    RNScalar score = assignment->Score();
    if (score > best_score) {
      best_assignment = assignment;
      best_score = score;
    }
  }

  // Return best assignment
  return best_assignment;
}



ObjectAssignment *ObjectDetection::
BestAssignment(ObjectModel *model) const
{
  // Find best assignment
  RNScalar best_score = 0;
  ObjectAssignment *best_assignment = NULL;
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    ObjectAssignment *assignment = segmentation->BestAssignment(model);
    RNScalar score = assignment->Score();
    if (score > best_score) {
      best_assignment = assignment;
      best_score = score;
    }
  }

  // Return best assignment
  return best_assignment;
}



ObjectAssignment *ObjectDetection::
GroundTruthAssignment(void) const
{
  // Find ground truth assignment
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    ObjectAssignment *assignment = segmentation->GroundTruthAssignment();
    if (assignment) return assignment;
  }

  // None found
  return NULL;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectDetection::
InsertSegmentation(ObjectSegmentation *segmentation)
{
  // Just checking
  assert(segmentation->detection_index == -1);
  assert(segmentation->detection == NULL);

  // Insert segmentation
  segmentation->detection = this;
  segmentation->detection_index = segmentations.NEntries();
  segmentations.Insert(segmentation);

  // Insert segmentation into parse
  if (parse) parse->InsertSegmentation(segmentation);
}



void ObjectDetection::
RemoveSegmentation(ObjectSegmentation *segmentation)
{
  // Just checking
  assert(segmentation->detection_index >= 0);
  assert(segmentation->detection_index < segmentations.NEntries());
  assert(segmentation->detection == this);

  // Remove segmentation from parse
  if (parse) parse->RemoveSegmentation(segmentation);

  // Remove segmentation
  RNArrayEntry *entry = segmentations.KthEntry(segmentation->detection_index);
  ObjectSegmentation *tail = segmentations.Tail();
  tail->detection_index = segmentation->detection_index;
  segmentations.EntryContents(entry) = tail;
  segmentations.RemoveTail();
  segmentation->detection_index = -1;
  segmentation->detection = NULL;
}



void ObjectDetection::
SetName(const char *name) 
{
  // Set name
  if (this->name) free(this->name);
  if (name) this->name = strdup(name);
  else this->name = NULL;
}



void ObjectDetection::
SetOrigin(const R3Point& origin)
{
  // Set origin
  this->origin = origin;
}



void ObjectDetection::
SetMaxRadius(RNLength radius)
{
  // Set maximum radius
  this->max_radius = radius;
}



void ObjectDetection::
SetMaxHeight(RNLength height)
{
  // Set maximum height
  this->max_height = height;
}



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

int CompareObjectDetections(const void *data1, const void *data2)
{
  // Assumes segmentations and assignments are sorted best-to-worst 
  ObjectDetection *detection1 = *((ObjectDetection **) data1);
  ObjectDetection *detection2 = *((ObjectDetection **) data2);
  if (detection1->NSegmentations() == 0) return 1;
  if (detection2->NSegmentations() == 0) return -1;
  ObjectSegmentation *segmentation1 = detection1->Segmentation(0);
  ObjectSegmentation *segmentation2 = detection2->Segmentation(0);
  if (segmentation1->NAssignments() == 0) return 1;
  if (segmentation2->NAssignments() == 0) return -1;
  ObjectAssignment *assignment1 = segmentation1->Assignment(0);
  ObjectAssignment *assignment2 = segmentation2->Assignment(0);
  if (assignment1->score > assignment2->score) return -1;
  if (assignment1->score < assignment2->score) return 1;
  return 0;
}

