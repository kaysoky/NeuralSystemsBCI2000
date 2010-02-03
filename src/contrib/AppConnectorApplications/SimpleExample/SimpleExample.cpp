////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An example that illustrates how to read/write data from an UDP
//   port.
//   Data read from the UDP port are written to stdout; data written into the
//   UDP port are read from stdin.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include "SockStream.h"

using namespace std;

int
main( int argc, char* argv[] )
{
  bool write = false;
  if( argc > 2 )
  {
    string arg = string( argv[1] );
    if( arg == "-w" )
      write = true;
    else if( arg == "-r" )
      write = false;
  }

  const char* address = "localhost:20320";
  if( argc > 1 )
    address = argv[argc-1];

  if( write )
  { // Read from stdin, write to socket.
    sending_udpsocket socket( address );
    sockstream output( socket );
    if( !output.is_open() )
      cerr << "Could not open " << address
           << " for output." << endl;

    string line;
    while( getline( cin, line ) )
      output << line << endl;
  }
  else
  { // Read from socket, write to stdout.
    receiving_udpsocket socket( address );
    sockstream input( socket );
    if( !input.is_open() )
      cerr << "Could not open " << address
           << " for input." << endl;

    string line;
    while( getline( input, line ) )
      cout << line << endl;
  }
  return 0;
}
