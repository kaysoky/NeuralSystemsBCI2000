// A tool to manipulate flags on COFF obj file sections.
// This should be possible using objcopy --set-section-flags but that seems to be broken
// for the debug flag.
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <inttypes.h>
#include "BinaryData.h"

using namespace std;

static const struct { const string name; uint32_t pattern; } flags[] =
{
  { "debug", 2 },
  { "read", 0x40000000 },
  { "write", 0x80000000 },
};

int main( int argc, char** argv )
{
  const char* pFile = 0;
  bool verbose = false;
  vector<string> sections;
  vector< pair<uint32_t, uint32_t> > setflags;
  for( int i = 1; i < argc; ++i )
  {
    uint32_t value = 0;
    bool found = false;
    switch( *argv[i] )
    {
      case '+':
        value = 1;
        /* fall through */
      case '-':
        switch( argv[i][1] )
        {
          case 'v':
            verbose = true;
            break;
          case 's':
            sections.push_back( argv[i] + 2 );
            break;
          default:
            for( size_t j = 0; j < sizeof( flags ) / sizeof( *flags ); ++j )
              if( flags[j].name == argv[i] + 1 )
              {
                found = true;
                setflags.push_back( make_pair( flags[j].pattern, value ) );
              }
            if( !found )
            {
              cerr << "Unknown flag name: " << argv[i] + 1 << endl;
              return -1;
            }
        }
        break;

      default:
        if( !pFile )
          pFile = argv[i];
        else
        {
          cerr << "Only one file may be given." << endl;
          return -1;
        }
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
  vector<streamoff> offsets( 1, 0 );
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
        bci::BinaryData<uint32_t, bci::LittleEndian> flags;
        streamoff pos = file.tellg();
        file.seekg( -flags.Size(), ios_base::cur );
        flags.Get( file );

        bool write = false;
        for( size_t j = 0; j < setflags.size(); ++j )
        {
          int pattern = setflags[j].first,
              value = setflags[j].second;
          if( value && !( flags & pattern ) )
          {
            flags = flags | pattern;
            write = true;
          }
          else if( !value && ( flags & pattern ) )
          {
            flags = flags & ~pattern;
            write = true;
          }
        }
        if( write )
        {
          ++count;
          file.seekp( pos - flags.Size() );
          flags.Put( file );
        }
      }
    }
  }
  if( !file )
  {
    cerr << "Error modifying flags" << endl;
    return -1;
  }
  if( verbose )
    cout << "Set flags on " << count << " sections." << endl;
  return 0;
}
