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
  public:
    static GenericADC* GetNewADC();

            GenericADC();
    virtual ~GenericADC();

    virtual int ADInit() = 0;
    virtual int ADReadDataBlock() = 0;
    virtual int ADShutdown() = 0;

    int GetSampleRate() const;

    BCI2000ERROR error;

  public: // !!
    GenericIntSignal* signal;

  protected:
    int samplerate;

};
#endif

