/* Include file for abstract network interface class */
#ifndef __RN__NET__H__
#define __RN__NET__H__



/* Begin namespace */
namespace gaps {



/* Class definition */

class RNNetwork {
public:
  // Constructor/destructor
  RNNetwork();
  virtual ~RNNetwork();
  
  // I/O functions/operators
  virtual int Write(const void *buffer, int length) = 0;
  virtual int Read(void *buffer, int length) = 0;
};


/* Network flags */

#define RN_NON_BLOCKING_NETWORK   0x1



/* Type definitions */

typedef void RNNetworkReceiverFunction(int);

  

// End namespace
}



// End include guard
#endif
