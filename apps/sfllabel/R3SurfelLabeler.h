/* Include file for the R3 surfel labeler class */



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class R3SurfelLabeler : public R3SurfelViewer {
public:

  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  R3SurfelLabeler(R3SurfelScene *scene, const char *loging_filename = NULL);
  virtual ~R3SurfelLabeler(void);


  //////////////////////
  //// UI CALLBACKS ////
  //////////////////////

  // Begin/end events
  void Initialize(void);
  void Terminate(void);

  // User input events
  int Redraw(void);
  int Resize(int width, int height);
  int MouseMotion(int x, int y);
  int MouseButton(int x, int y, int button, int state, int shift, int ctrl, int alt, int update_center_point = 1);
  int Keyboard(int x, int y, int key, int shift, int ctrl, int alt);


  //////////////////
  //// COMMANDS ////
  //////////////////

  // Save scene 
  int Sync(void);

  // Snapshot scene (save copy)
  int Snapshot(void);

  // Camera control
  int ZoomCamera(RNScalar scale = 0);

  // Object selection 
  int SelectPickedObject(int x, int y, RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE);
  int SelectEnclosedObjects(const R2Box& box, RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE, RNBoolean unlabeled_only = FALSE);
  int SelectEnclosedObjects(const R2Polygon& polygon, RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE, RNBoolean unlabeled_only = FALSE);
  int SelectIntersectedObjects(const R2Polygon& polygon, RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE, RNBoolean unlabeled_only = FALSE);
  int SelectOverlappedObjects(RNScalar min_overlap_fraction = 0.9, RNLength overlap_tolerance = 0.25, RNBoolean unlabeled_only = FALSE);
  int SelectAllObjects(RNBoolean unlabeled_only = FALSE);
  int SelectSuggestedObject(void);

  // Create label assignments 
  int AssignLabelToObject(R3SurfelObject *object, R3SurfelLabel *label, RNScalar confidence, int originator, int command);
  int AssignLabelToPickedObject(R3SurfelLabel *label, int x, int y);
  int AssignLabelToSelectedObjects(R3SurfelLabel *label);
  int AssignNewLabelToPickedObject(int x, int y);
  int AssignNewLabelToSelectedObjects(void);

  // Confirm label assignments
  int ConfirmLabelOnPickedObject(int x, int y);
  int ConfirmLabelsOnSelectedObjects(void);
  int ConfirmLabelsOnAllObjects(void);

  // Object hierarchy adjustments
  int MergeSelectedObjects(void);
  int UnmergeSelectedObjects(void);
  int SplitSelectedObjects(void);

  // Object attribute adjustments
  int AssignAttributeToPickedObject(int x, int y, RNFlags attribute, const char *attribute_name, int value);
  int AssignAttributeToSelectedObjects(RNFlags attribute, const char *attribute_name, int value);
  
  // Other commands
  int Undo(void);
  int Redo(void);
  int Reset(void);

  
  ////////////////////////
  //// PROPERTY QUERY ////
  ////////////////////////

  // Subwindow/menu properties
  const char *Message(void) const;
  int SelectionVisibility(void) const;
  int MessageVisibility(void) const;
  int StatusVisibility(void) const;
  int CommandMenuVisibility(void) const;
  int LabelMenuVisibility(void) const;
  int AttributeMenuVisibility(void) const;

  // Snapshot properties
  const char *SnapshotDirectory(void) const;

  // Labeling statistics
  RNScalar CurrentLabelPrecision(void) const;
  RNScalar CurrentLabelRecall(void) const;
  RNScalar CurrentLabelFMeasure(void) const;

  // Labeling statistics
  RNScalar HumanLabelPrecision(void) const;
  RNScalar HumanLabelRecall(void) const;
  RNScalar HumanLabelFMeasure(void) const;

  // Labeling statistics
  RNScalar PredictedLabelPrecision(void) const;
  RNScalar PredictedLabelRecall(void) const;
  RNScalar PredictedLabelFMeasure(void) const;


  ///////////////////////////////
  //// PROPERTY MANIPULATION ////
  ///////////////////////////////

  // Subwindow/menu manipulation (visibility: 0=off, 1=on, -1=toggle)
  void SetMessage(const char *fmt, ...);
  void SetSelectionVisibility(int visibility);
  void SetMessageVisibility(int visibility);
  void SetStatusVisibility(int visibility);
  void SetCommandMenuVisibility(int visibility);
  void SetLabelMenuVisibility(int visibility);
  void SetAttributeMenuVisibility(int visibility);

  // Snapshot manipulation
  void SetSnapshotDirectory(const char *directory_name);

  
  ////////////////////////
  //// LOGGING
  ////////////////////////

  // Logging functions
  int StartLogging(const char *filename) const;
  int EndLogging(void) const;
  int IsLogging(void) const;
  int ReadLog(const char *filename);
  
public:

  //////////////////////////////////////////
  //// LOW_LEVEL MANIPULATION FUNCTIONS ////
  //////////////////////////////////////////

  // Logging functions
  void BeginCommand(int type, double operand = 0);
  void EndCommand(void);
  void PrintCommand(class R3SurfelLabelerCommand *command, FILE *fp = NULL) const;
  void PrintCheckpoint(FILE *fp = NULL) const;

  // Object selection functions
  int NObjectSelections(void) const;
  R3SurfelObject *ObjectSelection(int k) const;
  R3Box ObjectSelectionBBox(void) const;
  void InsertObjectSelection(R3SurfelObject *object);
  void RemoveObjectSelection(R3SurfelObject *object);
  int EmptyObjectSelections(void);

  // Label assignment functions
  int InsertLabelAssignment(R3SurfelLabelAssignment *assignment);
  int RemoveLabelAssignment(R3SurfelLabelAssignment *assignment);
  int EmptyLabelAssignments(void);

  // Object hierarchy functions
  R3SurfelLabel *CreateLabel(R3SurfelLabel *parent = NULL, const char *name = NULL);
  R3SurfelObject *CreateObject(R3SurfelObject *parent = NULL, const char *name = NULL);
  int SetObjectParent(R3SurfelObject *object, R3SurfelObject *parent);
  int SplitObject(R3SurfelObject *object, R3SurfelObject *parent, const R3SurfelConstraint& constraint,
    RNArray<R3SurfelObject *> *resultA = NULL, RNArray<R3SurfelObject *> *resultB = NULL);

  // Attribute assignment functions
  int AssignAttribute(R3SurfelObject *object, RNFlags attribute, RNBoolean value);
  
  // Pointset functions
  R3SurfelPointSet *ObjectSelectionPointSet(void) const;

  // Draw functions
  void DrawObjectSelections(void) const;
  void DrawObjectLabels(void) const;

  // Selection utility functions
  int RasterizeObjectMask(unsigned int *object_mask);
  
  // Debug functions
  int IsValid(void) const;


  ///////////////////////////////
  //// MENU FUNCTIONS ////
  ///////////////////////////////

  // Command menu functions
  void DrawCommandMenu(void) const;
  int PickCommandMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt);

  // Attribute menu functions
  R2Box AttributeMenuBBox(void) const;
  void UpdateAttributeMenu(void);
  void DrawAttributeMenu(void) const;
  int PickAttributeMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt);

  // Label menu functions
  R2Box LabelMenuBBox(void) const;
  void UpdateLabelMenu(void);
  void DrawLabelMenu(void) const;
  int PickLabelMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt);
  
  
  /////////////////////////////////////
  //// LOW_LEVEL INPUT FUNCTIONS ////
  /////////////////////////////////////

  // Keyboard callback with extra variable for tab key state
  int Keyboard(int x, int y, int key, int shift, int ctrl, int alt, int tab);

  
  /////////////////////////////////////
  //// LOW_LEVEL DRAWING FUNCTIONS ////
  /////////////////////////////////////

  // Message window functions
  void DrawMessage(void) const;

  // Status window functions
  void DrawStatus(void) const;

  // Lasso drawing functions
  void DrawRubberBox(RNBoolean front_buffer = TRUE, RNBoolean xor_op = TRUE) const;
  void DrawRubberPolygon(RNBoolean front_buffer = TRUE, RNBoolean xor_op = TRUE) const;
  void DrawRubberLine(RNBoolean front_buffer = TRUE, RNBoolean xor_op = TRUE) const;


  /////////////////////////////////////
  //// LOW_LEVEL WORKING SET MANAGEMENT
  /////////////////////////////////////

  // Working set updates
  virtual void UpdateWorkingSet(void);
  virtual void UpdateWorkingSet(const R3Viewer& view);
  virtual void UpdateWorkingSet(const R3Point& center, RNScalar target_resolution, RNScalar focus_radius);
  virtual void ObjectifyWorkingSet(void);


