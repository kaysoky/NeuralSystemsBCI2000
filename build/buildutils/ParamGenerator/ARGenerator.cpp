////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: The generator for AR filtering parameters.
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
#include "ARGenerator.h"
#include <iostream>
#include <sstream>

using namespace std;

void
ARGenerator::Generate( ParamEnv& ioParams ) const
{
  float SamplingRate = ioParams( "SamplingRate" );

  float WindowLength = static_cast<float>( RandInt( 5, 10 ) ) * 0.1f;
  ioParams.Add( "ARFilter float WindowLength= 0" );
  ioParams( "WindowLength" ) = ::ceil( WindowLength / ioParams( "SampleBlockSize" ) * SamplingRate );

  int Detrend = RandInt( 0, 2 );
  ioParams.Add( "ARFilter int Detrend= 0" );
  ioParams( "Detrend" ) = Detrend;

  int ModelOrder = RandInt( 16, 25 );
  ioParams.Add( "ARFilter int ModelOrder= 0" );
  ioParams( "ModelOrder" ) = ModelOrder;

  enum { amplitudeSpectrum, powerSpectrum, coefficients };
  int OutputType = RandInt( 0, 2 );
  ioParams.Add( "ARFilter int OutputType= 0" );
  ioParams( "OutputType" ) = OutputType;

  int EvaluationsPerBin = RandInt( 10, 60 );
  ioParams.Add( "ARFilter int EvaluationsPerBin= 0" );
  ioParams( "EvaluationsPerBin" ) = EvaluationsPerBin;

  float FirstBinCenter = static_cast<float>( RandInt( 0, 25 ) ) * 0.01f,
        LastBinCenter = static_cast<float>( RandInt( 26, 49 ) ) * 0.01f,
        BinWidth = static_cast<float>( RandInt( 18, 35 ) ) * 0.001f;
  int numBins = OutputType == coefficients ?
                ModelOrder :
                static_cast<int>( ::floor( ( LastBinCenter - FirstBinCenter ) / BinWidth ) );
  ioParams.Add( "ARFilter float FirstBinCenter= 0" );
  ioParams.Add( "ARFilter float LastBinCenter= 0" );
  ioParams.Add( "ARFilter float BinWidth= 0" );
  if( RandInt( 0, 1 ) ) // use units in AR configuration
  {
    ostringstream oss;
    oss << FirstBinCenter * SamplingRate << "Hz";
    ioParams( "FirstBinCenter" ) = oss.str();
    oss.str( "" );
    oss << LastBinCenter * SamplingRate << "Hz";
    ioParams( "LastBinCenter" ) = oss.str();
    oss.str( "" );
    oss << BinWidth * SamplingRate << "Hz";
    ioParams( "BinWidth" ) = oss.str();
  }
  else
  {
    ioParams( "FirstBinCenter" ) = FirstBinCenter;
    ioParams( "LastBinCenter" ) = LastBinCenter;
    ioParams( "BinWidth" ) = BinWidth;
  }

  ioParams.Add( "LinearClassifier matrix Classifier= 0 0" );
  int numClassifierRows = RandInt( 1, 6 );
  ioParams( "Classifier" )->SetNumRows( numClassifierRows );
  ioParams( "Classifier" )->SetNumColumns( 4 );
  int spatialFilterOutputCh = ioParams( "SpatialFilterLabels" )->NumValues();
  for( int i = 0; i < numClassifierRows; ++i )
  {
    int inputCh = RandInt( 1, spatialFilterOutputCh ),
        inputBin = RandInt( 1, numBins ),
        outputCh = ( i == 0 ) ? 2 : RandInt( 1, 2 );
    if( OutputType != coefficients && RandInt( 0, 1 ) ) // use units/labels?
    {
      ioParams( "Classifier" )( i, 0 ) = string( ioParams( "SpatialFilterLabels" )( inputCh - 1 ) );
      ostringstream oss;
      oss << ( FirstBinCenter + ( inputBin - 1 ) * BinWidth ) * SamplingRate << "Hz";
      ioParams( "Classifier" )( i, 1 ) = oss.str();
    }
    else
    {
      ioParams( "Classifier" )( i, 0 ) = inputCh;
      ioParams( "Classifier" )( i, 1 ) = inputBin;
    }
    ioParams( "Classifier" )( i, 2 ) = outputCh;
    ioParams( "Classifier" )( i, 3 ) = RandInt( -500, 500 ) * 0.001;
  }
}
