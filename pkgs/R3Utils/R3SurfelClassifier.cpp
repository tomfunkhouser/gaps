/* Source file for the surfel scene classifier class */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// Surfel predictor constructor/destructor
////////////////////////////////////////////////////////////////////////

R3SurfelClassifier::
R3SurfelClassifier(R3SurfelScene *scene)
  : scene(scene),
    training_set()
{
  // Initialize training set
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelLabel *label = object->HumanLabel();
    if (label && strcmp(label->Name(), "Unknown")) {
      training_set.Insert(object);
    }
  }

  // Predict label assignments
  // PredictLabelAssignments();
}



R3SurfelClassifier::
~R3SurfelClassifier(void)
{
}



////////////////////////////////////////////////////////////////////////
// Label assignment functions
////////////////////////////////////////////////////////////////////////

int R3SurfelClassifier::
PredictLabelAssignment(R3SurfelObject *object) const
{
  // Check labels
  if (scene->NLabels() <= 1) return 0;

  /////////// REMOVE PREVIOUS PREDICTED ASSIGNMENTS /////////

  // Find previous predicted assignments
  RNArray<R3SurfelLabelAssignment *> previous_assignments;
  for (int i = 0; i < object->NLabelAssignments(); i++) {
    R3SurfelLabelAssignment *a = object->LabelAssignment(i);
    if (a->Originator() != R3_SURFEL_MACHINE_ORIGINATOR) continue;
    previous_assignments.Insert(a);
  }

  // Remove previous predicted assignments
  for (int i = 0; i < previous_assignments.NEntries(); i++) {
    R3SurfelLabelAssignment *a = previous_assignments.Kth(i);
    scene->RemoveLabelAssignment(a);
  }

  /////////// COMPUTE DISTANCE TO CLOSEST TRAINING OBJECT FOR EACH LABEL  /////////

  // Initialize best distance for each label type
  RNScalar *best_squared_distance_per_label = new RNScalar [ scene->NLabels() ];
  for (int i = 0; i < scene->NLabels(); i++) {
    best_squared_distance_per_label[i] = FLT_MAX;
  }

  // Get feature vector for object
  // const R3SurfelFeatureVector& vector = object->FeatureVector();
  R3SurfelFeatureVector vector(3);  // TEMPORARY
  vector.SetValue(0, object->Centroid().X());
  vector.SetValue(1, object->Centroid().Y());
  vector.SetValue(2, object->Centroid().Z());
  
  // Find best distance for each label type
  for (int i = 0; i < training_set.NEntries(); i++) {
    // Get object
    R3SurfelObject *training_object = training_set.Kth(i);

    // Get label
    R3SurfelLabel *training_label = training_object->HumanLabel();
    if (!training_label) continue;

    // Get feature vector
    // const R3SurfelFeatureVector& training_vector = training_object->FeatureVector();
    R3SurfelFeatureVector training_vector(3); // TEMPORARY
    training_vector.SetValue(0, training_object->Centroid().X());
    training_vector.SetValue(1, training_object->Centroid().Y());
    training_vector.SetValue(2, training_object->Centroid().Z());

    // Compute squared distance to feature vector
    RNScalar squared_distance = vector.EuclideanDistanceSquared(training_vector);

    // Remember squared distance if it is the best for the label type
    if (squared_distance < best_squared_distance_per_label[training_label->SceneIndex()]) {
      best_squared_distance_per_label[training_label->SceneIndex()] = squared_distance;
    }
  }

  /////////// FIND BEST LABEL  /////////

  // Find best label
  R3SurfelLabel *best_label = NULL;
  RNScalar best_squared_distance = FLT_MAX;
  RNScalar second_best_squared_distance = FLT_MAX;
  for (int i = 0; i < scene->NLabels(); i++) {
    if (best_squared_distance_per_label[i] < best_squared_distance) {
      best_label = scene->Label(i);
      second_best_squared_distance = best_squared_distance;
      best_squared_distance = best_squared_distance_per_label[i];
    }
    else if (best_squared_distance_per_label[i] < second_best_squared_distance) {
      second_best_squared_distance = best_squared_distance_per_label[i];
    }
  }


  //////// COMPUTE CONFIDENCE /////////

   // Compute confidence
  RNScalar confidence = 0;
  if ((best_label == NULL) || (best_squared_distance == FLT_MAX)) {
    confidence = 0;
  }
  else if (second_best_squared_distance == FLT_MAX) {
    confidence = RN_EPSILON;
  }
  else if (second_best_squared_distance == 0) {
    confidence = 0.5;
  }
  else {
    confidence = 1.0 - 2.0 * best_squared_distance / (best_squared_distance + second_best_squared_distance);
  }

  // Check confidence
  RNScalar min_confidence = 0;
  if (confidence < min_confidence) {
    best_label = scene->FindLabelByName("Unknown");
    confidence = 0;
  }

  //////// CREATE ASSIGNMENT ///////////

  // Insert label assignment
  if (best_label) {
    R3SurfelLabelAssignment *assignment = new R3SurfelLabelAssignment(object, best_label, 
      confidence, R3_SURFEL_MACHINE_ORIGINATOR);
    scene->InsertLabelAssignment(assignment);
  }

  // Delete temporary data
  delete [] best_squared_distance_per_label;

  // Return success
  return 1;
}



