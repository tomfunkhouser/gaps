/* Include file for UDP network interface class */
#ifndef __RN__UDP__H__
#define __RN__UDP__H__



/* Begin namespace */
namespace gaps {



// Class definition

class RNUdp : public RNNetwork {
public:
  // Constructor/destructor
  RNUdp(RNInternetAddress addr = 0, 
	RNInternetPort port = RNdefault_internet_port, 
	RNBoolean blocking = TRUE,
	RNNetworkReceiverFunction *receiver = NULL,
	int write_buffer_size = 0, 
	int read_buffer_size = 0);
  ~RNUdp(void);


  // Property functions/operators
  inline const int Socket(void) const { return sock; }

  // I/O functions/operators
  int Write(const void *buffer, int length);
  int Write(RNInternetPort port, const void *buffer, int length);
  int Write(RNInternetAddress address, const void *buffer, int length);
  int Write(RNInternetAddress address, RNInternetPort port,
	    const void *buffer, int length);
  int Write(const char *hostname, const void *buffer, int length);
  int Write(const char *hostname, RNInternetPort port,
	    const void *buffer, int length);
  int Read(void *buffer, int length);
  int Read(void *buffer, int length, RNInternetAddress *src);

  
private:
  RNInternetAddress address;
  RNInternetPort port;
  RNNetworkReceiverFunction *receiver;
  RNFlags flags;

# if (RN_OS == RN_WINDOWS)
    SOCKET sock;
# else
    int sock;
# endif    
};

  

// End namespace
}



// End include guard
#endif
