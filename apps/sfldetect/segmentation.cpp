////////////////////////////////////////////////////////////////////////
// Source file for segmentation class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Constructor/destructor functions
////////////////////////////////////////////////////////////////////////

ObjectSegmentation::
ObjectSegmentation(const char *filename,
  const R3Point& origin, RNLength radius, RNLength height, RNScalar score)
  : parse(NULL),
    parse_index(-1),
    detection(NULL),
    detection_index(-1),
    assignments(),
    filename((filename) ? strdup(filename) : NULL),
    pointset(),
    origin(RN_UNKNOWN, RN_UNKNOWN, RN_UNKNOWN),
    radius(RN_UNKNOWN),
    height(RN_UNKNOWN),
    score(RN_UNKNOWN)
    
{ 
  // Get filename
  char buffer[4096];
  if (filename) {
    if (!RNFileExists(filename)) {
      if (parse && parse->SegmentationDirectory()) {
        sprintf(buffer, "%s/%s", parse->SegmentationDirectory(), filename);
        filename = buffer;
      }
    }

    // Read pointset
    if (!pointset.ReadFile(filename)) {
      fprintf(stderr, "Unable to read pointset from %s\n", filename);
      return;
    }
  }

   // Update properties
  if (origin != R3unknown_point) SetOrigin(origin);
  else { R3Point o = pointset.Centroid(); o[2] = pointset.BBox().ZMin(); SetOrigin(o); }
  pointset.SetOrigin(this->origin);
  if (radius != RN_UNKNOWN) SetRadius(radius);
  else SetRadius(pointset.Radius());
  if (height != RN_UNKNOWN) SetHeight(height);
  else SetHeight(pointset.Height());
  pointset.UpdatePoints();
}  



ObjectSegmentation::
ObjectSegmentation(const ObjectPointSet& pset, 
  const R3Point& origin, RNLength radius, RNLength height, RNScalar score)
  : parse(NULL),
    parse_index(-1),
    detection(NULL),
    detection_index(-1),
    assignments(),
    filename(NULL),
    pointset(pset),
    origin(RN_UNKNOWN,RN_UNKNOWN,RN_UNKNOWN),
    radius(RN_UNKNOWN),
    height(RN_UNKNOWN),
    score(RN_UNKNOWN)
{ 
  // Update properties
  if (origin != R3unknown_point) SetOrigin(origin);
  else { R3Point o = pointset.Centroid(); o[2] = pointset.BBox().ZMin(); SetOrigin(o); }
  pointset.SetOrigin(this->origin);
  if (radius > 0) SetRadius(radius);
  else SetRadius(pointset.Radius());
  if (height > 0) SetHeight(height);
  else SetHeight(pointset.Height());
  pointset.UpdatePoints();
}  



ObjectSegmentation::
ObjectSegmentation(const ObjectSegmentation& segmentation)
  : parse(NULL),
    parse_index(-1),
    detection(NULL),
    detection_index(-1),
    assignments(),
    filename((segmentation.filename) ? strdup(segmentation.filename) : NULL),
    pointset(segmentation.pointset),
    origin(segmentation.origin),
    radius(segmentation.radius),
    height(segmentation.height),
    score(segmentation.score)
{ 
}  



ObjectSegmentation::
~ObjectSegmentation(void) 
{ 
  // Delete assignments
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    delete assignment;
  }

  // Remove from detection
  if (detection_index >= 0) detection->RemoveSegmentation(this);

  // Remove from parse
  if (parse_index >= 0) parse->RemoveSegmentation(this);

  // Delete filename
  if (filename) free(filename);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

ObjectLabel *ObjectSegmentation::
BestLabel(void) const
{
  // Find best label
  ObjectModel *best_model = BestModel();
  if (!best_model) return NULL;
  return best_model->Label();
}



ObjectLabel *ObjectSegmentation::
GroundTruthLabel(void) const
{
  // Find ground truth label
  ObjectModel *ground_truth_model = GroundTruthModel();
  if (!ground_truth_model) return NULL;
  return ground_truth_model->Label();
}



ObjectModel *ObjectSegmentation::
BestModel(void) const
{
  // Find best model
  ObjectAssignment *best_assignment = BestAssignment();
  if (!best_assignment) return NULL;
  return best_assignment->Model();
}



