/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    UParameter.cpp                                                  *
 * Comment:   This unit provides support for system-wide parameters           *
 *            and parameter lists                                             *
 * Version:   0.22                                                            *
 * Authors:   Gerwin Schalk, Juergen Mellinger                                *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.08 - 03/30/2000 - First commented version                               *
 * V0.10 - 05/12/2000 - added AddParam2List(char *paramline)                  *
 *                      CloneParameter2List now updates the values of an      *
 *                      existing parameter                                    *
 * V0.11 - 06/09/2000 - Archive flag to each parameter                        *
 *                      better validity check in ParseParameter()             *
 * V0.13 - 08/09/2000 - Parameter supports datatype matrix                    *
 * V0.14 - 09/25/2000 - load and save parameter files                         *
 * V0.16 - 04/30/2001 - sorting of parameter lists; numerous other changes    *
 * V0.17 - 06/20/2002 - introduction of const, private/protected              *
 *                      juergen.mellinger@uni-tuebingen.de                    *
 * V0.18 - 01/31/2003 - fixed bug in SaveParameterList()                      *
 * V0.19 - 01/09/2003 - completely rewrote implementation based on STL,       *
 *                      juergen.mellinger@uni-tuebingen.de                    *
 * V0.20 - 05/07/2003 - Added textual index labels for matrices and lists, jm *
 * V0.21 - 05/15/2003 - Fixed invalid iterator problem in SaveParameterList(),*
 *                      jm                                                    *
 * V0.22 - 05/30/2003 - Fixed index synchronization bug in                    *
 *                      PARAM::SetNumValues(), jm                             *
 * V0.23 - 11/24/2003 - Fixed parsing of matrices with 0x0 size               *
 *                      Preserve existing values in SetDimensions, jm         *
 * V0.24 - 11/28/2003 - Added aliases for some functions that contain         *
 *                      dimension names, e.g. PARAM::GetNumRows(),            *
 *                      PARAM::RowLabels(), jm                                *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "UParameter.h"

#include <sstream>
#include <fstream>
#include <set>
#include <assert>

using namespace std;

// This helper class defines what we accept as delimiting single-character
// symbol pairs for index lists and sub-parameters in a parameter line.
class Brackets
{
 public:
  static bool IsOpening( char c )
  {
    return ClosingMatch( c ) != '\0';
  }
  static char ClosingMatch( char c )
  {
    size_t pos = BracketPairs.find( c );
    if( pos != string::npos && !( pos & 1 ) ) // even positions are opening brackets
      return BracketPairs[ pos + 1 ];
    return '\0';
  }
  static const string BracketPairs;
  static const string OpeningDefault;
  static const string ClosingDefault;
};
const string Brackets::BracketPairs = "{}[]";
const string Brackets::OpeningDefault = BracketPairs.substr( 0, 1 );
const string Brackets::ClosingDefault = BracketPairs.substr( 1, 1 );

/////////////////////////////////////////////////////////////////////////////
// PARAMLIST definitions                                                   //
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Function:   operator[]
// Purpose:    Access a parameter by its name.
// Parameters: Parameter name.
// Returns:    Returns a reference to a parameter with a given name.
// **************************************************************************
PARAM&
PARAMLIST::operator[]( const std::string& inName )
{
  param_index::const_iterator i = mIndex.find( inName );
  if( i == mIndex.end() )
  {
    mIndex[ inName ] = size();
    resize( size() + 1 );
    i = mIndex.find( inName );
  }
  return param_container::operator[]( i->second );
}

const PARAM&
PARAMLIST::operator[]( const std::string& inName ) const
{
  static PARAM defaultParam( "", "Default" );
  const PARAM* result = &defaultParam;
  param_index::const_iterator i = mIndex.find( inName );
  if( i != mIndex.end() )
    result = &param_container::operator[]( i->second );
  return *result;
}

// **************************************************************************
// Function:   GetNumParameters
// Purpose:    Returns the number of parameters in the list
// Parameters: N/A
// Returns:    the number of parameters
// **************************************************************************
// Now defined inline as an alias to Size().

