/* Include file for GAPS basics module */

#ifndef __RN__BASICS__H__
#define __RN__BASICS__H__



/* Compatability switch include files */

#include "RNCompat.h"



/* External include files */

#include "RNExtern.h"



/* Base class for GAPS modules */

#include "RNBase.h"



/* Error reporting include files */

#include "RNError.h"



/* Memory management include files */

#include "RNMem.h"



/* File management include files */

#include "RNFile.h"



/* Basic bitflags include files */

#include "RNFlags.h"



/* Class type include files */

#include "RNType.h"



/* Math include files */

#include "RNScalar.h"
#include "RNIntval.h"



/* Dynamic array include files */

#include "RNArray.h"
#include "RNQueue.h"
#include "RNHeap.h"



/* Graphics utility include files */

#include "RNGrfx.h"
#include "RNRgb.h"



/* OS utility include files */

#include "RNTime.h"



/* SVD stuff */

#include "RNSvd.h"



/* Initialization functions */

int RNInitBasics(void);
void RNStopBasics(void);



#endif







