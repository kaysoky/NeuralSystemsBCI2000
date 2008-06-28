////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An example that illustrates how to read data from an UDP port.
//   Data are written to stdout.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include "SockStream.h"

using namespace std;

int
main( int argc, char* argv[] )
{
  const char* address = "localhost:20320";
  if( argc > 1 )
    address = argv[1];

  receiving_udpsocket socket( address );
  sockstream input( socket );
  if( !input.is_open() )
    cerr << "Could not open " << address
         << "." << endl;

  string line;
  while( getline( input, line ) )
    cout << line << endl;
  return 0;
}
