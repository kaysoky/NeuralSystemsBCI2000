/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
/*
This "twoDOverlay" class resembles the 2D overlay on top of the 3D environment.
The 2D
*/

//---------------------------------------------------------------------------
#include "header.h"
class twoDOverlay
{
private:
        bool                    isOn;
        bool                    hasPicTexture;
        bool                    hasTraTexture;

        string                  overlayPicture; //overlay picture file name
        
        string                  overlayTrans;   //overlay transparency file name
        
        int                     overlayBri;
        GLfloat	                cursorX;
	GLfloat 	        cursorY;

	GLfloat	                cursorSizeRad;  //this is the radius of the cursor
	GLfloat	                cursorG;
	GLfloat	                cursorB;
	GLfloat	                cursorR;

        
public:
        GLuint                  pictureData;
        GLuint                  transData;
/**************************************CONTRUCTRO******************************/
        twoDOverlay()                                   {setAll();}
        void setAll();
/***********************************Modifier***********************************/
        void setOverlayPicture (string pic)             {overlayPicture = pic; }
        void setOverlayTrans (string tra)               {overlayTrans = tra;   }
        void setOverStatus (bool onOrOff)               {isOn = onOrOff;}
	void setCurPosition (GLfloat textX, GLfloat textY)
        {
                cursorX = textX;
                cursorY = textY;
        }
        void setCurColor (GLfloat R, GLfloat G, GLfloat B)
        {
                cursorR = R;
                cursorB = B;
                cursorG = G;
        }
	void setCursorSizeRad (GLfloat rad)             {cursorSizeRad = rad; }
        void setOverlayBri(int bri)                     {overlayBri = bri;}
        void setPictureData(GLuint data)                {pictureData = data;}
        void setTransData(GLuint data)                  {transData = data;}
        void setPicTexture(bool status)                 {hasPicTexture = status;}
        void setTraTexture(bool status)                 {hasTraTexture = status;}
        
/***************************Accessor*******************************************/
        string  getOverlayPicture()                     {return overlayPicture;}
        string  getOverlayTrans()                       {return overlayTrans;}
        bool    getStatus()                             {return isOn;}
        GLfloat getCurX()                              {return cursorX;}
        GLfloat getCurY()                              {return cursorY;}
        GLfloat getColorR()                             {return cursorR;}
        GLfloat getColorB()                             {return cursorB;}
        GLfloat getColorG()                             {return cursorG;}
        GLfloat getCursorSize()                         {return cursorSizeRad;}
        int     getOverlayBri()                         {return overlayBri;}
        GLuint  getPictureData()                        {return pictureData;}
        GLuint  getTransData()                          {return transData;}
        bool getPicTexture()                            {return hasPicTexture;}
        bool getTraTexture()                            {return hasTraTexture;}

};//twoDOverlay


void twoDOverlay::setAll()
{
        isOn = true;
        hasPicTexture = false;
        hasTraTexture = false;
        overlayPicture = "";
        overlayTrans = "";
        overlayBri = 255;
        cursorX = 0.0f;
        cursorY = 0.0f;

        cursorSizeRad = 0.8f;
        cursorG = 0.0f;
        cursorB = 0.0f;
        cursorR = 0.0f;

        pictureData = NULL;
        transData = NULL;


}//setAll


