////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: The generator for source module parameters.
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
////////////////////////////////////////////////////////////////////
#include "SourceGenerator.h"
#include <sstream>

using namespace std;

void
SourceGenerator::Generate( ParamEnv& ioParams ) const
{
  float SamplingRate = static_cast<float>( RandInt( 100, 500 ) );
  ioParams.Add( "Source int SamplingRate= 0" );
  ioParams( "SamplingRate" ) = SamplingRate;
  float blockDuration = RandInt( 40, 400 ) * 0.001f;
  int SampleBlockSize = static_cast<int>( blockDuration * SamplingRate );
  ioParams.Add( "Source int SampleBlockSize= 0" );
  ioParams( "SampleBlockSize" ) = SampleBlockSize;

  const int cMinCh = 4,
            cMaxCh = 64;
  int SourceCh = RandInt( cMinCh, cMaxCh );
  ioParams.Add( "Source int SourceCh= 0" );
  ioParams( "SourceCh" ) = SourceCh;

  float SourceChOffset = 0,
        SourceChGain = RandInt( 1, 100 ) * 0.01f;
  ioParams.Add( "Source floatlist SourceChOffset= 0" );
  ioParams.Add( "Source floatlist SourceChGain= 0" );
  ioParams.Add( "Source stringlist ChannelNames= 0" );
  ioParams( "SourceChOffset" )->SetNumValues( SourceCh );
  ioParams( "SourceChGain" )->SetNumValues( SourceCh );
  ioParams( "ChannelNames" )->SetNumValues( SourceCh );
  for( int i = 0; i < SourceCh; ++i )
  {
    ioParams( "SourceChOffset" )( i ) = SourceChOffset;
    ioParams( "SourceChGain" )( i ) = SourceChGain;
    ostringstream oss;
    oss << "_" << i << "Chnl_" << i;
    ioParams( "ChannelNames" )( i ) = oss.str();
  }

  int AlignChannels = RandInt( 0, 1 );
  ioParams.Add( "Source int AlignChannels= 0" );
  ioParams( "AlignChannels" ) = AlignChannels;

  int useChannelNames = RandInt( 0, 1 );
  int TransmitCh = RandInt( cMinCh, 16 );
  ioParams.Add( "Source list TransmitChList= 0" );
  ioParams( "TransmitChList" )->SetNumValues( TransmitCh );
  ioParams.Add( "ParamGenerator list SpatialFilterInputLabels= 0" );
  ioParams( "SpatialFilterInputLabels" )->SetNumValues( TransmitCh );
  for( int i = 0; i < TransmitCh; ++i )
  {
    if( useChannelNames )
    {
      ioParams( "TransmitChList" )( i ) = string( ioParams( "ChannelNames" )( RandInt( 0, SourceCh - 1 ) ) );
      ioParams( "SpatialFilterInputLabels" )( i ) = string( ioParams( "TransmitChList" )( i ) );
    }
    else
    {
      ioParams( "TransmitChList" )( i ) = RandInt( 1, SourceCh );
      ioParams( "SpatialFilterInputLabels" )( i ) = i + 1;
    }
  }
}
