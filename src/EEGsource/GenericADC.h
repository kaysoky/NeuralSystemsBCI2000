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
    // This static member function of the GenericADC class is meant to
    // be implemented along with a subclass of GenericADC.
    // Its sole purpose is to make subclassing transparent for the
    // code in EEGSource/UMain.cpp .
    static GenericADC* GetNewADC( PARAMLIST*, STATELIST* );

    // Constructors/Destructors.
            GenericADC();
    virtual ~GenericADC();

    // Accessors that can be overridden by derived classes.
    virtual const GenericIntSignal* Signal() { return signal; }
    virtual int GetSampleRate() const        { return samplerate; }

    // Interfacing functions that must be implemented by derived classes.
    virtual int ADInit() = 0;
    virtual int ADReadDataBlock() = 0;
    virtual int ADShutdown() = 0;

    BCI2000ERROR error;

  protected:
    int samplerate;
    GenericIntSignal* signal;
};
#endif

