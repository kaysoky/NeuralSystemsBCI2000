/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/

//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include <math.h>

#include "NormalFilter.h"
#include "Param.h"
#include "GenericVisualization.h"

using namespace std;

RegisterFilter( NormalFilter, 2.E );

// **************************************************************************
// Function:   NormalFilter
// Purpose:    This is the constructor for the NormalFilter Class
//             It is the "Normalizer
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
NormalFilter::NormalFilter()
: vis( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
   "Statistics float YMean=  5.0 "
     "5.0  -100.0  100.0 // Normal Filter Up / Down Intercept",
   "Statistics float YGain=  5.0 "
     "5.0  -100.0  100.0 // Normal Filter Up / Down Slope",
   "Statistics float XMean= -5.0 "
     "-5.0  -100.0  100.0 // Normal Filter Left/Right Intercept",
   "Statistics float XGain= 5.0 "
     "5.0  0.0  100.0 // Normal Filter Left/Right Slope",
   "Visualize int VisualizeNormalFiltering= 1 "
     "0 0 1 // visualize Normal filtered signals (0=no 1=yes)",
 END_PARAMETER_DEFINITIONS

}


// **************************************************************************
// Function:   ~NormalFilter
// Purpose:    This is the destructor for the NormalFilter Normal
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
NormalFilter::~NormalFilter()
{
 delete vis;

}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void NormalFilter::Preflight( const SignalProperties& inSignalProperties,
                                    SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.

  // Resource availability checks.
  /* The normalizer filter seems not to depend on external resources. */

  // Input signal checks.
  PreflightCondition( inSignalProperties.Accommodates( SignalProperties( 2, 1 ) ) );

  // Requested output signal properties.
  outSignalProperties = SignalProperties( cNumControlSignals, 1 );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the NormalFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void NormalFilter::Initialize( const SignalProperties&, const SignalProperties& )
{
  int visualizeyn = 0;

  ymean= Parameter("YMean");
  ygain= Parameter("YGain");
  xmean= Parameter("XMean");
  xgain= Parameter("XGain");

  visualizeyn= Parameter("VisualizeNormalFiltering");

  if( visualizeyn == 1 )
  {
    visualize=true;
    delete vis;
    vis= new GenericVisualization( SourceID::Normalizer );
    vis->Send(CfgID::WindowTitle, "Normalizer");
    vis->Send(CfgID::MinValue, "-40");
    vis->Send(CfgID::MaxValue, "40");
    vis->Send(CfgID::NumSamples, "256");
  }
  else
  {
    visualize=false;
  }

  return;
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Normal routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void NormalFilter::Process(const GenericSignal& input, GenericSignal& output)
{
  // actually perform the Normal Filtering on the input and write it into the output signal
  float val_ud = ( input( 0, 0 ) - ymean ) * ygain;;
  output( 0, 0 ) = val_ud;

  float val_lr = ( input( 1, 0 ) - xmean ) * xgain;
  output( 1, 0 ) = val_lr;

  if( visualize )
  {
    vis->Send( output );
  }
  return;
}

int NormalFilter::UpdateParameters( float new_ymean, float new_ygain, float new_xmean, float new_xgain )
{
  ymean= new_ymean;
  ygain= new_ygain;
  xmean= new_xmean;
  xgain= new_xgain;

  return(1);
}




