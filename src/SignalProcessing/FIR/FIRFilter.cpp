//---------------------------------------------------------------------------

#include "PCHIncludes.h"
#pragma hdrstop

#include <stdio.h>

#include "FIRFilter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

RegisterFilter( FIRFilter, 2.C );

// **************************************************************************
// Function:   FIRFilter
// Purpose:    This is the constructor for the FIRFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
FIRFilter::FIRFilter()
: fir( NULL ),
  vis( NULL ),
  nPoints( 0 )
{
  nPoints= 1;               // for now, a constant at 1- only one output/channel

  BEGIN_PARAMETER_DEFINITIONS
    "FIRFilter int FIRWindows= 2 2 1 8 "
      "// FIR- number of input blocks",
    "FIRFilter int FIRDetrend= 0 0 0 2 "
      "// Detrend data?  0=no 1=mean 2= linear",
    "FIRFilter int Integration= 1 0 0 1 "
      "// FIR result Integration 0 = mean 1 = rms",
    "FIRFilter int FIRFilteredChannels= 4 4 1 64 "
      "// Number of FIR Filtered Filtered Channels",
    "FIRFilter matrix FIRFilterKernal= 4 4 "
      " 1 0 0 0"
      " 0 1 0 0"
      " 0 0 1 0"
      " 0 0 0 1"
             " 64 -100 100  // Fir Filter Kernal Weights",
    "Visualize int VisualizeTemporalFiltering= 1 0 0 1 "
      "// visualize Temporal filtered signals (0=no 1=yes)",
  END_PARAMETER_DEFINITIONS
}


// **************************************************************************
// Function:   ~FIRFilter
// Purpose:    This is the destructor for the FIRFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
FIRFilter::~FIRFilter()
{
  delete vis;
  delete fir;
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void FIRFilter::Preflight( const SignalProperties& inSignalProperties,
                                 SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  Parameter( "SamplingRate" );
  PreflightCondition(
    Parameter( "FIRFilteredChannels" ) == Parameter( "FIRFilterKernal" )->GetNumValuesDimension1() );
  PreflightCondition(
    Parameter( "SampleBlockSize" ) == Parameter( "FIRFilterKernal" )->GetNumValuesDimension2() );
  PreflightCondition(
     Parameter( "FIRFilterKernal" )->GetNumValuesDimension1() <= MAX_M );
  PreflightCondition(
     Parameter( "FIRFilterKernal" )->GetNumValuesDimension2() <= MAX_N );

  // Resource availability checks.
  /* The FIR filter seems not to depend on external resources. */

  // Input signal checks.
  PreflightCondition( Parameter( "FIRFilteredChannels" ) <= inSignalProperties.Channels() );

  // Requested output signal properties.
  outSignalProperties = SignalProperties( inSignalProperties.Channels(), nPoints );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the FIRFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void FIRFilter::Initialize()
{
int     i,j;
int     visualizeyn;
char    cur_buf[256];
int     nBuf;

 delete fir;
 fir= new FIR;

        samples=     Parameter( "SampleBlockSize" );
        datawindows= Parameter( "FIRWindows" );
        detrend=     Parameter("FIRDetrend" );
        visualizeyn= Parameter("VisualizeTemporalFiltering");
        integrate=   Parameter("Integration");
        hz=          Parameter("SamplingRate");
        m_coef=      Parameter("FIRFilteredChannels");     // get output dim of spatial filtering

        n_coef= samples * datawindows;
        n_coef= Parameter("FIRFilterKernal")->GetNumValuesDimension2();
        m_coef= Parameter("FIRFilterKernal")->GetNumValuesDimension1();

  for(i=0;i<m_coef;i++)
  {
        for(j=0;j<n_coef;j++)
        {
                coeff[i][j]= Parameter("FIRFilterKernal",i,j);
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
        vis= new GenericVisualization;
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
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Temporal routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void FIRFilter::Process(const GenericSignal *input, GenericSignal *output)
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
}



