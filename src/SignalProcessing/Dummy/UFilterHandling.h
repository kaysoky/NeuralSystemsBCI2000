//---------------------------------------------------------------------------

#ifndef UFilterHandlingH
#define UFilterHandlingH
//---------------------------------------------------------------------------

#include "UParameter.h"
#include "UState.h"
#include "UCoreComm.h"
#include "UGenericSignal.h"

#include <Scktcomp.hpp>

class FILTERS
{
protected:
public:
       FILTERS(PARAMLIST *ParamList, STATELIST *StateList);
       ~FILTERS();
       int      Initialize(PARAMLIST *plist, STATEVECTOR *svector, CORECOMM *);
       int      Process(char *buf);
       int      Resting(char *buf);
       bool     was_error;
       GenericSignal *SignalF;
};
#endif

