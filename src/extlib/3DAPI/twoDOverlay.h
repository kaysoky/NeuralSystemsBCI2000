///////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: This "twoDOverlay" class resembles the 2D overlay on top of the
//   3D environment.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#ifndef TWO_D_OVERLAY_H
#define TWO_D_OVERLAY_H

#include "glheaders.h"
#include "primObj.h"

class twoDOverlay : public overlayObj
{
private:
        GLuint                  mPictureTexture;
        GLuint                  mTranslucencyTexture;
        std::string             mPictureFile;      //overlay picture file name
        std::string             mTranslucencyFile; //overlay transparency file name
        float                   mBrightness;

        static void drawQuad( const int* inRect );

public:
/**************************************CONTRUCTRO******************************/
        twoDOverlay(Scene& inScene):overlayObj( inScene ),
          mPictureTexture(NULL),mTranslucencyTexture(NULL),mBrightness(1)
          {setDrawingOrder(1);}
/***********************************Modifier***********************************/
        void setPicture( const std::string& pic )      {mPictureFile=pic;}
        void setTranslucency( const std::string& tra ) {mTranslucencyFile=tra;}
        void setBrightness(float bri)                  {mBrightness=bri;}

/***************************Accessor*******************************************/
        const std::string& getPicture() const          {return mPictureFile;}
        const std::string& getTranslucency() const     {return mTranslucencyFile;}
        float getBrightness() const                    {return mBrightness;}

protected:
        void onInitialize();
        void onCleanup();
        void onRender() const;
};//twoDOverlay

#endif // TWO_D_OVERLAY_H
