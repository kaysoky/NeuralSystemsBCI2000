// A tool to extract (mangled) export names from a dynamic library.
#include <iostream>
#include "DylibImports.h"

using namespace std;

int main( int argc, char** argv )
{
  if( argc < 2 )
  {
    cerr << "Expected name of a dynamic library (without extension)" << endl;
    return -1;
  }
  Dylib::Library lib( argv[1] );
  if( !lib.Error().empty() )
  {
    cerr << lib.Error() << endl;
    return -1;
  }
  const Dylib::Exports& exports = lib.Exports();
  cout << "EXPORTS\n";
  for( size_t i = 0; i < exports.size(); ++i )
    cout << exports[i].name << "\n";
  return 0;
}