// **************************************************************************
// Function:   GetParamPtr
// Purpose:    given a parameter name, returns the pointer to a PARAM object
// Parameters: name - name of the parameter
// Returns:    pointer to a PARAM object or
//             NULL, if no parameter with this name exists in the list
// **************************************************************************
const PARAM*
PARAMLIST::GetParamPtr( const std::string& inName ) const
{
  param_index::const_iterator i = mIndex.find( inName );
  return i != mIndex.end() ? &at( i->second ) : NULL;
}

PARAM*
PARAMLIST::GetParamPtr( const std::string& inName )
{
  param_index::const_iterator i = mIndex.find( inName );
  return i != mIndex.end() ? &at( i->second ) : NULL;
}

// **************************************************************************
// Function:   GetParamPtr
// Purpose:    given an index (0..GetListCount()-1), returns the pointer to a PARAM object
// Parameters: idx - index of the parameter
// Returns:    pointer to a PARAM object or
//             NULL, if the specified index is out of range
// **************************************************************************
const PARAM*
PARAMLIST::GetParamPtr( size_t idx ) const
{
  return idx < size() ? &param_container::operator[]( idx ) : NULL;
}

PARAM*
PARAMLIST::GetParamPtr( size_t idx)
{
  return idx < size() ? &param_container::operator[]( idx ) : NULL;
}

// **************************************************************************
// Function:   Clear
// Purpose:    deletes all parameters in the parameter list
//             as a result, the list still exists, but does not contain any parameter
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
PARAMLIST::Clear()
{
  clear();
  RebuildIndex();
}

// **************************************************************************
// Function:   Add
// Purpose:    adds a new parameter to the list of parameters
//             if a parameter with the same name already exists,
//             it updates the currently stored parameter with the provided values
// Parameters: inLine - ASCII string, as defined in project description,
//                           defining this parameter
// Returns:    true if inLine is a correct parameter line, false otherwise
// **************************************************************************
bool
PARAMLIST::Add( const string& inLine )
{
  istringstream linestream( inLine );
  PARAM param;
  if( linestream >> param )
    ( *this )[ param.mName ] = param;
  return linestream;
}

