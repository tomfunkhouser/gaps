/* Include file for the R3 surfel labeler class */
#ifndef __R3_SURFEL_LABELER_H__
#define __R3_SURFEL_LABELER_H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class R3SurfelLabeler : public R3SurfelViewer {
public:

  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor/destructor functions
  R3SurfelLabeler(R3SurfelScene *scene, const char *logging_filename = NULL);
  virtual ~R3SurfelLabeler(void);


  //////////////////////
  //// UI CALLBACKS ////
  //////////////////////

  // Begin/end events
  virtual void Initialize(void);
  virtual void Terminate(void);

  // User input events
  virtual int Redraw(void);
  virtual int Resize(int width, int height);
  virtual int MouseMotion(int x, int y);
  virtual int MouseButton(int x, int y, int button, int state, int shift, int ctrl, int alt, int update_center_point = 1);
  virtual int Keyboard(int x, int y, int key, int shift, int ctrl, int alt);


  //////////////////
  //// COMMANDS ////
  //////////////////

  // Save scene 
  virtual int Sync(void);

  // Snapshot scene (save copy)
  virtual int Snapshot(void);

  // Camera control
  virtual void ZoomCamera(RNScalar scale = 0);

  // Object selection 
  virtual int SelectPickedObject(int x, int y,
    RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE);
  virtual int SelectObjects(const RNArray<R3SurfelObject *>& objects, int command_type,
    RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE);
  virtual int SelectEnclosedObjects(const R2Box& box,
    RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE, RNBoolean unlabeled_only = FALSE);
  virtual int SelectEnclosedObjects(const R2Polygon& polygon,
    RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE, RNBoolean unlabeled_only = FALSE);
  virtual int SelectEnclosedObjects(const R3OrientedBox& box,
    RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE, RNBoolean unlabeled_only = FALSE);
  virtual int SelectIntersectedObjects(const R2Polygon& polygon,
    RNBoolean shift = FALSE, RNBoolean ctrl = FALSE, RNBoolean alt = FALSE, RNBoolean unlabeled_only = FALSE);
  virtual int SelectOverlappedObjects(
    RNScalar min_overlap_fraction = 0.9, RNLength overlap_tolerance = 0.25, RNBoolean unlabeled_only = FALSE);
  virtual int SelectOverlappedObjects(const R3OrientedBox& box,
    RNScalar min_overlap_fraction = 0.9, RNLength overlap_tolerance = 0.25, RNBoolean unlabeled_only = FALSE);
  virtual int SelectAllObjects(RNBoolean unlabeled_only = FALSE);
  virtual int SelectSuggestedObject(RNBoolean unlabeled_only = FALSE);

  // Create label assignments 
  virtual int AssignLabelToObject(R3SurfelObject *object, R3SurfelLabel *label, RNScalar confidence, int originator, int command);
  virtual int AssignLabelToPickedObject(R3SurfelLabel *label, int x, int y);
  virtual int AssignLabelToSelectedObjects(R3SurfelLabel *label);
  virtual int AssignNewLabelToPickedObject(int x, int y);
  virtual int AssignNewLabelToSelectedObjects(void);

  // Confirm label assignments
  virtual int ConfirmLabelOnPickedObject(int x, int y);
  virtual int ConfirmLabelsOnSelectedObjects(void);
  virtual int ConfirmLabelsOnAllObjects(void);

  // Object hierarchy adjustments
  virtual int MergeSelectedObjects(void);
  virtual int UnmergeSelectedObjects(void);
  virtual int SplitSelectedObjects(void);

  // Object attribute adjustments
  virtual int AssignAttributeToPickedObject(int x, int y, RNFlags attribute, const char *attribute_name, int value);
  virtual int AssignAttributeToSelectedObjects(RNFlags attribute, const char *attribute_name, int value);
  
  // Object attribute adjustments
  virtual int AssignOBBToSelectedObject(const R3OrientedBox& obb, RNScalar confidence, int originator);
  
  // Other commands
  virtual int Undo(void);
  virtual int Redo(void);
  virtual int Reset(void);

  
  ////////////////////////
  //// PROPERTY QUERY ////
  ////////////////////////

  // Subwindow/menu/manipulator properties
  const char *Message(void) const;
  const R3OrientedBoxManipulator& OBBManipulator(void) const;
  int SelectionVisibility(void) const;
  int OBBManipulatorVisibility(void) const;
  int MessageVisibility(void) const;
  int StatusVisibility(void) const;
  int CommandMenuVisibility(void) const;
  int LabelMenuVisibility(void) const;
  int AttributeMenuVisibility(void) const;
  RNBoolean IsOBBManipulatorVisible(void) const;

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
  virtual void SetMessage(const char *fmt, ...);

  // OBB manipulator manipulation
  virtual void SetOBBManipulator(const R3OrientedBoxManipulator& obb_manipulator);
  
  // Visibility manpulation
  virtual void SetSelectionVisibility(int visibility);
  virtual void SetOBBManipulatorVisibility(int visibility);
  virtual void SetMessageVisibility(int visibility);
  virtual void SetStatusVisibility(int visibility);
  virtual void SetCommandMenuVisibility(int visibility);
  virtual void SetLabelMenuVisibility(int visibility);
  virtual void SetAttributeMenuVisibility(int visibility);

  // Snapshot manipulation
  virtual void SetSnapshotDirectory(const char *directory_name);

  
