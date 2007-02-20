/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include "DummyFilter.h"

RegisterFilter( DummyFilter, 2.Z );

// **************************************************************************
// Function:   DummyFilter
// Purpose:    This is the constructor for the DummyFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
DummyFilter::DummyFilter()
{
}

// **************************************************************************
// Function:   ~DummyFilter
// Purpose:    This is the destructor for the DummyFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
DummyFilter::~DummyFilter()
{
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void DummyFilter::Preflight( const SignalProperties& inSignalProperties,
                                   SignalProperties& outSignalProperties ) const
{
  outSignalProperties = SignalProperties( 2, 1 );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the DummyFilter
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void DummyFilter::Initialize()
{
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the calibration routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void DummyFilter::Process(const GenericSignal *input, GenericSignal *output)
{
}