// **************************************************************************
// Function:   Delete
// Purpose:    deletes a parameter of a given name in the list of parameters
//             it also frees the memory for this particular parameter
//             it does not do anything, if the parameter does not exist
// Parameters: name - name of the parameter
// Returns:    N/A
// **************************************************************************
void
PARAMLIST::Delete( const std::string& inName )
{
  if( mIndex.find( inName ) != mIndex.end() )
  {
    erase( &at( mIndex[ inName ] ) );
    RebuildIndex();
  }
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of the entire
//             parameter list.
//             For partial output, use another instance of type PARAMLIST
//             to hold the desired subset as in PARAMLIST::SaveParameterList().
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
PARAMLIST::WriteToStream( ostream& os ) const
{
  for( const_iterator i = begin(); os && i != end(); ++i )
    os << *i << '\n';
  return os;
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of the entire
//             parameter list. The list is cleared before reading.
//             For partial input, use another instance of type PARAMLIST
//             to hold the desired subset as in PARAMLIST::LoadParameterList().
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    N/A
// **************************************************************************
istream&
PARAMLIST::ReadFromStream( istream& is )
{
  clear();
  PARAM param;
  is >> ws;
  while( !is.eof() && is >> param >> ws )
    ( *this )[ param.mName ] = param;
  return is;
}

// **************************************************************************
// Function:   WriteBinary
// Purpose:    Member function for binary stream output of the entire
//             parameter list.
//             For partial output, use another instance of type PARAMLIST
//             to hold the desired subset as in PARAMLIST::SaveParameterList().
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
PARAMLIST::WriteBinary( ostream& os ) const
{
  for( const_iterator i = begin(); i != end(); ++i )
    i->WriteBinary( os );
  return os;
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for binary stream input of the entire
//             parameter list. The list is cleared before reading.
//             For partial input, use another instance of type PARAMLIST
//             to hold the desired subset as in PARAMLIST::LoadParameterList().
// Parameters: Input stream to read from.
// Returns:    N/A
// **************************************************************************
istream&
PARAMLIST::ReadBinary( istream& is )
{
  return ReadFromStream( is );
}

// **************************************************************************
// Function:   Save
// Purpose:    Saves the current list of paramters in a parameter file
// Parameters: char *filename - filename to save the list to
//             usetags - if usetags is true, then the "tag" value in each parameter
//                       determines whether the parameter should be saved
//                       if the tag value in the parameter is true, then the
//                       parameter will NOT be saved
//                       if usetags is false, then all parameters are saved
// Returns:    true - successful
//             false - error (disk full, etc.)
// **************************************************************************
bool
PARAMLIST::Save( const string& inFileName, bool inUseTags ) const
{
  ofstream file( inFileName.c_str() );
  if( !file.is_open() )
    return false;
  // If desired, exclude parameters tagged in the parameter list.
  if( inUseTags )
  {
    PARAMLIST paramsToSave;
    for( const_iterator i = begin(); i != end(); ++i )
      if( !i->mTag )
        paramsToSave[ i->mName ] = *i;
    file << paramsToSave;
  }
  else
    file << *this;
  return !file.fail();
}

// **************************************************************************
// Function:   LoadParameterList
// Purpose:    Loads the current list of parameters from a parameter file
//             It does NOT load system critical dynamic parameters (e.g., ports,
//             IP addresses)
// Parameters: char *filename - filename of the parameterlist
//             usetags - if usetags is true, then the "tag" value in each parameter
//                       determines whether the parameter should be loaded
//                       if the tag value in the parameter is "true", then the
//                       parameter will NOT be loaded
//                       if usetags is false, then all parameters are loaded
//             nonexisting - if true, load parameters, even if they currently do
//                       not exist in the list
// Returns:    true - successful
//             false - error
// **************************************************************************
bool
PARAMLIST::Load( const string& inFileName, bool inUseTags, bool inImportNonexisting )
{
  ifstream file( inFileName.c_str() );
  PARAMLIST paramsFromFile;
  file >> paramsFromFile;

  typedef set<string> nameset;
  nameset unwantedParams;

#if 1 // This section's functionality will go into operator code.
  // Exclude parameters from unwanted sections.
  const char* unwantedSections[] = { "System", };
  for( size_t j = 0; j < sizeof( unwantedSections ) / sizeof( *unwantedSections ); ++j )
    for( const_iterator i = paramsFromFile.begin(); i != paramsFromFile.end(); ++i )
      if( PARAM::strciequal( i->mSection, unwantedSections[ j ] ) )
        unwantedParams.insert( i->mName );
#endif

  // If desired, exclude parameters tagged in the main parameter list.
  if( inUseTags )
    for( const_iterator i = paramsFromFile.begin(); i != paramsFromFile.end(); ++i )
    {
      param_index::const_iterator f = mIndex.find( i->mName );
      if( f != mIndex.end() && at( f->second ).mTag )
        unwantedParams.insert( i->mName );
    }

  // If desired, exclude parameters missing from the main parameter list.
  if( !inImportNonexisting )
    for( const_iterator i = paramsFromFile.begin(); i!= paramsFromFile.end(); ++i )
      if( mIndex.find( i->mName ) == mIndex.end() )
        unwantedParams.insert( i->mName );

  for( nameset::const_iterator i = unwantedParams.begin(); i != unwantedParams.end(); ++i )
    paramsFromFile.DeleteParam( *i );

  for( const_iterator i = paramsFromFile.begin(); i != paramsFromFile.end(); ++i )
    ( *this )[ i->mName ] = *i;

  return !file.fail();
}

// **************************************************************************
// Function:   RebuildIndex
// Purpose:    Rebuilds the Name-to-Position index maintained for parameter
//             access by name.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
PARAMLIST::RebuildIndex()
{
  mIndex.clear();
  for( size_t i = 0; i < size(); ++i )
    mIndex[ ( *this )[ i ].mName ] = i;
}

/////////////////////////////////////////////////////////////////////////////
// PARAM definitions                                                       //
/////////////////////////////////////////////////////////////////////////////
static const char* sDefaultValue = "0";
static const string sCommentSeparator = "//";

const std::ctype<char>&
PARAM::ct()
{
  static const std::ctype<char>& _ct
    = std::use_facet<std::ctype<char> >( std::locale() );
  return _ct;
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
void
PARAM::SetDimensions( size_t inDimension1, size_t inDimension2 )
{
  if( inDimension2 == 0 )
    mType = "list";
  else
    mType = "matrix";
  // To preserve existing values' indices, insert/remove values as needed.
  size_t dim1 = GetNumValuesDimension1(),
  dim2 = GetNumValuesDimension2();
  if( inDimension2 > dim2 )
    for( size_t i = 0; i < dim1; ++i )
      mValues.insert( mValues.begin() + i * inDimension2 + dim2,
                                          inDimension2 - dim2, sDefaultValue );
  else
    for( size_t i = 0; i < dim1; ++i )
      mValues.erase( mValues.begin() + ( i + 1 ) * inDimension2,
                                   mValues.begin() + i * inDimension2 + dim2 );
  // mDim1Index will be resized by SetNumValues().
  mDim2Index.resize( inDimension2 );
  SetNumValues( inDimension1 * inDimension2 );
}

// **************************************************************************
// Function:   PARAM
// Purpose:    The constructor for the PARAM object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PARAM::PARAM()
: mChanged( false ),
  mArchive( false ),
  mTag( false ),
  mValues( 1, sDefaultValue )
{
}

// **************************************************************************
// Function:   PARAM
// Purpose:    Constructs and initializes a parameter object
// Parameters: self-explanatory
// Returns:    N/A
// **************************************************************************
PARAM::PARAM( const char* inName, const char* inSection,
              const char* inType, const char* inValue,
              const char* inDefaultvalue, const char* inLowrange,
              const char* inHighrange, const char* inComment )
:
  mDefaultvalue( inDefaultvalue ),
  mLowrange( inLowrange ),
  mHighrange( inHighrange ),
  mComment( inComment ),
  mChanged( false ),
  mArchive( false ),
  mTag( false )
{
  SetName( inName );
  SetSection( inSection );
  SetType( inType );
  SetValue( inValue );
}

// **************************************************************************
// Function:   PARAM
// Purpose:    Constructs and initializes a parameter object, based upon
//             a parameter string, as defined in the project outline
// Parameters: char *paramstring
// Returns:    N/A
// **************************************************************************
PARAM::PARAM( const char* line )
: mChanged( false ),
  mArchive( false ),
  mTag( false )
{
  istringstream iss( line );
  if( !( iss >> *this ) )
    throw __FUNC__ ": invalid parameter line";
}

// **************************************************************************
// Function:   ~PARAM
// Purpose:    The destructor for the PARAM object
//             it deletes the list of values that this parameter holds
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
// Now the members' standard destructors take care.

// **************************************************************************
// Function:   GetValue
// Purpose:    Returns a pointer to the FIRST value of this parameter
//             (most parameters, except the *list parameter types, only
//             have one value anyways)
// Parameters: N/A
// Returns:    char pointer to the value
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   GetNumValues
// Purpose:    Returns the number of values in this parameter
//             for most types of parameters, this will be one
// Parameters: N/A
// Returns:    number of values in this parameter
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   SetNumValues
// Purpose:    Sets the number of values in this parameter
// Parameters: new number of values in this parameter
// Returns:    N/A
// **************************************************************************
void
PARAM::SetNumValues( size_t n )
{
  if( n > 1 && mType.find( "list" ) == mType.npos
            && mType.find( "matrix" ) == mType.npos )
    mType = "list";
  mValues.resize( n, sDefaultValue );
  // mDim2Index will always have a size > 0.
  // If n is not a multiple of mDim2Index' size something is logically wrong.
  // However, this has not been treated as an error up to now.
  mDim1Index.resize( n / mDim2Index.size() );
  mChanged = true;
}

// **************************************************************************
// Function:   GetDimension1
// Purpose:    Returns the number of values in dimension1 of this parameter
//             except for parameters of type matrix, this will always be one
// Parameters: N/A
// Returns:    number of values in dimension1 of this parameter
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   GetDimension2
// Purpose:    Returns the number of values in dimension2 of this parameter
//             except for parameters of type matrix, this will always be one
// Parameters: N/A
// Returns:    number of values in dimension2 of this parameter
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   GetValue
// Purpose:    Returns a pointer to the i'th value of this parameter
//             (most parameters, except the *list parameter types, only
//             have one value anyways)
//             All versions of PARAM::GetValue() call this function.
// Parameters: idx ... index of the value
// Returns:    char pointer to the value
//             if idx is out of bounds, it returns a pointer to the first value
// **************************************************************************
const char*
PARAM::GetValue( size_t idx ) const
{
  size_t numValues = GetNumValues();
  const char* retValue = sDefaultValue;
  if( numValues != 0 )
  {
    if( idx >= numValues )
      idx = 0;
    retValue = mValues[ idx ];
  };
  return retValue;
}

// **************************************************************************
// Function:   GetValue
// Purpose:    Returns a pointer to the x1'th/x2'th value of this parameter
//             this call only makes sense, if the parameter is of type matrix
// Parameters: x1, x2 ... index of the value for both dimensions
// Returns:    char pointer to the value
//             if the idxs are out of bounds, it returns a pointer to the first value
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   SetSection
// Purpose:    sets the section name of the parameter
// Parameters: char pointer to the section name
// Returns:    N/A
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   SetType
// Purpose:    sets the type of the parameter
// Parameters: char pointer to the type name
// Returns:    N/A
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   SetName
// Purpose:    sets the name of the parameter
//             Note: parameter names are case insensitive
// Parameters: char pointer to the name
// Returns:    N/A
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   SetValue
// Purpose:    sets the (first) value of the parameter
// Parameters: char pointer to the value
// Returns:    N/A
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   SetValue
// Purpose:    sets the idx'th value of the parameter
//             All versions of PARAM::SetValue() call this function.
// Parameters: src - char pointer to the value
//             idx - index of the value (0...GetNumValues()-1)
// Returns:    N/A
// **************************************************************************
void
PARAM::SetValue( const string& value, size_t idx )
{
  if( GetNumValues() <= idx )
    SetNumValues( idx + 1 );
  mValues[ idx ] = value;
  mChanged = true;
}

// **************************************************************************
// Function:   SetValue
// Purpose:    sets the x1'th/x2'th value of the parameter
//             only makes sense, if the type of the parameter is matrix
// Parameters: src - char pointer to the value
//             x1 - index of the value (0...GetNumValuesDimension1()-1)
//             x2 - index of the value (0...GetNumValuesDimension2()-1)
// Returns:    N/A
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   GetSection
// Purpose:    Returns a pointer to this parameter's section name
// Parameters: N/A
// Returns:    char pointer to the section name
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   GetType
// Purpose:    Returns a pointer to this parameter's type
// Parameters: N/A
// Returns:    char pointer to the type
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   GetName
// Purpose:    Returns a pointer to this parameter's name
//             Note: parameter names are case insensitive
// Parameters: N/A
// Returns:    char pointer to the name
// **************************************************************************
// Now defined inline.

// **************************************************************************
// Function:   GetComment
// Purpose:    Returns a pointer to this parameter's comment
// Parameters: N/A
// Returns:    char pointer to the comment
// **************************************************************************
// Now defined inline.

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
PARAM::ReadFromStream( istream& is )
{
  mChanged = true;
  mArchive = false;
  mTag = false;
  mSection.clear();
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
    is >> value;
    SetSection( value );
    is >> value;
    SetType( value );
    if( is >> value && value.length() > 0 && *value.rbegin() == '=' )
      SetName( value.substr( 0, value.length() - 1 ) );
    else
      is.setstate( ios::failbit );
  }

  if( mType.find( "matrix" ) != mType.npos )
  {
    is >> mDim1Index >> mDim2Index;
    if( mDim2Index.size() < 1 )
      mDim2Index.resize( 1 );
  }
  else if( mType.find( "list" ) != mType.npos )
  {
    is >> mDim1Index;
    mDim2Index.resize( 1 );
  }
  else
  {
    mDim1Index.resize( 1 );
    mDim2Index.resize( 1 );
  }

  // Not all matrix/list entries are required for a parameter definition.
  mValues.resize( mDim1Index.size() * mDim2Index.size(), sDefaultValue );
  is >> ws;
  values_type::iterator i = mValues.begin();
  while( i != mValues.end() && is.peek() != EOF
                               && delimiters.find( is.peek() ) == string::npos )
    is >> *i++ >> ws;

  // The remaining elements are optional.
  string remainder;
  {
    int c = is.peek();
    while( is && c != EOF && delimiters.find( c ) == string::npos )
    {
      remainder += is.get();
      c = is.peek();
    }
  }
  size_t commentSepPos = remainder.rfind( sCommentSeparator );
  if( commentSepPos != remainder.npos )
  {
    size_t commentPos = commentSepPos + sCommentSeparator.length();
    while( commentPos < remainder.size() && ct().is( ct().space, remainder[ commentPos ] ) )
      ++commentPos;
    mComment = remainder.substr( commentPos );
    remainder = remainder.substr( 0, commentSepPos );
  }

  istringstream iss( remainder );
  EncodedString* finalEntries[] =
  {
    &mDefaultvalue,
    &mLowrange,
    &mHighrange
  };
  size_t numFinalEntries = sizeof( finalEntries ) / sizeof( *finalEntries ),
         entry = 0;
  while( entry < numFinalEntries && iss >> value >> ws )
    *finalEntries[ entry++ ] = value;
  while( entry < numFinalEntries )
    *finalEntries[ entry++ ] = EncodedString( "" );
  if( unnamedParam )
    is.get();
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
PARAM::WriteToStream( ostream& os ) const
{
  bool isUnnamed = mName.empty();
  if( isUnnamed ) // Un-named parameters are enclosed in brackets.
    os << Brackets::OpeningDefault << ' ' << mType << ' ';
  else
    os << GetSection() << ' ' << GetType() << ' ' << GetName() << "= ";

  if( mType == "matrix" )
    os << LabelsDimension1() << ' ' << LabelsDimension2() << ' ';
  else if( mType.find( "list" ) != mType.npos )
    os << Labels() << ' ';
  for( size_t i = 0; i < GetNumValues(); ++i )
    os << Value( i ) << ' ';
  if( !( mDefaultvalue.empty() && mLowrange.empty() && mHighrange.empty() ) )
    os << mDefaultvalue << ' '
       << mLowrange << ' '
       << mHighrange << ' ';
  if( !mComment.empty() )
     os << sCommentSeparator << ' ' << mComment << ' ';

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
PARAM::ReadBinary( istream& is )
{
  ReadFromStream( is );
  // Some old modules out there don't send CRLF after binary PARAM messages.
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
PARAM::WriteBinary( ostream& os ) const
{
  return WriteToStream( os ).write( "\r\n", 2 );
}

// **************************************************************************
// Function:   operator=
// Purpose:    Assignment operator to prevent setting of certain properties
//             from e.g. parameter files.
// Parameters: PARAM instance to be assigned to *this.
// Returns:    *this.
// **************************************************************************
PARAM&
PARAM::operator=( const PARAM& p )
{
  if( this != &p )
  {
    // We prevent assignment of certain values if a parameter's name
    // is set and the parameter that is to be copied has the same
    // name.
    if( mName.empty() || mName != p.mName )
    {
      mSection = p.mSection;
      mName = p.mName;
      mType = p.mType;
      mDefaultvalue = p.mDefaultvalue;
      mLowrange = p.mLowrange;
      mHighrange = p.mHighrange;
      mComment = p.mComment;
    }

    mDim1Index = p.mDim1Index;
    mDim2Index = p.mDim2Index;
    mValues = p.mValues;

    mChanged = p.mChanged;
    mArchive = p.mArchive;
    mTag = p.mTag;
  }
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
// labelIndexer definitions                                                //
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Function:   operator[]
// Purpose:    Maps string labels to numerical indices.
// Parameters: String label.
// Returns:    Numerical index associated with the label.
// **************************************************************************
PARAM::indexer_base::mapped_type
PARAM::labelIndexer::operator[]( const string& label ) const
{
  sync();
  indexer_base::mapped_type retIndex = 0;
  indexer_base::iterator i = forwardIndex.find( label );
  if( i != forwardIndex.end() )
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
PARAM::labelIndexer::Exists( const string& label ) const
{
  sync();
  return forwardIndex.find( label ) != forwardIndex.end();
}

// **************************************************************************
// Function:   operator[]
// Purpose:    Maps numerical indices to string labels.
// Parameters: String label.
// Returns:    String label associated with the index.
// **************************************************************************
const string&
PARAM::labelIndexer::operator[]( size_t index ) const
{
  static string naString( "N/A" );
  const string* retString = &naString;
  if( index < reverseIndex.size() )
    retString = &reverseIndex[ index ];
  return *retString;
}

string&
PARAM::labelIndexer::operator[]( size_t index )
{
  needSync = true;
  return reverseIndex.at( index );
}

// **************************************************************************
// Function:   resize
// Purpose:    Changes the number of labels managed by this indexer and
//             fills the index with default labels.
//             We don't bother deleting labels from the forward index.
// Parameters: New size.
// Returns:    N/A
// **************************************************************************
void
PARAM::labelIndexer::resize( size_t newSize )
{
  if( forwardIndex.size() > 0 && newSize > reverseIndex.size() )
    needSync = true;
  while( reverseIndex.size() < newSize )
    reverseIndex.push_back( TrivialLabel( reverseIndex.size() ) );
  reverseIndex.resize( newSize );
}

// **************************************************************************
// Function:   sync
// Purpose:    Rebuilds the forward index if the needSync flag is set.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
PARAM::labelIndexer::sync() const
{
  if( needSync )
  {
    forwardIndex.clear();
    for( size_t i = 0; i < reverseIndex.size(); ++i )
      forwardIndex[ reverseIndex[ i ] ] = i;
    needSync = false;
  }
}

// **************************************************************************
// Function:   TrivialLabel
// Purpose:    Return a trivial label associated with a given numerical index.
// Parameters: Numerical Index.
// Returns:    Label.
// **************************************************************************
const string&
PARAM::labelIndexer::TrivialLabel( size_t index )
{
  typedef map<size_t, string> buffer;
  static buffer labelBuffer;
  if( labelBuffer[ index ] == "" )
  {
    // This should be the only place where a statement
    // is made about how a trivial label is formed.
    const int trivialBase = 1; // Channels are counted from 1,
                               // so trivial labels should start with 1 to avoid
                               // user confusion.
    ostringstream oss;
    oss << index + trivialBase;
    labelBuffer[ index ] = oss.str();
  }
  return labelBuffer[ index ];
}

// **************************************************************************
// Function:   IsTrivial
// Purpose:    Check if the labels actually contain information.
// Parameters: N/A
// Returns:    bool
// **************************************************************************
bool
PARAM::labelIndexer::IsTrivial() const
{
  sync();
  bool trivial = true;
  for( size_t i = 0; trivial && i < reverseIndex.size(); ++i )
    trivial &= ( reverseIndex[ i ] == TrivialLabel( i ) );
  return trivial;
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             label indexer.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
PARAM::labelIndexer::ReadFromStream( istream& is )
{
  clear();
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
        reverseIndex.clear();
        istringstream labels( labelsList );
        indexer_base::key_type currentToken;
        while( labels >> currentToken )
          reverseIndex.push_back( currentToken );
        needSync = true;
      }
    }
    else
    { // There is no bracket, so let's read a plain number.
      size_t size;
      if( is >> size )
        resize( size );
    }
  }
  return is;
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             label indexer.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
PARAM::labelIndexer::WriteToStream( ostream& os ) const
{
  if( IsTrivial() )
    os << size();
  else
  {
    os << Brackets::OpeningDefault << ' ';
    for( size_t i = 0; i < reverseIndex.size(); ++i )
    {
      reverseIndex[ i ].WriteToStream( os, Brackets::ClosingDefault );
      os << ' ';
    }
    os << Brackets::ClosingDefault;
  }
  return os;
}

/////////////////////////////////////////////////////////////////////////////
// paramValue definitions                                                 //
/////////////////////////////////////////////////////////////////////////////
PARAM  PARAM::paramValue::sParamBuf;
string PARAM::paramValue::sStringBuf;

// **************************************************************************
// Function:   Kind
// Purpose:    Returns the kind of parameter value this instance represents.
// Parameters: kind of paramValue.
// Returns:    N/A
// **************************************************************************
int
PARAM::paramValue::Kind() const
{
  int result = Null;
  if( mpParam )
  {
    string type = mpParam->GetType();
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
// Purpose:    Assigns the content of the paramValue from another
//             instance of paramValue.
// Parameters: paramValue reference.
// Returns:    N/A
// **************************************************************************
void
PARAM::paramValue::Assign( const paramValue& p )
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
    PARAM* temp = mpParam;
    mpParam = new PARAM( *p.mpParam );
    delete temp;
  }
}

// **************************************************************************
// Function:   Assign
// Purpose:    Assigns the content of the paramValue from a string value.
// Parameters: string reference.
// Returns:    N/A
// **************************************************************************
void
PARAM::paramValue::Assign( const string& s )
{
  EncodedString* temp = mpString;
  mpString = new EncodedString( s );
  delete temp;
  delete mpParam;
  mpParam = NULL;
}

// **************************************************************************
// Function:   Assign
// Purpose:    Assigns the content of the paramValue from a PARAM instance.
// Parameters: string reference.
// Returns:    N/A
// **************************************************************************
void
PARAM::paramValue::Assign( const PARAM& p )
{
  PARAM* temp = mpParam;
  mpParam = new PARAM( p );
  delete temp;
  delete mpString;
  mpString = NULL;
}

// **************************************************************************
// Function:   ToString
// Purpose:    Returns a paramValue in string form.
//             If the paramValue is a PARAM, the string will be the PARAM
//             definition line, enclosed in brackets.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
const string&
PARAM::paramValue::ToString() const
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
// Purpose:    Returns a paramValue as a PARAM.
//             If the paramValue is not a PARAM, the result will be a single
//             valued PARAM.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
const PARAM*
PARAM::paramValue::ToParam() const
{
  const PARAM* result = mpParam;
  if( !result )
  {
    ConstructParamBuf();
    result = &sParamBuf;
  }
  return result;
}

// **************************************************************************
// Function:   ToParam
// Purpose:    Returns a paramValue as a PARAM.
//             If the paramValue is not a PARAM, the result will be a single
//             valued PARAM.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PARAM*
PARAM::paramValue::ToParam()
{
  PARAM* result = mpParam;
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
PARAM::paramValue::WriteToStream( ostream& os ) const
{
  assert( !( mpString && mpParam ) );
  if( mpParam )
    os << *mpParam;
  else if( mpString )
    mpString->WriteToStream( os, Brackets::BracketPairs );
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
PARAM::paramValue::ReadFromStream( istream& is )
{
  delete mpString;
  mpString = NULL;
  delete mpParam;
  mpParam = NULL;
  if( is >> ws )
  {
    if( Brackets::IsOpening( is.peek() ) )
    {
      mpParam = new PARAM;
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
// Purpose:    Constructs a PARAM from a single or NULL value in a
//             static PARAM buffer.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
PARAM::paramValue::ConstructParamBuf() const
{
  if( mpString )
  {
    sParamBuf.SetNumValues( 1 );
    sParamBuf.Value( 0 ) = *mpString;
  }
  else
    sParamBuf.SetDimensions( 0, 0 );
}


