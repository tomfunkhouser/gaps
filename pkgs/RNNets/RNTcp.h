/* Include file for TCP network interface class */
#ifndef __RN__TCP__H__
#define __RN__TCP__H__



/* Begin namespace */
namespace gaps {



class RNTcp : public RNNetwork {
public:
  // Constructor/destructor
  RNTcp(RNInternetAddress addr = 0, 
	RNInternetPort port = RNdefault_internet_port,
	RNBoolean server = FALSE,
	RNBoolean blocking = TRUE,
	RNNetworkReceiverFunction *receiver = NULL,
	int write_buffer_size = 0, 
	int read_buffer_size = 0);
  ~RNTcp(void);

  
  // Property functions/operators
  const int Socket(void) const { return sock; }
  
  // I/O functions/operators
  int Write(const void *buffer, int length);
  int Read(void *buffer, int length);

  
private:
  RNInternetAddress address;
  RNInternetPort port;
  RNNetworkReceiverFunction *receiver;
  RNFlags flags;

#if (RN_OS == RN_WINDOWS)
  SOCKET sock;
#else
  int sock;
#endif  
}; 



// End namespace
}



// End include guard
#endif
