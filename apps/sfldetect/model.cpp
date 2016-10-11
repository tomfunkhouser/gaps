////////////////////////////////////////////////////////////////////////
// Source file for model class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Constructor/destructor functions
////////////////////////////////////////////////////////////////////////

ObjectModel::
ObjectModel(const char *filename,
  const R3Point& origin, RNLength radius, RNLength height)
  : parse(NULL),
    parse_index(-1),
    label(NULL),
    label_index(-1),
    assignments(),
    filename((filename) ? strdup(filename) : NULL),
    pointset(),
    origin(RN_UNKNOWN, RN_UNKNOWN, RN_UNKNOWN),
    radius(RN_UNKNOWN),
    height(RN_UNKNOWN)
    
{ 
  // Get filename
  char buffer[4096];
  if (!RNFileExists(filename)) {
    if (parse && parse->ModelDirectory()) {
      sprintf(buffer, "%s/%s", parse->ModelDirectory(), filename);
      filename = buffer;
    }
  }

  // Read pointset
  if (!pointset.ReadFile(filename)) {
    fprintf(stderr, "Unable to read pointset from %s\n", filename);
    return;
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



ObjectModel::
ObjectModel(const ObjectPointSet& pset, 
  const R3Point& origin, RNLength radius, RNLength height)
  : parse(NULL),
    parse_index(-1),
    label(NULL),
    label_index(-1),
    assignments(),
    filename(NULL),
    pointset(pset),
    origin(RN_UNKNOWN, RN_UNKNOWN, RN_UNKNOWN),
    radius(RN_UNKNOWN),
    height(RN_UNKNOWN)
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



ObjectModel::
~ObjectModel(void) 
{ 
  // Delete assignments
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    delete assignment;
  }

  // Remove from label
  if (label_index >= 0) label->RemoveModel(this);

  // Remove from parse
  if (parse_index >= 0) parse->RemoveModel(this);

  // Delete filename
  if (filename) free(filename);
}



////////////////////////////////////////////////////////////////////////
// Assignment access functions
////////////////////////////////////////////////////////////////////////

ObjectAssignment *ObjectModel::
Assignment(const ObjectSegmentation *segmentation) const
{
  // Find assignment
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    if (assignment->Segmentation() == segmentation) return assignment;
  }

  // Not found
  return NULL;
}



////////////////////////////////////////////////////////////////////////
// Assignment manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectModel::
InsertAssignment(ObjectAssignment *assignment)
{
  // Just checking
  assert(assignment->model_index == -1);
  // assert(assignment->model == NULL);

  // Insert assignment into model
  assignment->model = this;
  assignment->model_index = assignments.NEntries();
  assignments.Insert(assignment);

  // Insert assignment into parse
  if (parse && !assignment->Parse()) parse->InsertAssignment(assignment);
}



void ObjectModel::
RemoveAssignment(ObjectAssignment *assignment)
{
  // Just checking
  assert(assignment->model_index >= 0);
  assert(assignment->model_index < assignments.NEntries());
  assert(assignment->model == this);

  // Remove assignment from parse
  if (parse) parse->RemoveAssignment(assignment);

  // Remove assignment from model
  RNArrayEntry *entry = assignments.KthEntry(assignment->model_index);
  ObjectAssignment *tail = assignments.Tail();
  tail->model_index = assignment->model_index;
  assignments.EntryContents(entry) = tail;
  assignments.RemoveTail();
  assignment->model_index = -1;
  assignment->model = NULL;
}



////////////////////////////////////////////////////////////////////////
// Property manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectModel::
SetPointSet(const ObjectPointSet& pointset)
{
  // Set pointset
  this->pointset = pointset;

  // Update pointset
  this->pointset.UpdatePoints();
}



void ObjectModel::
SetOrigin(const R3Point& origin)
{
  // Set origin
  this->origin = origin;
}



void ObjectModel::
SetRadius(RNLength radius)
{
  // Set radius
  this->radius = radius;
}



void ObjectModel::
SetHeight(RNLength height)
{
  // Set height
  this->height = height;
}



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

int CompareObjectModels(const void *data1, const void *data2)
{
  // Assumes assignments are sorted best-to-worst for each model
  ObjectModel *model1 = *((ObjectModel **) data1);
  ObjectModel *model2 = *((ObjectModel **) data2);
  if (model1->NAssignments() == 0) return 1;
  if (model2->NAssignments() == 0) return -1;
  ObjectAssignment *assignment1 = model1->Assignment(0);
  ObjectAssignment *assignment2 = model2->Assignment(0);
  if (assignment1->score > assignment2->score) return -1;
  if (assignment1->score < assignment2->score) return 1;
  return 0;
}

