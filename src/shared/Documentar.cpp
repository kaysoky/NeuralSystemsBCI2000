////////////////////////////////////////////////////////////////////////////////
//
// File: Shared/Documentar.cpp
//
// Date: Apr 1, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A file which, when linked into a BCI2000 module,
//              results in an executable that documents the module's filter
//              and signal connections, its parameters, and its states.
//              Used by "make doc".
//
////////////////////////////////////////////////////////////////////////////////

#include "PCHIncludes.h"
#pragma hdrstop

#include <cstdio>
#include <iostream>
#include <set>
#include <sstream>

#include "UParameter.h"
#include "UState.h"
#include "UGenericFilter.h"
#include "UGenericSignal.h"

#define DOC_CHANNELS 64
#define DOC_ELEMENTS 16

#ifndef DOC_EXTENSION
# define DOC_EXTENSION ".docinfo"
#endif

using namespace std;

class Table : public ostringstream
{
 public:
  void WriteToStream( ostream& os ) const
  {
    vector< vector< string > > cells( 1 );

    {
      size_t row = 0;
      string s = str();
      size_t b, e;
      for( b = 0, e = s.find_first_of( "\t\n" ); e != string::npos;
                             b = e + 1, e = s.find_first_of( "\t\n", e + 1 ) )
      {
        cells[ row ].push_back( s.substr( b, e - b ) );
        if( s[ e ] == '\n' )
        {
          ++row;
          cells.push_back();
        }
      }
      if( b < s.length() )
        cells[ row ].push_back( s.substr( b ) );
    }

    size_t maxCols = 0;
    for( size_t row = 0; row < cells.size(); ++row )
      if( cells[ row ].size() > maxCols )
        maxCols = cells[ row ].size();

    vector< size_t > maxLengths( maxCols, 0 );
    for( size_t row = 0; row < cells.size(); ++row )
      for( size_t col = 0; col < cells[ row ].size(); ++col )
        if( cells[ row ][ col ].length() > maxLengths[ col ] )
          maxLengths[ col ] = cells[ row ][ col ].length();

#ifdef LATEXOUTPUT
    os << '\n'
       << cells[ 0 ][ 0 ];
    for( size_t col = 1; col < cells[ 0 ].size(); ++col )
      os << " & " << cells[ 0 ][ col ];
    os << "\\\\ \\hline \n";
    for( size_t row = 1; row < cells.size(); ++row )
    {
      os << '\n'
         << cells[ row ][ 0 ];
      for( size_t col = 1; col < cells[ row ].size(); ++col )
        os << " & " << cells[ row ][ col ];
      os << "\\\\ \n";
    }
#else // LATEXTOUTPUT
    os << '\n';
    for( size_t col = 0; col < cells[ 0 ].size(); ++col )
      os << ' '
         << cells[ 0 ][ col ]
         << string( maxLengths[ col ] - cells[ 0 ][ col ].length(), ' ' );
    os << '\n';
    for( size_t col = 0; col < cells[ 0 ].size(); ++col )
      os << ' ' << string( maxLengths[ col ], '-' );
    for( size_t row = 1; row < cells.size(); ++row )
    {
      os << '\n';
      for( size_t col = 0; col < cells[ row ].size(); ++col )
      os << ' '
         << cells[ row ][ col ]
         << string( maxLengths[ col ] - cells[ row ][ col ].length(), ' ' );
    }
#endif // LATEXOUTPUT
  }
};

ostream& operator<<( ostream& os, Table& tb )
{ tb.WriteToStream( os ); tb.str( "" ); return os; }

int
BCIError::bci_ostream::bci_stringbuf::sync()
{
  int r = stringbuf::sync();
  ++num_flushes;
  if( on_flush )
  {
    cout << str();
    str( "" );
  }
  return r;
}

struct Documentar{ int operator()( int, const char** ); };
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
    Application->Initialize();
  }
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }

  string filename = _argv[ 0 ],
         modulename;
  size_t pos = filename.rfind( "\\" );
  if( pos != string::npos )
    filename = filename.substr( pos + 1 );
  pos = filename.find( "_" );
  if( pos != string::npos )
    filename = filename.substr( pos + 1 );
  pos = filename.rfind( "." );
  if( pos != string::npos )
    filename = filename.substr( 0, pos );
  modulename = filename;
  filename += DOC_EXTENSION;
  freopen( filename.c_str(), "w", stdout );
  const char** av = new const char*[ _argc ];
  av[ 0 ] = modulename.c_str();
  for( int i = 1; i <= _argc; ++i )
    av[ i ] = _argv[ i ];
  int ret = Documentar()( _argc, av );
  delete[] av;
  fclose( stdout );
  return ret;
}

