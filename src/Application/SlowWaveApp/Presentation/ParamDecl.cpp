/////////////////////////////////////////////////////////////////////////////
//
// File: ParamDecl.cpp
//
// Date: Oct 17, 2001
//
// Author: Juergen Mellinger
//
// Description: Use the macros in this file for declaring and defining
//      TTD/BCI2000 parameters in a centralized resource-like fashion.
//
//      The cpp file that _defines_ the _TParamDef globals
//      must #define DEFINE_PARAMS before including this file.
//
//      Each parameter declared has a global variable of type
//      _TParamDef associated whose name is the parameter's name
//      prefixed with '_param_'. This way the compiler will complain
//      about parameters with identical names (an error difficult
//      to track down otherwise).
//      By accessing a parameter's name via an element
//      of its associated _TParamDef variable
//      (i.e. _not_ by just converting a given name into a
//      string using '#') we make the compiler notify us of typos in
//      parameter names (another error difficult to track down
//      otherwise).
//
// Changes: Jan 31, 2004, jm: Made ParamDecl.cpp a separate file
//      and introduced functions to replace extensive macros.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "ParamDecl.h"
#include "UParameter.h"
#include <list>
#include <string>
#include <sstream>

using namespace std;

list<class _TParamDef*>&
_TParamDef::params()
{
  static list<class _TParamDef*> _params;
  return _params;
}

const char*
_TParamDef::param_name_suffix( const char* inName, long inSuffix )
{
  static string s;
  s = inName;
  ostringstream os;
  os << inSuffix;
  unsigned int i = s.find( RUNTIME_SUFFIX );
  assert( i != string::npos );
  s.replace( i, sizeof( RUNTIME_SUFFIX ) - 1, os.str() );
  return s.c_str();
}

void
_TParamDef::param_delete_suffix( PARAMLIST* inList, const char* inName, long inSuffix )
{
  string _name = inName;
  unsigned int _replacePos = _name.find( RUNTIME_SUFFIX );
  assert( _replacePos != string::npos );
  ostringstream _suffix;
  _suffix << inSuffix;
  _name.replace( _replacePos, sizeof( RUNTIME_SUFFIX ) - 1, _suffix.str() );
#ifdef BCI2000
  inList->Delete( _name );
#else
  inList->DeleteParam( ( char* )_name.c_str() );
#endif
}