protected:

  ////////////////////////////
  //// INTERNAL VARIABLES ////
  ////////////////////////////

  // Label visibility
  int object_label_visibility;

  // Classifier
  R3SurfelClassifier classifier;
  int classify_after_change_label;

  // Segmenter
  R3SurfelSegmenter segmenter;
  int segment_after_change_label;

  // Object selections
  RNArray<R3SurfelObject *> selection_objects;
  int selection_visibility;

  // Command logging
  class R3SurfelLabelerCommand *current_command;
  RNArray<class R3SurfelLabelerCommand *> undo_stack;
  int undo_index;
  char *logging_filename;
  FILE *logging_fp;

  // Rubber box
  RNBoolean select_box_active;
  R2Point rubber_box_corners[2];

  // Rubber polygon
  RNBoolean click_polygon_active;
  RNBoolean split_polygon_active;
  RNBoolean select_polygon_active;
  int num_rubber_polygon_points;
  static const int max_rubber_polygon_points = 4096;
  R2Point rubber_polygon_points[max_rubber_polygon_points];

  // Rubber line
  RNBoolean split_line_active;
  R2Point rubber_line_points[2];

  // Message
  char *message;
  int message_visibility;

  // Status
  int status_visibility;

  // Command menu
  int command_menu_visibility;

  // Attribute menu
  int attribute_menu_visibility;
  std::vector<RNFlags> attribute_menu_flags;
  std::vector<const char *> attribute_menu_names;
  std::vector<unsigned char> attribute_menu_keystrokes;
  int attribute_menu_item_width;
  int attribute_menu_item_height;
  void *attribute_menu_font;

  // Label menu
  int label_menu_visibility;
  RNArray<R3SurfelLabel *> label_menu_list;
  int label_menu_item_width;
  int label_menu_item_height;
  void *label_menu_font;

  // Snapshots
  char *snapshot_directory;
  
  // Object stuff
  std::vector<RNScalar> object_selection_times;
};



