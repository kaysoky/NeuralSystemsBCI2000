//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ClassFilter.h"

#include <stdio.h>
// FILE *classfile;


//---------------------------------------------------------------------------

#pragma package(smart_init)

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
{

char line[512];

// instance=my_instance;

 vis=NULL;

 strcpy(line,"Filtering matrix MUD= 10 2  0 0 0 0 0 0 -1 -1 0 0 0 0 0 0 0 0 0 0 0 0  64  0 100  // Class Filter Additive Up / Down Weights");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line,"Filtering matrix XUD= 10 2  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  64  0 100  // Class Filter Multiplicative Up / Down Weights");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line,"Filtering matrix MLR= 10 2  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  64  0 100  // Class Filter Left / Right Weights");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line, "Visualize int VisualizeClassFiltering= 1 0 0 1  // visualize Class filtered signals (0=no 1=yes)");
 plist->AddParameter2List( line, strlen(line) );

}


// **************************************************************************
// Function:   ~ClassFilter
// Purpose:    This is the destructor for the ClassFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
ClassFilter::~ClassFilter()
{
 if( vis ) delete vis;
 vis= NULL;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the ClassFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int ClassFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm, int nbins)
{
int     i,j;
int     visualizeyn;
float   stop, start, bandwidth;

 statevector=new_statevector;
 corecomm=new_corecomm;

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {
        samples=atoi(paramlist->GetParamPtr("SampleBlockSize")->GetValue());
      // n_mat= atoi(paramlist->GetParamPtr("TransmitCh")->GetValue());
      //  m_mat= atoi(paramlist->GetParamPtr("ClassFilteredChannels")->GetValue());
        m_mat= atoi(paramlist->GetParamPtr("SpatialFilteredChannels")->GetValue());
        visualizeyn= atoi(paramlist->GetParamPtr("VisualizeClassFiltering")->GetValue() );

  // there should be a parameter for n bins- otherwise this classifier is specific to mem

  //      start=       atof(paramlist->GetParamPtr( "StartMem" )->GetValue());
  //      stop=        atof(paramlist->GetParamPtr( "StopMem" )->GetValue() );
  //      bandwidth=   atof(paramlist->GetParamPtr( "MemBandWidth" )->GetValue() );
  //      n_mat= (int)(( stop - start ) / bandwidth) + 1;

        n_mat=  nbins;

  }
 catch(...)
  {
  error.SetErrorMsg("Either SampleBlockSize, SpatialFilteredChannels, or VisualizeClassFiltering is not defined");
  return(0);
  }

  for(i=0;i<n_mat;i++)
  {
        for(j=0;j<m_mat;j++)
        {
                mat_ud[i][j]= atof( paramlist->GetParamPtr("MUD")->GetValue(i,j) );
                xat_ud[i][j]= atof( paramlist->GetParamPtr("XUD")->GetValue(i,j) );
                mat_lr[i][j]= atof( paramlist->GetParamPtr("MLR")->GetValue(i,j) );

        }
  }


 if( visualizeyn == 1 )
 {
        visualize=true;
        if (vis) delete vis;
        vis= new GenericVisualization( paramlist, corecomm);
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_WINDOWTITLE, "Classifier");
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MINVALUE, "-40");
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_MAXVALUE, "40");
        vis->SendCfg2Operator(SOURCEID_CLASSIFIER, CFGID_NUMSAMPLES, "256"); }
 else
 {
        visualize=false;
 }

 return(1);
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Class routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int ClassFilter::Process(GenericSignal *input, GenericSignal *output)
{
int     in_channel, out_channel,sample;
float   val_ud;
float   val_lr;
float   value;
float   prod;
int prodflag;

 // actually perform the Class Filtering on the input and write it into the output signal

        val_ud= 0;
        val_lr= 0;

        for(sample=0; sample<input->MaxElements; sample++)
        {
            for(in_channel=0; in_channel<input->Channels; in_channel++)
            {
                value= input->GetValue(in_channel, sample);
                val_ud+=  value * mat_ud[sample][in_channel];
                val_lr+=  value * mat_lr[sample][in_channel];
            }
        }

        prod= 1;
        prodflag= 0;

        for(in_channel=0; in_channel<input->Channels; in_channel++)
        {

            for(sample=0; sample<input->MaxElements; sample++)
            {
                value= input->GetValue(in_channel, sample);
                if( xat_ud[sample][in_channel] == 0 )  ;
                else
                {
                        prodflag++;
                        prod*=  value * xat_ud[sample][in_channel];
                }
            }
        }

        if( prodflag > 0 ) val_ud+= prod;

        output->SetValue( 0, 0, val_ud );
        output->SetValue( 1, 0, val_lr );

        if( visualize )
        {
              vis->SetSourceID(SOURCEID_CLASSIFIER);
              vis->Send2Operator(output);
        }
   return(1);
}