int R3SurfelClassifier::
PredictLabelAssignments(const RNArray<R3SurfelObject *>& objects) const
{
  // Check labels
  if (scene->NLabels() <= 1) return 0;

  // Predict label assignments for all objects in set
  for (int i = 0; i < objects.NEntries(); i++) {
    R3SurfelObject *object = objects.Kth(i);
    if (object->HumanLabelAssignment()) continue;
    if (!PredictLabelAssignment(object)) return 0;
  }

  // Return success
  return 1;
}



int R3SurfelClassifier::
PredictLabelAssignments(void) const
{
  // Predict label assignments for all objects in scene (except root)
  for (int i = 1; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    R3SurfelObjectAssignment *assignment = object->CurrentLabelAssignment();
    if (assignment && (assignment->Originator() == R3_SURFEL_HUMAN_ORIGINATOR)) continue;
    if (assignment && (assignment->Confidence() >= 1.0)) continue;
    if (!PredictLabelAssignment(object)) return 0;
  }

   // Return success
  return 1;
}





////////////////////////////////////////////////////////////////////////
// Input/output functions
////////////////////////////////////////////////////////////////////////

int R3SurfelClassifier::
ReadFile(const char *filename)
{
  // Return success
  return 1;
}



int R3SurfelClassifier::
WriteFile(const char *filename) const
{
  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Input/output functions
////////////////////////////////////////////////////////////////////////

void R3SurfelClassifier::
UpdateAfterInsertObject(R3SurfelObject *object)
{
  // Add to training set
  R3SurfelLabel *label = object->HumanLabel();
  if (label && strcmp(label->Name(), "Unknown")) {
    if (!training_set.FindEntry(object)) {
      training_set.Insert(object);
    }
  }
}



void R3SurfelClassifier::
UpdateBeforeRemoveObject(R3SurfelObject *object)
{
  // Remove from training set
  R3SurfelLabel *label = object->HumanLabel();
  if (label && strcmp(label->Name(), "Unknown")) {
    if (training_set.FindEntry(object)) {
      training_set.Remove(object);
    }
  }
}



void R3SurfelClassifier::
UpdateAfterInsertLabelAssignment(R3SurfelLabelAssignment *assignment)
{
  // Get convenient variables
  R3SurfelObject *object = assignment->Object();
  R3SurfelLabel *label = assignment->Label();

  // Update training set
  if (object->HumanLabel() && (assignment->Originator() == R3_SURFEL_HUMAN_ORIGINATOR)) {
    if (strcmp(label->Name(), "Unknown")) {
      if (!training_set.FindEntry(object)) {
        // Insert object into training set 
        training_set.Insert(object);
      }
    }
  }
}



void R3SurfelClassifier::
UpdateBeforeRemoveLabelAssignment(R3SurfelLabelAssignment *assignment)
{
  // Get convenient variables
  R3SurfelObject *object = assignment->Object();
  R3SurfelLabel *label = assignment->Label();
   
  // Check assignment
  if (object->HumanLabel() && (assignment->Originator() == R3_SURFEL_HUMAN_ORIGINATOR)) {
    if (strcmp(label->Name(), "Unknown")) {
      if (training_set.FindEntry(object)) {
        // Remove object from training set 
        training_set.Remove(object);
      }
    }
  }
}



void R3SurfelClassifier::
UpdateAfterInsertFeature(R3SurfelFeature *feature)
{
}



void R3SurfelClassifier::
UpdateBeforeRemoveFeature(R3SurfelFeature *feature)
{
}



}; // end namespace
