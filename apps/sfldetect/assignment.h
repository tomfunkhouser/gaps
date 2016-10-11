/////////////////////////////////////////////////////////////////////////
// Include file for object segmentation label assignment class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class ObjectAssignment {
public:
  ////////////////////////////////////////
  // Constructor/destructor functions
  ////////////////////////////////////////

  // Constructor destructor
  ObjectAssignment(ObjectSegmentation *segmentation = NULL, ObjectModel *model = NULL, 
    const R3Affine& model_to_segmentation_transformation = R3identity_affine,
    RNScalar score = RN_UNKNOWN, RNBoolean ground_truth = FALSE);
  ~ObjectAssignment(void);


  ////////////////////////////////////////
  // Access functions
  ////////////////////////////////////////

  // Parse access
  ObjectParse *Parse(void) const;
  int ParseIndex(void) const;

  // Segmentation access
  ObjectSegmentation *Segmentation(void) const;
  int SegmentationIndex(void) const;

  // Model access
  ObjectModel *Model(void) const;
  int ModelIndex(void) const;

  // Property access
  const R3Affine& ModelToSegmentationTransformation(void) const;
  R3Affine SegmentationToModelTransformation(void) const;
  RNScalar Score(void) const;
  RNBoolean IsGroundTruth(void) const;


  ////////////////////////////////////////
  // Manipulation functions
  ////////////////////////////////////////

  // Property manipulation
  void SetModelToSegmentationTransformation(const R3Affine& transformation);
  void SetSegmentationToModelTransformation(const R3Affine& transformation);
  void SetScore(RNScalar score);
  void SetGroundTruth(RNBoolean ground_truth);


  ////////////////////////////////////////
  // Internal stuff
  ////////////////////////////////////////

public:
  friend class ObjectParse;
  ObjectParse *parse;
  int parse_index;
  friend class ObjectSegmentation;
  ObjectSegmentation *segmentation;
  int segmentation_index;
  friend class ObjectModel;
  ObjectModel *model;
  int model_index;
  R3Affine model_to_segmentation_transformation;
  RNScalar score;
  RNBoolean ground_truth;
};



////////////////////////////////////////////////////////////////////////
// Extern functions (used for sorting)
////////////////////////////////////////////////////////////////////////

extern int CompareObjectAssignments(const void *data1, const void *data2);



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline ObjectParse *ObjectAssignment::
Parse(void) const
{
  // Return parse
  return parse;
}



inline int ObjectAssignment::
ParseIndex(void) const
{
  // Return parse index
  return parse_index;
}



inline ObjectSegmentation *ObjectAssignment::
Segmentation(void) const
{
  // Return segmentation
  return segmentation;
}



inline int ObjectAssignment::
SegmentationIndex(void) const
{
  // Return segmentation index
  return segmentation_index;
}



inline ObjectModel *ObjectAssignment::
Model(void) const
{
  // Return model
  return model;
}



inline int ObjectAssignment::
ModelIndex(void) const
{
  // Return model index
  return model_index;
}



inline const R3Affine& ObjectAssignment::
ModelToSegmentationTransformation(void) const
{
  // Return transformation
  return model_to_segmentation_transformation;
}



inline R3Affine ObjectAssignment::
SegmentationToModelTransformation(void) const
{
  // Return inverse transformation
  return model_to_segmentation_transformation.Inverse();
}
  


inline RNScalar ObjectAssignment::
Score(void) const
{
  // Return score
  return score;
}



inline RNBoolean ObjectAssignment::
IsGroundTruth(void) const
{
  // Return whether assignment is ground truth
  return ground_truth;
}






