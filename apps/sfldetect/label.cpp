////////////////////////////////////////////////////////////////////////
// Source file for label class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Constructor/desctructor functions
////////////////////////////////////////////////////////////////////////

ObjectLabel::
ObjectLabel(const char *name,
  const R3Point& origin, RNLength max_radius, RNLength max_height)
  : parse(NULL),
    parse_index(-1),
    models(),
    name((name) ? strdup(name) : NULL),
    origin(origin),
    max_radius(max_radius),
    max_height(max_height)
{ 
}  



ObjectLabel::
~ObjectLabel(void) 
{ 
  // Delete models
  for (int i = 0; i < NModels(); i++) {
    ObjectModel *model = Model(i);
    delete model;
  }

  // Remove from parse
  if (parse_index >= 0) parse->RemoveLabel(this);

  // Delete name
  if (name) free(name); 
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectLabel::
InsertModel(ObjectModel *model)
{
  // Just checking
  assert(model->label_index == -1);
  assert(model->label == NULL);

  // Insert model
  model->label = this;
  model->label_index = models.NEntries();
  models.Insert(model);

  // Insert model into parse
  if (parse) parse->InsertModel(model);
}



void ObjectLabel::
RemoveModel(ObjectModel *model)
{
  // Just checking
  assert(model->label_index >= 0);
  assert(model->label_index < models.NEntries());
  assert(model->label == this);

  // Remove model from parse
  if (parse) parse->RemoveModel(model);

  // Remove model
  RNArrayEntry *entry = models.KthEntry(model->label_index);
  ObjectModel *tail = models.Tail();
  tail->label_index = model->label_index;
  models.EntryContents(entry) = tail;
  models.RemoveTail();
  model->label_index = -1;
  model->label = NULL;
}



void ObjectLabel::
SetName(const char *name) 
{
  // Set name
  if (this->name) free(this->name);
  if (name) this->name = strdup(name);
  else this->name = NULL;
}



void ObjectLabel::
SetOrigin(const R3Point& origin)
{
  // Set origin
  this->origin = origin;
}



void ObjectLabel::
SetMaxRadius(RNLength radius)
{
  // Set maximum radius
  this->max_radius = radius;
}



void ObjectLabel::
SetMaxHeight(RNLength height)
{
  // Set maximum height
  this->max_height = height;
}



////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

int CompareObjectLabels(const void *data1, const void *data2)
{
  // Assumes models and assignments are sorted best-to-worst 
  ObjectLabel *label1 = *((ObjectLabel **) data1);
  ObjectLabel *label2 = *((ObjectLabel **) data2);
  if (label1->NModels() == 0) return 1;
  if (label2->NModels() == 0) return -1;
  ObjectModel *model1 = label1->Model(0);
  ObjectModel *model2 = label2->Model(0);
  if (model1->NAssignments() == 0) return 1;
  if (model2->NAssignments() == 0) return -1;
  ObjectAssignment *assignment1 = model1->Assignment(0);
  ObjectAssignment *assignment2 = model2->Assignment(0);
  if (assignment1->score > assignment2->score) return -1;
  if (assignment1->score < assignment2->score) return 1;
  return 0;
}

