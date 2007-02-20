/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//------------------------------------------------------------------------------

#include "UParameter.h"
#include "UGenericVisualization.h"
#include "ClassFilter.h"
#include "UBCIError.h"


RegisterFilter( ClassFilter, 2.D );

// *****************************************************************************
// Function:   ClassFilter
// Purpose:    This is the constructor for the ClassFilter class
//             It is the "Classifier" or whatever one wants to call it
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// *****************************************************************************

ClassFilter::ClassFilter()
: vis( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
   "Filtering matrix MUD= 2 5 "
     "1 5 0 0 -1 "
     "2 5 0 0 -1 "
     "64  0 100 // Class Filter Additive Up / Down Weights",
   "Filtering matrix MLR= 2 5 "
     "0 0 0 0 0 "
     "0 0 0 0 0 "
     "64  0 100 // Class Filter Left / Right Weights",
   "Filtering int ClassMode= 1 0 1 2 "
     "// Classifier mode 1= simple 2= interaction",
   "Visualize int VisualizeClassFiltering= 1 0 0 1  "
     "// visualize Class filtered signals (0=no 1=yes)",
 END_PARAMETER_DEFINITIONS

 for (int i=0; i<cNumControlSignals; i++)
  {
  wtmat[i]=NULL;
  feature[i]=NULL;
  }
//  BEGIN_STATE_DEFINITIONS
//    "ARVal 16 0 0 0",
//  END_STATE_DEFINITIONS

}


// *****************************************************************************
// Function:   ~ClassFilter
// Purpose:    This is the destructor for the ClassFilter class
// Parameters: N/A
// Returns:    N/A
// *****************************************************************************

