#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "FIRClassFilter.h"
#include "FIRFilter.h"

#include <stdio.h>
//FILE *classfile;


//---------------------------------------------------------------------------

#pragma package(smart_init)

RegisterFilter( FIRClassFilter, 2.D );

// **************************************************************************
// Function:   FIRClassFilter
// Purpose:    This is the constructor for the FIRClassFilter class
//             It is the "Classifier" or whatever one wants to call it
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
FIRClassFilter::FIRClassFilter()
: vis( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
   "Filtering matrix MUD= 10 2 "
     " 0  0"  // 0
     " 0  0"  // 1
     " 0  0"  // 2
     "-1 -1"  // 3
     " 0  0"  // 4
     " 0  0"  // 5
     " 0  0"  // 6
     " 0  0"  // 7
     " 0  0"  // 8
     " 0  0"  // 9
       " 64  0 100  // Class Filter Up / Down Weights",
   "Filtering matrix MLR= 10 2 "
     " 0  0"  // 0
     " 0  0"  // 1
     " 0  0"  // 2
     " 0  0"  // 3
     " 0  0"  // 4
     " 0  0"  // 5
     " 0  0"  // 6
     " 0  0"  // 7
     " 0  0"  // 8
     " 0  0"  // 9
       " 64  0 100  // Class Filter Left / Right Weights",
   "Visualize int VisualizeClassFiltering= 1 0 0 1 "
     "// visualize Class filtered signals (0=no 1=yes)",
 END_PARAMETER_DEFINITIONS
}


// **************************************************************************
// Function:   ~FIRClassFilter
// Purpose:    This is the destructor for the FIRClassFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
FIRClassFilter::~FIRClassFilter()
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
void FIRClassFilter::Preflight( const SignalProperties& inSignalProperties,
                                      SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  Parameter( "SampleBlockSize" );

  // Resource availability checks.
  /* The class filter seems not to depend on external resources. */

  // Input signal checks.
  // There should be a more thorough check here.
  for( size_t channel = 0; channel < inSignalProperties.Channels(); ++channel )
    PreflightCondition( inSignalProperties.GetNumElements( channel ) > 0 );

  // Requested output signal properties.
  outSignalProperties = SignalProperties( cNumControlSignals, 1 );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the FIRClassFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void FIRClassFilter::Initialize()
{
int     i,j;
int visualizeyn;
float stop,start,bandwidth;

        samples=Parameter("SampleBlockSize");
      // n_mat= Parameter("TransmitCh");
      //  m_mat= Parameter("ClassFilteredChannels");
        m_mat= Parameter("SpatialFilteredChannels");
        visualizeyn= Parameter("VisualizeClassFiltering");

  // there should be a parameter for n bins- otherwise this classifier is specific to mem

  //      start=       Parameter( "StartMem" );
  //      stop=        Parameter( "StopMem" );
  //      bandwidth=   Parameter( "MemBandWidth" );
  //      n_mat= (int)(( stop - start ) / bandwidth) + 1;

#if 0
        n_mat=  nbins;
#else
     n_mat = Parameter( "MUD" )->GetNumValuesDimension1();
#endif

  for(i=0;i<n_mat;i++)
  {
        for(j=0;j<m_mat;j++)
        {
                mat_ud[i][j]= Parameter("MUD",i,j);
                mat_lr[i][j]= Parameter("MLR",i,j);

        }
  }


 if( visualizeyn == 1 )
 {
        visualize=true;
        delete vis;
        vis= new GenericVisualization;
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_WINDOWTITLE, "Classifier");
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MINVALUE, "-40");
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MAXVALUE, "40");
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_NUMSAMPLES, "256"); }
 else
 {
        visualize=false;
 }
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Class routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void FIRClassFilter::Process(const GenericSignal *input, GenericSignal *output)
{
float   val_ud;
float   val_lr;
float   value;

 // actually perform the Class Filtering on the input and write it into the output signal

        val_ud= 0;
        val_lr= 0;

        for(size_t sample=0; sample<input->MaxElements(); sample++)
        {
            for(size_t in_channel=0; in_channel<input->Channels(); in_channel++)
            {
                value= input->GetValue(in_channel, sample);
                val_ud+=  value * mat_ud[sample][in_channel];
                val_lr+=  value * mat_lr[sample][in_channel];
            }
        }
        output->SetValue( 0, 0, val_ud );
        output->SetValue( 1, 0, val_lr );

        if( visualize )
        {
              vis->SetSourceID(SOURCEID_CLASSIFIER);
              vis->Send2Operator(output);
        }
}



