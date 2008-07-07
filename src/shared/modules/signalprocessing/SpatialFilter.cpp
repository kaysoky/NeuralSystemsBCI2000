////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The SpatialFilter computes a linear transformation of its
//   input signal, given by a matrix-valued parameter.
//   In this matrix, input channels correspond to columns, and output channels
//   to rows.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <numeric>
#include "SpatialFilter.h"
#include "MeasurementUnits.h"

using namespace std;

RegisterFilter( SpatialFilter, 2.B );

SpatialFilter::SpatialFilter() :
numRows(0),
numCols(0),
mFilterMatrix(NULL)
{
 BEGIN_PARAMETER_DEFINITIONS

	"Filtering int SpatialFilterType= 1 2 0 3 "
     "// spatial filter type "
	 "0: none, "
     "1: full matrix, "
     "2: sparse matrix, "
     "3: common average reference (CAR) "
     "(enumeration)",
	"Filtering:SpatialFilter intlist SpatialFilterCAROutput= 0 % % % % "
		"// list of output channels for the CAR if used",
   "Filtering:SpatialFilter matrix SpatialFilter= 4 4 "
     "1 0 0 0 "
     "0 1 0 0 "
     "0 0 1 0 "
     "0 0 0 1 "
	 "0 % % // columns represent input channels, rows represent output channels",

 END_PARAMETER_DEFINITIONS
}


SpatialFilter::~SpatialFilter()
{
}


void
SpatialFilter::Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const
{
	Output = Input;
	Output.ChannelLabels().Clear();
	Output.ChannelUnit().Clear();
  switch( int( Parameter( "SpatialFilterType" ) ) )
  {
    case none:
      Output = Input;
      break;

      case fullMatrix:
        // Parameter/Input consistency.
        if( Input.Channels() != Parameter( "SpatialFilter" )->NumColumns() )
          bcierr << "The input signal's number of channels must match "
                 << "the number of columns in the SpatialFilter parameter"
                 << endl;
        // Output signal description.
        Output = Input;
        Output.SetChannels( 0 ).SetChannels( Parameter( "SpatialFilter" )->NumRows() );
        if( !Parameter( "SpatialFilter" )->RowLabels().IsTrivial() )
      for( int i = 0; i < Parameter( "SpatialFilter" )->NumRows(); ++i )
        Output.ChannelLabels()[ i ] = Parameter( "SpatialFilter" )->RowLabels()[ i ];

      break; // END CASE fullMatrix

   case sparseMatrix:
     if( Parameter( "SpatialFilter" )->NumColumns() != 3 )
     {
         bcierr << "The SpatialFilter parameter must have 3 columns when representing "
                << "a sparse matrix: input channel, output channel, weight"
                << endl;
       }
       else
       {
         int maxOutputIdx = 0;
         for( int i = 0; i < Parameter( "SpatialFilter" )->NumRows(); ++i )
         {
           string inputChannelAddress = Parameter( "SpatialFilter" )( i, 0 );
           int inputIdx = Input.ChannelIndex( inputChannelAddress );
           if( inputIdx < 0 || inputIdx >= Input.Channels() )
             bcierr << "Invalid input channel specification \"" << inputChannelAddress
                    << "\" in SpatialFilter, row " << i + 1
                    << endl;

           int outputIdx = Parameter( "SpatialFilter" )( i, 1 ) - 1;
           if( outputIdx < 0 )
             bcierr << "Invalid output channel (" << outputIdx + 1
                    << ") in SpatialFilter, row " << i + 1
                    << endl;
           maxOutputIdx = max( maxOutputIdx, outputIdx );
         }
         Output.SetChannels( maxOutputIdx + 1 );
     }
     break; // END CASE sparseMatrix

   case commonAverage:
     Output = Input;

     if ( Parameter( "SpatialFilterCAROutput" )->NumValues() > 0)
     {
      Output.SetChannels( Parameter( "SpatialFilterCAROutput" )->NumValues() );
       for( int i = 0; i < Parameter( "SpatialFilterCAROutput" )->NumValues(); ++i )
       {
         string inputChannelAddress = Parameter( "SpatialFilterCAROutput" )( i );
         int inputIdx = Input.ChannelIndex( inputChannelAddress );
         if( inputIdx < 0 || inputIdx > Input.Channels() )
         bcierr << "Invalid channel specification \"" << inputChannelAddress
            << "\" in SpatialFilterCAROutput(" << i << ")"
            << endl;

        //propogate the channel labels
        Output.ChannelLabels()[ i ] = inputChannelAddress;
       }
     }
     else
     {
      Output.SetChannels( Input.Channels());
      for( int i = 0; i < Input.Channels(); ++i )
        Output.ChannelLabels()[ i ] = Input.ChannelLabels()[i];
     }
     break; // END CASE commonAverage
   }
}          

