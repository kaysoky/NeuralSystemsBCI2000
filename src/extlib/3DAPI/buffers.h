////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: A buffer class to make loading textures and fonts
//   more efficient.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BUFFERS_H
#define BUFFERS_H

#include "glheaders.h"
#include <map>
#include <string>

class buffers
{
 public:
  static AUX_RGBImageRec* loadWindowsBitmap( const std::string& );

  static GLuint loadTexture( const std::string& );
  static void releaseTexture( const std::string& );

  static GLuint loadFont2D( const std::string&, int );
  static void releaseFont2D( const std::string&, int );
  static const ABCFLOAT* getFontData2D( GLuint );

  static GLuint loadFont3D( const std::string& );
  static void releaseFont3D( const std::string& );
  static const GLYPHMETRICSFLOAT* getFontData3D( GLuint );

 private:
  static std::map<std::string, GLuint> sTextureHandles;
  static std::map<GLuint, int> sTextureUsage;

  static std::string buildFontName2D( const std::string&, int );
  struct Font2DEntry
  {
    Font2DEntry() : usage( 0 ) {}
    ABCFLOAT abcf[256];
    int      usage;
  };
  static std::map<std::string, GLuint> sFontHandles2D;
  static std::map<GLuint, Font2DEntry> sFontData2D;

  struct Font3DEntry
  {
    Font3DEntry() : usage( 0 ) {}
    GLYPHMETRICSFLOAT gmf[256];
    int               usage;
  };
  static std::map<std::string, GLuint> sFontHandles3D;
  static std::map<GLuint, Font3DEntry> sFontData3D;
};

#endif // BUFFERS_H


