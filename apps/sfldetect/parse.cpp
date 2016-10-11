///////////////////////////////////////////////////////////////////////
// Source file for object parse class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Constructor/destructor functions
////////////////////////////////////////////////////////////////////////

ObjectParse::
ObjectParse(const char *model_directory, const char *segmentation_directory)
  : labels(),
    models(),
    segmentations(),
    detections(),
    assignments(),
    model_directory((model_directory) ? strdup(model_directory) : NULL),
    segmentation_directory((segmentation_directory) ? strdup(segmentation_directory) : NULL)
{ 
}  

  

ObjectParse::
~ObjectParse(void) 
{ 
  // Delete assignments
  while (NAssignments() > 0) {
    ObjectAssignment *assignment = Assignment(NAssignments()-1);
    delete assignment;
  }

  // Delete models
  while (NModels() > 0) {
    ObjectModel *model = Model(NModels()-1);
    delete model;
  }

  // Delete segmentations
  while (NSegmentations() > 0) {
    ObjectSegmentation *segmentation = Segmentation(NSegmentations()-1);
    delete segmentation;
  }

  // Delete labels
  while (NLabels() > 0) {
    ObjectLabel *label = Label(NLabels()-1);
    delete label;
  }

  // Delete detections
  while (NDetections() > 0) {
    ObjectDetection *detection = Detection(NDetections()-1);
    delete detection;
  }

  // Delete directory names
  if (model_directory) free(model_directory);
  if (segmentation_directory) free(segmentation_directory);
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

ObjectLabel *ObjectParse::
Label(const char *name) const
{
  // Find label by name
  for (int i = 0; i < NLabels(); i++) {
    ObjectLabel *label = Label(i);
    if (!label->Name()) continue;
    if (!strcmp(label->Name(), name)) return label;
  }
  return NULL;
}



ObjectDetection *ObjectParse::
Detection(const char *name) const
{
  // Find detection by name
  for (int i = 0; i < NDetections(); i++) {
    ObjectDetection *detection = Detection(i);
    if (!detection->Name()) continue;
    if (!strcmp(detection->Name(), name)) return detection;
  }
  return NULL;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void ObjectParse::
Empty(void)
{
  // Empty everything
  EmptyAssignments();
  EmptyModels();
  EmptySegmentations();
  EmptyLabels();
  EmptyDetections();
}



void ObjectParse::
EmptyLabels(void)
{
  // Remove all labels
  // (does not delete labels)
  while (NLabels() > 0) {
    RemoveLabel(Label(NLabels()-1));
  }
}



void ObjectParse::
InsertLabel(ObjectLabel *label)
{
  // Just checking
  assert(label->parse_index == -1);
  assert(label->parse == NULL);

  // Insert label
  label->parse = this;
  label->parse_index = labels.NEntries();
  labels.Insert(label);

  // Insert models
  for (int i = 0; i < label->NModels(); i++) {
    ObjectModel *model = label->Model(i);
    InsertModel(model);
  }
}



void ObjectParse::
RemoveLabel(ObjectLabel *label)
{
  // Just checking
  assert(label->parse_index >= 0);
  assert(label->parse_index < labels.NEntries());
  assert(label->parse == this);

  // Remove models 
  for (int i = 0; i < label->NModels(); i++) {
    ObjectModel *model = label->Model(i);
    RemoveModel(model);
  }

  // Remove label
  RNArrayEntry *entry = labels.KthEntry(label->parse_index);
  ObjectLabel *tail = labels.Tail();
  tail->parse_index = label->parse_index;
  labels.EntryContents(entry) = tail;
  labels.RemoveTail();
  label->parse_index = -1;
  label->parse = NULL;
}



void ObjectParse::
EmptyModels(void)
{
  // Remove all models
  // (does not delete models)
  while (NModels() > 0) {
    RemoveModel(Model(NModels()-1));
  }
}



void ObjectParse::
InsertModel(ObjectModel *model)
{
  // Just checking
  assert(model->parse_index == -1);
  assert(model->parse == NULL);

  // Insert model
  model->parse = this;
  model->parse_index = models.NEntries();
  models.Insert(model);

  // Insert assignments
  for (int i = 0; i < model->NAssignments(); i++) {
    ObjectAssignment *assignment = model->Assignment(i);
    if (assignment->Parse() != NULL) continue;
    InsertAssignment(assignment);
  }
}



void ObjectParse::
RemoveModel(ObjectModel *model)
{
  // Just checking
  assert(model->parse_index >= 0);
  assert(model->parse_index < models.NEntries());
  assert(model->parse == this);

  // Remove assignments 
  for (int i = 0; i < model->NAssignments(); i++) {
    ObjectAssignment *assignment = model->Assignment(i);
    RemoveAssignment(assignment);
  }

  // Remove model
  RNArrayEntry *entry = models.KthEntry(model->parse_index);
  ObjectModel *tail = models.Tail();
  tail->parse_index = model->parse_index;
  models.EntryContents(entry) = tail;
  models.RemoveTail();
  model->parse_index = -1;
  model->parse = NULL;
}



void ObjectParse::
EmptySegmentations(void)
{
  // Remove all segmentations
  // (does not delete segmentations)
  while (NSegmentations() > 0) {
    RemoveSegmentation(Segmentation(NSegmentations()-1));
  }
}



void ObjectParse::
InsertSegmentation(ObjectSegmentation *segmentation)
{
  // Just checking
  assert(segmentation->parse_index == -1);
  assert(segmentation->parse == NULL);

  // Insert segmentation
  segmentation->parse = this;
  segmentation->parse_index = segmentations.NEntries();
  segmentations.Insert(segmentation);

  // Insert assignments
  for (int i = 0; i < segmentation->NAssignments(); i++) {
    ObjectAssignment *assignment = segmentation->Assignment(i);
    if (assignment->Parse() != NULL) continue;
    InsertAssignment(assignment);
  }
}



void ObjectParse::
RemoveSegmentation(ObjectSegmentation *segmentation)
{
  // Just checking
  assert(segmentation->parse_index >= 0);
  assert(segmentation->parse_index < segmentations.NEntries());
  assert(segmentation->parse == this);

  // Remove assignments 
  for (int i = 0; i < segmentation->NAssignments(); i++) {
    ObjectAssignment *assignment = segmentation->Assignment(i);
    RemoveAssignment(assignment);
  }

  // Remove segmentation
  RNArrayEntry *entry = segmentations.KthEntry(segmentation->parse_index);
  ObjectSegmentation *tail = segmentations.Tail();
  tail->parse_index = segmentation->parse_index;
  segmentations.EntryContents(entry) = tail;
  segmentations.RemoveTail();
  segmentation->parse_index = -1;
  segmentation->parse = NULL;
}



void ObjectParse::
EmptyDetections(void)
{
  // Remove all detections
  // (does not delete detections)
  while (NDetections() > 0) {
    RemoveDetection(Detection(NDetections()-1));
  }
}



void ObjectParse::
InsertDetection(ObjectDetection *detection)
{
  // Just checking
  assert(detection->parse_index == -1);
  assert(detection->parse == NULL);

  // Insert detection
  detection->parse = this;
  detection->parse_index = detections.NEntries();
  detections.Insert(detection);

  // Insert segmentations
  for (int i = 0; i < detection->NSegmentations(); i++) {
    ObjectSegmentation *segmentation = detection->Segmentation(i);
    InsertSegmentation(segmentation);
  }
}



void ObjectParse::
RemoveDetection(ObjectDetection *detection)
{
  // Just checking
  assert(detection->parse_index >= 0);
  assert(detection->parse_index < detections.NEntries());
  assert(detection->parse == this);

  // Remove segmentations
  for (int i = 0; i < detection->NSegmentations(); i++) {
    ObjectSegmentation *segmentation = detection->Segmentation(i);
    RemoveSegmentation(segmentation);
  }

  // Remove detection
  RNArrayEntry *entry = detections.KthEntry(detection->parse_index);
  ObjectDetection *tail = detections.Tail();
  tail->parse_index = detection->parse_index;
  detections.EntryContents(entry) = tail;
  detections.RemoveTail();
  detection->parse_index = -1;
  detection->parse = NULL;
}



void ObjectParse::
EmptyAssignments(void)
{
  // Remove all assignments
  // (does not delete assignments)
  while (NAssignments() > 0) {
    RemoveAssignment(Assignment(NAssignments()-1));
  }
}



void ObjectParse::
InsertAssignment(ObjectAssignment *assignment)
{
  // Just checking
  assert(assignment->parse_index == -1);
  assert(assignment->parse == NULL);

  // Insert assignment into parse
  assignment->parse = this;
  assignment->parse_index = assignments.NEntries();
  assignments.Insert(assignment);
}



void ObjectParse::
RemoveAssignment(ObjectAssignment *assignment)
{
  // Just checking
  assert(assignment->parse_index >= 0);
  assert(assignment->parse_index < assignments.NEntries());
  assert(assignment->parse == this);

  // Remove assignment from parse
  RNArrayEntry *entry = assignments.KthEntry(assignment->parse_index);
  ObjectAssignment *tail = assignments.Tail();
  tail->parse_index = assignment->parse_index;
  assignments.EntryContents(entry) = tail;
  assignments.RemoveTail();
  assignment->parse_index = -1;
  assignment->parse = NULL;
}



void ObjectParse::
SetModelDirectory(const char *dirname)
{
  // Set name of directory containing model files
  if (model_directory) free(model_directory);
  if (dirname) model_directory = strdup(dirname);
  else model_directory = strdup("models");
}



void ObjectParse::
SetSegmentationDirectory(const char *dirname) 
{
  // Set name of directory containing segmentation files
  if (segmentation_directory) free(segmentation_directory);
  if (dirname) segmentation_directory = strdup(dirname);
  else segmentation_directory = strdup("segmentations");
}



void ObjectParse::
Sort(void)
{
  // Sort segmentation assignments
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    segmentation->assignments.Sort(CompareObjectAssignments);
    for (int j = 0; j < segmentation->NAssignments(); j++) {
      ObjectAssignment *assignment = segmentation->Assignment(j);
      assignment->segmentation_index = j;
    }
  }

  // Sort model assignments
  for (int i = 0; i < NModels(); i++) {
    ObjectModel *model = Model(i);
    model->assignments.Sort(CompareObjectAssignments);
    for (int j = 0; j < model->NAssignments(); j++) {
      ObjectAssignment *assignment = model->Assignment(j);
      assignment->model_index = j;
    }
  }

  // Sort detection segmentations
  for (int i = 0; i < NDetections(); i++) {
    ObjectDetection *detection = Detection(i);
    detection->segmentations.Sort(CompareObjectSegmentations);
    for (int j = 0; j < detection->NSegmentations(); j++) {
      ObjectSegmentation *segmentation = detection->Segmentation(j);
      segmentation->detection_index = j;
    }
  }

  // Sort label models
  for (int i = 0; i < NLabels(); i++) {
    ObjectLabel *label = Label(i);
    label->models.Sort(CompareObjectModels);
    for (int j = 0; j < label->NModels(); j++) {
      ObjectModel *model = label->Model(j);
      model->label_index = j;
    }
  }

  // Sort all assignments
  assignments.Sort(CompareObjectAssignments);
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    assignment->parse_index = i;
  }

  // Sort all models
  models.Sort(CompareObjectModels);
  for (int i = 0; i < NModels(); i++) {
    ObjectModel *model = Model(i);
    model->parse_index = i;
  }

  // Sort all segmentations
  segmentations.Sort(CompareObjectSegmentations);
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    segmentation->parse_index = i;
  }

  // Sort all labels
  labels.Sort(CompareObjectLabels);
  for (int i = 0; i < NLabels(); i++) {
    ObjectLabel *label = Label(i);
    label->parse_index = i;
  }

  // Sort all detections
  detections.Sort(CompareObjectDetections);
  for (int i = 0; i < NDetections(); i++) {
    ObjectDetection *detection = Detection(i);
    detection->parse_index = i;
  }
}



