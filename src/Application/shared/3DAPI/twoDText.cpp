/*
This "twoDText" class holds the variabled of the 2D text that is going to lay on
the top of the 2D overlay
*/

//---------------------------------------------------------------------------
#include "header.h"


class twoDText
{
private:
        GLfloat         textX;
        GLfloat         textY;
        string          textFont;
        string          caption;
        int             textSize ;
        GLfloat         textColorR;
        GLfloat         textColorG ;
        GLfloat         textColorB;

        GLuint          fontID;                 //storage for the 2D font
        HFONT           oldFont;

        

public:
        
/****************************CONSTRUCTOR***************************************/
        twoDText()                              {setAll();}
        void setAll();
/****************************Accessor******************************************/
        string  getCaption()                    {return caption;}
        GLfloat getTextX()                      {return textX;}
        GLfloat getTextY()                      {return textY;}
        string  getTextFont()                   {return textFont;}
        int     getTextSize()                   {return textSize;}
        GLfloat getColorR()                     {return textColorR;}
        GLfloat getColorG()                     {return textColorG;}
        GLfloat getColorB()                     {return textColorB;}
        GLuint  getFontID()                     {return fontID;}
        HFONT    getOldFont()                               {return oldFont;}
/****************************Modifiers*****************************************/

        void setCaption (string cap)            {caption = cap;}
	void setTextPosition (GLfloat x, GLfloat y)
        {
                textX = x;
                textY = y;
        }
	void setTextFont (string FontName)
        {
                textFont = FontName;
        }
	void setTextColor (GLfloat R, GLfloat G, GLfloat B)
        {
                textColorR = R;
                textColorG = G;
                textColorB = B;
        }
	void setTextSize(int font)              {textSize = font;}
        void setFontID(GLuint input)            {fontID = input;}
        void setOldFont(HFONT font)                     {oldFont = font;}



};//twoDText

void twoDText::setAll()
{
        caption = "";
        textX = 0.0f;
        textY = 0.0f;
        textFont = "";
        textSize = 12;
        textColorR = 0.0f;
        textColorG  = 0.0f;
        textColorB   = 0.0f;
        fontID = NULL;
        oldFont = NULL;
}//setAll
