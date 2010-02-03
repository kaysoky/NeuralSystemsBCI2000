///////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: This "twoDText" class holds the variables of the 2D text that is
//   going to lay on the top of the 2D overlay
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#ifndef TWO_D_TEXT_H
#define TWO_D_TEXT_H

#include "glheaders.h"
#include "primObj.h"
#include <map>

class twoDText : public overlayObj
{
private:
        GLfloat         textX;
        GLfloat         textY;
        std::string     textFont;
        std::string     caption;
        int             textSize;
        int             alignment;
        GLfloat         textColorR;
        GLfloat         textColorG ;
        GLfloat         textColorB;
        GLuint          fontID;                 //storage for the 2D font

public:
        enum { left, center, right };
/****************************CONSTRUCTOR***************************************/
        twoDText(Scene& inScene):overlayObj(inScene),
                   textX(0),textY(0),textSize(12),alignment(left),
                   textColorR(0),textColorG(0),textColorB(0),fontID(NULL)
                  {setDrawingOrder(3);}
        virtual ~twoDText() {}
/****************************Accessor******************************************/
        const std::string& getCaption() const   {return caption;}
        GLfloat getTextX() const                {return textX;}
        GLfloat getTextY() const                {return textY;}
        const std::string& getTextFont() const  {return textFont;}
        int     getTextSize() const             {return textSize;}
        int     getAlignment() const            {return alignment;}
        GLfloat getColorR() const               {return textColorR;}
        GLfloat getColorG() const               {return textColorG;}
        GLfloat getColorB() const               {return textColorB;}
        GLuint  getFontID() const               {return fontID;}
/****************************Modifiers*****************************************/

        void setCaption (const std::string& cap)     {caption = cap;}
        void setTextPosition (GLfloat x, GLfloat y)
        {
                textX = x;
                textY = y;
        }
        void setTextFont (const std::string& FontName)
        {
                textFont = FontName;
        }
        void setAlignment( int a )
        {
                alignment = a;
        }
        void setTextColor (GLfloat R, GLfloat G, GLfloat B)
        {
                textColorR = R;
                textColorG = G;
                textColorB = B;
        }
        void setTextSize(int size)              {textSize = size;}

protected:
        virtual void onInitialize();
        virtual void onCleanup();
        virtual void onRender() const;

};//twoDText

#endif // TWO_D_TEXT_H
