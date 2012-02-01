////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that returns a subset of input channels in its output
//   signal.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
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
    int idxIn = static_cast<int>( Input.ChannelIndex( addressIn ) );
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
    mChannelList.push_back( static_cast<size_t>( Input.ChannelIndex( Parameter( "TransmitChList" )( i ) ) ) );
}

void TransmissionFilter::Process( const GenericSignal& Input,
                                        GenericSignal& Output )
{
  for( size_t i = 0; i < mChannelList.size(); ++i )
    for( int j = 0; j < Input.Elements(); ++j )
      Output( i, j ) = Input( mChannelList[ i ], j );
}

