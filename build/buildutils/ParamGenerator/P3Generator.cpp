////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: The generator for P300 filtering parameters.
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
#include "P3Generator.h"
#include <iostream>
#include <sstream>

using namespace std;

void
P3Generator::Generate( ParamEnv& ioParams ) const
{
  float SamplingRate = ioParams( "SamplingRate" );
  int SampleBlockSize = ioParams( "SampleBlockSize" );
  ostringstream oss;
  float EpochLength = static_cast<float>( RandInt( 300, 1500 ) ),
        numBlocks = EpochLength / SampleBlockSize * SamplingRate / 1000.0f ;
  if( RandInt( 0, 1 ) )
    oss << EpochLength << "ms";
  else
    oss << ::ceil( numBlocks );
  ioParams.Add( "P3TemporalFilter float EpochLength= 0" );
  ioParams( "EpochLength" ) = oss.str();
  ioParams.Add( "Application float StimulusDuration= 1" );
  ioParams( "StimulusDuration" ) = RandInt( 1, 3 );
  ioParams.Add( "Application float PreSequenceDuration= 0" );
  ioParams( "PreSequenceDuration" ) = 2 * ioParams( "StimulusDuration" );
  ioParams.Add( "Application float PostSequenceDuration= 0" );
  ioParams( "PostSequenceDuration" ) = numBlocks + 2 * ioParams( "StimulusDuration" );

  ioParams.Add( "LinearClassifier matrix Classifier= 0 0" );
  int numClassifierRows = RandInt( 20, 200 );
  ioParams( "Classifier" )->SetNumRows( numClassifierRows );
  ioParams( "Classifier" )->SetNumColumns( 4 );
  int spatialFilterOutputCh = ioParams( "SpatialFilterLabels" )->NumValues();
  for( int i = 0; i < numClassifierRows; ++i )
  {
    int inputCh = RandInt( 1, spatialFilterOutputCh ),
        inputOffset = RandInt( 1, static_cast<int>( ::floor( numBlocks * SampleBlockSize ) ) );
    if( RandInt( 0, 1 ) ) // use units/labels?
    {
      ioParams( "Classifier" )( i, 0 ) = string( ioParams( "SpatialFilterLabels" )( inputCh - 1 ) );
      ostringstream oss;
      oss << ::floor( inputOffset / SamplingRate * 1000.0 ) << "ms";
      ioParams( "Classifier" )( i, 1 ) = oss.str();
    }
    else
    {
      ioParams( "Classifier" )( i, 0 ) = inputCh;
      ioParams( "Classifier" )( i, 1 ) = inputOffset;
    }
    ioParams( "Classifier" )( i, 2 ) = 1;
    ioParams( "Classifier" )( i, 3 ) = RandInt( -500, 500 ) * 0.001;
  }
}
