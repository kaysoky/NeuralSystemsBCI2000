//---------------------------------------------------------------------------

#ifndef GenericADCH
#define GenericADCH
//---------------------------------------------------------------------------

#include "UBCI2000Error.h"
#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"

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
        BCI2000ERROR    error;
};
#endif

