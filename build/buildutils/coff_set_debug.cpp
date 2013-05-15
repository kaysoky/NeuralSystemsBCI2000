// A tool to set the debug flag on COFF obj file sections.
// This should be possible using objcopy --set-section-flags but that seems to be broken
// for the debug flag.
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main( int argc, char** argv )
{
  const char* pFile = 0;
  bool verbose = false;
  vector<string> sections;
  int value = 1;
  for( int i = 1; i < argc; ++i )
  {
    if( *argv[i] != '-' )
    {
      if( !pFile )
        pFile = argv[i];
      else
      {
        cerr << "Only one file may be given." << endl;
        return -1;
      }
    }
    else switch( argv[i][1] )
    {
      case 'v':
        verbose = true;
        break;
      case 's':
        sections.push_back( argv[i] + 2 );
        break;
      case '1':
        value = 1;
        break;
      case '0':
        value = 0;
        break;
      default:
        cerr << "Illegal option." << endl;
        return -1;
    }
  }
  if( !pFile )
  {
    cerr << "Expected name of a COFF object file or archive" << endl;
    return -1;
  }
  fstream file;
  file.open( pFile, ios_base::in | ios_base::out | ios_base::binary );
  if( !file.is_open() )
  {
    cerr << "Could not open file in read/write mode: " << pFile << endl;
    return -1;
  }
  vector<size_t> offsets( 1, 0 );
  const char sig[] = "!<arch>\n";
  char ghdr[sizeof( sig ) - 1];
  file.read( ghdr, sizeof( ghdr ) );
  if( !::memcmp( sig, ghdr, sizeof( ghdr ) ) )
  {
    offsets.clear();
    char fhdr[60];
    while( file.read( fhdr, sizeof( fhdr ) ) && fhdr[58] == 0x60 && fhdr[59] == 0x0a )
    {
      int length = ::atoi( fhdr + 48 );
      if( ::memcmp( fhdr, "/ ", 2 ) && ::memcmp( fhdr, "// ", 3 ) && length > 0 )
        offsets.push_back( file.tellg() );
      if( length & 1 )
        ++length;
      file.seekg( length, ios_base::cur );
    }
  }
  file.clear();
  int count = 0;
  for( size_t f = 0; file && f < offsets.size(); ++f )
  {
    file.seekg( offsets[f] );
    char header[40] = "";
    file.read( header, 20 );
    unsigned char* p = reinterpret_cast<unsigned char*>( header + 2 );
    int nSections = *p++;
    nSections |= *p << 8;
    for( int i = 0; i < nSections; ++i )
    {
      file.read( header, sizeof( header ) );
      string name( header, 8 );
      name = name.substr( 0, ::strlen( name.c_str() ) );
      if( find( sections.begin(), sections.end(), name ) != sections.end() )
      {
        int offset = 3,
            debug = 2,
            flags = header[sizeof( header ) - offset];
        bool write = false;
        if( value && !( flags & debug ) )
        {
          flags |= debug;
          ++count;
          write = true;
        }
        else if( !value && ( flags & debug ) )
        {
          flags &= ~debug;
          ++count;
          write = true;
        }
        if( write )
        {
          size_t pos = file.tellg();
          file.seekp( -offset, ios_base::cur );
          file.put( flags );
          file.seekg( pos );
        }
      }
    }
  }
  if( !file )
  {
    cerr << "Error modifying flags" << endl;
    return -1;
  }
  if( verbose && value )
    cout << "Set debug flags on " << count << " sections." << endl;
  else if( verbose )
    cout << "Cleared debug flag on " << count << " sections. " << endl;
  return 0;
}
