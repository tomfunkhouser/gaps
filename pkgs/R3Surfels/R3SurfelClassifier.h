/* Include file for the R3 surfel classifier class */
#ifndef __R3_SURFEL_CLASSIFIER_H__
#define __R3_SURFEL_CLASSIFIER_H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class R3SurfelClassifier {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  R3SurfelClassifier(R3SurfelScene *scene);
  ~R3SurfelClassifier(void);


  /////////////////////////////
  //// ACCESS FUNCTIONS ////
  /////////////////////////////

  // Get scene
  R3SurfelScene *Scene(void) const;

  
  ////////////////////////////////////
  //// LABEL ASSIGNMENT FUNCTIONS ////
  ////////////////////////////////////

  // Predict and assign label for a given object
  int PredictLabelAssignment(R3SurfelObject *object) const;

  // Predict and assign label for a given set of objects
  int PredictLabelAssignments(const RNArray<R3SurfelObject *>& objects) const;

  // Predict and assign labels for any objects in the scene chosen by classifer
  int PredictLabelAssignments(void) const;


  ///////////////////////
  //// I/O FUNCTIONS ////
  ///////////////////////

  // I/O of classifier data
  int ReadFile(const char *filename);
  int WriteFile(const char *filename) const;


  ///////////////////////////
  //// UPTDATE FUNCTIONS ////
  ///////////////////////////

  // Update classifer with different objects
  void UpdateAfterInsertObject(R3SurfelObject *object);
  void UpdateBeforeRemoveObject(R3SurfelObject *object);

  // Update classifer with different training data
  void UpdateAfterInsertLabelAssignment(R3SurfelLabelAssignment *assignment);
  void UpdateBeforeRemoveLabelAssignment(R3SurfelLabelAssignment *assignment);

  // Update classifer with different features
  void UpdateAfterInsertFeature(R3SurfelFeature *feature);
  void UpdateBeforeRemoveFeature(R3SurfelFeature *feature);


////////////////////////////////////////////////////////////////////////
// INTERNAL STUFF BELOW HERE
////////////////////////////////////////////////////////////////////////

protected:
  R3SurfelScene *scene;
  RNArray<R3SurfelObject *> training_set;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTIONS
////////////////////////////////////////////////////////////////////////

inline R3SurfelScene *R3SurfelClassifier::
Scene(void) const
{
  // Return scene
  return scene;
}



}; // end namespace



#endif
