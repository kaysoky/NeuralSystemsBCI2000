////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: A buffer class to make loading textures and fonts
//   more efficient.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "buffers.h"
#include <fstream>
#include <sstream>

using namespace std;


map<string, GLuint> buffers::sTextureHandles;
map<GLuint, int> buffers::sTextureUsage;

map<string, GLuint> buffers::sFontHandles2D;
map<GLuint, buffers::Font2DEntry> buffers::sFontData2D;

map<std::string, GLuint> buffers::sFontHandles3D;
map<GLuint, buffers::Font3DEntry> buffers::sFontData3D;

AUX_RGBImageRec*
buffers::loadWindowsBitmap( const std::string& inBitmapFile )
{
   AUX_RGBImageRec* result = NULL;
   HBITMAP bmp = ::LoadImage( NULL, inBitmapFile.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
   if( bmp != NULL )
   {
     result = reinterpret_cast<AUX_RGBImageRec*>( ::malloc( sizeof( AUX_RGBImageRec ) ) );
     BITMAPINFO bmi;
     bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
     bmi.bmiHeader.biBitCount = 0;
     HDC dc = ::CreateCompatibleDC( NULL );
     bool success = ::GetDIBits( dc, bmp, 0, 0, NULL, &bmi, DIB_RGB_COLORS );
     if( success )
     {
       bmi.bmiHeader.biBitCount = 24;
       bmi.bmiHeader.biCompression = BI_RGB;
       bmi.bmiHeader.biPlanes = 1;
       result->sizeX = bmi.bmiHeader.biWidth;
       result->sizeY = bmi.bmiHeader.biHeight;
       result->data = NULL;
       success = ::GetDIBits( dc, bmp, 0, result->sizeY, NULL, &bmi, DIB_RGB_COLORS );
       if( success )
       {
         result->data = reinterpret_cast<unsigned char*>( ::malloc( bmi.bmiHeader.biSizeImage ) );
         success = ::GetDIBits( dc, bmp, 0, result->sizeY, result->data, &bmi, DIB_RGB_COLORS );
         if( !success )
           ::free( result->data );
       }
       if( success )
       {
         for( int i = 0; i < result->sizeX * result->sizeY; ++i )
         { // Convert Windows BGR into GL RGB
           unsigned char tmp = result->data[ 3 * i ];
           result->data[ 3 * i ] = result->data[ 3 * i + 2 ];
           result->data[ 3 * i + 2 ] = tmp;
         }
       }
     }
     if( !success )
     {
       ::free( result );
       result = NULL;
     }
     ::DeleteDC( dc );
     ::DeleteObject( bmp );
   }
   return result;
}

GLuint
buffers::loadTexture( const std::string& inTextureFile )
{
  GLuint textureHandle = NULL;
  if( !inTextureFile.empty() )
  {
    textureHandle = sTextureHandles[inTextureFile];
    if( textureHandle == NULL )
    {
      bool fileExists = ifstream( inTextureFile.c_str() ).is_open();
      if( fileExists )
      {
        AUX_RGBImageRec* texImg = loadWindowsBitmap( inTextureFile.c_str() );
        if( texImg )
        {
          glGenTextures( 1, &textureHandle );
          if( textureHandle != NULL )
          {
            glBindTexture( GL_TEXTURE_2D, textureHandle );
            gluBuild2DMipmaps( GL_TEXTURE_2D, 3, texImg->sizeX, texImg->sizeY, GL_RGB, GL_UNSIGNED_BYTE, texImg->data );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glBindTexture( GL_TEXTURE_2D, NULL );
          }
          if( texImg->data )
            ::free( texImg->data );
          ::free( texImg );
        }
      }
      sTextureHandles[inTextureFile] = textureHandle;
    }
    if( textureHandle != NULL )
      ++sTextureUsage[textureHandle];
  }
  return textureHandle;
}

void
buffers::releaseTexture( const std::string& inTextureFile )
{
  GLuint textureHandle = sTextureHandles[inTextureFile];
  if( textureHandle && --sTextureUsage[textureHandle] < 1 )
  {
    glDeleteTextures( 1, &textureHandle );
    sTextureHandles.erase( inTextureFile );
    sTextureUsage.erase( textureHandle );
  }
}


std::string
buffers::buildFontName2D( const std::string& inFontName, int inFontSize )
{
  ostringstream fontName;
  fontName << inFontName << inFontSize;
  return fontName.str();
}

GLuint
buffers::loadFont2D( const std::string& inFontName, int inFontSize )
{
  string name = buildFontName2D( inFontName, inFontSize );
  GLuint fontID = sFontHandles2D[name];
  if( fontID == NULL )
  {
    fontID = glGenLists(256); // Generate the list for the font
    HFONT hFont = ::CreateFont(
      inFontSize,           // Our desired HEIGHT of the font
      0,                  // The WIDTH (If we leave this zero it will pick the best width depending on the height)
      0,                  // The angle of escapement
      0,                  // The angle of orientation
      FW_BOLD,            // The font's weight (We want it bold)
      FALSE,              // Italic - We don't want italic
      FALSE,              // Underline - We don't want it underlined
      FALSE,              // Strikeout - We don't want it strikethrough
      ANSI_CHARSET,       // This is the type of character set
      OUT_TT_PRECIS,      // The Output Precision
      CLIP_DEFAULT_PRECIS,        // The Clipping Precision
      ANTIALIASED_QUALITY,        // The quality of the font - We want anitaliased fonts
      FF_DONTCARE|DEFAULT_PITCH,  // The family and pitch of the font.  We don't care.
      inFontName.c_str()          // The font name (Like "Arial", "Courier", etc...)
    );
    HDC hDC = ::CreateCompatibleDC( NULL );
    HFONT oldFont = ::SelectObject(hDC, hFont);
    if( oldFont )
    {
      ::wglUseFontBitmaps(hDC, 0,255, fontID);  // Builds 255 bitmap characters
      ::GetCharABCWidthsFloat(hDC,0,255,sFontData2D[fontID].abcf);
      ::SelectObject(hDC, oldFont); // Selects The Font We Want
      sFontHandles2D[name] = fontID;
      sFontData2D[fontID].usage = 1;
    }
    else
    {
      fontID = NULL;
      glDeleteLists( fontID, 256 );
    }
    ::DeleteObject(hFont);  // Delete The Font
    ::DeleteDC( hDC );
  }
  return fontID;
}

void
buffers::releaseFont2D( const std::string& inFontName, int inFontSize )
{
  string name = buildFontName2D( inFontName, inFontSize );
  GLuint fontID = sFontHandles2D[name];
  if( fontID != NULL && --sFontData2D[fontID].usage < 1 )
  {
    glDeleteLists( fontID, 256 );
    sFontHandles2D.erase( name );
    sFontData2D.erase( fontID );
  }
}

const ABCFLOAT*
buffers::getFontData2D( GLuint inHandle )
{
  return sFontData2D.find( inHandle ) != sFontData2D.end()
         ? sFontData2D[inHandle].abcf
         : NULL;
}


GLuint
buffers::loadFont3D( const std::string& inFontName )
{
  GLuint fontID = sFontHandles3D[inFontName];
  if( fontID == NULL )
  { //build the font
    fontID = glGenLists( 256 ); // Storage for 256 characters
    HDC hDC = ::CreateCompatibleDC( NULL );
    HFONT hFont = ::CreateFont( 0,                           // Height of font
                                0,                           // Width of font
                                0,                           // Angle of escapement
                                0,                           // Orientation angle
                                FW_BOLD,                     // Font weight
                                FALSE,                       // Italic
                                FALSE,                       // Underline
                                FALSE,                       // Strikeout
                                ANSI_CHARSET,                // Character set identifier
                                OUT_TT_PRECIS,               // Output precision
                                CLIP_DEFAULT_PRECIS,         // Clipping precision
                                PROOF_QUALITY,               // Output quality
                                FF_DONTCARE|DEFAULT_PITCH,   // Family and pitch
                                inFontName.c_str());         // Font name

    HFONT prevFont = ::SelectObject( hDC, hFont ); // Selects the font we created
    bool success = ::wglUseFontOutlines(  hDC,               // Select the temporary DC
                                          0,                 // Starting character
                                          256,               // Number of display lists to build
                                          fontID,            // Starting display lists
                                          0.01,              // Deviation from the true outlines
                                          0.3,               // Font thickness in the Z direction
                                          WGL_FONT_POLYGONS, // Use polygons, not lines
                                          sFontData3D[ fontID ].gmf // Address of buffer to receive data
                                        );
    ::SelectObject( hDC, prevFont );
    ::DeleteObject( hFont );
    ::DeleteDC( hDC );
    if( success )
    {
      sFontHandles3D[ inFontName ] = fontID;
      sFontData3D[ fontID ].usage = 1;
    }
    else
    {
      glDeleteLists( fontID, 256 );
      sFontHandles3D.erase( inFontName );
      fontID = NULL;
    }
  }
  return fontID;
}

void
buffers::releaseFont3D( const std::string& inFontName )
{
  GLuint fontID = sFontHandles3D[inFontName];
  if( fontID != NULL && --sFontData3D[fontID].usage < 1 )
  {
    glDeleteLists( fontID, 256 );
    sFontHandles3D.erase( inFontName );
    sFontData3D.erase( fontID );
  }
}

const GLYPHMETRICSFLOAT*
buffers::getFontData3D( GLuint inHandle )
{
  return sFontData3D.find( inHandle ) != sFontData3D.end()
         ? sFontData3D[inHandle].gmf
         : NULL;
}