int Documentar::operator()( int argc, const char** argv )
{
  const char newl = '\n';
  const char tab = '\t';
  Table table;

  cout << "Module: " << argv[ 0 ] << newl << newl;

  if( GenericFilter::Registrar::registrars.size() == 0 )
    cout << "No filters registered." << newl;
  else
  {
      cout << "Filter sequence:" << newl;

      table << "Position" << tab << "Filter Name" << newl;

      string currentPos;
      for( GenericFilter::registrarSet::iterator i
                 = GenericFilter::Registrar::registrars.begin();
                 i != GenericFilter::Registrar::registrars.end(); ++i )
      {
        if( currentPos != ( *i )->GetPosition() )
        {
          currentPos = ( *i )->GetPosition();
          table << currentPos;
        }
        table << tab << ( *i )->GetTypeid().name() << newl;
      }
      cout << table << newl;
  }

  PARAMLIST allParams;
  STATELIST allStates;

  SignalProperties inputProperties,
                   outputProperties( DOC_CHANNELS, DOC_ELEMENTS );
  string currentPos;
  typedef set<string> infoset;
  infoset filtersManipulatingCurrentOutput;

  for( GenericFilter::registrarSet::iterator i
           = GenericFilter::Registrar::registrars.begin();
           i != GenericFilter::Registrar::registrars.end(); ++i )
  {
    PARAMLIST filterParams;
    STATELIST filterStates;

    Environment::EnterConstructionPhase( &filterParams, &filterStates, NULL, NULL );
    GenericFilter* filter = ( *i )->NewInstance();

    if( filterParams.size() > 0 )
    {
      cout << newl
           << "Parameters defined by the "
           << ( *i )->GetTypeid().name()
           << " filter:"
           << newl;

      table << "Section" << tab << "Type" << tab << "Name"
            << tab << "Description" << newl;
      for( PARAMLIST::iterator i = filterParams.begin(); i != filterParams.end(); ++i )
      {
        PARAM* p = &i->second;
        table << p->GetSection() << tab << p->GetType() << tab << p->GetName()
              << tab << p->GetComment() << newl;
      }
      cout << table << newl;
      allParams.insert( filterParams.begin(), filterParams.end() );
    }

    if( filterStates.GetNumStates() > 0 )
    {
      cout << newl
           << "States defined by the "
           << ( *i )->GetTypeid().name()
           << " filter:"
           << newl;
      table << "Name" << tab << "Length" << newl;
      for( int i = 0; i < filterStates.GetNumStates(); ++i )
      {
        STATE* state = filterStates.GetStatePtr( i );
        table << state->GetName() << tab
              << state->GetLength() << newl;
        allStates.AddState2List( state );
      }
      cout << table << newl;
    }

    Environment::EnterPreflightPhase( &allParams, &allStates, NULL, NULL );
    if( currentPos != ( *i )->GetPosition() )
    {
      if( filtersManipulatingCurrentOutput.size() < 1 && currentPos != "" )
      {
        cout << "!Warning: No filter produces output on position "
             << currentPos << "." << newl;
      }
      else if( filtersManipulatingCurrentOutput.size() > 1 )
      {
        cout << "!Error: More than one filter has output on position "
             << currentPos << ": " << newl;
        for( infoset::iterator i = filtersManipulatingCurrentOutput.begin();
                i != filtersManipulatingCurrentOutput.end(); ++i )
           cout << "! " << *i << newl;
      }
      filtersManipulatingCurrentOutput.clear();
      inputProperties = outputProperties;
      outputProperties = SignalProperties( 0, 0 );
    }
    filter->Preflight( inputProperties, outputProperties );
    if( outputProperties != SignalProperties( 0, 0 ) )
      filtersManipulatingCurrentOutput.insert( typeid( filter ).name() );
  }
  return 0;
}

