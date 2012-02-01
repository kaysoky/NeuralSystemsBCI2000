////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A helper class that represents string labels for indexing
//   matrix and list parameters, and signals.
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

#include "LabelIndex.h"
#include "Brackets.h"
#include "PhysicalUnit.h"
#include <cmath>
#include <sstream>

using namespace std;

static string sNAString = "<n/a>";

LabelIndex::LabelIndex( const PhysicalUnit& inP )
: mNeedSync( false )
{
  Reset();
  PhysicalUnit::ValueType range = inP.RawMax() - inP.RawMin();
  mReverseIndex.resize( static_cast<size_t>( ::fabs( range ) + 1 ) );
  if( range >= 0 )
  {
    for( int i = 0; i < range + 1; ++i )
      mReverseIndex[i] = inP.RawToPhysical( inP.RawMin() + i );
  }
  else
  {
    range = -range;
    for( int i = 0; i < range + 1; ++i )
      mReverseIndex[i] = inP.RawToPhysical( inP.RawMin() - i );
  }
  mNeedSync = true;
}


// **************************************************************************
// Function:   operator[]
// Purpose:    Maps string labels to numerical indices.
// Parameters: String label.
// Returns:    Numerical index associated with the label.
// **************************************************************************
int
LabelIndex::operator[]( const string& inLabel ) const
{
  Sync();
  int retIndex = 0;
  IndexBase::iterator i = mForwardIndex.find( inLabel );
  if( i != mForwardIndex.end() )
    retIndex = i->second;
  return retIndex;
}

// **************************************************************************
// Function:   Exists
// Purpose:    Checks whether a given textual label exists in the index.
//             Needed because operator[] always returns a valid index.
// Parameters: String label.
// Returns:    Boolean value that indicates the existence of the argument.
// **************************************************************************
bool
LabelIndex::Exists( const string& inLabel ) const
{
  Sync();
  return mForwardIndex.find( inLabel ) != mForwardIndex.end();
}

// **************************************************************************
// Function:   operator[]
// Purpose:    Maps numerical indices to string labels.
// Parameters: String label.
// Returns:    String label associated with the index.
// **************************************************************************
const string&
LabelIndex::operator[]( size_t inIndex ) const
{
  const string* retString = &sNAString;
  if( inIndex < mReverseIndex.size() )
    retString = &mReverseIndex[ inIndex ];
  return *retString;
}

string&
LabelIndex::operator[]( size_t inIndex )
{
  mNeedSync = true;
  return mReverseIndex.at( inIndex );
}

// **************************************************************************
// Function:   Resize
// Purpose:    Changes the number of labels managed by this Index and
//             fills the index with default labels.
//             We don't bother deleting labels from the forward index.
// Parameters: New size.
// Returns:    N/A
// **************************************************************************
LabelIndex&
LabelIndex::Resize( size_t inNewSize )
{
  if( mForwardIndex.size() > 0 && inNewSize > mReverseIndex.size() )
    mNeedSync = true;
  while( mReverseIndex.size() < inNewSize )
    mReverseIndex.push_back( TrivialLabel( mReverseIndex.size() ) );
  mReverseIndex.resize( inNewSize );
  return *this;
}

// **************************************************************************
// Function:   Sync
// Purpose:    Rebuilds the forward index if the needSync flag is set.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
LabelIndex::Sync() const
{
  if( mNeedSync )
  {
    mForwardIndex.clear();
    for( size_t i = 0; i < mReverseIndex.size(); ++i )
      mForwardIndex[ mReverseIndex[ i ] ] = static_cast<int>( i );
    mNeedSync = false;
  }
}

// **************************************************************************
// Function:   TrivialLabel
// Purpose:    Return a trivial label associated with a given numerical index.
// Parameters: Numerical Index.
// Returns:    Label.
// **************************************************************************
string
LabelIndex::TrivialLabel( size_t index )
{
  const int trivialBase = 1; // Channels are counted from 1,
                             // so trivial labels should start with 1 to avoid
                             // user confusion.
  ostringstream oss;
  oss << index + trivialBase;
  return oss.str();
}

// **************************************************************************
// Function:   IsTrivial
// Purpose:    Check if the labels actually contain information.
// Parameters: N/A
// Returns:    bool
// **************************************************************************
bool
LabelIndex::IsTrivial() const
{
  Sync();
  bool trivial = true;
  for( size_t i = 0; trivial && i < mReverseIndex.size(); ++i )
    trivial &= ( mReverseIndex[ i ] == TrivialLabel( i ) );
  return trivial;
}

// **************************************************************************
// Function:   operator*=
// Purpose:    Perform outer multiplication with another LabelIndex.
// Parameters: LabelIndex to multiply with.
// Returns:    Result of multiplication.
// **************************************************************************
LabelIndex&
LabelIndex::operator*=( const LabelIndex& inL )
{
  IndexReverse newLabels( mReverseIndex.size() * inL.mReverseIndex.size() );
  for( size_t i = 0; i < mReverseIndex.size(); ++i )
    for( size_t j = 0; j < inL.mReverseIndex.size(); ++j )
      newLabels[mReverseIndex.size() * i + j] = mReverseIndex[i] + '&' + inL.mReverseIndex[j];
  mReverseIndex = newLabels;
  mNeedSync = true;
  return *this;
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             label Index.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
LabelIndex::ReadFromStream( istream& is )
{
  Reset();
  if( is >> ws )
  {
    // Check if the first character is an opening bracket.
    char endDelimiter = Brackets::ClosingMatch( is.peek() );
    if( endDelimiter != '\0' )
    { // The first character is an opening bracket,
      // Get the line up to the matching closing bracket.
      is.get();
      string labelsList;
      if( getline( is, labelsList, endDelimiter ) )
      {
        mReverseIndex.clear();
        istringstream labels( labelsList );
        IndexBase::key_type currentToken;
        while( labels >> currentToken )
          mReverseIndex.push_back( currentToken );
        mNeedSync = true;
      }
    }
    else
    { // There is no bracket, so let's read a plain number.
      size_t size;
      if( is >> size )
        Resize( size );
    }
  }
  return is;
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             label Index.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
LabelIndex::WriteToStream( ostream& os ) const
{
  static const char disallowedCharacters[] = { Brackets::ClosingDefault, '\0' };
  if( IsTrivial() )
    os << Size();
  else
  {
    os << Brackets::OpeningDefault << ' ';
    for( size_t i = 0; i < mReverseIndex.size(); ++i )
    {
      mReverseIndex[ i ].WriteToStream( os, disallowedCharacters );
      os << ' ';
    }
    os << Brackets::ClosingDefault;
  }
  return os;
}

