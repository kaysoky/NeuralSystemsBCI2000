////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A program that context-sensitively adds text to
//              input read from stdin.
//              See the sUsage variable for details.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <vector>

using namespace std;

static const char* sUsage =
 "Usage: add_lines <line1 begin> <line2 begin> <line1> <line2> ...\n"
 " Reading from stdin, the program waits for a line beginning with <line1 begin>,\n"
 " and then a line beginning with <line2 begin>.\n"
 " Then, inserts <line1> <line2> and all the following arguments as\n"
 " separate lines before that second line.\n"
 " All other text is forwarded to stdout unchanged.\n"
 " The program returns false if no match was found.\n";

int
main( int argc, char* argv[] )
{
  if( argc < 3 )
  {
    cout << sUsage;
    return -1;
  }
  string line1Begin = argv[ 1 ],
         line2Begin = argv[ 2 ];
  vector<string> lines;
  for( int i = 3; i < argc; ++i )
    lines.push_back( argv[ i ] );

  bool didInsert = false;

  string curLine;
  while( getline( cin, curLine )
    && curLine.substr( 0, line1Begin.length() ) != line1Begin )
    cout << curLine << '\n';
  cout << curLine << '\n';

  while( getline( cin, curLine )
    && curLine.substr( 0, line2Begin.length() ) != line2Begin )
    cout << curLine << '\n';

  if( cin )
  {
    didInsert = true;
    for( vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i )
      cout << *i << '\n';
  }

  do
    cout << curLine << '\n';
  while( getline( cin, curLine ) );

  return didInsert ? 0 : 1;
}
