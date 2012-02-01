////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: gschalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A type that represents a single BCI2000 parameter.
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

#include "Param.h"
#include "Brackets.h"
#include "BCIException.h"
#include "BCIAssert.h"

#include <sstream>
#include <cstdio>

using namespace std;

static const char* sDefaultValue = "0";
static const string sCommentSeparator = "//";
static const string cEmptyString = "";

const ctype<char>&
Param::ct()
{
  static const ctype<char>& _ct
    = use_facet<ctype<char> >( std::locale() );
  return _ct;
}

void
Param::tolower( string& s )
{
  for( string::iterator i = s.begin(); i != s.end(); ++i )
    *i = ct().tolower( *i );
}

void
Param::toupper( string& s )
{
  for( string::iterator i = s.begin(); i != s.end(); ++i )
    *i = ct().toupper( *i );
}

// **************************************************************************
// Function:   SetDimensions
// Purpose:    Sets the dimensions of a matrix parameter.
//             Converts the type if necessary.
//               1, 1: 1x1 matrix
//               0, 1: 0x1 matrix
//               1, 0: list of length 1
//               0, 0: list of length 0
// Parameters: inDimension1 - size in dimension 1
//             inDimension2 - size in dimension 2
// Returns:    N/A
// **************************************************************************
Param&
Param::SetDimensions( size_t inRows, size_t inCols )
{
  size_t newCols = inCols;
  if( newCols == 0 )
  {
    mType = "list";
    newCols = 1;
  }
  else
  {
    mType = "matrix";
  }
  // To preserve existing values' indices, insert/remove values as needed.
  size_t rows = NumRows(),
         cols = NumColumns();
  if( newCols > cols )
    for( size_t i = 0; i < rows; ++i )
      mValues.insert( mValues.begin() + i * newCols + cols,
                                               newCols - cols, sDefaultValue );
  else
    for( size_t i = 0; i < rows; ++i )
      mValues.erase( mValues.begin() + ( i + 1 ) * newCols,
                                   mValues.begin() + i * newCols + cols );
  // mDim1Index will be resized from SetNumValues().
  mDim2Index.Resize( newCols );
  return SetNumValues( inRows * inCols );
}

// **************************************************************************
// Function:   Param
// Purpose:    The constructor for the Param object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
Param::Param()
: mChanged( false ),
  mValues( 1, sDefaultValue )
{
}

// **************************************************************************
// Function:   Param
// Purpose:    Constructs and initializes a parameter object
// Parameters: self-explanatory
// Returns:    N/A
// **************************************************************************
Param::Param( const std::string& inName, const std::string& inSection,
              const std::string& inType, const std::string& inValue,
              const std::string& inDefaultValue, const std::string& inLowRange,
              const std::string& inHighRange, const std::string& inComment )
: mDefaultValue( inDefaultValue ),
  mLowRange( inLowRange ),
  mHighRange( inHighRange ),
  mComment( inComment ),
  mChanged( false )
{
  SetName( inName );
  SetSection( inSection );
  SetType( inType );
  SetNumValues( 1 );
  Value() = inValue;
}

// **************************************************************************
// Function:   Param
// Purpose:    Constructs and initializes a parameter object, based upon
//             a parameter string, as defined in the project outline
// Parameters: char *paramstring
// Returns:    N/A
// **************************************************************************
Param::Param( const std::string& line )
: mChanged( false )
{
  istringstream iss( line );
  if( !( iss >> *this ) )
    throw bciexception( "Invalid parameter line" );
}

// **************************************************************************
// Function:   SetNumValues
// Purpose:    Sets the number of values in this parameter
// Parameters: new number of values in this parameter
// Returns:    N/A
// **************************************************************************
Param&
Param::SetNumValues( size_t inN )
{
  if( inN > 1 && mType.find( "list" ) == mType.npos
              && mType.find( "matrix" ) == mType.npos )
    mType = "list";
  mValues.resize( inN, sDefaultValue );
  // mDim2Index will always have a size > 0.
  // If n is not a multiple of mDim2Index' size something is logically wrong.
  // However, this has not been treated as an error up to now.
  mDim1Index.Resize( inN / mDim2Index.Size() );
  mChanged = true;
  return *this;
}

