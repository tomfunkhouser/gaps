// Include file for sfllabel input/output functions
#ifndef __SFLLABEL_IO_H__
#define __SFLLABEL_IO_H__


////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Surfels/R3Surfels.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {

  

///////////////////////////////////////////////////////////////////////
// Scene I/O Functions
////////////////////////////////////////////////////////////////////////

extern R3SurfelScene *OpenScene(const char *scene_filename, const char *database_filename,
  int print_verbose = 0);

extern int CloseScene(R3SurfelScene *scene,
  int print_verbose = 0);



///////////////////////////////////////////////////////////////////////
// Image I/O Functions
////////////////////////////////////////////////////////////////////////

extern int ReadImagesFromPixelDatabase(R3SurfelScene *scene, const char *filename,
  double depth_scale = 2000, double depth_exponent = 0.5, int max_images = 0,
  int print_verbose = 0);

extern int ReadImagesFromDirectory(R3SurfelScene *scene, const char *directory_name,
  double depth_scale = 2000, double depth_exponent = 0.5, int max_images = 0,
  int print_verbose = 0);



}; // end namespace



#endif
