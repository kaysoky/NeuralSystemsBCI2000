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

// Signal dimensions for call to Preflight().
#define DOC_CHANNELS 64
#define DOC_ELEMENTS 16

using namespace std;

typedef enum
{
  plain,
  latex,
} OutputType;

class Table : public ostringstream
{
 public:
  explicit Table( OutputType output = plain ) : mOutputType( output ) {}
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

    switch( mOutputType )
    {
      case latex:
      {
        const char beforeCell[] = "\\verb@",
                   afterCell[] = "@";
        os << "\\\\\n"
           << "\\begin{tabular}{";
        for( size_t col = 0; col < cells[ 0 ].size(); ++col )
          os << 'l';
        os <<"}\n"
           << cells[ 0 ][ 0 ];
        for( size_t col = 1; col < cells[ 0 ].size(); ++col )
          os << " & " << cells[ 0 ][ col ];
        os << "\\\\ \\hline \n";
        for( size_t row = 1; row < cells.size(); ++row )
        {
          os << '\n';
          if( cells[ row ].size() > 0 )
            os << beforeCell << cells[ row ][ 0 ] << afterCell;
          for( size_t col = 1; col < cells[ row ].size(); ++col )
            os << " & " << beforeCell << cells[ row ][ col ] << afterCell;
          os << "\\\\ \n";
        }
        os << "\\end{tabular}\n";
      } break;
      default:
      {
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
      }
    }
  }

 private:
  OutputType mOutputType;
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
    cerr << str();
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
  const char** av = new const char*[ _argc ];
  av[ 0 ] = modulename.c_str();
  for( int i = 1; i <= _argc; ++i )
    av[ i ] = _argv[ i ];
  int ret = Documentar()( _argc, av );
  delete[] av;
  return ret;
}

int Documentar::operator()( int argc, const char** argv )
{
  OutputType outputType = plain;
  int  signalChannels = DOC_CHANNELS,
       signalElements = DOC_ELEMENTS;
  bool reportSignals = false;

  for( int i = 1; i < argc; ++i )
  {
    if( string( "--latexoutput" ) == argv[ i ] )
      outputType = latex;
    else if( string( "--signaldimensions" ) == argv[ i ] )
    {
      reportSignals = true;
      if( argc > i + 1 )
        signalChannels = ::atoi( argv[ i + 1 ] );
      if( argc > i + 2 )
        signalElements = ::atoi( argv[ i + 2 ] );
    }
  }

  const char newl = '\n';
  const char tab = '\t';
  Table table( outputType );

  switch( outputType )
  {
    case latex:
      cout << "\\subsection{Module \\texttt{" << argv[ 0 ] << "}}\n";
      break;
    default:
      cout << "Module: " << argv[ 0 ] << newl << newl;
      break;
  }

  PARAMLIST allParams;
  STATELIST allStates;

  SignalProperties inputProperties,
                   outputProperties( signalChannels, signalElements );
  string currentPos;
  typedef set<string> infoset;
  infoset filtersManipulatingCurrentOutput;
  ostringstream report;
  Table filterTable( outputType );
  filterTable << "Position" << tab << "Filter Name";
  if( reportSignals )
    filterTable << tab
                << "Input dimensions" << tab << "Output dimensions";
  filterTable << newl;

  for( GenericFilter::registrarSet::iterator i
           = GenericFilter::Registrar::Registrars().begin();
           i != GenericFilter::Registrar::Registrars().end(); ++i )
  {
    PARAMLIST filterParams;
    STATELIST filterStates;

    Environment::EnterConstructionPhase( &filterParams, &filterStates, NULL, NULL );
    GenericFilter* filter = ( *i )->NewInstance();

    if( filterParams.size() > 0 )
    {
      report << newl
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
      report << table << newl;
      allParams.insert( filterParams.begin(), filterParams.end() );
    }

    if( filterStates.GetNumStates() > 0 )
    {
      report << newl
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
      report << table << newl;
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
      filtersManipulatingCurrentOutput.insert( typeid( *filter ).name() );
    if( currentPos != ( *i )->GetPosition() )
      filterTable << ( *i )->GetPosition();
    filterTable << tab << typeid( *filter ).name();
    if( reportSignals )
      filterTable << tab
                  << "(" << inputProperties.Channels() << ", "
                  << inputProperties.MaxElements() << ')' << tab
                  << "(" << outputProperties.Channels() << ", "
                  << outputProperties.MaxElements() << ")";
    filterTable << newl;
  }

  if( GenericFilter::Registrar::Registrars().size() == 0 )
    cout << "No filters registered." << newl;
  else
    cout << filterTable << newl;
  cout << report.str() << newl;

  return 0;
}

