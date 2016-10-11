/* Include file for external stuff */



/* Standard dependency include files */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>



/* Machine dependent include files */

#if (RN_OS == RN_WINDOWS)
#   include <float.h>
#   include <windows.h>
#else 
#   include <float.h>
#   include <sys/time.h>
#   include <sys/resource.h>
#endif