ClassFilter::~ClassFilter()
{
 delete vis;

 for (int i=0; i<cNumControlSignals; i++)
  {
  delete [] wtmat[i];
  delete [] feature[i];
  }
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************

void ClassFilter::Preflight( const SignalProperties& inSignalProperties,
                                   SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  Parameter( "SampleBlockSize" );

  // Resource availability checks.

#if 0 // This is disabled because it breaks existing parameter files.
  // Input signal checks.
  for( size_t i = 0; i < Parameter( "MUD" )->GetNumValuesDimension1(); ++i )
  {
    PreflightCondition( Parameter( "MUD", i, 0 ) > 0 ); // Unlike the MLR case,
                                                        // equality is not
                                                        // allowed for by Process().
    PreflightCondition( Parameter( "MUD", i, 0 ) <= inSignalProperties.Channels() );
    PreflightCondition( Parameter( "MUD", i, 1 ) >= 0 );
    PreflightCondition( Parameter( "MUD", i, 1 ) <= inSignalProperties.Elements() );
    if( Parameter( "ClassMode" ) == 2 )
    {
      PreflightCondition( Parameter( "MUD", i, 2 ) >= 0 );
      PreflightCondition( Parameter( "MUD", i, 2 ) <= inSignalProperties.Channels() );
      PreflightCondition( Parameter( "MUD", i, 3 ) >= 0 );
      PreflightCondition( Parameter( "MUD", i, 3 ) <= inSignalProperties.Elements() );
    }
  }
  for( size_t i = 0; i < Parameter( "MLR" )->GetNumValuesDimension1(); ++i )
  {
    PreflightCondition( Parameter( "MLR", i, 0 ) >= 0 );
    PreflightCondition( Parameter( "MLR", i, 0 ) <= inSignalProperties.Channels() );
    PreflightCondition( Parameter( "MLR", i, 1 ) >= 0 );
    PreflightCondition( Parameter( "MLR", i, 1 ) <= inSignalProperties.Elements() );
    if( Parameter( "ClassMode" ) == 2 )
    {
      PreflightCondition( Parameter( "MLR", i, 2 ) >= 0 );
      PreflightCondition( Parameter( "MLR", i, 2 ) <= inSignalProperties.Channels() );
      PreflightCondition( Parameter( "MLR", i, 3 ) >= 0 );
      PreflightCondition( Parameter( "MLR", i, 3 ) <= inSignalProperties.Elements() );
    }
  }
#endif
  // Requested output signal properties.
  outSignalProperties = SignalProperties( cNumControlSignals, 1 );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the ClassFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void ClassFilter::Initialize()
{
  samples=Parameter( "SampleBlockSize" );
  visualize= ( int )Parameter( "VisualizeClassFiltering" );
  n_vmat= Parameter( "MUD" )->GetNumValuesDimension1();
  class_mode= Parameter( "ClassMode" );

  n_vmat= Parameter( "MUD" )->GetNumValuesDimension1();

  vc1.resize( n_vmat );
  vf1.resize( n_vmat );
  vc2.resize( n_vmat );
  vf2.resize( n_vmat );
  delete [] wtmat[ 0 ];
  wtmat[0] = new float[ n_vmat ];
  delete [] feature[ 0 ];
  feature[ 0 ] = new float[ n_vmat ];

  for(int i=0;i<n_vmat;i++)
  {
    if( class_mode == 2 )
    {
      vc1[i]= Parameter( "MUD", i, 0 );
      vf1[i]= Parameter( "MUD", i, 1 );
      vc2[i]= Parameter( "MUD", i, 2 );
      vf2[i]= Parameter( "MUD", i, 3 );
      wtmat[0][i]= Parameter( "MUD", i, 4 );
    }
    else
    {
      vc1[i]= Parameter( "MUD", i, 0 );
      vf1[i]= Parameter( "MUD", i, 1 );
      vc2[i]= 0;
      vf2[i]= 0;
      wtmat[0][i]= Parameter( "MUD", i, 2 );
    }
  }

  n_hmat= Parameter( "MLR" )->GetNumValuesDimension1();

  hc1.resize( n_hmat );
  hf1.resize( n_hmat );
  hc2.resize( n_hmat );
  hf2.resize( n_hmat );
  delete [] wtmat[ 1 ];
  wtmat[ 1 ] = new float [ n_hmat ];
  delete [] feature [ 1 ];
  feature[ 1 ] = new float [ n_hmat ];

  for(int i=0;i<n_hmat;i++)
  {
    if( class_mode == 2 )
    {
      hc1[i]= Parameter( "MLR", i, 0 );
      hf1[i]= Parameter( "MLR", i, 1 );
      hc2[i]= Parameter( "MLR", i, 2 );
      hf2[i]= Parameter( "MLR", i, 3 );
      wtmat[1][i]= Parameter( "MLR", i, 4 );
    }
    else
    {
      hc1[i]= Parameter( "MLR", i, 0 );
      hf1[i]= Parameter( "MLR", i, 1 );
      hc2[i]= 0;
      hf2[i]= 0;
      wtmat[1][i]= Parameter( "MLR", i, 2 );
    }
  }

  if( visualize )
  {
    delete vis;
    vis= new GenericVisualization;
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_WINDOWTITLE, "Classifier");
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MINVALUE, "-40");
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MAXVALUE, "40");
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_NUMSAMPLES, "256");
  }
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Class routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void ClassFilter::Process(const GenericSignal *input, GenericSignal *output)
{
  float   val_ud = 0;
  float   val_lr = 0;
  float   term1,term2;

  // actually perform the Class Filtering on the input and write it into the output signal

  for(int i=0;i<n_vmat;i++)       // need to get vmat - number of vertical classifying functions
  {
    term1= input->GetValue( vc1[i]-1, vf1[i]-1 );

    if( ( vc2[i] > 0  ) && ( vf2[i] > 0 ) )
    term2= input->GetValue( vc2[i]-1, vf2[i]-1 );
    else    term2= 1.0;

    feature[0][i]= term1 * term2;

    val_ud+= feature[0][i] * wtmat[0][i];


    //  solution= // need to transmit each result to statistics for LMS
  }

  for(int i=0;i<n_hmat;i++)
  {
    if( ( hc1[i]>0 ) && (hf1[i] > 0 ) )
    term1= input->GetValue( hc1[i]-1, hf1[i]-1 );
    else term1= 0;

    if( ( hc2[i] > 0  ) && ( hf2[i] > 0 ) )
    term2= input->GetValue( hc2[i]-1, hf2[i]-1 );
    else    term2= 1.0;

    feature[1][i]=  term1 * term2;

    val_lr+= feature[1][i] * wtmat[1][i];

    //  solution= // need to transmit each result to statistics for LMS
  }

  output->SetValue( 0, 0, val_ud );
  output->SetValue( 1, 0, val_lr );

 // State("ARVal")= (int)(val_ud*1000);

  if( visualize )
  {
    vis->SetSourceID(SOURCEID_CLASSIFIER);
    vis->Send2Operator(output);
  }
  return;
}