public:

  //////////////////////////////////////////
  //// LOW_LEVEL MANIPULATION FUNCTIONS ////
  //////////////////////////////////////////

  // Logging functions
  virtual void BeginCommand(int type, double operand = 0);
  virtual void EndCommand(void);
  virtual void PrintCommand(class R3SurfelLabelerCommand *command, FILE *fp = NULL) const;
  virtual void PrintCheckpoint(FILE *fp = NULL) const;

  // Object selection functions
  int NObjectSelections(void) const;
  R3SurfelObject *ObjectSelection(int k) const;
  R3Box ObjectSelectionBBox(void) const;
  virtual void InsertObjectSelection(R3SurfelObject *object);
  virtual void RemoveObjectSelection(R3SurfelObject *object);
  virtual int EmptyObjectSelections(void);

  // Label assignment functions
  virtual int InsertLabelAssignment(R3SurfelLabelAssignment *assignment);
  virtual int RemoveLabelAssignment(R3SurfelLabelAssignment *assignment);
  virtual int EmptyLabelAssignments(void);

  // Object hierarchy functions
  virtual R3SurfelLabel *CreateLabel(R3SurfelLabel *parent = NULL, const char *name = NULL);
  virtual R3SurfelObject *CreateObject(R3SurfelObject *parent = NULL, const char *name = NULL);
  virtual int SetObjectParent(R3SurfelObject *object, R3SurfelObject *parent);
  virtual int SplitObject(R3SurfelObject *object, R3SurfelObject *parent, const R3SurfelConstraint& constraint,
    RNArray<R3SurfelObject *> *resultA = NULL, RNArray<R3SurfelObject *> *resultB = NULL);

  // Attribute assignment functions
  virtual int AssignAttribute(R3SurfelObject *object, RNFlags attribute, RNBoolean value);

  // OBB assignment functions
  virtual int AssignOBB(R3SurfelObject *object, const R3OrientedBox& obb, RNScalar confidence, int originator);

  // Pointset functions
  R3SurfelPointSet *ObjectSelectionPointSet(void) const;

  // Draw functions
  virtual void DrawObjectSelections(void) const;
  virtual void DrawObjectLabels(void) const;

  // Selection utility functions
  virtual int RasterizeObjectMask(unsigned int *object_mask);

  // Undo command used for cycling through objects
  virtual int UndoCommandOfType(int command_type);
  
  // Debug functions
  virtual int IsValid(void) const;


  ///////////////////////////////
  //// MENU FUNCTIONS ////
  ///////////////////////////////

  // Command menu functions
  virtual void DrawCommandMenu(void) const;
  virtual int PickCommandMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt);

  // Attribute menu functions
  virtual R2Box AttributeMenuBBox(void) const;
  virtual void UpdateAttributeMenu(void);
  virtual void DrawAttributeMenu(void) const;
  virtual int PickAttributeMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt);

  // Label menu functions
  virtual R2Box LabelMenuBBox(void) const;
  virtual void UpdateLabelMenu(void);
  virtual void DrawLabelMenu(void) const;
  virtual int PickLabelMenu(int xcursor, int ycursor, int button, int state, RNBoolean shift, RNBoolean ctrl, RNBoolean alt);
  
  // OBB manipulator functions
  virtual void UpdateOBBManipulator(RNBoolean reset_manipulation = FALSE, RNBoolean update_oriented_box = FALSE, RNBoolean keep_orientation = FALSE);

  // Object OBB functions
  virtual void UpdateObjectOrientedBBoxes(void);

  
  /////////////////////////////////////
  //// LOW_LEVEL INPUT FUNCTIONS ////
  /////////////////////////////////////

  // Keyboard callback with extra variable for tab key state
  virtual int Keyboard(int x, int y, int key, int shift, int ctrl, int alt, int tab);

  
  /////////////////////////////////////
  //// LOW_LEVEL DRAWING FUNCTIONS ////
  /////////////////////////////////////

  // Oriented box manipulator drawing functions
  virtual void DrawOBBManipulator(void) const;
  
  // Message window drawing functions
  virtual void DrawMessage(void) const;

  // Status window drawing functions
  virtual void DrawStatus(void) const;

  // Lasso drawing functions
  virtual void DrawRubberBox(RNBoolean front_buffer = TRUE, RNBoolean xor_op = TRUE) const;
  virtual void DrawRubberPolygon(RNBoolean front_buffer = TRUE, RNBoolean xor_op = TRUE) const;
  virtual void DrawRubberLine(RNBoolean front_buffer = TRUE, RNBoolean xor_op = TRUE) const;


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

  // Oriented box manipulator
  R3OrientedBoxManipulator obb_manipulator;
  int obb_manipulator_visibility;
  
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

inline const R3OrientedBoxManipulator& R3SurfelLabeler::
OBBManipulator(void) const
{
  // Return obb manipulator
  return obb_manipulator;
}

  

inline const char *R3SurfelLabeler::
Message(void) const
{
  // Return message
  return message;
}
  

  
inline int R3SurfelLabeler::
SelectionVisibility(void) const
{
  // Return selection visibililty
  return selection_visibility;
}



inline int R3SurfelLabeler::
OBBManipulatorVisibility(void) const
{
  // Return obb manipulator visibililty
  return obb_manipulator_visibility;
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



} // end namespace



#endif
