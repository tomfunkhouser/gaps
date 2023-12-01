/* Source file for internet class  */



/* Library include files */

#include "RNNets/RNNets.h"



// Namespace

namespace gaps {



int RNInitInternet() 
{
    /* Return OK status */
    return TRUE;
}



void RNStopInternet()
{
}



char *
RNLocalInternetName(char *buffer, int length)
{
  // Return name of this host
  int status = gethostname(buffer, length);
  return (status) ? NULL : buffer;
} 



RNInternetAddress
RNLocalInternetAddress(void)
{
#if (RN_OS == RN_WINDOWS)
  char myname[128];
  gethostname(myname, 128);
  HOSTENT *pHostEnt = gethostbyname(myname);
  //IN_ADDR in;
  struct in_addr in;

  CopyMemory(&in, pHostEnt->h_addr_list[0], sizeof(pHostEnt->h_addr_list[0]));
//  char buffer[128];
//  sprintf(buffer, "My IP address: %s", inet_ntoa(in));
//  RNWarning(buffer);
  return (RNInternetAddress) in.S_un.S_addr;
#else
    // Return address of this host
    return gethostid();
#endif
}



char *
RNInternetNameFromAddress(RNInternetAddress addr, char *buffer, int length)
{
  // Search for host by address
  struct hostent *hp;
  hp = gethostbyaddr((char *) &addr, sizeof(RNInternetAddress), AF_INET);

  
  if (!hp) return NULL;
  assert(hp->h_addrtype == AF_INET);
  assert(hp->h_length == sizeof(RNInternetAddress));
  assert((hp->h_name) && (strlen(hp->h_name) < length));
  strncpy(buffer, hp->h_name, length);
  return buffer;
}  // RNInternetNameFromAddress



RNInternetAddress
RNInternetAddressFromName(const char *hostname)
{
  // Check type of hostname
  if (isdigit(hostname[0])) {
    // Name must be XXX.XXX.XXX.XXX 
    return inet_addr(hostname);
  }
  else {
    // Search for host by name
    struct hostent *hp = gethostbyname(hostname);
    if (!hp) return 0;
    assert(hp->h_addrtype == AF_INET);
    assert(hp->h_length == sizeof(RNInternetAddress));
    return *((RNInternetAddress *) hp->h_addr);
  }
}  // RNIp::RNInternetAddressFromName


  
} // namespace gaps