// **************************************************************************
// Function:   SetSection
// Purpose:    sets the section name of the parameter
// Parameters: char pointer to the section name
// Returns:    N/A
// **************************************************************************
Param&
Param::SetSection( const std::string& s )
{
  if( mSections.empty() )
    mSections.push_back( s );
  else
    mSections[ 0 ] = s;
  mChanged = true;
  return *this;
}

// **************************************************************************
// Function:   Value
// Purpose:    bounds-checked access to a parameter's value
// Parameters: value index/indices
// Returns:    value reference
// **************************************************************************
const Param::ParamValue&
Param::Value( size_t idx ) const
{
  if( idx >= mValues.size() )
    throw bciexception( "Index " << idx << " out of range when accessing parameter " << Name() );
  return mValues[idx];
}

Param::ParamValue&
Param::Value( size_t idx )
{
  mChanged = true;
  return const_cast<Param::ParamValue&>( const_cast<const Param*>( this )->Value( idx ) );
}

const Param::ParamValue&
Param::Value( size_t row, size_t col ) const
{
  BoundsCheck( row, col );
  return Value( row * NumColumns() + col );
}

Param::ParamValue&
Param::Value( size_t row, size_t col )
{
  BoundsCheck( row, col );
  return Value( row * NumColumns() + col );
}

