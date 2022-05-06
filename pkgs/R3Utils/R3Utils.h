/* Include file for R3 utils module */
#ifndef __R3__UTILS__H__
#define __R3__UTILS__H__



/* Class declarations */

namespace gaps {
class R3Segmentation;
class R3OrientedBoxManipulator;
class R3SurfelViewer;
class R3SurfelSegmenter;
class R3SurfelClassifier;
class R3SurfelLabeler;
class R3SurfelLabelerCommand;
}



/* Dependency include files */

#include "R3Graphics/R3Graphics.h"
#include "R3Surfels/R3Surfels.h"



/* Viewing utility include files */

#include "R3SurfelViewer.h"



/* Processing utility include files */

#include "R3Segmentation.h"



/* Manipulation utility include files */

#include "R3OrientedBoxManipulator.h"



/* Labeling utility include files */

#include "R3SurfelSegmenter.h"
#include "R3SurfelClassifier.h"
#include "R3SurfelLabeler.h"
#include "R3SurfelLabelerCommand.h"



/* Initialization functions */

namespace gaps{
int R3InitUtils(void);
void R3StopUtils(void);
}



#endif


