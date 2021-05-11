/* Include file for R3 utils module */
#ifndef __R3__UTILS__H__
#define __R3__UTILS__H__



/* Class declarations */

namespace gaps {
class R3Segmentation;
}



/* Dependency include files */

#include "R3Shapes/R3Shapes.h"



/* Utility include files */

#include "R3Segmentation.h"



/* Initialization functions */

namespace gaps{
int R3InitUtils(void);
void R3StopUtils(void);
}



#endif