ObjectModel *ObjectSegmentation::
GroundTruthModel(void) const
{
  // Find ground truth model
  ObjectAssignment *ground_truth_assignment = GroundTruthAssignment();
  if (!ground_truth_assignment) return NULL;
  return ground_truth_assignment->Model();
}



ObjectAssignment *ObjectSegmentation::
BestAssignment(void) const
{
  // Find best assignment
  RNScalar best_score = 0;
  ObjectAssignment *best_assignment = NULL;
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    if (assignment->IsGroundTruth()) continue;
    RNScalar score = assignment->Score();
    if (score > best_score) {
      best_assignment = assignment;
      best_score = score;
    }
  }

  // Return best assignment
  return best_assignment;
}



ObjectAssignment *ObjectSegmentation::
BestAssignment(const ObjectLabel *label) const
{
  // Find assignment
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    if (assignment->IsGroundTruth()) continue;
    ObjectModel *model = assignment->Model();
    if (model->Label() == label) return assignment;
  }

  // Not found
  return NULL;
}



ObjectAssignment *ObjectSegmentation::
BestAssignment(const ObjectModel *model) const
{
  // Find assignment
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    if (assignment->IsGroundTruth()) continue;
    if (assignment->Model() == model) return assignment;
  }

  // Not found
  return NULL;
}



ObjectAssignment *ObjectSegmentation::
GroundTruthAssignment(void) const
{
  // Find ground truth assignment
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    if (assignment->IsGroundTruth()) return assignment;
  }

  // Not found
  return NULL;
}



////////////////////////////////////////////////////////////////////////
// Assignment manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectSegmentation::
InsertAssignment(ObjectAssignment *assignment)
{
  // Just checking
  assert(assignment->segmentation_index == -1);
  // assert(assignment->segmentation == NULL);

  // Insert assignment into segmentation
  assignment->segmentation = this;
  assignment->segmentation_index = assignments.NEntries();
  assignments.Insert(assignment);

  // Insert assignment into parse
  if (parse && !assignment->Parse()) parse->InsertAssignment(assignment);
}



void ObjectSegmentation::
RemoveAssignment(ObjectAssignment *assignment)
{
  // Just checking
  assert(assignment->segmentation_index >= 0);
  assert(assignment->segmentation_index < assignments.NEntries());
  assert(assignment->segmentation == this);

  // Remove assignment from parse
  if (parse) parse->RemoveAssignment(assignment);

  // Remove assignment from segmentation
  RNArrayEntry *entry = assignments.KthEntry(assignment->segmentation_index);
  ObjectAssignment *tail = assignments.Tail();
  tail->segmentation_index = assignment->segmentation_index;
  assignments.EntryContents(entry) = tail;
  assignments.RemoveTail();
  assignment->segmentation_index = -1;
  assignment->segmentation = NULL;
}



////////////////////////////////////////////////////////////////////////
// Property manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectSegmentation::
SetPointSet(const ObjectPointSet& pointset)
{
  // Copy pointset
  this->pointset = pointset;

  // Update pointset
  this->pointset.UpdatePoints();
}



void ObjectSegmentation::
SetOrigin(const R3Point& origin)
{
  // Set origin
  this->origin = origin;
}



void ObjectSegmentation::
SetRadius(RNLength radius)
{
  // Set radius
  this->radius = radius;
}



void ObjectSegmentation::
SetHeight(RNLength height)
{
  // Set height
  this->height = height;
}



void ObjectSegmentation::
SetScore(RNScalar score)
{
  // Set score
  this->score = score;
}



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

int CompareObjectSegmentations(const void *data1, const void *data2)
{
  // Assumes assignments are sorted best-to-worst for each segmentation
  ObjectSegmentation *segmentation1 = *((ObjectSegmentation **) data1);
  ObjectSegmentation *segmentation2 = *((ObjectSegmentation **) data2);
  if (segmentation1->NAssignments() == 0) return 1;
  if (segmentation2->NAssignments() == 0) return -1;
  ObjectAssignment *assignment1 = segmentation1->Assignment(0);
  ObjectAssignment *assignment2 = segmentation2->Assignment(0);
  if (assignment1->score > assignment2->score) return -1;
  if (assignment1->score < assignment2->score) return 1;
  return 0;
}