////////////////////////////////////////////////////////////////////////
// Input/output functions
////////////////////////////////////////////////////////////////////////

int ObjectParse::
ReadFile(const char *filename)
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .txt)\n", filename);
    return 0;
  }

  // Read file of appropriate type
  if (!strncmp(extension, ".ssc", 4)) {
    if (!ReadAsciiFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".markers", 8)) {
    if (!ReadMarkersFile(filename)) return 0;
  }
  else {
    fprintf(stderr, "Unable to read file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Return success
  return 1;
}



int ObjectParse::
WriteFile(const char *filename) const
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .txt)\n", filename);
    return 0;
  }

  // Write file of appropriate type
  if (!strncmp(extension, ".ssc", 4)) {
    if (!WriteAsciiFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".arff", 5)) {
    if (!WriteArffFile(filename)) return 0;
  }
  else {
    fprintf(stderr, "Unable to write file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Ascii I/O functions
////////////////////////////////////////////////////////////////////////

int ObjectParse::
ReadAsciiFile(const char *filename)
{
  // Convenient variables
  char buffer[4096];
  int line_number = 0;
  RNArray<ObjectDetection *> read_detections;
  RNArray<ObjectSegmentation *> read_segmentations;
  RNArray<ObjectModel *> read_models;
  RNArray<ObjectLabel *> read_labels;

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open object parse file %s\n", filename);
    return 0;
  }

  // Read header
  if (!fgets(buffer, 4096, fp)) {
    fprintf(stderr, "Unable to read object parse file %s\n", filename);
    return 0;
  }

  // Check header
  if (strncmp(buffer, "OBJECT PARSE 1.0", 16)) {
    fprintf(stderr, "Error in header of oject parse file %s\n", filename);
    return 0;
  }

  // Read file
  while (fgets(buffer, 4096, fp)) {
    // Check line
    line_number++;
    char *bufferp = buffer;
    while (*bufferp && isspace(*bufferp)) bufferp++;
    if (!bufferp) continue;
    if (*bufferp == '#') continue;

    // Parse line
    char keyword[4096], name[4096];
    double cx, cy, cz, r, h, score, m[16];
    int label_index, detection_index, model_index, segmentation_index, ground_truth, dummy;
    if (sscanf(bufferp, "%s", keyword) == (unsigned int) 1) {
      if (!strcmp(keyword, "A")) {
        // Parse assignment
        if (sscanf(bufferp, "%s%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%d%d%d%d%d", keyword, 
          &segmentation_index, &model_index, 
          &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], 
          &m[8], &m[9], &m[10], &m[11], &m[12], &m[13], &m[14], &m[15], 
          &score, &ground_truth, &dummy, &dummy, &dummy, &dummy) != (unsigned int) 25) {
          fprintf(stderr, "Error parsing assignment at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Create assignment
        R3Affine transformation(R4Matrix(m), 0);
        ObjectSegmentation *segmentation = read_segmentations.Kth(segmentation_index);
        ObjectModel *model = models.Kth(model_index);
        ObjectAssignment *assignment = new ObjectAssignment(segmentation, model, transformation, score, ground_truth);
        segmentation->InsertAssignment(assignment);
        model->InsertAssignment(assignment);
      }
      else if (!strcmp(keyword, "S")) {
        // Parse segmentation
        if (sscanf(bufferp, "%s%d%lf%lf%lf%lf%lf%d%s%d%d%d%d%d", keyword, 
          &detection_index, &cx, &cy, &cz, &r, &h, &dummy, name, &dummy, &dummy, &dummy, &dummy, &dummy) != (unsigned int) 14) {
          fprintf(stderr, "Error parsing segmentation at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Create segmentation
        R3Point origin(cx, cy, cz);
        ObjectDetection *detection = read_detections.Kth(detection_index);
        const char *pointset_filename = (strcmp(name, "None")) ? name : NULL;
        ObjectSegmentation *segmentation = new ObjectSegmentation(pointset_filename, origin, r, h);
        detection->InsertSegmentation(segmentation);
        read_segmentations.Insert(segmentation);
      }
      else if (!strcmp(keyword, "M")) {
        // Parse model
        if (sscanf(bufferp, "%s%d%lf%lf%lf%lf%lf%d%s%d%d%d%d%d", keyword, 
          &label_index, &cx, &cy, &cz, &r, &h, &dummy, name, &dummy, &dummy, &dummy, &dummy, &dummy) != (unsigned int) 14) {
          fprintf(stderr, "Error parsing model at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Create model
        R3Point origin(cx, cy, cz);
        ObjectLabel *label = read_labels.Kth(label_index);
        const char *pointset_filename = (strcmp(name, "None")) ? name : NULL;
        ObjectModel *model = new ObjectModel(pointset_filename, origin, r, h);
        label->InsertModel(model);
        read_models.Insert(model);
      }
      else if (!strcmp(keyword, "L")) {
        // Parse label
        if (sscanf(bufferp, "%s%s%lf%lf%lf%lf%lf%d%d%d%d%d", keyword, 
          name, &cx, &cy, &cz, &r, &h, &dummy, &dummy, &dummy, &dummy, &dummy) != (unsigned int) 12) {
          fprintf(stderr, "Error parsing label at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Create label
        R3Point origin(cx, cy, cz);
        ObjectLabel *label = new ObjectLabel(name, origin, r, h);
        InsertLabel(label);
        read_labels.Insert(label);
      }
      else if (!strcmp(keyword, "D")) {
        // Parse detection
        if (sscanf(bufferp, "%s%s%lf%lf%lf%lf%lf%d%d%d%d%d", keyword, 
          name, &cx, &cy, &cz, &r, &h, &dummy, &dummy, &dummy, &dummy, &dummy) != (unsigned int) 12) {
          fprintf(stderr, "Error parsing detection at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Create detection
        R3Point origin(cx, cy, cz);
        ObjectDetection *detection = new ObjectDetection(name, origin, r, h);
        InsertDetection(detection);
        read_detections.Insert(detection);
      }
      else if (!strcmp(keyword, "MD")) {
        // Parse model directory
        if (sscanf(bufferp, "%s%s", keyword, name) != (unsigned int) 2) {
          fprintf(stderr, "Error parsing model directory at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Set model directory
        if (strcmp(name, "None")) SetModelDirectory(name);
      }
      else if (!strcmp(keyword, "SD")) {
        // Parse segmentation directory
        if (sscanf(bufferp, "%s%s", keyword, name) != (unsigned int) 2) {
          fprintf(stderr, "Error parsing segmentation directory at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Set segmentation directory
        if (strcmp(name, "None")) SetSegmentationDirectory(name);
      }
      else {
        fprintf(stderr, "Unrecognized keyword %s at line %d of %s\n", keyword, line_number, filename);
        return 0;
      }
    }
  }

  // Close file 
  fclose(fp);

  // Return success
  return 1;
}



int ObjectParse::
WriteAsciiFile(const char *filename) const
{
  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open %s\n", filename);
    return 0;
  }

  // Create model directory
  if (model_directory) {
    char cmd[4096];
    sprintf(cmd, "mkdir -p %s", model_directory);
    system(cmd);
  }

  // Create segmentation directory
  if (segmentation_directory) {
    char cmd[4096];
    sprintf(cmd, "mkdir -p %s", segmentation_directory);
    system(cmd);
  }

  // Write header
  fprintf(fp, "OBJECT PARSE 1.0\n");

  // Write directories
  if (ModelDirectory()) fprintf(fp, "MD %s\n", ModelDirectory());
  if (SegmentationDirectory()) fprintf(fp, "SD %s\n", SegmentationDirectory());

  // Write labels
  for (int i = 0; i < NLabels(); i++) {
    ObjectLabel *label = Label(i);
    const R3Point& origin = label->Origin();
    const char *name = (label->Name()) ? label->Name() : "NoName";
    fprintf(fp, "L %s  %g %g %g  %g %g  %d  0 0 0 0\n", name, 
      origin.X(), origin.Y(), origin.Z(),
      label->MaxRadius(), label->MaxHeight(), 
      label->NModels());  
  }

  // Write models
  for (int i = 0; i < NModels(); i++) {
    ObjectModel *model = Model(i);
    const R3Point& origin = model->Origin();
    ObjectLabel *label = model->Label();
    int label_index = label->ParseIndex();
    fprintf(fp, "M  %d  %g %g %g  %g %g   %d %s  %d  0 0 0 0\n", 
      label_index, 
      origin.X(), origin.Y(), origin.Z(), 
      model->Radius(), model->Height(), 
      model->PointSet().NPoints(), 
      (model->Filename()) ? model->Filename() : "None",
      model->NAssignments());  
  }

  // Write detections
  for (int i = 0; i < NDetections(); i++) {
    ObjectDetection *detection = Detection(i);
    const R3Point& origin = detection->Origin();
    const char *name = (detection->Name()) ? detection->Name() : "NoName";
    fprintf(fp, "D  %s  %g %g %g  %g %g  %d  0 0 0 0\n", name, 
      origin.X(), origin.Y(), origin.Z(),
      detection->MaxRadius(), detection->MaxHeight(), 
      detection->NSegmentations());
  }

  // Write segmentations
  for (int i = 0; i < NSegmentations(); i++) {
    ObjectSegmentation *segmentation = Segmentation(i);
    const R3Point& origin = segmentation->Origin();
    ObjectDetection *detection = segmentation->Detection();
    int detection_index = detection->ParseIndex();
    fprintf(fp, "S  %d  %g %g %g  %g %g  %d %s  %d  0 0 0 0\n", 
      detection_index,
      origin.X(), origin.Y(), origin.Z(), 
      segmentation->Radius(), segmentation->Height(),
      segmentation->PointSet().NPoints(), 
      (segmentation->Filename()) ? segmentation->Filename() : "None",
      segmentation->NAssignments());  
  }

  // Write assignments
  for (int i = 0; i < NAssignments(); i++) {
    ObjectAssignment *assignment = Assignment(i);
    ObjectSegmentation *segmentation = assignment->Segmentation();
    ObjectModel *model = assignment->Model();
    int segmentation_index = segmentation->ParseIndex();
    int model_index = model->ParseIndex();
    const R3Affine& transformation = assignment->ModelToSegmentationTransformation();
    const R4Matrix& m = transformation.Matrix();
    RNScalar score = assignment->Score();
    int ground_truth = (assignment->IsGroundTruth()) ? 1 : 0;
    fprintf(fp, "A  %d %d  %g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g  %g  %d  0 0 0 0\n", 
      segmentation_index, model_index,
      m[0][0], m[0][1], m[0][2], m[0][3], 
      m[1][0], m[1][1], m[1][2], m[1][3], 
      m[2][0], m[2][1], m[2][2], m[2][3], 
      m[3][0], m[3][1], m[3][2], m[3][3], 
      score, ground_truth);
  }

  // Close file
  fclose(fp);
  
  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Markers I/O functions
////////////////////////////////////////////////////////////////////////

static const char *
GetLabelNameFromMarkerIdentifier(int label_identifier)
{
  switch (label_identifier) {
  case 0: return "Unknown";
  case 1: return "AdvertKiosk";
  case 7: return "Bush";
  case 8: return "Car";
  case 12: return "Dumpster";
  case 14: return "FireHydrant";
  case 17: return "TrafficSign";
  case 22: return "SidewalkLight";
  case 23: return "StreetLight";
  case 24: return "MailBox";
  case 25: return "HighwaySign";
  case 26: return "NewspaperBox";
  case 29: return "ParkingMeter";
  case 31: return "ShortFencePost";
  case 32: return "RecycleBin";
  case 41: return "TrafficLight";
  case 42: return "TrafficSign";
  case 43: return "GarbageCan";
  case 44: return "Tree";
  case 47: return "Car";
  case 75: return "HighwaySign";
  case 81: return "Planter";
  case 82: return "MailBox";
  case 97: return "PhoneBooth";
  case 118: return "TrafficSign";
  case 120: return "StreetLight";
  case 121: return "Tree";
  case 124: return "NewspaperBox";
  case 125: return "FireHydrant";
  case 138: return "SidewalkLight";
  case 139: return "SidewalkLight";
  case 140: return "SidewalkLight";
  case 141: return "SidewalkLight";
  case 142: return "SidewalkLight";
  case 143: return "SidewalkLight";
  case 144: return "SidewalkLight";
  case 145: return "StreetLight";
  case 146: return "TrafficLight";  
  case 150: return "TrafficSign";
  case 151: return "TrafficSign";
  case 152: return "TrafficSign";
  case 153: return "TrafficSign";
  case 154: return "TrafficSign";
  case 157: return "AFrameSign";
  case 161: return "TrafficLight";
  case 162: return "TrafficLight";
  case 163: return "TrafficLight";
  case 164: return "TrafficLight";
  case 165: return "TrafficLight"; 
  case 171: return "ShortFencePost";
  case 172: return "ShortFencePost";
  case 173: return "ShortFencePost";
  case 174: return "TallFencePost";
  case 175: return "TallFencePost";
  case 176: return "ShortFencePost";
  case 177: return "TallFencePost";
  case 181: return "StreetLight";
  case 182: return "StreetLight";
  case 183: return "StreetLight";
  case 184: return "StreetLight";
  case 185: return "StreetLight";
  case 186: return "StreetLight";
  case 187: return "StreetLight";
  case 188: return "StreetLight";
  case 191: return "Car";
  case 192: return "Car";
  case 193: return "Car";
  case 194: return "Car";
  }

  // Not found
  return NULL;
}



int ObjectParse::
ReadMarkersFile(const char *filename)
{
  // Convenient variables
  char buffer[4096];
  int line_number = 0;

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open object parse file %s\n", filename);
    return 0;
  }

  // Read file
  while (fgets(buffer, 4096, fp)) {
    // Check line
    line_number++;
    char *bufferp = buffer;
    while (*bufferp && isspace(*bufferp)) bufferp++;
    if (!bufferp) continue;
    if (*bufferp == '#') continue;

    // Parse line
    char name[4096];
    int label_identifier;
    double cx, cy, cz, r, h;
    if (sscanf(bufferp, "%d%lf%lf%lf%lf%lf", &label_identifier, &cx, &cy, &cz, &r, &h) != (unsigned int) 6) {
      fprintf(stderr, "Unable to read marker on line %d of %s\n", line_number, filename);
      return 0;
    }

    // Create detection
    R3Point origin(cx, cy, cz);
    sprintf(name, "D%d", NDetections());
    ObjectDetection *detection = new ObjectDetection(name, origin, r, h);
    InsertDetection(detection);

    // Create segmentation
    char segmentation_filename[4096];
    sprintf(segmentation_filename, "blobs/%d_%.3f_%.3f_%.3f.xyz", label_identifier, cx, cy, cz);
    ObjectSegmentation *segmentation = new ObjectSegmentation(segmentation_filename, origin, r, h, 1.0);
    detection->InsertSegmentation(segmentation);

    // Create model and label, if necessary
    const char *label_name = GetLabelNameFromMarkerIdentifier(label_identifier);
    if (label_name) {
      // Get/create label
      ObjectLabel *label = Label(label_name);
      if (!label) {
        // Create label
        label = new ObjectLabel(label_name, origin, r, h);
        InsertLabel(label);
      }
      
      // Get/create model
      ObjectModel *model = (label->NModels() > 0) ? label->Model(0) : NULL;
      if (!model) {
        model = new ObjectModel(segmentation_filename, origin, r, h);
        label->InsertModel(model);
      }
    
      // Compute transformation
      R3Affine model_to_segmentation = R3identity_affine;
      model_to_segmentation.Translate(segmentation->Origin().Vector());
      model_to_segmentation.Translate(-(model->Origin().Vector()));
      
      // Create ground truth assignment
      ObjectAssignment *assignment = new ObjectAssignment(segmentation, model, model_to_segmentation, 1.0, TRUE);
      segmentation->InsertAssignment(assignment);
      model->InsertAssignment(assignment);
    }
  }

  // Close file 
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Feature file I/O functions
////////////////////////////////////////////////////////////////////////

int ObjectParse::
ReadGSVFeaturesFile(const char *filename)
{
  // Parameters
  RNLength max_radius = 2.0;
  RNLength max_height = 16.0;

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open object parse file %s\n", filename);
    return 0;
  }

  // Read file
  char keyword[1024];
  while (fscanf(fp, "%s", keyword) == (unsigned int) 1) {
    if (!strcmp(keyword, "F")) {
      // Parse feature type
      int feature_type;
      if (fscanf(fp, "%d", &feature_type) != (unsigned int) 1) {
        fprintf(stderr, "Invalid feature command\n");
        return 0;
      }

      // Parse feature info
      if (feature_type == 1) {
        // Read scan info
        char run_name[1024];
        int segment_index, scan_index, scanline_index, point_index;
        double score, px, py, pz, dx, dy, dz;
        if (fscanf(fp, "%s%d%d%d%d%lf%lf%lf%lf%lf%lf%lf", 
          run_name, &segment_index, &scan_index, &scanline_index, &point_index, &score,
          &px, &py, &pz, &dx, &dy, &dz) != (unsigned int) 12) {
          fprintf(stderr, "Error parsing feature from %s\n", filename);
          return 0;
        }

        // Create detection
        char name[4096];
        R3Point origin(px, py, pz);
        sprintf(name, "D%d", NDetections());
        ObjectDetection *detection = new ObjectDetection(name, origin, max_radius, max_height);
        InsertDetection(detection);
      }
    }
  }

  // Close file 
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Arff I/O functions
////////////////////////////////////////////////////////////////////////

int ObjectParse::
WriteArffFile(const char *filename) const
{
  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open arff file %s\n", filename);
    return 0;
  }

  // Write header
  fprintf(fp, "%% Parse 1.0\n");
  fprintf(fp, "@relation Parse\n");

  // Write attribute names
  for (int i = 0; i < NLabels(); i++) {
    ObjectLabel *label = Label(i);
    fprintf(fp, "@attribute %s real\n", label->Name());
  }

  // Write ground truth label names
  fprintf(fp, "@attribute label { ");
  if (!Label("Unknown")) fprintf(fp, "Unknown, ");
  for (int i = 0; i < NLabels(); i++) {
    ObjectLabel *label = Label(i);
    fprintf(fp, "%s, ", label->Name());
  }
  fprintf(fp, "}\n");

  // Write data
  fprintf(fp, "@data\n");
  for (int i = 0; i < NDetections(); i++) {
    ObjectDetection *detection = Detection(i);

    // Print best assignment score for each label
    for (int j = 0; j < NLabels(); j++) {
      ObjectLabel *label = Label(j);
      RNScalar best_score = 0;
      for (int k = 0; k < detection->NSegmentations(); k++) {
        ObjectSegmentation *segmentation = detection->Segmentation(k);
        for (int m = 0; m < label->NModels(); m++) {
          ObjectModel *model = label->Model(m);
          ObjectAssignment *assignment = segmentation->BestAssignment(model);
          if (!assignment) continue;
          if (assignment->IsGroundTruth()) continue;
          RNScalar score = assignment->Score();
          if (score > best_score) best_score = score;
        }
      }
      fprintf(fp, "%g ", best_score);
    }

    // Print ground truth label
    ObjectLabel *ground_truth_label = detection->GroundTruthLabel();
    if (ground_truth_label) fprintf(fp, "%s\n", ground_truth_label->Name());
    else fprintf(fp, "Unknown\n");
  }

  // Close file 
  fclose(fp);

  // Return success
  return 1;
}



