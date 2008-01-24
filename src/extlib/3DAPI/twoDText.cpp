///////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: This "twoDText" class holds the variables of the 2D text that is
//   going to lay on the top of the 2D overlay
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "twoDText.h"
#include "buffers.h"
#include <sstream>

using namespace std;

void
twoDText::onInitialize()
{
  fontID = buffers::loadFont2D( textFont, textSize );
  overlayObj::onInitialize();
}

void
twoDText::onCleanup()
{
  buffers::releaseFont2D( textFont, textSize );
  fontID = NULL;
  overlayObj::onCleanup();
}

void
twoDText::onRender() const
{
  glDisable( GL_TEXTURE_1D );
  glDisable( GL_TEXTURE_2D );
  glColor3f( textColorR, textColorG, textColorB );
  glMatrixMode( GL_MODELVIEW );

  const ABCFLOAT* metrics = buffers::getFontData2D( fontID );
  if( metrics != NULL )
  {
    istringstream iss( caption );
    string line;
    int i = 0;
    while( getline( iss, line ) )
    {
      GLfloat delta = 0;
      for( size_t j = 0; j < line.length(); ++j )
        delta += metrics[line[j]].abcfA
               + metrics[line[j]].abcfB
               + metrics[line[j]].abcfC;
      switch( alignment )
      {
        case twoDText::left:
          delta = 0;
          break;
        case twoDText::right:
          delta = -delta;
          break;
        case twoDText::center:
          delta = -delta / 2;
          break;
      }
      glRasterPos2f( textX + delta,  textY - i * textSize ); // Set the drawing position
      // Now, before we set the list base, we need to save off the current one.
      glPushAttrib(GL_LIST_BIT);                    // This saves the list base information
      // Then we want to set the list base to the font's list base, which should be 1 in our case.
      // That way when we call our display list it will start from the font's lists'.
      glListBase(fontID);                           // This sets the lists base
      // Now comes the actually rendering.  We pass in the length of the string,
      // then the data types (which are characters so its a UINT), then the actually char array.
      // This will then take the ASCII value of each character and associate it with a bitmap.
      glCallLists( line.length(), GL_UNSIGNED_BYTE, line.data() );
      glPopAttrib();
      ++i;
    }
  }
}

