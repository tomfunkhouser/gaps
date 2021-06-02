// Source file for GSV laser class



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "GSV.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Constructor/destructor functions
////////////////////////////////////////////////////////////////////////

GSVLaser::
GSVLaser(int device_type)
  : run(NULL),
    run_index(-1),
    surveys(),
    device_type(device_type),
    max_sweep_index(0),
    max_beam_index(0),
    bbox(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX),
    data(NULL)
{
  // Set max sweep and beam indices
  if (device_type == GSV_SICK_LASER) {
    max_sweep_index = 0;
    max_beam_index = 179;
  }
  else if (device_type == GSV_VLP16_LASER) {
    max_sweep_index = 1799;
    max_beam_index = 15;
  }
}



GSVLaser::
~GSVLaser(void)
{
  // Remove laser from run
  if (run) run->RemoveLaser(this);

  // Remove surveys from this laser
  while (NSurveys() > 0) RemoveSurvey(Survey(0));
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

GSVScene *GSVLaser::
Scene(void) const
{
  // Return scene
  if (!run) return NULL;
  return run->Scene();
}



int GSVLaser::
SceneIndex(void) const
{
  // Get convenient variables
  GSVScene *scene = Scene();
  if (!scene) return -1;

  // Compute scene index
  int scene_index = RunIndex();
  for (int i = 0; i < run->SceneIndex(); i++) {
    GSVRun *r = scene->Run(i);
    scene_index += r->NLasers();
  }

  // Return scene index
  return scene_index;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void GSVLaser::
InsertSurvey(GSVSurvey *survey)
{
  // Just checking
  assert(survey->laser_index == -1);
  assert(survey->laser == NULL);

  // Insert survey
  survey->laser = this;
  survey->laser_index = surveys.NEntries();
  surveys.Insert(survey);

  // Invalidate bounding box
  InvalidateBBox();
}



void GSVLaser::
RemoveSurvey(GSVSurvey *survey)
{
  // Just checking
  assert(survey->laser_index >= 0);
  assert(survey->laser_index < surveys.NEntries());
  assert(survey->laser == this);

  // Remove survey
  RNArrayEntry *entry = surveys.KthEntry(survey->laser_index);
  GSVSurvey *tail = surveys.Tail();
  tail->laser_index = survey->laser_index;
  surveys.EntryContents(entry) = tail;
  surveys.RemoveTail();
  survey->laser_index = -1;
  survey->laser = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



////////////////////////////////////////////////////////////////////////
// Display functions
////////////////////////////////////////////////////////////////////////

void GSVLaser::
Draw(RNFlags flags) const
{
  // Draw surveys
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    survey->Draw(flags);
  }
}



void GSVLaser::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print laser header
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "Laser %d:", run_index);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");

  // Add indentation to prefix
  char indented_prefix[1024];
  sprintf(indented_prefix, "%s  ", prefix);

 // Print surveys
  for (int i = 0; i < NSurveys(); i++) {
    GSVSurvey *survey = Survey(i);
    survey->Print(fp, indented_prefix, suffix);
  }
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void GSVLaser::
UpdateBBox(void) 
{
  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < NSurveys(); i++) {
    bbox.Union(Survey(i)->BBox());
  }
}



void GSVLaser::
InvalidateBBox(void) 
{
  // Invalidate bounding box
  if (bbox.XMin() != FLT_MAX) {
    bbox = R3Box(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
    if (run) run->InvalidateBBox();
  }
}



} // namespace gaps
