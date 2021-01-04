/* Source file for the surfel scene labeler command stuff */



////////////////////////////////////////////////////////////////////////
// Command logging stuff
////////////////////////////////////////////////////////////////////////

// Command types

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
  R3_SURFEL_LABELER_NUM_COMMANDS
};



// Command names

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
  "SPLIT_SELECTION"
};



// Command class definition

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

  // This is only used by R3SurfelLabeler
  friend class R3SurfelLabeler;
};



// Command class constructor

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
    removed_parent_assignments()
{
}



void R3SurfelLabeler::
BeginCommand(int type, double operand)
{
  // Check current command
  assert(!current_command);

  // Start current command
  current_command = new R3SurfelLabelerCommand(this, type, operand);
}



void R3SurfelLabeler::
EndCommand(void)
{
  // Check current command
  assert(current_command);

  // Insert command into undo stack, if it can be undone
  switch (current_command->type) {
  case R3_SURFEL_LABELER_ZOOM_COMMAND:
  case R3_SURFEL_LABELER_SELECT_NONE_COMMAND:
  case R3_SURFEL_LABELER_SELECT_PICKED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_ENCLOSED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_OVERLAPPING_COMMAND:
  case R3_SURFEL_LABELER_SELECT_UNLABELED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_SUGGESTED_COMMAND:
  case R3_SURFEL_LABELER_SELECT_ALL_COMMAND:
  case R3_SURFEL_LABELER_LABEL_PICKED_COMMAND:
  case R3_SURFEL_LABELER_LABEL_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_RELABEL_PICKED_COMMAND:
  case R3_SURFEL_LABELER_RELABEL_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_CONFIRM_PICKED_COMMAND:  
  case R3_SURFEL_LABELER_CONFIRM_SELECTION_COMMAND:  
  case R3_SURFEL_LABELER_CONFIRM_ALL_COMMAND:  
  case R3_SURFEL_LABELER_REFUTE_PICKED_COMMAND:  
  case R3_SURFEL_LABELER_REFUTE_SELECTION_COMMAND:  
  case R3_SURFEL_LABELER_REFUTE_ALL_COMMAND: 
  case R3_SURFEL_LABELER_MERGE_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_UNMERGE_SELECTION_COMMAND:
  case R3_SURFEL_LABELER_SPLIT_SELECTION_COMMAND:
    // Delete commands that were undone
    for (int i = undo_index+1; i < undo_stack.NEntries(); i++) 
      delete undo_stack.Kth(i);
    undo_stack.Truncate(undo_index+1);

    // Insert command into undo stack
    undo_stack.Insert(current_command);
    undo_index = undo_stack.NEntries() - 1;
    break;
  }

  // Write command to log
  if (logging_fp) {
    PrintCommand(current_command, logging_fp);
  }

  // Write checkpoint to log
  if (logging_fp) {
    const RNScalar checkpoint_interval = 3600;
    static RNScalar last_checkpoint_time = -FLT_MAX;
    if ((current_command->type == R3_SURFEL_LABELER_INITIALIZE_COMMAND) || 
        (current_command->type == R3_SURFEL_LABELER_TERMINATE_COMMAND) || 
        (current_command->type == R3_SURFEL_LABELER_SYNC_COMMAND) || 
        (current_command->timestamp >= last_checkpoint_time + checkpoint_interval)) {
      last_checkpoint_time = current_command->timestamp;
      //PrintCheckpoint(logging_fp);
    }
  }

  // End current command
  current_command = NULL;
}



