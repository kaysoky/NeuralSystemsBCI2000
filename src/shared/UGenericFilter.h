//---------------------------------------------------------------------------

#ifndef UGenericFilterH
#define UGenericFilterH
//---------------------------------------------------------------------------


#include "UCoreComm.h"
#include "UState.h"
#include "UParameter.h"
#include "UGenericSignal.h"

class GenericFilter 
{
protected:
       STATEVECTOR       *statevector;
       CORECOMM          *corecomm;
public:
       GenericFilter();
       GenericFilter(PARAMLIST *ParamList, STATELIST *StateList);
       GenericFilter(PARAMLIST *ParamList, STATELIST *StateList, int instance);
       ~GenericFilter();
       int Initialize(PARAMLIST *ParamList, STATEVECTOR *statevector, TCustomWinSocket *opsocket);
       int Process(GenericSignal *Input, GenericSignal *Output);
       int UpdateParameters( void );
};
#endif


