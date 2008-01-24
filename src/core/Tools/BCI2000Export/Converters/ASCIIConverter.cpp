//////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BCIReader class for data file output in ASCII format.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ASCIIConverter.h"
#include "GenericSignal.h"
#include "State.h"
#include "BCIError.h"

#include <string>
#include <iomanip>

using namespace std;

ASCIIConverter::ASCIIConverter( int precision )
: mPrecision( precision )
{
}

ASCIIConverter::~ASCIIConverter()
{
  ExitOutput();
}

void
ASCIIConverter::InitOutput( OutputInfo& inInfo )
{
  const string bciExtension = ".dat";
  string  dataFileName = ".ascii",
          baseName( inInfo.name ),
          lowerBaseName( baseName );
  for( string::iterator i = lowerBaseName.begin(); i != lowerBaseName.end(); ++i )
      *i = tolower( *i );
  int lengthDiff = baseName.length() - bciExtension.length();
  if( ( lengthDiff > 0 ) && ( lowerBaseName.substr( lengthDiff ) == bciExtension ) )
      baseName = baseName.substr( 0, lengthDiff );

  dataFileName = baseName + dataFileName;

  {
    ifstream input( dataFileName.c_str() );
    if( input.is_open() )
    {
      bcierr << "Data file \"" << dataFileName << "\" exists, will not be touched.\n\n"
             << "Aborting conversion." << endl;
      return;
    }
  }

  mDataFile.open( dataFileName.c_str(), ios_base::out | ios_base::binary );
  if( !mDataFile.is_open() )
  {
      bcierr << "Could not open \"" << dataFileName << "\" for writing.\n\n"
             << "Make sure you have write access to the folder containing "
             << "the input files." << endl;
      return;
  }

  for( unsigned long channel = 0; channel < inInfo.numChannels; ++channel )
  {
    if( ( *inInfo.channelNames )[ channel ].empty() )
      mDataFile << " Ch" << channel + 1;
    else
      mDataFile << ' ' << ( *inInfo.channelNames )[ channel ];
  }

  mStateValues.resize( inInfo.stateNames->size() );
  for( unsigned long state = 0; state < inInfo.stateNames->size(); ++state )
  {
    mDataFile << ' ' << ( *inInfo.stateNames )[ state ];
    mStateIndices[ ( *inInfo.stateNames )[ state ] ] = state;
    mStateValues[ state ] = 0;
  }
  mDataFile << scientific << endl;
  if( mPrecision != defaultPrecision )
     mDataFile << setprecision( mPrecision );

  if( !mDataFile )
    bcierr << "Error writing to \"" << dataFileName << "\""<< endl;
}

void
ASCIIConverter::ExitOutput()
{
  mDataFile.close();
}

void
ASCIIConverter::OutputSignal( const GenericSignal& inSignal, long /*inSamplePos*/ )
{
  Idle();
  for( int sample = 0; sample < inSignal.Elements(); ++sample )
  {
    for( int channel = 0; channel < inSignal.Channels(); ++channel )
    {
      float value = inSignal( channel, sample );
      mDataFile << ' ' << value;
    }
    for( size_t state = 0; state < mStateValues.size(); ++state )
      mDataFile << ' ' << mStateValues[ state ];

    mDataFile << '\n';
  }
  if( !mDataFile )
    bcierr << "Error writing data file" << endl;
}

void
ASCIIConverter::OutputStateChange( const State& inState, short inValue, long /*inSamplePos*/ )
{
  Idle();
  mStateValues[ mStateIndices[ inState.Name() ] ] = inValue;
}

