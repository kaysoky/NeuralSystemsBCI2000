#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UFilterHandling.h"
#include <assert>

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   FILTERS
// Purpose:    This is the constructor for the FILTERS class
//             it creates all filters we might want to use
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
//             sets the variable error to true, if there was an error
// **************************************************************************
FILTERS::FILTERS(PARAMLIST *plist, STATELIST *slist)
{
char line[256];

 was_error=false;

 SignalF=new GenericSignal(1, 1);
 SignalF->SetValue(0, 0, 0);

 strcpy(line, "Filtering int NumControlSignals= 2 1 1 128    // the number of transmitted control signals");
 plist->AddParameter2List(line, strlen(line));
}


// **************************************************************************
// Function:   ~FILTERS
// Purpose:    This is the deconstructor for the FILTERS class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
FILTERS::~FILTERS()
{
 if (SignalF) delete SignalF;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    Initialize all the filters
// Parameters: plist - pointer to the PARAMETERLIST
//             svector - pointer to the STATEVECTOR
//             opsocket - pointer to the open socket connection to the operator
// Returns:    0 - any filter had an error on Initialize()
//             1 - no error
// **************************************************************************
int FILTERS::Initialize(PARAMLIST *plist, STATEVECTOR *svector, CORECOMM *corecomm)
{
 return(1);
}


int FILTERS::Resting( char *buf)
{
 return(0);
}


// **************************************************************************
// Function:   Process
// Purpose:    Process all the filters
// Parameters: buf - char * to the buffer received from the EEGsource
// Returns:    0 - any filter had an error on Process()
//             1 - no error
// **************************************************************************
int FILTERS::Process(char *buf)
{
 return(1);
}



