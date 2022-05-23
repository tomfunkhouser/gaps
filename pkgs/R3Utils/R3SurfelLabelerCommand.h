/* Include file for the surfel scene labeler command stuff */
#ifndef __R3_SURFEL_LABELER_COMMAND_H__
#define __R3_SURFEL_LABELER_COMMAND_H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// Command types
////////////////////////////////////////////////////////////////////////

enum {
  R3_SURFEL_LABELER_INITIALIZE_COMMAND,  
  R3_SURFEL_LABELER_TERMINATE_COMMAND,  
  R3_SURFEL_LABELER_SYNC_COMMAND,  
  R3_SURFEL_LABELER_ZOOM_COMMAND,  
  R3_SURFEL_LABELER_CAPTURE_IMAGE_COMMAND,  
  R3_SURFEL_LABELER_SELECT_NONE_COMMAND,  
  R3_SURFEL_LABELER_SELECT_PICKED_COMMAND,  
  R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND,  
  R3_SURFEL_LABELER_SELECT_OVERLAPPING_COMMAND,  
  R3_SURFEL_LABELER_SELECT_UNLABELED_COMMAND,  
  R3_SURFEL_LABELER_SELECT_SUGGESTED_COMMAND,  
  R3_SURFEL_LABELER_SELECT_ALL_COMMAND,  
  R3_SURFEL_LABELER_LABEL_PICKED_COMMAND,  
  R3_SURFEL_LABELER_LABEL_SELECTION_COMMAND,  
  R3_SURFEL_LABELER_RELABEL_PICKED_COMMAND,  
  R3_SURFEL_LABELER_RELABEL_SELECTION_COMMAND,  
  R3_SURFEL_LABELER_CONFIRM_PICKED_COMMAND,  
  R3_SURFEL_LABELER_CONFIRM_SELECTION_COMMAND,  
  R3_SURFEL_LABELER_CONFIRM_ALL_COMMAND,  
  R3_SURFEL_LABELER_REFUTE_PICKED_COMMAND,  
  R3_SURFEL_LABELER_REFUTE_SELECTION_COMMAND,  
  R3_SURFEL_LABELER_REFUTE_ALL_COMMAND,  
  R3_SURFEL_LABELER_SET_AERIAL_VISIBILITY_COMMAND,  
  R3_SURFEL_LABELER_SET_TERRESTRIAL_VISIBILITY_COMMAND,  
  R3_SURFEL_LABELER_SET_LABEL_NAME_VISIBILITY_COMMAND,  
  R3_SURFEL_LABELER_SET_SURFEL_COLOR_SCHEME_COMMAND,  
  R3_SURFEL_LABELER_SET_LABEL_VISIBILITY_COMMAND,  
  R3_SURFEL_LABELER_UNDO_COMMAND,
  R3_SURFEL_LABELER_REDO_COMMAND,
  R3_SURFEL_LABELER_RESET_COMMAND,
  R3_SURFEL_LABELER_MERGE_SELECTION_COMMAND,
  R3_SURFEL_LABELER_UNMERGE_SELECTION_COMMAND,
  R3_SURFEL_LABELER_SPLIT_SELECTION_COMMAND,
  R3_SURFEL_LABELER_ASSIGN_ATTRIBUTE_COMMAND,
  R3_SURFEL_LABELER_ASSIGN_OBB_COMMAND,
  R3_SURFEL_LABELER_NUM_COMMANDS
};



////////////////////////////////////////////////////////////////////////
// Command names
////////////////////////////////////////////////////////////////////////

extern const char *command_names[R3_SURFEL_LABELER_NUM_COMMANDS];



////////////////////////////////////////////////////////////////////////
// Command support class definitions
////////////////////////////////////////////////////////////////////////

struct R3SurfelAttributeAssignment {
public:
  R3SurfelAttributeAssignment(void)
    : object(NULL), attribute(0), 
      previous_value(0), new_value(0) {};
public:
  R3SurfelObject *object;
  RNFlags attribute;
  RNBoolean previous_value;
  RNBoolean new_value;
};

struct R3SurfelOBBAssignment {
public:
  R3SurfelOBBAssignment(void)
    : object(NULL),
      obb(R3null_oriented_box), previous_obb(R3null_oriented_box),
      confidence(0), previous_confidence(0),
      originator(R3_SURFEL_MACHINE_ORIGINATOR), previous_originator(R3_SURFEL_MACHINE_ORIGINATOR) {};
public:
  R3SurfelObject *object;
  R3OrientedBox obb, previous_obb;
  RNScalar confidence, previous_confidence;
  int originator, previous_originator;
};



////////////////////////////////////////////////////////////////////////
// Command class definition
////////////////////////////////////////////////////////////////////////

class R3SurfelLabelerCommand {
  // Constructor
  R3SurfelLabelerCommand(R3SurfelLabeler *labeler, int type, double operand = 0);

  // Command type
  int type;

  // Command operand
  double operand;

  // Labeler state before command was executed
  R3Camera camera;
  R3Point center_point;

  // Time since beginning of execution
  RNScalar timestamp;

  // Object selections
  RNArray<R3SurfelObject *> inserted_object_selections;
  RNArray<R3SurfelObject *> removed_object_selections;

  // Label assignments
  RNArray<R3SurfelLabelAssignment *> inserted_label_assignments;
  RNArray<R3SurfelLabelAssignment *> removed_label_assignments;

  // Parent assignments
  RNArray<R3SurfelObject *> part_parent_assignments;
  RNArray<R3SurfelObject *> inserted_parent_assignments;
  RNArray<R3SurfelObject *> removed_parent_assignments;

  // Attribute assignments
  std::vector<R3SurfelAttributeAssignment> attribute_assignments;
  
  // OBB assignments
  std::vector<R3SurfelOBBAssignment> obb_assignments;
  
  // This is only used by R3SurfelLabeler
  friend class R3SurfelLabeler;
};



}; // end namespace



#endif
