//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
//  FILE *tempfile;

#include "ARFilter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// **************************************************************************
// Function:   TemporalFilter
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

 vis= NULL;
 mem= NULL;

  //     instance=my_instance;

  //      tempfile= fopen("tempfile.asc","w+");

        strcpy(line,"MEMFilter float StartMem= -1.5 0.0 0.0  512.0   // Start of Spectrum in Hz");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"MEMFilter float StopMem= 30.0 30.0 0.0 512.0   // End of Spectrum in Hz");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"MEMFilter float deltaMem= 0.2 0.2 0.02 2.00    // Resolution (line density)");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"MEMFilter float MemBandWidth= 3.0 3.0 0.5 32.0 // Spectral Bandwidth in Hz");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"MEMFilter int MemModelOrder= 10 10 2 32        // AR model order");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"MEMFilter int MemWindows= 2 2 1 8          // AR- number of input blocks");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"MEMFilter int MemDetrend= 0 0 0 2  // Detrend data?  0=no 1=mean 2= linear");
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
       if( mem ) delete mem;
       mem= NULL;

  //      fclose( tempfile );
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
        int nBuf;

        statevector=new_statevector;
        corecomm=new_corecomm;

 if (mem) delete mem;
 mem= new MEM();

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {
        samples=     atoi(paramlist->GetParamPtr( "SampleBlockSize" )->GetValue());
        start=       atof(paramlist->GetParamPtr( "StartMem" )->GetValue());
        stop=        atof(paramlist->GetParamPtr( "StopMem" )->GetValue() );
        delta=       atof(paramlist->GetParamPtr( "deltaMem" )->GetValue() );
        bandwidth=   atof(paramlist->GetParamPtr( "MemBandWidth" )->GetValue() );
        modelorder=  atoi(paramlist->GetParamPtr( "MemModelOrder" )->GetValue() );
        datawindows= atoi(paramlist->GetParamPtr( "MemWindows" ) ->GetValue() );
        detrend=     atoi(paramlist->GetParamPtr("MemDetrend" )->GetValue() );
        visualizeyn= atoi(paramlist->GetParamPtr("VisualizeTemporalFiltering")->GetValue() );
        hz=          atoi(paramlist->GetParamPtr("SamplingRate")->GetValue());
  }
 catch(...)
  { return(0); }

  mem->setStart( start );
  mem->setStop( stop );
  mem->setDelta( delta );
  mem->setHz( hz );       // need to do something bout this !!!
  mem->setModelOrder( modelorder );
  mem->setBandWidth( bandwidth );
  mem->setTrend( detrend );

  nBins= (int)(( stop - start ) / bandwidth) + 1;
  nBuf= samples * datawindows;

  for(i=0;i<MAX_M;i++)
        for(j=0;j<nBuf;j++)
                datwin[i][j]= 0;

 if( visualizeyn == 1 )
 {
        visualize=true;
        if (vis) delete vis;
        vis= new GenericVisualization( paramlist, corecomm);
        vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_WINDOWTITLE, "Temporal Filter");
        sprintf(cur_buf, "%d", nBins);
        vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_NUMSAMPLES, cur_buf);
        vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MINVALUE, "0");
        vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_MAXVALUE, "10");
        for (i=0; i<nBins; i++)
         {
         sprintf(cur_buf, "%03d %.0f", i, (float)start+(float)i*(float)bandwidth);
         vis->SendCfg2Operator(SOURCEID_TEMPORALFILT, CFGID_XAXISLABEL, cur_buf);
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
float pwr[MAXBINS];

int ocount,ncount;
int i,j,k;

static count= 0;

 // actually perform the Temporal Filtering on the input and write it into the output signal

    //    fprintf(tempfile,"%5d ",count);
        count++;

        winlgth= datawindows * samples;

        for(i=0;i<input->Channels();i++)
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



                mem->setData( winlgth, datwin[i] );

                mem->get_mem();
                out_channel= mem->get_pwr( pwr );

            //    fprintf(tempfile,"Output Channels = %4d MaxElements= %5d \n",output->Channels,output->MaxElements);

                for(j=0;j<out_channel;j++)
                {
                      output->SetValue( i, j, pwr[j] );

                }
           //    fprintf(tempfile,"v= %8.3f p= %8.3f  ",value[0],pwr[16]);
        }

    //    fprintf(tempfile,"\n");

        if( visualize )
        {
              vis->SetSourceID(SOURCEID_TEMPORALFILT);
              vis->Send2Operator(output);
        }

   return(1);
}



