#undef USE_LOGFILE
//---------------------------------------------------------------------------
#pragma hdrstop
#ifdef USE_LOGFILE
# include <stdio.h>
#endif // USE_LOGFILE
#include "UParameter.h"
#include "UGenericVisualization.h"
#include "ClassFilter.h"

#ifdef USE_LOGFILE
FILE *classfile;
#endif // USE_LOGFILE

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
ClassFilter::ClassFilter(PARAMLIST *plist, STATELIST *slist)
: vis( NULL )
{
 const char* params[] =
 {
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
 };
 const size_t numParams = sizeof( params ) / sizeof( *params );
 for( size_t i = 0; i < numParams; ++i )
   plist->AddParameter2List( params[ i ] );

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
// Function:   Initialize
// Purpose:    This function parameterizes the ClassFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void ClassFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
  statevector=new_statevector;
  corecomm=new_corecomm;

  int visualizeyn = 0;

  try // in case one of the parameters is not defined (should always be, since we requested them)
  {
    samples=atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    visualizeyn= atoi(paramlist->GetParamPtr("VisualizeClassFiltering")->GetValue() );
    n_vmat= paramlist->GetParamPtr("MUD")->GetNumValuesDimension1();
    class_mode= atoi( paramlist->GetParamPtr("ClassMode")->GetValue() );

    for(int i=0;i<n_vmat;i++)
    {
      if( class_mode == 2 )
      {
        vc1[i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,0) );
        vf1[i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,1) );
        vc2[i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,2) );
        vf2[i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,3) );
        wtmat[0][i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,4) );
      }
      else
      {
        vc1[i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,0) );
        vf1[i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,1) );
        vc2[i]= 0;
        vf2[i]= 0;
        wtmat[0][i]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,3) );
      }
    }

    n_hmat= paramlist->GetParamPtr("MLR")->GetNumValuesDimension1();

    for(int i=0;i<n_hmat;i++)
    {
      if( class_mode == 2 )
      {
        hc1[i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,0) );
        hf1[i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,1) );
        hc2[i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,2) );
        hf2[i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,3) );
        wtmat[1][i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,4) );
      }
      else
      {
        hc1[i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,0) );
        hf1[i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,1) );
        hc2[i]= 0;
        hf2[i]= 0;
        wtmat[1][i]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,3) );
      }
    }
  }
  catch(...)
  { return; }

  if( visualizeyn == 1 )
  {
    visualize=true;
    delete vis;
    vis= new GenericVisualization( paramlist, corecomm);
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_WINDOWTITLE, "Classifier");
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MINVALUE, "-40");
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MAXVALUE, "40");
    vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_NUMSAMPLES, "256");
  }
  else
  {
    visualize=false;
  }

  return;
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



