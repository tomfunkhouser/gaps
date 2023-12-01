/* Source file for RING modem class  */



/* Library include files */

#include "RNNets/RNNets.h"



// Namespace

namespace gaps {



int RNInitModem() 
{
    /* Return OK status */
    return TRUE;
}



void RNStopModem()
{
}



RNModem::
RNModem(const char *comname, const char *initstring)
{
#if (RN_OS == RN_WINDOWS)
    // Open communications device
    if ((com = CreateFile(comname, GENERIC_READ | GENERIC_WRITE,
	0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
	RNAbort("Unable to open communications device");
    }

    // Reset modem
    DWORD nbytes;
    char buffer[128];
    sprintf(buffer, "AT &F\r");
    if (!WriteFile(com, buffer, strlen(buffer), &nbytes, NULL))
	RNAbort("Unable to initialize communications device");
    assert(nbytes == strlen(buffer));
    
    // Send initialization string
    if (initstring) {
	DWORD nbytes;
	char buffer[128];
	sprintf(buffer, "%s\r", initstring);
	if (!WriteFile(com, buffer, strlen(buffer), &nbytes, NULL))
	    RNAbort("Unable to initialize communications device");
	assert(nbytes == strlen(buffer));
    }
#else
    RNAbort("Not implemented");
#endif
}



RNModem::
~RNModem(void)
{
#if (RN_OS == RN_WINDOWS)
    // Close communications device
    if (!CloseHandle(com)) {
	RNFail("Unable to close communications device");
    }
#else
    RNAbort("Not implemented");
#endif
}



int RNModem::
Write(const void *buffer, int length)
{
#if (RN_OS == RN_WINDOWS)
    // Write data
    DWORD nbytes;
    if (!WriteFile(com, buffer, length, &nbytes, NULL))
	RNAbort("Unable to write to communications device");
    return (int) nbytes;
#else
    RNAbort("Not implemented");
    return 0;
#endif
}



int RNModem::
Read(void *buffer, int length)
{
#if (RN_OS == RN_WINDOWS)
    // Read data
    DWORD nbytes;
    if (!ReadFile(com, buffer, length, &nbytes, NULL))
	RNAbort("Unable to read from communications device");
    return (int) nbytes;
#else
    RNAbort("Not implemented");
    return 0;
#endif
}


  
} // namespace gaps
