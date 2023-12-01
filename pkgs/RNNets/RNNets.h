/* Include file for nets module */
#ifndef __RN__NETS__H__
#define __RN__NETS__H__



/* Dependency include files */

#include "RNBasics/RNBasics.h"



/* System include files */

#include <iostream>
#if (RN_OS == RN_WINDOWS)
# include <winsock2.h>
# include <ws2tcpip.h> 
# define close(a) closesocket(a)  // from Irix to NT
#else
# include <ctype.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# define _BSD_SIGNALS
# include <signal.h>
# include <fcntl.h>
# include <unistd.h>
#endif



/* Abstract network interface */

#include "RNNet.h"



/* Internet interface include files */

#include "RNIp.h"
#include "RNUdp.h"
#include "RNTcp.h"



/* Modem interface include files */

#include "RNModem.h"



/* Initialization functions */

namespace gaps {
int RNInitNets();
void RNStopNets();
}


#endif






