/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    UParameter.cpp                                                  *
 * Comment:   This unit provides support for system-wide parameters           *
 *            and parameter lists                                             *
 * Version:   0.19                                                            *
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
 ******************************************************************************/
#pragma hdrstop

#include "UParameter.h"

#include <fstream>
#include <sstream>
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
// Parameters: paramstring - ASCII string, as defined in project description,
//                           defining this parameter
// Returns:    N/A
// **************************************************************************
void
PARAMLIST::AddParameter2List( const char* line, size_t length )
{
  PARAM param;
  param.ParseParameter( line, length );
  if( param.valid )
    ( *this )[ param.name ] = param;
}

// **************************************************************************
// Function:   MoveParameter2List
// Purpose:    adds a new parameter to the list of parameters
//             this function assumes that the specified parameter object
//             will not be freed anywhere else (but in the destructor of the
//             paramlist object). The difference to CloneParameter2List() is
//             that it does not actually copy the whole param object
//             This function will not add a parameter to the list, if one
//             with the same name already exists
// Note:
// The implementation is not consistent with the spec given above.
// According to the spec, the caller wouldn't know if the parameter was actually
// added, so if not, the PARAM instance would never be deleted.
// Because the spec states that the caller may not delete the argument, we
// always delete it as soon as possible (after copying it) to avoid a memory
// hole.
// jm
//
// Parameters: param - pointer to an existing PARAM object
// Returns:    N/A
// **************************************************************************
void
PARAMLIST::MoveParameter2List( PARAM* newparam )
{
  if( newparam && find( newparam->GetName() ) == end() )
    ( *this )[ newparam->GetName() ] = *newparam;
  delete newparam;
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
// Function:   CloneParameter2List
// Purpose:    adds a new parameter to the list of parameters
//             The difference to MoveParameter2List() is that it actually
//             physically copies the param object (the specified param
//             object can then be freed elsewhere)
//             This function will UPDATE the values for a parameter in the list,
//             if one with the same name already exists
// Parameters: param - pointer to an existing PARAM object
// Returns:    N/A
// **************************************************************************
void
PARAMLIST::CloneParameter2List( const PARAM* param )
{
 if( GetParamPtr( param->GetName() ) )
   DeleteParam( param->GetName() );
 MoveParameter2List( new PARAM( *param ) );
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
// Returns:    N/A
// **************************************************************************
void
PARAMLIST::WriteToStream( ostream& os ) const
{
  for( const_iterator i = begin(); os && i != end(); ++i )
    os << i->second << '\n';
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
void
PARAMLIST::ReadFromStream( istream& is )
{
  clear();
  PARAM param;
  is >> ws;
  do
  {
    is >> param >> ws;
    ( *this )[ param.name ] = param;
  } while( !is.eof() );
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
    PARAMLIST paramsToSave( *this );
    for( iterator i = paramsToSave.begin(); i != paramsToSave.end(); ++i )
      if( i->second.tag )
        paramsToSave.erase( i );
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
const string commentSeparator = "//";
const ctype<char>& PARAM::ct = use_facet<ctype<char> >( locale() );

// **************************************************************************
// Function:   SetDimensions
// Purpose:    Sets the dimensions of a matrix parameter.
//             It does not do anything, if the parameter is not a matrix.
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
   dimension2 = inDimension2;
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
: dimension2( 1 ),
  valid( false ),
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
: dimension2( 1 ),
  defaultvalue( inDefaultvalue ),
  lowrange( inLowrange ),
  highrange( inHighrange ),
  comment( inComment ),
  valid( false ),
  archive( false ),
  tag( false )
{
 SetName( inName );
 SetSection( inSection );
 SetType( inType );
 SetValue( inValue );
 valid = inName && inSection && inType && inValue;
}

// **************************************************************************
// Function:   PARAM
// Purpose:    Constructs and initializes a parameter object, based upon
//             a parameter string, as defined in the project outline
// Parameters: char *paramstring
// Returns:    N/A
// **************************************************************************
PARAM::PARAM( const char* paramstring )
: dimension2( 0 ),
  valid( false ),
  archive( false ),
  tag( false )
{
  ParseParameter( paramstring, strlen( paramstring ) );
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
// Now defined inline.

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
  const char* retValue = "0";
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
  if( value != "" )
    values[ idx ] = value;
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
// Function:   get_argument
// Purpose:    parses the parameter line that is being sent in the core
//             communication, or as stored in any BCI2000 .prm file
//             it returns the next token that is being delimited by either
//             a ' ' or '='
// Note:
// This function is now obsolete as far as the PARAMLIST/PARAM classes are
// concerned. It will be kept as long as it is referenced from elsewhere,
// but please do not use it for new code.
// jm
//
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int PARAM::get_argument(int ptr, char *buf, const char *line, int maxlen)
{
 // skip trailing spaces, if any
 while ((line[ptr] == '=') || (line[ptr] == ' ') && (ptr < maxlen))
  ptr++;

 // go through the string, until we either hit a space, a '=', or are at the end
 while ((line[ptr] != '=') && (line[ptr] != ' ') && (line[ptr] != '\n') && (line[ptr] != '\r') && (ptr < maxlen))
  {
  *buf=line[ptr];
  ptr++;
  buf++;
  }

 *buf=0;
 return(ptr);
}

// **************************************************************************
// Function:   GetParamLine
// Purpose:    Returns a parameter line in ASCII format
//             Tis parameter line is constructed, based upon the current
//             values in the PARAM object
// Note:
// Calling GetParamLine() may invalidate its previous return value.
// jm
//
// Parameters: N/A
// Returns:    a pointer to the parameter line
// **************************************************************************
const char*
PARAM::GetParamLine() const
{
 static string buffer;
 ostringstream paramline;
 paramline << *this << ends;
 buffer = paramline.str();
 return buffer.c_str();
}

// **************************************************************************
// Function:   ParseParameter
// Purpose:    This routine is called by coremessage->ParseMessage()
//             it parses the provided ASCII parameter line and initializes
//             the PARAM object accordingly, i.e., it fills in values, name,
//             section name, type, etc.
// Parameters: line - pointer to the ASCII parameter line
//             length - length of this parameter line
// Returns:    ERRPARAM_INVALIDPARAM if the parameter line is invalid, or
//             ERRPARAM_NOERR
// **************************************************************************
int
PARAM::ParseParameter( const char* paramline, size_t length )
{
  if( paramline == NULL )
    return ERRPARAM_INVALIDPARAM;
  int err = ERRPARAM_NOERR;
  string line( paramline, length );
  if( length == 0 )
    line = string( paramline );
  istringstream linestream( line );
  linestream >> *this;
  if( !linestream )
    err = ERRPARAM_INVALIDPARAM;
  return err;
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             parameter.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    N/A
// **************************************************************************
void
PARAM::ReadFromStream( istream& is )
{
  valid = false;
  archive = false;
  tag = false;
  values.clear();
  string declaration,
         definition;
  getline( is, declaration, '=' );
  getline( is, definition );
  size_t commentSepPos = definition.rfind( commentSeparator );
  if( commentSepPos != definition.npos )
  {
    size_t commentPos = commentSepPos + commentSeparator.length();
    while( commentPos < definition.size() && ct.is( ct.space, definition[ commentPos ] ) )
      ++commentPos;
    comment = definition.substr( commentPos );
    definition = definition.substr( 0, commentSepPos );
  }
  // Parse the parameter's declaration.
  istringstream linestream( declaration );
  string value;
  linestream >> value;
  SetSection( value );
  linestream >> value;
  SetType( value );
  linestream >> value;
  SetName( value );
  if( !linestream )
  {
    is.setstate( ios::failbit );
    return;
  }
  linestream.clear();
  // Parse the parameter's definition.
  linestream.str( definition );
  size_t dimension1;
  if( type == "matrix" )
  {
    linestream >> dimension1 >> dimension2;
  }
  else if( type.find( "list" ) != type.npos )
  {
    linestream >> dimension1;
    dimension2 = 1;
  }
  else
  {
    dimension1 = 1;
    dimension2 = 1;
  }
  if( !linestream )
    is.setstate( ios::failbit );

  linestream >> value;
  while( linestream && values.size() < dimension1 * dimension2 )
  {
    values.push_back( value );
    linestream >> value;
  }
  // Not all matrix/list entries are required for a parameter definition.
  values.resize( dimension1 * dimension2, "0" );

  // These entries are not required for a parameter definition.
  string* finalEntries[] =
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
    *finalEntries[ i ] = "0";
    ++i;
  }

  valid = !is.fail();
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             parameter.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    N/A
// **************************************************************************
void
PARAM::WriteToStream( ostream& os ) const
{
  os << GetSection() << ' ' << GetType() << ' ' << GetName() << "= ";
  if( type == "matrix" )
    os << GetNumValuesDimension1() << ' ' << GetNumValuesDimension2() << ' ';
  else if( type.find( "list" ) != type.npos )
    os << GetNumValues() << ' ';
  for( size_t i = 0; i < GetNumValues(); ++i )
    os << GetValue( i ) << ' ';
  os << defaultvalue << ' '
     << lowrange << ' '
     << highrange << ' '
     << commentSeparator << ' ' << comment;
}

