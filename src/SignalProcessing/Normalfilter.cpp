#undef USE_LOGFILE
//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include <math.h>
#ifdef USE_LOGFILE
# include <stdio.h>
#endif // USE_LOGFILE

#include "NormalFilter.h"
#include "UParameter.h"
#include "UGenericVisualization.h"

#ifdef USE_LOGFILE
FILE *Normalfile;
#endif // USE_LOGFILE

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
   "Filtering float UD_A=  5.0 "
     "5.0  -100.0  100.0 // Normal Filter Up / Down Intercept",
   "Filtering float UD_B=  5.0 "
     "5.0  -100.0  100.0 // Normal Filter Up / Down Slope",
   "Filtering float LR_A= -5.0 "
     "-5.0  -100.0  100.0 // Normal Filter Left/Right Intercept",
   "Filtering float LR_B= 5.0 "
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

#ifdef USE_LOGFILE
  if( Normalfile != NULL )
  {
    fprintf(Normalfile,"Destructor \n");
    fclose( Normalfile );
  }
#endif // USE_LOGFILE
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
  for( size_t channel = 0; channel < inSignalProperties.Channels(); ++channel )
    PreflightCondition( inSignalProperties.GetNumElements( channel ) > 0 );

  // Requested output signal properties.
  outSignalProperties = inSignalProperties;
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the NormalFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void NormalFilter::Initialize()
{
  int visualizeyn = 0;

#ifdef USE_LOGFILE
  fprintf(Normalfile,"Initialize and try \n");
#endif // USE_LOGFILE

  ud_a= Parameter("UD_A");
  ud_b= Parameter("UD_B");
  lr_a= Parameter("LR_A");
  lr_b= Parameter("LR_B");

  visualizeyn= Parameter("VisualizeNormalFiltering");

  if( visualizeyn == 1 )
  {
    visualize=true;
    delete vis;
    vis= new GenericVisualization;
    vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_WINDOWTITLE, "Normalizer");
    vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MINVALUE, "-40");
    vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_MAXVALUE, "40");
    vis->SendCfg2Operator(SOURCEID_NORMALIZER, CFGID_NUMSAMPLES, "256");
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
void NormalFilter::Process(const GenericSignal *input, GenericSignal *output)
{
  float   val_ud;
  float   val_lr;
  float   value;

  // actually perform the Normal Filtering on the input and write it into the output signal
  int sample= 0;

  for(size_t in_channel=0; in_channel<input->Channels(); in_channel++)
  {
    value= input->GetValue(in_channel, sample);
// ??
    if( in_channel == 0 ) val_ud=  ( value - ud_a ) * ud_b;
    if( in_channel == 1 ) val_lr=  ( value - lr_a ) * lr_b;
  }
  output->SetValue( 0, 0, val_ud );
  output->SetValue( 1, 0, val_lr );

#ifdef USE_LOGFILE
  fprintf(Normalfile,"Process val_ud= %8.3f  val_lr= %8.3f\n",val_ud,val_lr);
#endif // USE_LOGFILE

  if( visualize )
  {
    vis->SetSourceID(SOURCEID_NORMALIZER);
    vis->Send2Operator( output );
  }
  return;
}

int NormalFilter::UpdateParameters( float new_ud_a, float new_ud_b, float new_lr_a, float new_lr_b )
{
  ud_a= new_ud_a;
  ud_b= new_ud_b;
  lr_a= new_lr_a;
  lr_b= new_lr_b;

  return(1);
}


