/////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A command line tool that splits BCI2000 dat files into
//   a number of smaller BCI2000 dat files, in order to work around
//   the 2 GB limitation in load_bcidat.
////////////////////////////////////////////////////////////////////////
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

int
main( int argc, char** argv )
{
  bool printUsage = false,
       verbose = false;
  int outputSizeM = 1024;
  const char* inputFileName = NULL;
  if( argc < 2 )
    printUsage = true;
  for( int i = 1; i < argc; ++i )
  {
    if( *argv[i] == '-' )
    {
      switch( argv[i][1] )
      {
        case 's':
        case 'S':
          outputSizeM = atoi( argv[i] + 2 );
          break;

        case 'v':
        case 'V':
          verbose = true;
          break;

        case 'h':
        case 'H':
        default:
          printUsage = true;
          break;
      }
    }
    else
      inputFileName = argv[i];
  }
  if( inputFileName == NULL )
    printUsage = true;

  if( printUsage )
  {
    cout << "Usage: split_bcidat -s<output size in MB> <input file>" << endl;
    return 0;
  }

  ifstream inputFile( inputFileName, ios::in | ios::binary );
  if( !inputFile.is_open() )
  {
    cerr << "Could not open " << inputFileName << " for input" << endl;
    return -1;
  }

  int headerLen = 0,
      sourceCh = 0,
      statevectorLen = 0,
      dataSize = 0;
  string token;
  inputFile >> token;
  if( token == "BCI2000V=" )
  {
    inputFile >> token >> token >> headerLen
              >> token >> sourceCh
              >> token >> statevectorLen
              >> token >> token;
    if( token == "float32" )
      dataSize = 4;
    else if( token == "int32" )
      dataSize = 4;
    else if( token == "int16" )
      dataSize = 2;
  }
  else if( token == "HeaderLen=" )
  {
    inputFile >> headerLen >> token
              >> sourceCh >> token
              >> statevectorLen;
    dataSize = 2;
  }
  if( headerLen == 0 || dataSize == 0 )
  {
    cerr << "Could not read " << inputFileName << " as a BCI2000 data file" << endl;
    return -1;
  }

  char* pHeader = new char[headerLen];
  inputFile.seekg( 0, ios_base::beg );
  inputFile.read( pHeader, headerLen );
  int blockSize = 1024 * ( sourceCh * dataSize + statevectorLen ),
      nBlocks = ( outputSizeM * 1024 * 1024 - headerLen ) / blockSize;
  if( nBlocks < 1 )
    nBlocks = 1;
  char* pBuffer = new char[blockSize];
  int part = 1;
  while( inputFile )
  {
    string outputFileName = inputFileName;
    outputFileName = outputFileName.substr( 0, outputFileName.length() - 4 );
    ostringstream oss;
    oss << "P" << setw(2) << setfill('0') << part << ".dat";
    outputFileName = outputFileName + oss.str();
    ofstream outputFile( outputFileName.c_str(), ios::out | ios::binary );
    if( !outputFile.is_open() )
    {
      cerr << "Could not open " << outputFileName << " for output" << endl;
      return -1;
    }
    if( verbose )
      cout << "Writing " << outputFileName << endl;
    outputFile.write( pHeader, headerLen );
    for( int block = 0; ( block < nBlocks ) && inputFile; ++block )
    {
      inputFile.read( pBuffer, blockSize );
      outputFile.write( pBuffer, inputFile.gcount() );
      if( verbose )
        cout << '.' << flush;
    }
    if( verbose )
      cout << endl;
    ++part;
  }
}

