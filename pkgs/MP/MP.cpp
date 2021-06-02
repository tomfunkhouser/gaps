/* Source file for MP module */



/* Include files */

#include "MP.h"



// Namespace

namespace gaps {



/* Private variables */

static int MP_active_count = 0;



int MPInit(void)
{
    // Check whether are already initialized 
    if ((MP_active_count++) > 0) return TRUE;

    // Initialize dependencies
    if (!R3InitShapes()) return FALSE;

    // return OK status 
    return TRUE;
}



void MPStop(void)
{
    // Check whether have been initialized 
    if ((--MP_active_count) > 0) return;

    // Stop dependencies
    R3StopShapes();
}



} // namespace gaps
