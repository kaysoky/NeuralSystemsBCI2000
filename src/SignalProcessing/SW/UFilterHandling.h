#ifndef UFilterHandlingH
#define UFilterHandlingH

#include "UEnvironment.h"
#include "UGenericSignal.h"

class FILTERS : public Environment
{
  public:
    FILTERS( void*, void* );
    ~FILTERS();
    int Initialize( void*, void*, void* );
    int Process( const char* );
    int Resting( void* );

    GenericSignal* SignalF;
    bool was_error;
    
  private:
    GenericSignal mInputBuffer,
                  mOutputBuffer;
};
#endif // UFilterHandlingH

