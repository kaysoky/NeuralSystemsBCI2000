//---------------------------------------------------------------------------

#ifndef UFilterHandlingH
#define UFilterHandlingH
//---------------------------------------------------------------------------

#include <Scktcomp.hpp>

#include "CalibrationFilter.h"
#include "P3TemporalFilter.h"
#include "SpatialFilter.h"
#include "FIRFilter.h"
#include "ClassFilter.h"
#include "NormalFilter.h"
// #include "StatFilter.h"

class FILTERS
{
protected:
       GenericSignal *CreateGenericSignal(int transmitchannels, int samples, char *buf);
       int  MA, MB, MC, MC2,MD, ME, MF;  // spatial dimensions of the signals
       int  NA, NB, NC, NC2,ND, NE, NF;  // temporal dimensions of the signals
public:
       FILTERS(PARAMLIST *ParamList, STATELIST *StateList);
       ~FILTERS();
       int      Initialize(PARAMLIST *plist, STATEVECTOR *svector, CORECOMM *);
       int      Process(char *buf);
       int      Resting(char *buf);
       bool     was_error;
       CalibrationFilter  *calfilter;
       SpatialFilter      *spatfilter;
       FIRFilter          *firfilter;
       P3TemporalFilter   *tempfilter;
       ClassFilter        *classfilter;
       NormalFilter       *normalfilter;
       // StatFilter         *statfilter;
       GenericSignal      *SignalA, *SignalB, *SignalC, *SignalC2,*SignalD, *SignalE, *SignalF;
};
#endif

