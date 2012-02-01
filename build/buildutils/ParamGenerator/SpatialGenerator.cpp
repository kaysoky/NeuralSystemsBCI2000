////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: The generator for spatial filtering parameters.
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
#include "SpatialGenerator.h"
#include <iostream>
#include <sstream>

using namespace std;

void
SpatialGenerator::Generate( ParamEnv& ioParams ) const
{
  int TransmitCh = ioParams( "SpatialFilterInputLabels" )->NumValues();
  enum { none, full, sparse, car, numFilterTypes };
  int SpatialFilterType = RandInt( 0, numFilterTypes - 1 );
  ioParams.Add( "SpatialFilter int SpatialFilterType= 0" );
  ioParams( "SpatialFilterType" ) = SpatialFilterType;
  int SFUseThreading = RandInt( 0, 1 );
  ioParams.Add( "SpatialFilter int SFUseThreading= 0" );
  ioParams( "SFUseThreading" ) = SFUseThreading;

  int spatialFilterOutputCh = 0;
  // The following parameter is not used by BCI2000 but provides
  // information about available channels when generating linear classifier
  // parameters later on.
  ioParams.Add( "ParamGenerator stringlist SpatialFilterLabels= 0" );
  switch( SpatialFilterType )
  {
    case none:
    case car:
      spatialFilterOutputCh = TransmitCh;
      ioParams( "SpatialFilterLabels" )->SetNumValues( spatialFilterOutputCh );
      for( int i = 0; i < TransmitCh; ++i )
        ioParams( "SpatialFilterLabels" )( i ) = string( ioParams( "SpatialFilterInputLabels" )( i ) );
      break;

    case full:
    case sparse:
      spatialFilterOutputCh = RandInt( 1, TransmitCh );
      ioParams( "SpatialFilterLabels" )->SetNumValues( spatialFilterOutputCh );
      for( int i = 0; i < spatialFilterOutputCh; ++i )
      {
        ostringstream oss;
        oss << "_" << i << "_Sf_" << i;
        ioParams( "SpatialFilterLabels" ) ( i ) = oss.str();
      }
      break;

    default:
      cerr << "Illegal SpatialFilterType" << endl;
  }
  ioParams.Add( "SpatialFilter matrix SpatialFilter= 0 0" );
  switch( SpatialFilterType )
  {
    case none:
      break;

    case full:
      ioParams( "SpatialFilter" )->SetNumRows( spatialFilterOutputCh );
      ioParams( "SpatialFilter" )->SetNumColumns( TransmitCh );
      for( int row = 0; row < spatialFilterOutputCh; ++row )
      {
        ioParams( "SpatialFilter" )->RowLabels()[row] = string( ioParams( "SpatialFilterLabels" )( row ) );
        for( int col = 0; col < TransmitCh; ++col )
          ioParams( "SpatialFilter" )( row, col ) = RandInt( -500, 500 ) * 0.001;
      }
      break;

    case sparse:
    {
      int sparseRows = RandInt( TransmitCh, 42 );
      ioParams( "SpatialFilter" )->SetNumRows( sparseRows );
      ioParams( "SpatialFilter" )->SetNumColumns( 3 );
      for( int row = 0; row < sparseRows; ++row )
      {
        ioParams( "SpatialFilter" )( row, 0 ) = string( ioParams( "SpatialFilterInputLabels" )( RandInt( 0, TransmitCh - 1 ) ) );
        if( row < spatialFilterOutputCh ) // make sure we have at least one entry for each output channel
          ioParams( "SpatialFilter" )( row, 1 ) = string( ioParams( "SpatialFilterLabels" )( row ) );
        else
          ioParams( "SpatialFilter" )( row, 1 ) = string( ioParams( "SpatialFilterLabels" )( RandInt( 0, spatialFilterOutputCh - 1 ) ) );
        ioParams( "SpatialFilter" )( row, 2 ) = RandInt( -500, 500 ) * 0.001;
      }
    } break;

    case car:
      ioParams.Add( "SpatialFilter list SpatialFilterCAROutput= 0" );
      ioParams( "SpatialFilterCAROutput" )->SetNumValues( spatialFilterOutputCh );
      for( int i = 0; i < spatialFilterOutputCh; ++i )
        ioParams( "SpatialFilterCAROutput" )( i ) = string( ioParams( "SpatialFilterLabels" )( i ) );
      break;
  }

}
