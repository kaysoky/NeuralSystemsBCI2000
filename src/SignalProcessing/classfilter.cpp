#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#undef USE_LOGFILE

#ifdef USE_LOGFILE
# include <stdio.h>
#endif // USE_LOGFILE
#include "UParameter.h"
#include "UGenericVisualization.h"
#include "ClassFilter.h"

#ifdef USE_LOGFILE
FILE *classfile;
#endif // USE_LOGFILE

RegisterFilter( ClassFilter, 2.D );

// **************************************************************************
// Function:   ClassFilter
// Purpose:    This is the constructor for the ClassFilter class
//             It is the "Classifier" or whatever one wants to call it
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
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

#ifdef USE_LOGFILE
 classfile= fopen("Classifier.asc","w+");
#endif // USE_LOGFILE
}


// **************************************************************************
// Function:   ~ClassFilter
// Purpose:    This is the destructor for the ClassFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
ClassFilter::~ClassFilter()
{
 delete vis;
#ifdef USE_LOGFILE
 fclose( classfile );
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
void ClassFilter::Preflight( const SignalProperties& inSignalProperties,
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
  outSignalProperties = SignalProperties( Parameter( "NumControlSignals" ), 1 );
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

#ifdef USE_LOGFILE
    fprintf(classfile,"%2d %2d %2d %7.3f %7.3f %7.2f ",i,vc1[i],vf1[i],val_ud,feature[0][i],wtmat[0][i]);
#endif // USE_LOGFILE

    //  solution= // need to transmit each result to statistics for LMS
  }
#ifdef USE_LOGFILE
  fprintf(classfile,"\n");
#endif // USE_LOGFILE

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

  if( visualize )
  {
    vis->SetSourceID(SOURCEID_CLASSIFIER);
    vis->Send2Operator(output);
  }
  return;
}