void R3SurfelLabeler::
PrintCommand(R3SurfelLabelerCommand *command, FILE *fp) const
{
  // Just checking
  if (!command) return;
  if (!fp) fp = stdout;

  // Print 
  const R3Point& eye = command->camera.Origin();
  const R3Vector& towards = command->camera.Towards();
  const R3Vector& up = command->camera.Up();

  // Count label assignments inserted by human
  int ninserted_label_assignments = 0;
  for (int j = 0; j < command->inserted_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->inserted_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_LABEL_ASSIGNMENT_HUMAN_ORIGINATOR) continue;
    ninserted_label_assignments++;
  }

  // Count label assignments removed by human
  int nremoved_label_assignments = 0;
  for (int j = 0; j < command->removed_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->removed_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_LABEL_ASSIGNMENT_HUMAN_ORIGINATOR) continue;
    nremoved_label_assignments++;
  }

  fprintf(fp, "CMD %d %g %g    ", command->type, command->operand, command->timestamp);
  fprintf(fp, "%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f ", eye[0], eye[1], eye[2], towards[0], towards[1], towards[2], up[0], up[1], up[2]);
  fprintf(fp, "    ");

  fprintf(fp, "%d", command->inserted_object_selections.NEntries());
  for (int j = 0; j < command->inserted_object_selections.NEntries(); j++) 
    fprintf(fp, " %d", command->inserted_object_selections[j]->SceneIndex());
  fprintf(fp, "    ");

  fprintf(fp, "%d", command->removed_object_selections.NEntries());
  for (int j = 0; j < command->removed_object_selections.NEntries(); j++) 
    fprintf(fp, " %d", command->removed_object_selections[j]->SceneIndex());
  fprintf(fp, "    ");

  fprintf(fp, "%d", ninserted_label_assignments);
  for (int j = 0; j < command->inserted_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->inserted_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_LABEL_ASSIGNMENT_HUMAN_ORIGINATOR) continue;
    fprintf(fp, " ( %d %d %g %d )", assignment->Object()->SceneIndex(), assignment->Label()->SceneIndex(), assignment->Confidence(), assignment->Originator());
  }
  fprintf(fp, "    ");

  fprintf(fp, "%d", nremoved_label_assignments);
  for (int j = 0; j < command->removed_label_assignments.NEntries(); j++) {
    R3SurfelLabelAssignment *assignment = command->removed_label_assignments[j];
    if (assignment->Originator() != R3_SURFEL_LABEL_ASSIGNMENT_HUMAN_ORIGINATOR) continue;
    fprintf(fp, " ( %d %d %g %d )", assignment->Object()->SceneIndex(), assignment->Label()->SceneIndex(), assignment->Confidence(), assignment->Originator());
  }
  fprintf(fp, "    ");

  fprintf(fp, "%d", command->part_parent_assignments.NEntries());
  for (int j = 0; j < command->part_parent_assignments.NEntries(); j++) {
    R3SurfelObject *part = command->part_parent_assignments[j];
    R3SurfelObject *removed_parent = command->removed_parent_assignments[j];
    R3SurfelObject *inserted_parent = command->inserted_parent_assignments[j];
    fprintf(fp, " ( %d %d %d )", part->SceneIndex(), removed_parent->SceneIndex(), inserted_parent->SceneIndex());
  }
  fprintf(fp, "    ");

  fprintf(fp, "\n");
  fflush(fp);
}



void R3SurfelLabeler::
PrintCheckpoint(FILE *fp) const
{
  // Just checking
  if (!fp) fp = stdout;

  // Count assignments
  int ntotal = 0;
  int npredicted = 0;
  int nconfirmed = 0;
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->NParts() > 0) continue;
    if (!object->GroundTruthLabel()) continue;
    if (object->HumanLabel()) nconfirmed++;
    else if (object->PredictedLabel()) npredicted++;
    ntotal++;
  }

  // Print status
  fprintf(fp, "CHECKPOINT  %g  %d   %d %d %d    %g %g %g   %g %g %g    %g %g %g", 
    CurrentTime(),
    scene->NLabels(), ntotal, nconfirmed, npredicted,
    CurrentLabelPrecision(), CurrentLabelRecall(), CurrentLabelFMeasure(),
    HumanLabelPrecision(),  HumanLabelRecall(), HumanLabelFMeasure(),
    PredictedLabelPrecision(),  PredictedLabelRecall(), PredictedLabelFMeasure());
  fprintf(fp, "    ");

  // Print labels
  for (int i = 0; i < scene->NLabels(); i++) {
    R3SurfelLabel *label = scene->Label(i);
    fprintf(fp, " L %d %s", label->SceneIndex(), label->Name());
  }
  fprintf(fp, "    ");

  // Print object assignments
  for (int i = 0; i < scene->NObjects(); i++) {
    R3SurfelObject *object = scene->Object(i);
    if (object->NParts() > 0) continue;
    R3SurfelLabel *ground_truth_label = object->GroundTruthLabel();
    R3SurfelLabel *human_label = object->HumanLabel();
    R3SurfelLabelAssignment *predicted_assignment = object->PredictedLabelAssignment();
    R3SurfelLabel *predicted_label = object->PredictedLabel();
    fprintf(fp, " O %d %s %d %d %d %g", object->SceneIndex(), object->Name(), 
      (ground_truth_label) ? ground_truth_label->SceneIndex() : -1, 
      (human_label) ? human_label->SceneIndex() : -1, 
      (predicted_label) ? predicted_label->SceneIndex() : -1, 
      (predicted_assignment) ? predicted_assignment->Confidence() : -1);
  }
  fprintf(fp, "\n");
  fflush(fp);
}




