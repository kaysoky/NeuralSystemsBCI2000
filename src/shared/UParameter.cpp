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

#include <fstream>
#include <set>

using namespace std;
/////////////////////////////////////////////////////////////////////////////
// PARAMLIST definitions                                                   //
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Function:   GetNumParameters
// Purpose:    Returns the number of parameters in the list
// Parameters: N/A
// Returns:    the number of parameters
// **************************************************************************
// Now defined inline as an alias to std::map<>::size().

// **************************************************************************
// Function:   GetParamPtr
// Purpose:    given a parameter name, returns the pointer to a PARAM object
// Parameters: name - name of the parameter
// Returns:    pointer to a PARAM object or
//             NULL, if no parameter with this name exists in the list
// **************************************************************************
const PARAM*
PARAMLIST::GetParamPtr( const char* name ) const
{
  const PARAM* retParam = NULL;
  const_iterator i = find( name );
  if( i != end() )
    retParam = &i->second;
  return retParam;
}

PARAM*
PARAMLIST::GetParamPtr( const char* name )
{
  PARAM* retParam = NULL;
  iterator i = find( name );
  if( i != end() )
    retParam = &i->second;
  return retParam;
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
  // This is quite inefficient. For traversing the list,
  // use its iterators, or for_each() from <algorithm>.
  const PARAM* retParam = NULL;
  if( idx < size() )
  {
    const_iterator i = begin();
    advance( i, idx );
    retParam = &i->second;
  }
  return retParam;
}

PARAM *
PARAMLIST::GetParamPtr( size_t idx)
{
  // This is quite inefficient. For traversing the list,
  // use its iterators, or for_each() from <algorithm>.
  PARAM* retParam = NULL;
  if( idx < size() )
  {
    iterator i = begin();
    advance( i, idx );
    retParam = &i->second;
  }
  return retParam;
}

// **************************************************************************
// Function:   ClearParamList
// Purpose:    deletes all parameters in the parameter list
//             as a result, the list still exists, but does not contain any parameter
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
// Now defined inline as an alias to std::map<>::clear().

// **************************************************************************
// Function:   AddParameter2List
// Purpose:    adds a new parameter to the list of parameters
//             if a parameter with the same name already exists,
//             it updates the currently stored parameter with the provided values
// Parameters: inLine - ASCII string, as defined in project description,
//                           defining this parameter
// Returns:    true if inLine is a correct parameter line, false otherwise
// **************************************************************************
bool
PARAMLIST::AddParameter2List( const char* inLine, size_t inLength )
{
  if( inLine == NULL )
    return false;
  string line( inLine, inLength );
  if( inLength == 0 )
    line = string( inLine );
  istringstream linestream( line );

  PARAM param;
  if( linestream >> param )
    ( *this )[ param.name ] = param;
  return linestream;
}

