// #define USE_LOGFILE

#include "PCHIncludes.h"
#pragma hdrstop

#include "ARFilter.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef USE_LOGFILE
FILE* tempfile;
#endif // USE_LOGFILE

RegisterFilter( ARTemporalFilter, 2.C );

// **************************************************************************
// Function:   ARTemporalFilter
// Purpose:    This is the constructor for the ARTemporalFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
ARTemporalFilter::ARTemporalFilter()
: vis( NULL ),
  mem( NULL )
{
#ifdef USE_LOGFILE
  tempfile= fopen("tempfile.asc","w+");
#endif // USE_LOGFILE

 BEGIN_PARAMETER_DEFINITIONS
  "MEMFilter float StartMem= 0.0 0.0 0.0 512.0 "
      "// Start of Spectrum in Hz",
  "MEMFilter float StopMem= 30.0 30.0 0.0 512.0 "
      "// End of Spectrum in Hz",
  "MEMFilter float deltaMem= 0.2 0.2 0.02 2.00 "
      "// Resolution (line density)",
  "MEMFilter float MemBandWidth= 3.0 3.0 0.5 32.0 "
      "// Spectral Bandwidth in Hz",
  "MEMFilter int MemModelOrder= 10 10 2 32 "
      "// AR model order",
  "MEMFilter int MemWindows= 2 2 1 8 "
      "// AR- number of input blocks",
  "MEMFilter int MemDetrend= 0 0 0 2 "
      "// Detrend data?  0=no 1=mean 2= linear",
  "Visualize int VisualizeTemporalFiltering= 1 0 0 1 "
      "// visualize Temporal filtered signals (0=no 1=yes)",
 END_PARAMETER_DEFINITIONS
}

// **************************************************************************
// Function:   ~ARTemporalFilter
// Purpose:    This is the destructor for the ARTemporalFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
ARTemporalFilter::~ARTemporalFilter()
{
  delete vis;
  delete mem;
#ifdef USE_LOGFILE
  fclose( tempfile );
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
void ARTemporalFilter::Preflight( const SignalProperties& inSignalProperties,
                                        SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  Parameter( "SampleBlockSize" );
  Parameter( "SamplingRate" );

  // Resource availability checks.
  /* The normalizer filter seems not to depend on external resources. */

  // Input signal checks.
  for( size_t channel = 0; channel < inSignalProperties.Channels(); ++channel )
    PreflightCondition( inSignalProperties.GetNumElements( channel ) > 0 );

  // Requested output signal properties.
  int nBins = ( Parameter( "StopMem" ) - Parameter( "StartMem" ) ) / Parameter( "MemBandWidth" ) + 1;
  outSignalProperties = SignalProperties( inSignalProperties.Channels(), nBins );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the ARTemporalFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void ARTemporalFilter::Initialize()
{
  int     i,j;
  int     visualizeyn;
  char    cur_buf[256];
  int nBuf;

  delete mem;
  mem= new MEM();

  samples=     Parameter( "SampleBlockSize" );
  start=       Parameter( "StartMem" );
  stop=        Parameter( "StopMem" );
  delta=       Parameter( "deltaMem" );
  bandwidth=   Parameter( "MemBandWidth" );
  modelorder=  Parameter( "MemModelOrder" );
  datawindows= Parameter( "MemWindows" ) ;
  detrend=     Parameter("MemDetrend" );
  visualizeyn= Parameter("VisualizeTemporalFiltering");
  hz=          Parameter("SamplingRate");

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
        vis= new GenericVisualization;
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

 return;
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Temporal routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void ARTemporalFilter::Process(const GenericSignal *input, GenericSignal *output)
{
int   out_channel;
float value[MAXDATA];
float pwr[MAXBINS];

int ocount,ncount;
int i,j,k;

static count= 0;

 // actually perform the Temporal Filtering on the input and write it into the output signal

#ifdef USE_LOGFILE
        fprintf(tempfile,"%5d ",count);
#endif // USE_LOGFILE
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

#ifdef USE_LOGFILE
                fprintf(tempfile,"Output Channels = %4d MaxElements= %5d \n",output->Channels,output->MaxElements);
#endif // USE_LOGFILE

                for(j=0;j<out_channel;j++)
                {
                      output->SetValue( i, j, pwr[j] );

                }
#ifdef USE_LOGFILE
               fprintf(tempfile,"v= %8.3f p= %8.3f  ",value[0],pwr[16]);
#endif // USE_LOGFILE
        }

#ifdef USE_LOGFILE
        fprintf(tempfile,"\n");
#endif // USE_LOGFILE

        if( visualize )
        {
              vis->SetSourceID(SOURCEID_TEMPORALFILT);
              vis->Send2Operator(output);
        }

   return;
}



