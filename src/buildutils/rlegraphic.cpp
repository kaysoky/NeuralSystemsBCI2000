////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A program that converts graphics files into rle-
//   encoded C++ resource files.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include <vcl.h>

int ToGray( int c )
{
  int gray = ( c & 0xff ) + ( ( c >> 8 ) & 0xff ) + ( ( c >> 16 ) & 0xff );
  return gray / 3;
}

int
main( int argc, char** argv )
{
  const char* usage = "Usage: rlegraphic <option> <filename>\n"
                      " Options are: --help Show this help\n";

  const char* inputFile = NULL;
  bool showUsage = argc < 2;
  for( int i = 1; i < argc; ++i )
  {
    if( string( "--help" ) == argv[ i ] )
      showUsage = true;
    else
      inputFile = argv[ i ];
  }
  if( showUsage )
  {
    cout << usage;
    return -1;
  }
  TPicture* pPicture = NULL;
  Graphics::TBitmap*  pBitmap = NULL;
  TCanvas*  pCanvas = NULL;
  try
  {
    pPicture = new TPicture;
    pPicture->LoadFromFile( inputFile );
    pBitmap = new Graphics::TBitmap;
    pBitmap->Width = pPicture->Width;
    pBitmap->Height = pPicture->Height;
    pCanvas = pBitmap->Canvas;
    pCanvas->Draw( 0, 0, pPicture->Graphic );
  }
  catch( ... )
  {
    cerr << "Could not load " << inputFile << " as a graphic file." << endl;
    return -1;
  }
  string fileName = ChangeFileExt( ExtractFileName( inputFile ), "" ).c_str();
  int color = ToGray( pCanvas->Pixels[0][0] ),
      count = 0;
  cout << hex << "struct\n"
          "{\n"
          "  int width,  // pixels per scan line\n"
          "      height; // number of scan lines\n"
          "  struct\n"
          "  { // run-length encoded bitmap data\n"
          "    char  color;\n"
          "    short count;\n"
          "  } data[];\n"
          "}\n" << fileName << "_rle[] =\n"
          "{\n"
          "  {0,0x" << pBitmap->Width << "}, // width\n"
          "  {0,0x" << pBitmap->Height << "}, // height\n";
  for( int y = 0; y < pPicture->Height; ++y )
    for( int x = 0; x < pPicture->Width; ++x )
    {
      if( ToGray( pCanvas->Pixels[x][y] ) != color )
      {
        cout << "  {0x" << color << ",0x" << count << "},\n";
        color = ToGray( pCanvas->Pixels[x][y] );
        count = 0;
      }
      ++count;
    }
  if( count > 0 )
    cout << "  {0x" << color << ",0x" << count << "},\n";
  cout << "  {0x0,0x0} // terminating entry\n"
          "};\n";
  delete pBitmap;
  delete pPicture;
  return 0;
}

