/* Source file for nets module */



/* Include files */

#include "RNNets/RNNets.h"



// Namespace

namespace gaps {



/* Private variables */

static int RNnets_active_count = 0;



int RNInitNets()
{
    // Check whether are already initialized 
    if ((RNnets_active_count++) > 0) return TRUE;

    // Initialize submodules 
    // ???

    // Return OK status 
    return TRUE;
}



void RNStopNets()
{
    // Check whether have been initialized 
    if ((--RNnets_active_count) > 0) return;

    // Stop submodules 
    // ???
}



} // namespace gaps
