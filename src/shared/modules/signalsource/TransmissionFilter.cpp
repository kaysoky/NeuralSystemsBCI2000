////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that returns a subset of input channels in its output
//   signal.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TransmissionFilter.h"

#include "defines.h"
#include "BCIError.h"

#include <limits>

using namespace std;

RegisterFilter( TransmissionFilter, 1.2 );

TransmissionFilter::TransmissionFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Online%20Processing list TransmitChList= 4 1 2 3 4 % % % "
      "// list of transmitted channels",
  END_PARAMETER_DEFINITIONS
}

void TransmissionFilter::Preflight( const SignalProperties& Input,
                                          SignalProperties& Output ) const
{
  Output = Input;
  Output.SetName( "Transmitted Channels" );
  Output.SetChannels( Parameter( "TransmitChList" )->NumValues() );
  for( int idxOut = 0; idxOut < Parameter( "TransmitChList" )->NumValues(); ++idxOut )
  {
    string addressIn = Parameter( "TransmitChList" )( idxOut );
    int idxIn = Input.ChannelIndex( addressIn );
    if( idxIn < 0 || idxIn >= Input.Channels() )
      bcierr << "TransmitChList entry " << addressIn
             << " is not a valid channel specification"
             << endl;
    else if( !Input.ChannelLabels().IsTrivial() )
      Output.ChannelLabels()[ idxOut ] = Input.ChannelLabels()[ idxIn ];
  }
}

void TransmissionFilter::Initialize( const SignalProperties& Input,
                                     const SignalProperties& /*Output*/ )
{
  mChannelList.clear();
  for( int i = 0; i < Parameter( "TransmitChList" )->NumValues(); ++i )
    mChannelList.push_back( Input.ChannelIndex( Parameter( "TransmitChList" )( i ) ) );
}

void TransmissionFilter::Process( const GenericSignal& Input,
                                        GenericSignal& Output )
{
  for( size_t i = 0; i < mChannelList.size(); ++i )
    for( int j = 0; j < Input.Elements(); ++j )
      Output( i, j ) = Input( mChannelList[ i ], j );
}

