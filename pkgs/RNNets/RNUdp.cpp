/* Source file for UDP network interface class */



// Include files

#include "RNNets/RNNets.h"



// Namespace

namespace gaps {



/* Private variables */

static const struct sockaddr_in RNudp_address = { AF_INET };



RNUdp::
RNUdp(RNInternetAddress address, RNInternetPort port, 
      RNBoolean blocking, RNNetworkReceiverFunction *receiver,
      int write_buffer_size, int read_buffer_size)
    : address(address),
      port(port),
      receiver(receiver),
      flags((blocking) ? RN_NO_FLAGS : RN_NON_BLOCKING_NETWORK)
{
  // create socket
  sock = socket(AF_INET, SOCK_DGRAM, 0);

#if (RN_OS == RN_WINDOWS)
  if (sock == INVALID_SOCKET) sock = -1;  // INVALID_SOCKET == 0
#endif

  if (sock < 0) RNAbort("Unable to open socket");

  // Setup udp address 
  struct sockaddr_in addr = RNudp_address;
  if ((address) && (!IN_MULTICAST(address)))
    addr.sin_addr.s_addr = htonl(address);
  else addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  // Set socket options to allow address re-use 
  int one = 1;
  // NT setsockopt returns SOCKET_ERROR == -1 in case of error
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		 (char *) &one, sizeof(one)) < 0) {
    RNAbort("Unable to set SO_REUSEADDR");
    close(sock);
  }  // if, set SO_REUSEADDR

#if (0 && (RN_OS != RN_WINDOWS))
  // Set socket options to allow port re-use (not supported in winsock2.h)
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,
		 (char *) &one, sizeof(one)) < 0) {
    RNAbort("Unable to set SO_REUSEPORT");
    close(sock);
  }
#endif

  // Bind to specific address
  // failing that, listen on any address at designated port 
  if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      char message[128];
#if (RN_OS == RN_WINDOWS)
      int error_code = WSAGetLastError();
      sprintf(message, "NT: Unable to bind address, Errorcode: %d", error_code);
#else
      sprintf(message, "IRIX: Unable to bind address");
#endif
      RNAbort(message);
      close(sock);
    }
  }  // if, bind to specific address

  // Add socket to multicast group 
  if (address && (IN_MULTICAST(address))) {
    static struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = address;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		   (char *) &mreq, sizeof(mreq)) < 0) {
      RNAbort("Unable to join multicast group");
      close(sock);
    }
  }

  // Set socket write buffer size
  if (write_buffer_size > 0) {
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
		   (char *) &write_buffer_size, sizeof(write_buffer_size)) < 0) {
      RNAbort("Unable to set SO_SNDBUF");
      close(sock);
    }
  }

  // Set socket read buffer size
  if (read_buffer_size > 0) {
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		   (char *) &read_buffer_size, sizeof(read_buffer_size)) < 0) {
      RNAbort("Unable to set SO_RCVBUF");
      close(sock);
    }
  }

  // Setup asynchronous callbacks 
  if (receiver) {
#if (RN_OS != RN_WINDOWS)  
    // Set the signal handler for SIGIO 
    if (signal(SIGIO, receiver) == SIG_ERR) {
      RNAbort("Unable to set SIGIO handler");
      close(sock);
    }

    // Set the process receiving SIGIO/SIGURG to us 
    if (fcntl(sock, F_SETOWN, getpid()) < 0) {
      RNAbort("Unable to set socket F_SETOWN");
      close(sock);
    }

    // Allow receipt of asynchronous IO signals 
    if (fcntl(sock, F_SETFL, FASYNC) < 0) {
      RNAbort("Unable to set socket F_SETFL");
      close(sock);
    }
#endif
  }
  else {
    // Set up non-blocking i/o 
    if (!blocking) {
#if (RN_OS != RN_WINDOWS)
      if (fcntl(sock, F_SETFL, FNDELAY) < 0) {
	      RNAbort("Unix: Unable to set socket non-blocking");
	      close(sock);
      }
#else
      unsigned long argp = 1;
      if (ioctlsocket(sock, FIONBIO, &argp) == SOCKET_ERROR) {
        RNAbort("NT: Unable to set socket non-blocking");
        close(sock);
      }
#endif
    }
  }
}  // RNUdp constructor



RNUdp::~RNUdp(void)
{
  // This is just to stop compiler from warning about unused private variables
  receiver = NULL;

  // Close socket
  if (sock) close(sock);
}  // destructor



int
RNUdp::Write(const void *buffer, int length)
{
  return Write(address, port, buffer, length);
}



int
RNUdp::Write(RNInternetPort port, const void *buffer, int length)
{
  return Write(address, port, buffer, length);
}



int
RNUdp::Write(RNInternetAddress address, const void *buffer, int length)
{
  return Write(address, port, buffer, length);
}



int
RNUdp::Write(RNInternetAddress address, RNInternetPort port,
	     const void *buffer, int length)
{
  // Construct socket address
  struct sockaddr_in addr = RNudp_address;
  addr.sin_addr.s_addr = address;
  addr.sin_port = htons(port);

#if (RN_OS == RN_WINDOWS)
  return sendto(sock, (const char *) buffer, length, 0,
		(struct sockaddr *) &addr, sizeof(addr));
#else
  return sendto(sock, buffer, length, 0, (struct sockaddr *) &addr, sizeof(addr));
#endif
  
}  // RNUdp::Write



int
RNUdp::Write(const char *hostname, const void *buffer, int length)
{
  return Write(RNInternetAddressFromName(hostname), port, buffer, length);
}



int
RNUdp::Write(const char *hostname, RNInternetPort port,
	     const void *buffer, int length)
{
  return Write(RNInternetAddressFromName(hostname), port, buffer, length);
}



int
RNUdp::Read(void *buffer, int length)
{
  struct sockaddr_in addr;
#if (RN_OS == RN_WINDOWS)
  int recvsize = sizeof(addr);
  int nbytes = recvfrom(sock, (char *) buffer, length, 0,
			(struct sockaddr *) &addr, &recvsize);
#else
  socklen_t recvsize = sizeof(addr);
  int nbytes = recvfrom(sock, buffer, length, 0,
			(struct sockaddr *) &addr, &recvsize);
#endif  
  return nbytes;
}  // RNUdp::Read



int
RNUdp::Read(void *buffer, int length, RNInternetAddress *src)
{
  struct sockaddr_in addr;
#if (RN_OS == RN_WINDOWS)  
  int recvsize = sizeof(addr);
  int nbytes = recvfrom(sock, (char *) buffer, length, 0,
			(struct sockaddr *) &addr, &recvsize);
#else
  socklen_t recvsize = sizeof(addr);
  int nbytes = recvfrom(sock, buffer, length, 0,
			(struct sockaddr *) &addr, &recvsize);
#endif  
  if (src) *src = addr.sin_addr.s_addr;
  return nbytes;
}  // RNUdp::Read



} // namespace gaps
