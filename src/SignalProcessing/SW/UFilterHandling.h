//---------------------------------------------------------------------------

#ifndef UFilterHandlingH
#define UFilterHandlingH
//---------------------------------------------------------------------------

#include <Scktcomp.hpp>

#include "CalibrationFilter.h"
#include "SpatialFilter.h"
#include "SWFilter.h"
#include "ClassFilter.h"
#include "NormalFilter.h"

class FILTERS : public Environment
{
protected:
       GenericSignal *CreateGenericSignal(int transmitchannels, int samples, char *buf);
       int  MA, MB, MC, MD, ME, MF;  // spatial dimensions of the signals
       int NA, NB, NC, ND, NE, NF;   // temporal dimensions of the signals
public:
       FILTERS(PARAMLIST *ParamList, STATELIST *StateList);
       ~FILTERS();
       int      Initialize(PARAMLIST *plist, STATEVECTOR *svector, CORECOMM *);
       int      Process(char *buf);
       int      Resting(char *buf);
       bool     was_error;
       CalibrationFilter  *calfilter;
       SpatialFilter      *spatfilter;
       ClassFilter        *classfilter;
       NormalFilter       *normalfilter;
       TSW                *SWFilter;
       TFBArteCorrection  *FBArteCorrection;
       TSetBaseline       *SetBaseline;

       GenericFilter*  mpFFTFilter,
                    *  mpAvgDisplay;

       GenericSignal  *SignalA, *SignalB, *SignalC, *SignalD, *SignalE, *SignalF;
};
#endif