// **************************************************************************
// Function:   DeleteParam
// Purpose:    deletes a parameter of a given name in the list of parameters
//             it also frees the memory for this particular parameter
//             it does not do anything, if the parameter does not exist
// Parameters: name - name of the parameter
// Returns:    N/A
// **************************************************************************
// Now defined inline as an alias to std::map<>::erase().

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
    os << i->second << '\n';
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
    ( *this )[ param.name ] = param;
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
    i->second.WriteBinary( os );
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
// Function:   SaveParameterList
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
PARAMLIST::SaveParameterList( const char* filename, bool usetags ) const
{
  ofstream file( filename );
  if( !file.is_open() )
    return false;
  // If desired, exclude parameters tagged in the parameter list.
  if( usetags )
  {
    PARAMLIST paramsToSave;
    for( const_iterator i = begin(); i != end(); ++i )
      if( !i->second.tag )
        paramsToSave[ i->first ] = i->second;
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
PARAMLIST::LoadParameterList( const char* filename, bool usetags, bool importnonexisting )
{
  ifstream file( filename );
  PARAMLIST paramsFromFile;
  file >> paramsFromFile;

  typedef set<string> nameset;
  nameset unwantedParams;

#if 1 // This section's functionality will go into operator code.
  // Exclude parameters from unwanted sections.
  const char* unwantedSections[] = { "System", };
  for( size_t j = 0; j < sizeof( unwantedSections ) / sizeof( *unwantedSections ); ++j )
    for( const_iterator i = paramsFromFile.begin(); i != paramsFromFile.end(); ++i )
      if( PARAM::strciequal( i->second.section, unwantedSections[ j ] ) )
        unwantedParams.insert( i->first );
#endif

  // If desired, exclude parameters tagged in the main parameter list.
  if( usetags )
    for( const_iterator i = paramsFromFile.begin(); i != paramsFromFile.end(); ++i )
    {
      const_iterator f = find( i->first );
      if( f != end() && f->second.tag )
        unwantedParams.insert( i->first );
    }

  // If desired, exclude parameters missing from the main parameter list.
  if( !importnonexisting )
    for( const_iterator i = paramsFromFile.begin(); i!= paramsFromFile.end(); ++i )
      if( find( i->first ) == end() )
        unwantedParams.insert( i->first );

  for( nameset::const_iterator i = unwantedParams.begin(); i != unwantedParams.end(); ++i )
    paramsFromFile.erase( *i );

  for( const_iterator i = paramsFromFile.begin(); i != paramsFromFile.end(); ++i )
    ( *this )[ i->first ] = i->second;

  return !file.fail();
}

/////////////////////////////////////////////////////////////////////////////
// PARAM definitions                                                       //
/////////////////////////////////////////////////////////////////////////////
static const char* const sDefaultValue = "0";
static const string sCommentSeparator = "//";
const ctype<char>& PARAM::ct = use_facet<ctype<char> >( locale() );

// **************************************************************************
// Function:   SetDimensions
// Purpose:    Sets the dimensions of a matrix parameter.
//             It does not do anything, if the parameter is not a matrix.
// NOTE:       The two dimensions are in reverse order wrt
//             SetValue/GetValue.
// Parameters: inDimension1 - size in dimension 1
//             inDimension2 - size in dimension 2
// Returns:    N/A
// **************************************************************************
void
PARAM::SetDimensions( size_t inDimension1, size_t inDimension2 )
{
 // Don't do anything if this is not a matrix parameter.
 if( type == "matrix" && inDimension2 > 0 )
 {
   // To preserve existing values' indices, insert/remove values as needed.
   size_t dim1 = GetNumValuesDimension1(),
          dim2 = GetNumValuesDimension2();
   if( inDimension2 > dim2 )
     for( size_t i = 0; i < dim1; ++i )
       values.insert( values.begin() + i * inDimension2 + dim2, inDimension2 - dim2,
                                                     encodedString( sDefaultValue ) );
   else
     for( size_t i = 0; i < dim1; ++i )
       values.erase( values.begin() + ( i + 1 ) * inDimension2, values.begin() + i * inDimension2 + dim2 );
   // dim1_index will be resized by SetNumValues().
   dim2_index.resize( inDimension2 );
   SetNumValues( inDimension1 * inDimension2 );
 }
}

// **************************************************************************
// Function:   PARAM
// Purpose:    The constructor for the PARAM object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PARAM::PARAM()
: changed( false ),
  archive( false ),
  tag( false )
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
  defaultvalue( inDefaultvalue ),
  lowrange( inLowrange ),
  highrange( inHighrange ),
  comment( inComment ),
  changed( false ),
  archive( false ),
  tag( false )
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
: changed( false ),
  archive( false ),
  tag( false )
{
  istringstream iss( line );
  iss >> *this;
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
  values.resize( n, encodedString( sDefaultValue ) );
  // dim2_index will always have a size > 0.
  // If n is not a multiple of dim2_index' size something is logically wrong.
  // But it has not been an error up to now.
  dim1_index.resize( n / dim2_index.size() );
  changed = true;
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
    retValue = values[ idx ].c_str();
  }
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
  values[ idx ] = value;
  changed = true;
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
  changed = true;
  archive = false;
  tag = false;
  values.clear();
  string declaration;
  getline( is, declaration, '=' );
  string definition;
  {
    int i = is.peek();
    while( is && i != '\r' && i != '\n' && i != EOF )
    {
      definition += is.get();
      i = is.peek();
    }
  }
  size_t commentSepPos = definition.rfind( sCommentSeparator );
  if( commentSepPos != definition.npos )
  {
    size_t commentPos = commentSepPos + sCommentSeparator.length();
    while( commentPos < definition.size() && ct.is( ct.space, definition[ commentPos ] ) )
      ++commentPos;
    comment = definition.substr( commentPos );
    definition = definition.substr( 0, commentSepPos );
  }
  // Parse the parameter's declaration.
  istringstream linestream( declaration );
  encodedString value;
  linestream >> value;
  SetSection( value );
  linestream >> value;
  SetType( value );
  linestream >> value;
  SetName( value );
  if( !linestream )
    is.setstate( ios::failbit );
  else
  {
    linestream.clear();
    // Parse the parameter's definition.
    linestream.str( definition );
    if( type == "matrix" )
    {
      linestream >> dim1_index >> dim2_index;
      if( dim2_index.size() < 1 )
        dim2_index.resize( 1 );
      }
      else if( type.find( "list" ) != type.npos )
      {
      linestream >> dim1_index;
      dim2_index.resize( 1 );
    }
    else
    {
      dim1_index.resize( 1 );
      dim2_index.resize( 1 );
    }
    if( !linestream )
      is.setstate( ios::failbit );

    linestream >> value;
    while( linestream && values.size() < dim1_index.size() * dim2_index.size() )
    {
      values.push_back( value );
      linestream >> value;
    }
    // Not all matrix/list entries are required for a parameter definition.
    values.resize( dim1_index.size() * dim2_index.size(), encodedString( sDefaultValue ) );

    // These entries are not required for a parameter definition.
    encodedString* finalEntries[] =
    {
      &defaultvalue,
      &lowrange,
      &highrange
    };
    size_t numFinalEntries = sizeof( finalEntries ) / sizeof( *finalEntries ),
           i = 0;
    while( linestream && i < numFinalEntries )
    {
      *finalEntries[ i ] = value;
      linestream >> value;
      ++i;
    }
    while( i < numFinalEntries )
    {
      *finalEntries[ i ] = encodedString( sDefaultValue );
      ++i;
    }
  }
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
  os << GetSection() << ' ' << GetType() << ' ' << GetName() << "= ";
  if( type == "matrix" )
    os << LabelsDimension1() << ' ' << LabelsDimension2() << ' ';
  else if( type.find( "list" ) != type.npos )
    os << Labels() << ' ';
  for( size_t i = 0; i < GetNumValues(); ++i )
    os << encodedString( GetValue( i ) ) << ' ';
  os << defaultvalue << ' '
     << lowrange << ' '
     << highrange << ' '
     << sCommentSeparator << ' ' << comment;
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
    if( name.empty() || name != p.name )
    {
      section = p.section;
      name = p.name;
      type = p.type;
      defaultvalue = p.defaultvalue;
      lowrange = p.lowrange;
      highrange = p.highrange;
      comment = p.comment;
    }

    dim1_index = p.dim1_index;
    dim2_index = p.dim2_index;
    values = p.values;

    changed = p.changed;
    archive = p.archive;
    tag = p.tag;
  }
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
// encodedString definitions                                               //
/////////////////////////////////////////////////////////////////////////////
const char escapeChar = '%';
// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             string value.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
PARAM::encodedString::ReadFromStream( istream& is )
{
  string newContent;
  if( is >> newContent )
  {
    size_t pos = newContent.find( escapeChar, 0 );
    while( pos != npos )
    {
      newContent.erase( pos, 1 );

      size_t numDigits = 0;
      char curDigit;
      int hexValue = 0;
      while( pos + numDigits < newContent.size() && numDigits < 2
             && ::isxdigit( curDigit = newContent[ pos + numDigits ] ) )
      {
        if( !::isdigit( curDigit ) )
          curDigit = ::toupper( curDigit ) - 'A' + 10;
        else
          curDigit -= '0';
        hexValue = ( hexValue << 4 ) + curDigit;
        ++numDigits;
      }
      newContent.erase( pos, numDigits );
      if( hexValue > 0 )
        newContent.insert( pos, 1, ( char )hexValue );

      pos = newContent.find( escapeChar, pos + 1 );
    }
    *this = newContent;
  }
  return is;
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             encoded string value.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into; list of characters that may not
//             appear in the output.
// Returns:    Stream written into.
// **************************************************************************
ostream&
PARAM::encodedString::WriteToStream( ostream& os, const string& forbiddenChars ) const
{
  if( empty() )
    os << escapeChar;
  else
  {
    const string& self = *this;
    ostringstream oss;
    oss << hex;
    for( size_t pos = 0; pos < size(); ++pos )
    {
      if( ::isprint( self[ pos ] )
          && !::isspace( self[ pos ] )
          && forbiddenChars.find( self[ pos ] ) == npos )
      {
        oss << self[ pos ];
        if( self[ pos ] == escapeChar )
          oss << escapeChar;
      }
      else
        oss << escapeChar
            << ( int )( ( self[ pos ] >> 4 ) & 0x0f )
            << ( int )( self[ pos ] & 0x0f );
    }
    os << oss.str();
  }
  return os;
}

/////////////////////////////////////////////////////////////////////////////
// labelIndexer definitions                                                //
/////////////////////////////////////////////////////////////////////////////

// Whatever we accept as delimiting single-character symbol pairs for our
// index list in a parameter line.
// The first entry is the default pair (used for output).
const char* bracketPairs[] = { "{}", "()", "[]", "<>" };

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
    char possibleDelimiter = is.peek();
    const char* endDelimiter = NULL;
    size_t i = 0;
    while( endDelimiter == NULL && i < sizeof( bracketPairs ) / sizeof( *bracketPairs ) )
    {
      if( possibleDelimiter == bracketPairs[ i ][ 0 ] )
        endDelimiter = &bracketPairs[ i ][ 1 ];
      ++i;
    }
    if( endDelimiter != NULL )
    { // The first character is an opening bracket,
      // Get the line up to the matching closing bracket.
      is.get();
      string labelsList;
      if( getline( is, labelsList, *endDelimiter ) )
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
    os << bracketPairs[ 0 ][ 0 ] << ' ';
    for( size_t i = 0; i < reverseIndex.size(); ++i )
    {
      reverseIndex[ i ].WriteToStream( os, &bracketPairs[ 0 ][ 1 ] );
      os << ' ';
    }
    os << bracketPairs[ 0 ][ 1 ];
  }
  return os;
}

/////////////////////////////////////////////////////////////////////////////
// type_adapter definitions                                                //
/////////////////////////////////////////////////////////////////////////////
PARAM PARAM::type_adapter::null_p;

