/* Include file for internet support */
#ifndef __RN__IP__H__
#define __RN__IP__H__



/* Begin namespace */
namespace gaps {



/* Initialization functions */

int RNInitInternet();
void RNStopInternet();



/* Address definition */

typedef unsigned long RNInternetAddress;



/* Port definition */

typedef int RNInternetPort;
#define RNdefault_internet_port 1111



/* Utility functions */

char *RNLocalInternetName(char *buffer, int length);
RNInternetAddress RNLocalInternetAddress(void);
char *RNInternetNameFromAddress(RNInternetAddress addr, char *buffer, int length);
RNInternetAddress RNInternetAddressFromName(const char *hostname);



// End namespace
}



// End include guard
#endif