#ifdef BCI2000
void
_TParamDef::param_add_all( PARAMLIST* inList, const char* inSectionPrefix, long inDefaultDimension )
{
  string  sectionPrefix( inSectionPrefix ),
          externalPrefix( "ext_" ),
          runtimeSuffix( RUNTIME_SUFFIX ),
          runtimeElement( RUNTIME_ELEMENT );
  for( list<_TParamDef*>::const_iterator i = _TParamDef::params().begin();
        i != params().end(); ++i )
  {
    PARAM p( ( *i )->definitionLine() );
    string section = p.GetSection();
    if( section.find( externalPrefix ) != 0 )
    {
      if( section != sectionPrefix )
      {
         section = sectionPrefix + "_" + section;
         p.SetSection( section );
      }
      string name = p.GetName();
      size_t suffixPos = name.find( runtimeSuffix );
      if( suffixPos != name.npos )
      {
        p.SetType( "matrix" );
        string value = p.GetValue();
        size_t pos = value.find( runtimeSuffix );
        p.SetDimensions( inDefaultDimension + 1, 1 );
        for( int i = p.GetNumValuesDimension1() - 1; i >= 0; --i )
        {
          // Replace RUNTIME_SUFFIX in the comment and use it as a row label.
          ostringstream os;
          os << i;
          string comment = p.GetComment();
          size_t comment_pos = comment.find( runtimeSuffix );
          if( comment_pos != comment.npos )
            comment.replace( comment_pos, runtimeSuffix.length(), os.str() );
          p.LabelsDimension1()[ i ] = comment;
          // Replace RUNTIME_SUFFIX in the parameter values.
          if( pos == value.npos )
            p.SetValue( value, i, 0 );
          else
          {
            string newValue = value;
            newValue.replace( pos, runtimeSuffix.length(), os.str() );
            p.SetValue( newValue, i, 0 );
          }
        }
        p.LabelsDimension2()[ 0 ] = "";

        ostringstream oss;
        oss << p;
        string line = oss.str();
        // Replace RUNTIME_SUFFIX in the parameter name.
        pos = line.find( runtimeSuffix );
        if( pos != line.npos )
          line.replace( pos, runtimeSuffix.length(), "" );
        // Replace RUNTIME_SUFFIX in the comment.
        pos = line.rfind( runtimeSuffix );
        if( pos != line.npos )
          line.replace( pos, runtimeSuffix.length(), "N" );
        name.replace( suffixPos, runtimeSuffix.length(), "" );
        if( !inList->Exists( name ) )
          inList->Add( line );
      }
      else
      { // The parameter might be a special instance for a certain suffix.
        // This will only work for true suffices, i.e. at the end of the name.
        size_t pos = name.find_last_not_of( "0123456789" );
        PARAM* param = NULL;
        if( pos != name.length() - 1 )
        {
          string paramName = name.substr( 0, pos + 1 );
          if( inList->Exists( paramName ) )
            param = &( *inList )[ paramName ];
        }
        if( param != NULL && string( "matrix" ) == param->GetType() )
        {
          long suffix = ::atoi( name.substr( pos ).c_str() );
          param->SetValue( p.GetValue(), suffix, 0 );
          param->LabelsDimension1()[ suffix ] = p.GetComment();
        }
        else if( !PARAM_EXISTS( inList, p.GetName() ) )
        {
          ostringstream oss;
          oss << p;
          string line = oss.str();
          // Runtime elements are not supported, the user must supply a value.
          size_t rtPos;
          while( ( rtPos = line.find( runtimeElement ) ) != line.npos )
            line.replace( rtPos, runtimeElement.length(), "N/A" );
          inList->Add( line );
        }
      }
    }
  }
}
#endif // BCI2000

PARAM*
_TParamDef::param_get_ptr_suffix( PARAMLIST* inList, const char* inName, long inSuffix )
{                                                                               
  PARAM* result = NULL;
  string _name = inName,
         _blankName = _name;
  unsigned int _replacePos = _name.find( RUNTIME_SUFFIX );
  assert( _replacePos != string::npos );
  _blankName.erase( _replacePos, sizeof( RUNTIME_SUFFIX ) - 1 );
  ostringstream _suffix;
  _suffix << inSuffix;
  _name.replace( _replacePos, sizeof( RUNTIME_SUFFIX ) - 1, _suffix.str() );
#ifdef BCI2000
  if( inList->Exists( _name ) )
    result = &( *inList )[ _name ];
  else
  {
    static PARAM p;
    if( inList->Exists( _blankName ) )
    {
      PARAM& param = ( *inList )[ _blankName ];
      p.SetType( "list" );
      p.SetNumValues( param.GetNumValues() );
      for( size_t i = 0; i < param.GetNumValuesDimension2(); ++i )
        p.SetValue( param.GetValue( inSuffix, i ), i );
      result = &p;
    }
  }
#else // BCI2000
  result = inList->GetParamPtr( const_cast< char* >( _name.c_str() ) );
  if( result == NULL )
  {
    static PARAM p;
    PARAM* param = inList->GetParamPtr( const_cast< char* >( _blankName.c_str() ) );
    if( param != NULL )
    {
      p.SetType( "list" );
      p.SetNumValues( param->GetNumValues() );
      for( size_t i = 0; i < param->GetNumValuesDimension2(); ++i )
        p.SetValue( param->GetValue( inSuffix, i ), i );
      result = &p;
    }
  }
#endif // BCI2000
  return result;
}

#ifdef BCI2000
void
_TParamDef::beautify_name()
{
  // Remove the PR prefix from name and definition.
  const char prefix[] = "PR";
  const int  prefixLength = sizeof( prefix ) - 1;
  if( !::strncmp( _name, prefix, prefixLength ) )
  {
    char* namepos = _definitionLine + string( _definitionLine ).find( _name );
    for( int i = 0; i < prefixLength; ++i )
      *namepos++ = ' ';
    _name += prefixLength;
  }
}
#endif // BCI2000
