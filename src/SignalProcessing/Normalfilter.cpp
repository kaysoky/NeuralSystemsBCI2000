//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "NormalFilter.h"
#include <math.h>

#include <stdio.h>
// FILE *Normalfile;


//---------------------------------------------------------------------------

#pragma package(smart_init)

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
NormalFilter::NormalFilter(PARAMLIST *plist, STATELIST *slist)
{

char line[512];

 vis=NULL;
// instance=my_instance;

 //    Normalfile= fopen("Normalfile.asc","w+");
 //    fprintf(Normalfile,"Constructor \n");

 strcpy(line,"Filtering float UD_A=  5.0 5.0  -100.0  100.0   // Normal Filter Up / Down Intercept");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line,"Filtering float UD_B=  5.0 5.0  -100.0  100.0   // Normal Filter Up / Down Slope");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line,"Filtering float LR_A= -5.0 -5.0  -100.0  100.0   // Normal Filter Left/Right Intercept");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line,"Filtering float LR_B= 5.0 5.0  0.0  100.0   // Normal Filter Left/Right Slope");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line, "Visualize int VisualizeNormalFiltering= 1 0 0 1  // visualize Normal filtered signals (0=no 1=yes)");
 plist->AddParameter2List( line, strlen(line) );

}


// **************************************************************************
// Function:   ~NormalFilter
// Purpose:    This is the destructor for the NormalFilter Normal
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
NormalFilter::~NormalFilter()
{
 if( vis ) delete vis;
 vis= NULL;

//  fprintf(Normalfile,"Destructor \n");

//  fclose( Normalfile );
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the NormalFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void NormalFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
int visualizeyn;

// fprintf(Normalfile,"Initialize and try \n");

 statevector=new_statevector;
 corecomm=new_corecomm;

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {

       ud_a= atof(paramlist->GetParamPtr("UD_A")->GetValue());
       ud_b= atof(paramlist->GetParamPtr("UD_B")->GetValue());
       lr_a= atof(paramlist->GetParamPtr("LR_A")->GetValue());
       lr_b= atof(paramlist->GetParamPtr("LR_B")->GetValue());

       visualizeyn= atoi(paramlist->GetParamPtr("VisualizeNormalFiltering")->GetValue() );

  }
 catch(...)
  { return; }

 if( visualizeyn == 1 )
 {
        visualize=true;
        if (vis) delete vis;
        vis= new GenericVisualization( paramlist, corecomm);
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
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void NormalFilter::Process(const GenericSignal *input, GenericSignal *output)
{
int     in_channel, out_channel,sample;
float   val_ud;
float   val_lr;
float   value;


 // actually perform the Normal Filtering on the input and write it into the output signal

     sample= 0;

     for(in_channel=0; in_channel<input->Channels(); in_channel++)
     {
                value= input->GetValue(in_channel, sample);
                if( in_channel == 0 ) val_ud=  ( value - ud_a ) * ud_b;
                if( in_channel == 1 ) val_lr=  ( value - lr_a ) * lr_b;
     }
     output->SetValue( 0, 0, val_ud );
     output->SetValue( 1, 0, val_lr );

 //    fprintf(Normalfile,"Process val_ud= %8.3f  val_lr= %8.3f\n",val_ud,val_lr);
  
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



