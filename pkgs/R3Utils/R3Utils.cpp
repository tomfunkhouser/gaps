/* Source file for GAPS utils module */



/* Include files */

#include "R3Utils.h"



// Namespace

namespace gaps {



/* Private variables */

static int R3utils_active_count = 0;



int R3InitUtils(void)
{
    // Check whether are already initialized 
    if ((R3utils_active_count++) > 0) return TRUE;

    // Initialize dependencies
    if (!R3InitShapes()) return FALSE;

    // return OK status 
    return TRUE;
}



void R3StopUtils(void)
{
    // Check whether have been initialized 
    if ((--R3utils_active_count) > 0) return;

    // Stop dependencies
    R3StopShapes();
}



} // namespace gaps