void
Param::BoundsCheck( size_t row, size_t col ) const
{
  if( static_cast<int>( row ) >= NumRows() )
    throw bciexception( "Row index " << row << " out of range when accessing parameter " << Name() );
  if( static_cast<int>( col ) >= NumColumns() )
    throw bciexception( "Column index" << col << " out of range when accessing parameter " << Name() );
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             parameter.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
Param::ReadFromStream( istream& is )
{
  mChanged = true;
  mSections.clear();
  mType.clear();
  mName.clear();
  mValues.clear();

  is >> ws;
  string delimiters = "\n\r";
  EncodedString value;
  // Unnamed parameters are enclosed in bracket pairs, and they omit section and name.
  bool unnamedParam = Brackets::IsOpening( is.peek() );
  if( unnamedParam )
  {
    delimiters += Brackets::ClosingMatch( is.get() );
    is >> value;
    SetType( value );
  }
  else
  {
    is >> mSections
       >> mType;
    if( is >> value && value.length() > 0 && *value.rbegin() == '=' )
      SetName( value.substr( 0, value.length() - 1 ) );
    else
      is.setstate( ios::failbit );
  }

  if( mType.find( "matrix" ) != mType.npos )
  {
    is >> mDim1Index >> mDim2Index;
    if( mDim2Index.Size() < 1 )
      mDim2Index.Resize( 1 );
  }
  else if( mType.find( "list" ) != mType.npos )
  {
    is >> mDim1Index;
    mDim2Index.Resize( 1 );
  }
  else
  {
    mDim1Index.Resize( 1 );
    mDim2Index.Resize( 1 );
  }

  bool syntaxError = !is;
  // Not all matrix/list entries are required for a parameter definition.
  mValues.resize( mDim1Index.Size() * mDim2Index.Size(), sDefaultValue );
  ValueContainer::iterator i = mValues.begin();
  while( i != mValues.end() && is.peek() != EOF
                               && delimiters.find( is.peek() ) == string::npos )
    is >> ws >> *i++;

  // Remaining elements are optional.
  string remainder;
  while( is && is.peek() != EOF && delimiters.find( is.peek() ) == string::npos )
    remainder += is.get();

  size_t commentSepPos = remainder.rfind( sCommentSeparator );
  if( commentSepPos != string::npos )
  {
    size_t commentPos = commentSepPos + sCommentSeparator.length();
    while( commentPos < remainder.size() && ct().is( ct().space, remainder[ commentPos ] ) )
      ++commentPos;
    mComment = remainder.substr( commentPos );
    remainder = remainder.substr( 0, commentSepPos ) + " ";
  }
  istringstream iss( remainder );
  EncodedString* finalEntries[] =
  {
    &mDefaultValue,
    &mLowRange,
    &mHighRange
  };
  size_t numFinalEntries = sizeof( finalEntries ) / sizeof( *finalEntries ),
         entry = 0;
  while( entry < numFinalEntries && iss >> ws >> value )
    *finalEntries[ entry++ ] = value;
  while( entry < numFinalEntries )
    *finalEntries[ entry++ ] = EncodedString( "" );

  if( unnamedParam )
    is.get();
  // Use the stream's failbit to report syntax errors.
  is.clear();
  if( syntaxError )
    is.setstate( ios::failbit );
  return is;
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             parameter.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
Param::WriteToStream( ostream& os ) const
{
  bool isUnnamed = mName.empty();
  if( isUnnamed ) // Un-named parameters are enclosed in brackets.
    os << Brackets::OpeningDefault << ' ' << mType << ' ';
  else
    os << mSections << ' ' << mType << ' ' << mName << "= ";

  if( mType == "matrix" )
    os << RowLabels() << ' ' << ColumnLabels() << ' ';
  else if( mType.find( "list" ) != mType.npos )
    os << Labels() << ' ';
  for( int i = 0; i < NumValues(); ++i )
    os << Value( i ) << ' ';
  if( !( mDefaultValue.empty() && mLowRange.empty() && mHighRange.empty() ) )
    os << mDefaultValue << ' '
       << mLowRange << ' '
       << mHighRange << ' ';
  if( !mComment.empty() )
  {
    os << sCommentSeparator << ' ' << mComment;
    if( isUnnamed )
      os << ' ';
  }
  if( isUnnamed )
    os << Brackets::ClosingDefault;

  return os;
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for input of a single
//             parameter from a binary stream, as in a parameter message.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
Param::ReadBinary( istream& is )
{
  ReadFromStream( is );
  // Some old modules out there don't send CRLF after binary Param messages.
  if( !is.eof() && ( is.get() != '\r' ) )
    is.setstate( ios::failbit );
  if( !is.eof() && ( is.get() != '\n' ) )
    is.setstate( ios::failbit );
  return is;
}

// **************************************************************************
// Function:   WriteBinary
// Purpose:    Member function for output of a single
//             parameter into a binary stream, as in a parameter message.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
Param::WriteBinary( ostream& os ) const
{
  return WriteToStream( os ).write( "\r\n", 2 );
}

// **************************************************************************
// Function:   operator=
// Purpose:    Assignment from one parameter instance to another.
// Parameters: Param instance to be assigned.
// Returns:    *this.
// **************************************************************************
Param&
Param::operator=( const Param& p )
{
  if( this != &p )
  {
    mSections = p.mSections;
    mName = p.mName;
    mType = p.mType;
    mDefaultValue = p.mDefaultValue;
    mLowRange = p.mLowRange;
    mHighRange = p.mHighRange;
    mComment = p.mComment;
    AssignValues( p );
  }
  return *this;
}

// **************************************************************************
// Function:   AssignValues
// Purpose:    Assignment of parameter values
// Parameters: Param instance to be assigned.
// Returns:    *this.
// **************************************************************************
Param&
Param::AssignValues( const Param& p )
{
  if( this != &p )
  {
    mDim1Index = p.mDim1Index;
    mDim2Index = p.mDim2Index;
    mValues = p.mValues;

    mChanged = p.mChanged;
  }
  return *this;
}


/////////////////////////////////////////////////////////////////////////////
// paramValue definitions                                                  //
/////////////////////////////////////////////////////////////////////////////
Param  Param::ParamValue::sParamBuf;
string Param::ParamValue::sStringBuf;

// **************************************************************************
// Function:   Kind
// Purpose:    Returns the kind of parameter value this instance represents.
// Parameters: kind of ParamValue.
// Returns:    N/A
// **************************************************************************
int
Param::ParamValue::Kind() const
{
  int result = Null;
  if( mpParam )
  {
    string type = mpParam->Type();
    if( type.find( "list" ) != string::npos )
      result = List;
    else if( type.find( "matrix" ) != string::npos )
      result = Matrix;
    else
      result = Single;
  }
  else if( mpString )
    result = Single;
  return result;
}

// **************************************************************************
// Function:   Assign
// Purpose:    Assigns the content of the ParamValue from another
//             instance of ParamValue.
// Parameters: ParamValue reference.
// Returns:    N/A
// **************************************************************************
void
Param::ParamValue::Assign( const ParamValue& p )
{
  if( p.mpString == NULL )
  {
    delete mpString;
    mpString = NULL;
  }
  else
  {
    EncodedString* temp = mpString;
    mpString = new EncodedString( *p.mpString );
    delete temp;
  }
  if( p.mpParam == NULL )
  {
    delete mpParam;
    mpParam = NULL;
  }
  else
  {
    Param* temp = mpParam;
    mpParam = new Param( *p.mpParam );
    delete temp;
  }
}

// **************************************************************************
// Function:   Assign
// Purpose:    Assigns the content of the ParamValue from a string value.
// Parameters: string reference.
// Returns:    N/A
// **************************************************************************
void
Param::ParamValue::Assign( const string& s )
{
  EncodedString* temp = mpString;
  mpString = new EncodedString( s );
  delete temp;
  delete mpParam;
  mpParam = NULL;
}

// **************************************************************************
// Function:   Assign
// Purpose:    Assigns the content of the ParamValue from a Param instance.
// Parameters: string reference.
// Returns:    N/A
// **************************************************************************
void
Param::ParamValue::Assign( const Param& p )
{
  Param* temp = mpParam;
  mpParam = new Param( p );
  delete temp;
  delete mpString;
  mpString = NULL;
}

// **************************************************************************
// Function:   ToString
// Purpose:    Returns a ParamValue in string form.
//             If the ParamValue is a Param, the string will be the Param
//             definition line, enclosed in brackets.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
const string&
Param::ParamValue::ToString() const
{
  const string* result = mpString;
  if( !result )
  {
    ostringstream oss;
    oss << *this;
    sStringBuf = oss.str();
    result = &sStringBuf;
  }
  return *result;
}

// **************************************************************************
// Function:   ToParam
// Purpose:    Returns a ParamValue as a Param.
//             If the ParamValue is not a Param, the result will be a single
//             valued Param.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
const Param*
Param::ParamValue::ToParam() const
{
  const Param* result = mpParam;
  if( !result )
  {
    ConstructParamBuf();
    result = &sParamBuf;
  }
  return result;
}

// **************************************************************************
// Function:   ToParam
// Purpose:    Returns a ParamValue as a Param.
//             If the ParamValue is not a Param, the result will be a single
//             valued Param.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
Param*
Param::ParamValue::ToParam()
{
  Param* result = mpParam;
  if( !result )
  {
    ConstructParamBuf();
    result = &sParamBuf;
  }
  return result;
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             parameter value.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
Param::ParamValue::WriteToStream( ostream& os ) const
{
  bciassert( !( mpString && mpParam ) );
  if( mpParam )
    os << *mpParam;
  else if( mpString )
    mpString->WriteToStream( os, Brackets::BracketPairs() );
  else
    os << EncodedString( "" );
  return os;
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             parameter value.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
Param::ParamValue::ReadFromStream( istream& is )
{
  delete mpString;
  mpString = NULL;
  delete mpParam;
  mpParam = NULL;
  if( is >> ws )
  {
    if( Brackets::IsOpening( is.peek() ) )
    {
      mpParam = new Param;
      is >> *mpParam;
    }
    else
    {
      mpString = new EncodedString;
      is >> *mpString;
    }
  }
  return is;
}

// **************************************************************************
// Function:   ConstructParamBuf
// Purpose:    Constructs a Param from a single or NULL value in a
//             static Param buffer.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
Param::ParamValue::ConstructParamBuf() const
{
  if( mpString )
  {
    sParamBuf.SetNumValues( 1 );
    sParamBuf.Value( 0 ) = *mpString;
  }
  else
    sParamBuf.SetDimensions( 0, 0 );
}


