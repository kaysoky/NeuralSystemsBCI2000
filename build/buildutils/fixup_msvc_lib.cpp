// A tool to replace backslashes in MSVC .lib files, allowing GNU binutils to
// manipulate the resulting files.
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main( int argc, char** argv )
{
  if( argc < 2 )
  {
    cerr << "Expected name of a library archive" << endl;
    return -1;
  }
  fstream file;
  file.open( argv[1], ios_base::in | ios_base::out | ios_base::binary );
  if( !file.is_open() )
  {
    cerr << "Could not open file in read/write mode: " << argv[1] << endl;
    return -1;
  }
  char header[60] = "";
  const string sig = "!<arch>\n";
  file.read( header, sig.length() );
  bool found = false;
  int length = 0;
  if( sig == string( header, sig.length() ) ) do
  {
    file.read( header, sizeof( header ) );
    length = ::atoi( header + 48 );
    if( length % 2 )
      ++length;
    found = ( header[0] == '/' && header[1] == '/' );
  } while( !found && file.seekg( length, ios_base::cur ) );
  if( !found )
  {
    cerr << "Not a library archive: " << argv[1] << endl;
    return -1;
  }
  for( int i = 0; i < length; ++i )
  {
    if( file.get() == '\\' )
    {
      file.seekp( file.tellg() - streamoff( 1 ) );
      file.put( '_' );
      file.seekg( file.tellp() );
    }
  }
  if( !file )
  {
    cerr << "Error replacing backslash characters" << endl;
    return -1;
  }
  return 0;
}
