/* Source file for TCP network interface class */



// Include files

#include "RNNets/RNNets.h"



// Namespace

namespace gaps {



/* Private variables */

static const struct sockaddr_in RNtcp_address = { AF_INET };



RNTcp::
RNTcp(RNInternetAddress address, RNInternetPort port, 
      RNBoolean server, RNBoolean blocking, RNNetworkReceiverFunction *receiver,
      int write_buffer_size, int read_buffer_size)
    : address(address),
      port(port),
      receiver(receiver),
      flags((blocking) ? RN_NO_FLAGS : RN_NON_BLOCKING_NETWORK)
{
  // Create socket 
  sock = socket(AF_INET, SOCK_STREAM, 0);

#if (RN_OS == RN_WINDOWS)
  if (sock == INVALID_SOCKET) sock = -1;  // INVALID_SOCKET == 0
#endif
  
  if (sock < 0) RNAbort("Unable to open socket");

  // Setup client/server
  if (!server) {
    // Setup server address 
    struct sockaddr_in addr = RNtcp_address;
    addr.sin_addr.s_addr = address;
    addr.sin_port = htons(port);

    // Request connection
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      RNAbort("Unable to make connection");
      close(sock);
    }
  }
  else {
    // Setup connection address 
    struct sockaddr_in addr = RNtcp_address;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Get address size (for call to accept)
#   if (RN_OS == RN_WINDOWS)
        int addrsize = sizeof(addr);
#   else
        socklen_t addrsize = sizeof(addr);
#   endif

    // Bind to address
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      RNAbort("Unable to bind address");
      close(sock);
    }

    // Listen for connection
    if (listen(sock, 1) < 0) {
      RNAbort("Unable to listen for connections");
      close(sock);
    }

    // Wait for and accept connection
    int tmpsock = 0;
    if ((tmpsock = accept(sock, (struct sockaddr *) &addr, &addrsize)) < 0) {
      RNAbort("Unable to accept connection");
      close(sock);
    }
    else {
      close(sock);
      sock = tmpsock;
    }
  }

  // Set socket write buffer size
  if (write_buffer_size > 0) {
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *) &write_buffer_size, sizeof(write_buffer_size)) < 0) {
      RNAbort("Unable to set SO_SNDBUF");
      close(sock);
    }
  }

  // Set socket read buffer size
  if (read_buffer_size > 0) {
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &read_buffer_size, sizeof(read_buffer_size)) < 0) {
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
}  // RNTcp constructor



RNTcp::
~RNTcp(void)
{
    // This is just to stop compiler from warning about unused private variables
    address = 0;
    port = 0;
    receiver = NULL;
  
    // Close socket
    if (sock) close(sock);
}



int
RNTcp::Write(const void *buffer, int length)
{
#if (RN_OS == RN_WINDOWS)
  return send(sock, (const char *) buffer, length, 0);
#else  
  return send(sock, buffer, length, 0);
#endif
}  // RNTcp::Write



int RNTcp::
Read(void *buffer, int length)
{
#if (RN_OS == RN_WINDOWS)
  int result = recv(sock, (char *)  buffer, length, 0);
  if (result == SOCKET_ERROR) {
    cout << "NT RNTcp::Read error: " << WSAGetLastError() << endl;
  }
  return result;
#else
  return recv(sock, buffer, length, 0);
#endif
}  // RNTcp::Read



} // namespace gaps
