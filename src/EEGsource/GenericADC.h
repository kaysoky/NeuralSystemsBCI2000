//---------------------------------------------------------------------------

#ifndef GenericADCH
#define GenericADCH
//---------------------------------------------------------------------------
#endif

#ifndef UParameterH
 #include "UParameter.h"
#endif
#ifndef UStateH
 #include "UState.h"
#endif
#ifndef UGenericSignalH
 #include "UGenericSignal.h"
#endif

#define ADCERR_NOERR    0
#define ADCERR_INIT     1
#define ADCERR_ADREAD   2

class GenericADC
{
protected:
        int     samplerate;
public:
        GenericADC::GenericADC();
        GenericADC::~GenericADC();
        GenericIntSignal *signal;
        int     ADInit();
        int     ADReadDataBlock();
        int     ADShutdown();
        int     GetSampleRate();
};
