//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "FIRFilter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// **************************************************************************
// Function:   FIRFilter
// Purpose:    This is the constructor for the TemporalFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
TemporalFilter::TemporalFilter(PARAMLIST *plist, STATELIST *slist)
{
         char line[512];

  //      vis= NULL;
  //      mem= NULL;

         fir= NULL;

  //     instance=my_instance;

        nPoints= 1;               // for now, a constant at 1- only one output/channel

        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"FIRFilter int FIRWindows= 2 2 1 8          // FIR- number of input blocks");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"FIRFilter int FIRDetrend= 0 0 0 2          // Detrend data?  0=no 1=mean 2= linear");
        plist->AddParameter2List(line,strlen(line) );

        strcpy(line,"FIRFilter int Integration= 1 0 0 1         // FIR result Integration 0 = mean 1 = rms ");
        plist->AddParameter2List(line,strlen(line) );

        strcpy(line,"FIRFilter int FIRFilteredChannels= 4 4 1 64  // Number of FIR Filtered Filtered Channels");
        plist->AddParameter2List(line,strlen(line) );

        strcpy(line,"FIRFilter matrix FIRFilterKernal= 4 4 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 0 64 -100 100  // Fir Filter Kernal Weights");
        plist->AddParameter2List(line,strlen(line) );

        strcpy(line, "Visualize int VisualizeTemporalFiltering= 1 0 0 1  // visualize Temporal filtered signals (0=no 1=yes)");
        plist->AddParameter2List( line, strlen(line) );

}


// **************************************************************************
// Function:   ~TemporalFilter
// Purpose:    This is the destructor for the TemporalFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TemporalFilter::~TemporalFilter()
{

       if( vis ) delete vis;
       vis= NULL;
       if( fir ) delete fir;
       fir= NULL;
 }


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the TemporalFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int TemporalFilter::Initialize(PARAMLIST *paramlist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
int     i,j;
int     visualizeyn;
char    cur_buf[256];
int     nBuf;

 statevector=new_statevector;
 corecomm=new_corecomm;

 if (fir) delete fir;
 fir= new FIR();

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {
        samples=     atoi(paramlist->GetParamPtr( "SampleBlockSize" )->GetValue());
        datawindows= atoi(paramlist->GetParamPtr( "FIRWindows" ) ->GetValue() );
        detrend=     atoi(paramlist->GetParamPtr("FIRDetrend" )->GetValue() );
        visualizeyn= atoi(paramlist->GetParamPtr("VisualizeTemporalFiltering")->GetValue() );
        integrate=   atoi(paramlist->GetParamPtr("Integration")->GetValue() );
        hz=          atoi(paramlist->GetParamPtr("SamplingRate")->GetValue());
        m_coef=      atoi(paramlist->GetParamPtr("FIRFilteredChannels")->GetValue());     // get output dim of spatial filtering
  }
 catch(...)
  { return(0); }

        n_coef= samples * datawindows;
        n_coef= paramlist->GetParamPtr("FIRFilterKernal")->GetNumValuesDimension2();
        m_coef= paramlist->GetParamPtr("FIRFilterKernal")->GetNumValuesDimension1();

  for(i=0;i<m_coef;i++)
  {
        for(j=0;j<n_coef;j++)
        {
                coeff[i][j]= atof( paramlist->GetParamPtr("FIRFilterKernal")->GetValue(i,j) );
        }
        fir->setFIR( i, n_coef, coeff[i]);      // define FIR coefficients for each channel
  }

  nBuf= samples * datawindows;

  for(i=0;i<MAX_M;i++)
        for(j=0;j<MAX_N;j++)
                datwin[i][j]= 0;


 if( visualizeyn == 1 )
 {
        visualize=true;
        vis= new GenericVisualization( paramlist, corecomm);
        vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_WINDOWTITLE, "Temporal Filter");
        sprintf(cur_buf, "%d", 50);
        vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_NUMSAMPLES, cur_buf);
   //     vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MINVALUE, "0");
   //     vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MAXVALUE, "60");
        for (i=0; i<nPoints; i++)
         {
   //      sprintf(cur_buf, "%03d %.0f", i, (float)start+(float)i*(float)bandwidth);
   //      vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_XAXISLABEL, cur_buf);
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
// Purpose:    This function applies the Temporal routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int TemporalFilter::Process(GenericSignal *input, GenericSignal *output)
{
int   out_channel;
float value[MAXDATA];
float result[MAXDATA];
float rms= 0;
float mean= 0;

int ocount,ncount;
int i,j,k;

static count= 0;

// actually perform the Temporal Filtering on the input and write it into the output signal



        winlgth= datawindows * samples;

        for(i=0;i<input->Channels;i++)
        {
                for(j=datawindows-1;j>0;j--)
                {
                        for(k=0;k<samples;k++)
                        {
                                ncount= j*samples + k;
                                ocount= (j-1)*samples + k;
                                datwin[i][ncount]= datwin[i][ocount];
                        }
                }

                count= samples;

                for(j=0;j<samples;j++)
                {
                        count--;
                        datwin[i][j]= input->GetValue(i,count);
                }

                fir->convolve( i, winlgth, datwin[i], result );

                if( integrate == 1 )
                {
                  rms= fir->rms( winlgth - (n_coef-1), result );  // sub order
                  output->SetValue( i, 0, rms );
                }
                else
                {
                        mean= fir->mean( winlgth - (n_coef-1), result );  // sub order
                        output->SetValue( i, 0, mean );
                }
        }


        if( visualize )
        {
              vis->SetSourceID(SOURCEID_TEMPORALFILT);
              vis->Send2Operator(output);
        }

   return(1);
}



