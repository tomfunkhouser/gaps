/* Source file for the surfel scene labeler command stuff */



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// Command names
////////////////////////////////////////////////////////////////////////

const char *command_names[R3_SURFEL_LABELER_NUM_COMMANDS] = {
  "INITIALIZE",  
  "TERMINATE",  
  "SYNC",  
  "ZOOM",  
  "CAPTURE_IMAGE",  
  "SELECT_NONE",  
  "SELECT_PICKED",  
  "SELECT_ENCLOSED",  
  "SELECT_OVERLAPPING",  
  "SELECT_UNLABELED",  
  "SELECT_SUGGESTED",  
  "SELECT_ALL",  
  "LABEL_PICKED",  
  "LABEL_SELECTION",  
  "RELABEL_PICKED",  
  "RELABEL_SELECTION",  
  "CONFIRM_PICKED",  
  "CONFIRM_SELECTION",  
  "CONFIRM_ALL",  
  "REFUTE_PICKED",  
  "REFUTE_SELECTION",  
  "REFUTE_ALL",  
  "SET_AERIAL_VISIBILITY",  
  "SET_TERRESTRIAL_VISIBILITY",  
  "SET_LABEL_NAME_VISIBILITY",  
  "SET_SURFEL_COLOR_SCHEME",  
  "SET_LABEL_VISIBILITY",  
  "UNDO",
  "REDO",
  "RESET",
  "MERGE_SELECTION",
  "UNMERGE_SELECTION",
  "SPLIT_SELECTION",
  "ASSIGN_ATTRIBUTE",
  "ASSIGN_OBB"
};



////////////////////////////////////////////////////////////////////////
// Command class constructor
////////////////////////////////////////////////////////////////////////

R3SurfelLabelerCommand::
R3SurfelLabelerCommand(R3SurfelLabeler *labeler, int type, double operand)
  : type(type),
    operand(operand),
    camera(labeler->Camera()),
    center_point(labeler->CenterPoint()),
    timestamp(labeler->CurrentTime()),
    inserted_object_selections(),
    removed_object_selections(),
    inserted_label_assignments(),
    removed_label_assignments(),
    part_parent_assignments(),
    inserted_parent_assignments(),
    removed_parent_assignments(),
    attribute_assignments(),
    obb_assignments()
{
}



}; // end namespace
