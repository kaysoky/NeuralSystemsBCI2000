//---------------------------------------------------------------------------
#pragma hdrstop
#include "UParameter.h"
#include "UGenericVisualization.h"
#include "SpatialFilter.h"

// **************************************************************************
// Function:   SpatialFilter
// Purpose:    This is the constructor for the SpatialFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
SpatialFilter::SpatialFilter(PARAMLIST *plist, STATELIST *slist)
: vis( NULL )
{
 const char* params[] =
 {
   "Filtering int SpatialFilteredChannels= 2 "
     "2 1 64 // Number of Spatially Filtered Channels",
   "Filtering matrix SpatialFilterKernal= 2 4 "
     "1 0 0 0 "
     "0 1 0 0 "
     "64 -100 100 // Spatial Filter Kernal Weights",
/*
   "Source int TransmitCh= 4 "
     "5 1 128 // this is the number of transmitted channels",
*/
   "Visualize int VisualizeSpatialFiltering= 1 "
     "0 0 1 // visualize spatial filtered signals (0=no 1=yes)",
 };
 const size_t numParams = sizeof( params ) / sizeof( *params );
 for( size_t i = 0; i < numParams; ++i )
   plist->AddParameter2List( params[ i ] );
}


// **************************************************************************
// Function:   ~SpatialFilter
// Purpose:    This is the destructor for the SpatialFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SpatialFilter::~SpatialFilter()
{
  delete vis;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the SpatialFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void SpatialFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
  statevector=new_statevector;
  corecomm=new_corecomm;

  int visualizeyn = 0;

  try // in case one of the parameters is not defined (should always be, since we requested them)
  {
    samples=atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
    n_mat= atoi(paramlist->GetParamPtr("TransmitCh")->GetValue());
    m_mat= atoi(paramlist->GetParamPtr("SpatialFilteredChannels")->GetValue());
    visualizeyn= atoi(paramlist->GetParamPtr("VisualizeSpatialFiltering")->GetValue() );
  }
  catch(...)
  { return; }

  for(int i=0;i<m_mat;i++)
  {
    for(int j=0;j<n_mat;j++)
    {
      mat[i][j]= atof( paramlist->GetParamPtr("SpatialFilterKernal")->GetValue(i,j) );
    }
  }

  if( visualizeyn == 1 )
  {
    visualize=true;
    if (vis) delete vis;
    vis= new GenericVisualization( paramlist, corecomm);
    if (vis)
    {
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_WINDOWTITLE, "Spatial Filter");
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_MINVALUE, "-40");
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_MAXVALUE, "40");
      vis->SendCfg2Operator(SOURCEID_SPATFILT, CFGID_NUMSAMPLES, "256");
    }
  }
  else
  {
    visualize=false;
  }

  return;
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Spatial routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void SpatialFilter::Process(const GenericSignal *input, GenericSignal *output)
{
  // actually perform the Spatial Filtering on the input and write it into the output signal
  for(size_t sample=0; sample<input->MaxElements(); sample++)
  {
    for(size_t out_channel= 0; out_channel<output->Channels(); out_channel++)
    {
      float value= 0;
      for(size_t in_channel=0; in_channel<input->Channels(); in_channel++)
      {
        value+= mat[out_channel][in_channel] * input->GetValue(in_channel, sample);
      }
      output->SetValue(out_channel, sample, value);
    }
  }
  if( visualize )
  {
    vis->SetSourceID(SOURCEID_SPATFILT);
    vis->Send2Operator(output);
  }
  return;
}