void
SpatialFilter::Initialize( const SignalProperties& Input/*Input*/,
                           const SignalProperties& Output/*Output*/ )
{
  mSpatialFilterType = Parameter( "SpatialFilterType" );
	switch( mSpatialFilterType )
	{
		case none:
		{
			break;
		}
		case fullMatrix:
		{
			numRows = Parameter( "SpatialFilter" )->NumRows(),
			numCols = Parameter( "SpatialFilter" )->NumColumns();

			mFilterMatrix.resize(numRows);
			mSignalBuffer.resize(Input.Channels());
			for( size_t row = 0; row < numRows; ++row )
			{
				mFilterMatrix[row].resize(numCols);
				for( size_t col = 0; col < numCols; ++col )
					mFilterMatrix[ row ][ col ] = Parameter( "SpatialFilter" )( row, col );
			}
			break;
		}
		case sparseMatrix:
		{
			numRows = Parameter( "SpatialFilter" )->NumRows();
			numCols = Parameter( "SpatialFilter" )->NumColumns();
			mFilterMatrix.resize(numRows);
      string inputChannelAddress, outputChannelAddress;
			for( size_t row = 0; row < numRows; ++row )
			{
				mFilterMatrix[row].resize(numCols);
        inputChannelAddress = Parameter( "SpatialFilter" )( row, 0 );
        outputChannelAddress = Parameter( "SpatialFilter" )( row, 1 );
        mFilterMatrix[ row ][ 0 ] = Input.ChannelIndex( inputChannelAddress );;
        mFilterMatrix[ row ][ 1 ] = Output.ChannelIndex( outputChannelAddress );;
        mFilterMatrix[ row ][ 2 ] = Parameter( "SpatialFilter" )( row, 2 );
			}
			break;
		}

		case commonAverage:
		{
			mCARoutputList.clear();
			if (Parameter("SpatialFilterCAROutput")->NumValues() > 0)
			{
        string inputChannelAddress;
				for (int i = 0; i < Parameter("SpatialFilterCAROutput")->NumValues(); ++i)
        {
          inputChannelAddress = Parameter("SpatialFilterCAROutput")(i);
					mCARoutputList.push_back(Input.ChannelIndex(inputChannelAddress));
        }
			}
			else
			{
				for (int i = 0; i < Input.Channels(); ++i)
					mCARoutputList.push_back(i);
			}
			break;
		}
	}
}

void
SpatialFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
	switch( mSpatialFilterType )
	{
		case none:
		{
			Output = Input;
			break;
		}
		case fullMatrix:
		{
			double value;
			for( int sample = 0; sample < Input.Elements(); ++sample ){
				//copy the input signal to the buffer
				for (int inCh = 0; inCh < Input.Channels(); ++inCh)
					mSignalBuffer[inCh] = Input(inCh, sample);

				for (int outCh = 0; outCh < Output.Channels(); ++outCh)
					Output( outCh, sample ) = std::inner_product(&mSignalBuffer[0],
																&mSignalBuffer[Input.Channels()],
																&mFilterMatrix[outCh][0],
																(NumType)0);
			}
			break;
		}
		case sparseMatrix:
		{
			for (int sample = 0; sample < Input.Elements(); ++sample)
			{
				for (int ch = 0; ch < Output.Channels(); ch++)
					Output(ch, sample) = 0;
				for (unsigned int feature = 0; feature < numRows; ++ feature)
					Output(mFilterMatrix[feature][1], sample) += Input(mFilterMatrix[feature][0], sample)*mFilterMatrix[feature][2];
			}
			break;
		}

		case commonAverage:
		{
			double meanVal;
			for (int sample = 0; sample < Input.Elements(); ++sample)
			{
				for (int channel = 0; channel < Input.Channels(); ++channel)
					meanVal += Input(channel, sample);
				meanVal /= double(Input.Channels());

				float val;
				for (unsigned int outChannel = 0; outChannel < mCARoutputList.size(); outChannel++)
				{
					Output(outChannel, sample) = Input(mCARoutputList[outChannel], sample) - meanVal;
				}
			}
			break;
		}
    default:
      bcierr << "Unexpected filter type ("<< int( Parameter( "SpatialFilterType" ) ) << ")"<<endl;
      break;
	}
}


