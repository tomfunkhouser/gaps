/* Include file for RING modem class */
#ifndef __RN__MODEM__H__
#define __RN__MODEM__H__



/* Begin namespace */
namespace gaps {



/* Initialization functions */

int RNInitModem();
void RNStopModem();



/* Socket definition */

class RNModem : public RNNetwork {
    public:
        // Constructor functions
	RNModem(const char *comname, const char *initstring);
	~RNModem(void);

        // I/O functions/operators
	int Write(const void *buffer, int length);
	int Read(void *buffer, int length);

    private:
#if (RN_OS == RN_WINDOWS)
	HANDLE com;
#endif
};


  
// End namespace
}



// End include guard
#endif
