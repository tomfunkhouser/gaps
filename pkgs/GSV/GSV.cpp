/* Source file for GSV module */



/* Include files */

#include "GSV.h"



// Namespace

namespace gaps {



/* Private variables */

static int GSV_active_count = 0;



int GSVInit(void)
{
    // Check whether are already initialized 
    if ((GSV_active_count++) > 0) return TRUE;

    // Initialize dependencies
    if (!R3InitShapes()) return FALSE;

    // return OK status 
    return TRUE;
}



void GSVStop(void)
{
    // Check whether have been initialized 
    if ((--GSV_active_count) > 0) return;

    // Stop dependencies
    R3StopShapes();
}



} // namespace gaps
