/* Include file for the R3 surfel segmenter class */
#ifndef __R3_SURFEL_SEGMENTER_H__
#define __R3_SURFEL_SEGMENTER_H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class R3SurfelSegmenter {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  R3SurfelSegmenter(R3SurfelScene *scene);
  ~R3SurfelSegmenter(void);


  /////////////////////////////
  //// ACCESS FUNCTIONS ////
  /////////////////////////////

  // Get scene
  R3SurfelScene *Scene(void) const;


  //////////////////////////////////////
  //// OBJECT SEGMENTATION FUNCTIONS ////
  //////////////////////////////////////

  // Predict segmentations 
  int PredictObjectSegmentations(const RNArray<R3SurfelObject *>& objects);
  int PredictObjectSegmentations(void);


  ///////////////////////////
  //// UPTDATE FUNCTIONS ////
  ///////////////////////////

  // Update classifer with different objects
  void UpdateAfterInsertObject(R3SurfelObject *object);
  void UpdateBeforeRemoveObject(R3SurfelObject *object);

  // Update classifer with different training data
  void UpdateAfterInsertLabelAssignment(R3SurfelLabelAssignment *assignment);
  void UpdateBeforeRemoveLabelAssignment(R3SurfelLabelAssignment *assignment);


////////////////////////////////////////////////////////////////////////
// INTERNAL STUFF BELOW HERE
////////////////////////////////////////////////////////////////////////

public:
  R3SurfelScene *scene;
  RNArray<R3SurfelObject *> candidate_objects;
  RNArray<R3SurfelObject *> anchor_objects;
  int *candidate_object_index;
  int *anchor_object_index;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTIONS
////////////////////////////////////////////////////////////////////////

inline R3SurfelScene *R3SurfelSegmenter::
Scene(void) const
{
  // Return scene
  return scene;
}



}; // end namespace



#endif
