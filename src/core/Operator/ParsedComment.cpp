//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that handles parsing a parameter's comment
//       for display purposes.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ParsedComment.h"

#include "Param.h"
#include <sstream>
#include <cassert>

using namespace std;

ParsedComment::ParsedComment( const Param& p )
: mName( p.Name() ),
  mIndexBase( 0 ),
  mKind( unknown ),
  mComment( p.Comment() )
{
  if( !p.Sections().empty() )
    mHelpContext = *p.Sections().rbegin();

  // Look for "interpretation hints" in the comment.
  // Unknown hints (syntax errors) will show up in the operator's comment display.
  const struct
  {
    const char* keyword;
    Kind_type   kind;
  } hints[] =
  {
    { "(enumeration)", singleEntryEnum },
    { "(boolean)",     singleEntryBoolean },
    { "(inputfile)",   singleEntryInputFile },
    { "(outputfile)",  singleEntryOutputFile },
    { "(directory)",   singleEntryDirectory },
    { "(color)",       singleEntryColor },
  };
  for( size_t i = 0; ( mKind == unknown ) && ( i < sizeof( hints ) / sizeof( *hints ) ); ++i )
  {
    size_t hintPos = mComment.find( hints[ i ].keyword );
    if( hintPos != string::npos )
    {
      mComment = mComment.erase( hintPos );
      mKind = hints[ i ].kind;
    }
  }
  // For matrix and list type parameters, the hint is ignored.
  string paramType = p.Type();
  if( paramType.find( "matrix" ) != string::npos )
    mKind = matrixGeneric;
  else if( paramType.find( "list" ) != string::npos )
    mKind = listGeneric;
  else if( mKind == unknown )
    mKind = singleEntryGeneric;

  switch( mKind )
  {
    case singleEntryEnum:
      if( !ExtractEnumValues( p ) )
        mKind = singleEntryGeneric;
      break;

    case singleEntryBoolean:
      if( ExtractEnumValues( p ) && !IsBooleanEnum() )
        mKind = singleEntryGeneric;
      break;

    case singleEntryInputFile:
    case singleEntryOutputFile:
    case singleEntryDirectory:
    case singleEntryColor:
    case singleEntryGeneric:
    case listGeneric:
    case matrixGeneric:
      break;

    default:
      assert( false );
  }
}

bool
ParsedComment::ExtractEnumValues( const Param& p )
{
  // Only int type parameters can be enumerations or booleans.
  const string enumParamType = "int";
  if( p.Type() != enumParamType )
    return false;

  // Enumerations need a finite range.
  int lowRange = ::atoi( p.LowRange().c_str() ),
      highRange = ::atoi( p.HighRange().c_str() ),
      paramValue = ::atoi( p.Value().c_str() );
  if( lowRange != 0 && lowRange != 1
      || highRange <= lowRange
      || paramValue < lowRange
      || paramValue > highRange )
    return false;

  // Examine the comment: Does it contain an enumeration of all possible values?
  string comment = mComment;
  // Replace all punctuation marks with white space.
  const string cPunctuationChars = ",;:=()[]";
  size_t punctuationPos = comment.find_first_of( cPunctuationChars );
  while( punctuationPos != string::npos )
  {
    comment[ punctuationPos ] = ' ';
    punctuationPos = comment.find_first_of( cPunctuationChars );
  }

  map<int, int> histogram;
  istringstream is( comment );
  string        value,
                modifiedComment,
        *       currentLabel = &modifiedComment;
  while( is >> value )
  {
    // Using the >> operator for an int would accept "+" and similar strings as numbers.
    // We are only interested in groups of decimal digits.
    bool isNum = true;
    int numValue = 0;
    for( string::iterator i = value.begin(); isNum && i != value.end(); ++i )
    {
      if( *i < '0' || *i > '9' )
        isNum = false;
      else
      {
        numValue *= 10;
        numValue += *i - '0';
      }
    }
    if( isNum )
    {
      unsigned int index = numValue - lowRange;
      histogram[ index ]++;
      if( mValues.size() <= index )
        mValues.resize( index + 1 );
      currentLabel = &mValues[ index ];
    }
    else
    {
      if( !currentLabel->empty() )
        *currentLabel += " ";
      *currentLabel += value;
    }
  }

  bool isEnum = is.eof();

  // Each non-null value must be explained in the comment, thus appear exactly
  // once -- if in doubt, let's better return.
  for( size_t i = 1; isEnum && i < mValues.size(); ++i )
    if( histogram[ i ] != 1 )
      isEnum = false;

  // We consider this a boolean parameter.
  if( isEnum && lowRange == 0 && highRange == 1
      && histogram[ 0 ] == 0 && histogram[ 1 ] == 1 )
  {
    if( mValues.size() > 1 )
      modifiedComment = mValues[ 1 ];
    mValues.resize( 2 );
    mValues[ 0 ] = "no";
    mValues[ 1 ] = "yes";
  }

  if( mValues.size() != size_t( highRange - lowRange + 1 ) )
    isEnum = false;

  if( isEnum && mValues.size() > 0 && mValues[ 0 ] == "" )
    mValues[ 0 ] = "none";

  // Each of the other labels must now be non-empty.
  for( size_t i = 1; isEnum && i < mValues.size(); ++i )
    if( mValues[ i ].empty() )
      isEnum = false;

  if( isEnum )
  {
    mIndexBase = lowRange;
    mComment = modifiedComment;
  }
  else
    mValues.clear();
  return isEnum;
}

bool
ParsedComment::IsBooleanEnum() const
{
  if( mIndexBase != 0 )
    return false;
  if( mValues.size() != 2 )
    return false;
  if( mValues[ 0 ] != "no" && mValues[ 0 ] != "No" )
    return false;
  if( mValues[ 1 ] != "yes" && mValues[ 1 ] != "Yes" )
    return false;
  return true;
}

