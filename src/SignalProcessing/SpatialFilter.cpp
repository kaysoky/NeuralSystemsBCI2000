//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SpatialFilter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

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
{

char line[512];

// instance=my_instance;

/* probably don't need this */

 vis=NULL;

 strcpy(line,"Filtering int SpatialFilteredChannels= 2 2 1 64  // Number of Spatially Filtered Channels");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line,"Filtering matrix SpatialFilterKernal= 2 4 1 0 0 0 0 1 0 0 0 64 -100 100  // Spatial Filter Kernal Weights");
 plist->AddParameter2List(line,strlen(line) );

 // strcpy(line,"Source int TransmitCh=      4 5 1 128  // this is the number of transmitted channels");
 // plist->AddParameter2List( line, strlen(line) );

 strcpy(line, "Visualize int VisualizeSpatialFiltering= 1 0 0 1  // visualize spatial filtered signals (0=no 1=yes)");
 plist->AddParameter2List( line, strlen(line) );

}


// **************************************************************************
// Function:   ~SpatialFilter
// Purpose:    This is the destructor for the SpatialFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SpatialFilter::~SpatialFilter()
{
 if( vis ) delete vis;
 vis= NULL;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the SpatialFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int SpatialFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
int     i,j;
int visualizeyn;

 statevector=new_statevector;
 corecomm=new_corecomm;

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {
        samples=atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
        n_mat= atoi(paramlist->GetParamPtr("TransmitCh")->GetValue());
        m_mat= atoi(paramlist->GetParamPtr("SpatialFilteredChannels")->GetValue());
        visualizeyn= atoi(paramlist->GetParamPtr("VisualizeSpatialFiltering")->GetValue() );


  }
 catch(...)
  { return(0); }

  for(i=0;i<m_mat;i++)
  {
        for(j=0;j<n_mat;j++)
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

 return(1);
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Spatial routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int SpatialFilter::Process(GenericSignal *input, GenericSignal *output)
{
int     in_channel, out_channel,sample;
float   value;

 // actually perform the Spatial Filtering on the input and write it into the output signal

        for(sample=0; sample<input->MaxElements(); sample++)
        {
            for(out_channel= 0; out_channel<output->Channels(); out_channel++)
            {
                   value= 0;
                   for(in_channel=0; in_channel<input->Channels(); in_channel++)
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
   return(1);
}