////////////////////////////////////////////////////////////////////////
// ATTRIBUTE CONSTANTS
////////////////////////////////////////////////////////////////////////

#define R3_SURFEL_MOVING_ATTRIBUTE  R3_SURFEL_OBJECT_USER_FLAG_0
#define R3_SURFEL_GROUP_ATTRIBUTE   R3_SURFEL_OBJECT_USER_FLAG_1
#define R3_SURFEL_ALL_ATTRIBUTES    (R3_SURFEL_MOVING_ATTRIBUTE | R3_SURFEL_GROUP_ATTRIBUTE)



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTIONS
////////////////////////////////////////////////////////////////////////

inline int R3SurfelLabeler::
SelectionVisibility(void) const
{
  // Return selection visibililty
  return selection_visibility;
}



inline int R3SurfelLabeler::
MessageVisibility(void) const
{
  // Return message visibililty
  return message_visibility;
}



inline int R3SurfelLabeler::
StatusVisibility(void) const
{
  // Return status visibililty
  return status_visibility;
}



inline int R3SurfelLabeler::
CommandMenuVisibility(void) const
{
  // Return command menu visibililty
  return command_menu_visibility;
}



inline int R3SurfelLabeler::
LabelMenuVisibility(void) const
{
  // Return label menu visibililty
  return label_menu_visibility;
}



inline int R3SurfelLabeler::
AttributeMenuVisibility(void) const
{
  // Return attribute menu visibililty
  return attribute_menu_visibility;
}



inline const char *R3SurfelLabeler::
SnapshotDirectory(void) const
{
  // Return snapshot directory
  return snapshot_directory;
}



inline void R3SurfelLabeler::
SetMessage(const char *fmt, ...)
{
  // Free previous message
  if (message) free(message);
  message = NULL;

  // Set message (displayed at bottom of screen)
  if (fmt) {
    message = new char [4096];
    va_list args;
    va_start(args, fmt);
    vsprintf(message, fmt, args);
    va_end(args);
  }
}



inline void R3SurfelLabeler::
SetSelectionVisibility(int visibility)
{
  // Set selection visibililty
  if (visibility == -1) selection_visibility = 1 - selection_visibility;
  else if (visibility == 0) selection_visibility = 0;
  else selection_visibility = 1;
}



inline void R3SurfelLabeler::
SetMessageVisibility(int visibility)
{
  // Set message visibililty
  if (visibility == -1) message_visibility = 1 - message_visibility;
  else if (visibility == 0) message_visibility = 0;
  else message_visibility = 1;
}



inline void R3SurfelLabeler::
SetStatusVisibility(int visibility)
{
  // Set status visibililty
  if (visibility == -1) status_visibility = 1 - status_visibility;
  else if (visibility == 0) status_visibility = 0;
  else status_visibility = 1;
}



inline void R3SurfelLabeler::
SetCommandMenuVisibility(int visibility)
{
  // Set command menu visibililty
  if (visibility == -1) command_menu_visibility = 1 - command_menu_visibility;
  else if (visibility == 0) command_menu_visibility = 0;
  else command_menu_visibility = 1;
}



inline void R3SurfelLabeler::
SetLabelMenuVisibility(int visibility)
{
  // Set label menu visibililty
  if (visibility == -1) label_menu_visibility = 1 - label_menu_visibility;
  else if (visibility == 0) label_menu_visibility = 0;
  else label_menu_visibility = 1;
}



inline void R3SurfelLabeler::
SetAttributeMenuVisibility(int visibility)
{
  // Set attribute menu visibililty
  if (visibility == -1) attribute_menu_visibility = 1 - attribute_menu_visibility;
  else if (visibility == 0) attribute_menu_visibility = 0;
  else attribute_menu_visibility = 1;
}



inline void R3SurfelLabeler::
SetSnapshotDirectory(const char *directory_name)
{
  // Delete previous snapshot directory
  if (this->snapshot_directory) {
    free(this->snapshot_directory);
    this->snapshot_directory = NULL;
  }
  
  // Set new snapshot directory
  if (directory_name) {
    this->snapshot_directory = strdup(directory_name);
  }
}



inline int R3SurfelLabeler::
NObjectSelections(void) const
{
  // Return number of selected objects
  return selection_objects.NEntries();
}



inline R3SurfelObject *R3SurfelLabeler::
ObjectSelection(int k) const
{
  // Return kth selected objects
  return selection_objects.Kth(k);
}



