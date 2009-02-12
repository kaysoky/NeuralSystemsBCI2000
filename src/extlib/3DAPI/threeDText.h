///////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: This "threeDText" class resembles the 3D text in the 3D environment.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#ifndef THREE_D_TEXT_H
#define THREE_D_TEXT_H

#include "primObj.h"
#include <map>
#include <string>

class threeDText : public sceneObj
{
private:
        std::string mCaption,
                    mFont;
        GLfloat     mFontSize;

        CVector3    mDir;
        GLuint      mFontID;   // 3D font handle

public:
        enum { primitiveID = 4 };
        threeDText(Scene& inScene)
          :sceneObj(inScene),mFontSize(12),mFontID(NULL){mDir.x=mDir.y=mDir.z=0;}
        void threeDTextsetAll();

        void setFont(const std::string& f)      {mFont = f;}
        void setFontSize(GLfloat size)          {mFontSize = size;}
        void setCaption(const std::string& cap) {mCaption = cap;}

        void setDirection(GLfloat myDX, GLfloat myDY, GLfloat myDZ)
                                                {CVector3 c={myDX,myDY,myDZ}; mDir=c;}

        //GLuint getFontID() const                {return mThreeDfontID;}
        const std::string& getFont() const      {return mFont;}
        GLfloat getFontSize() const             {return mFontSize;}
        const std::string& getCaption() const   {return mCaption;}

        GLfloat getDirX()                       {return mDir.x;}
        GLfloat getDirY()                       {return mDir.y;}
        GLfloat getDirZ()                       {return mDir.z;}

protected:
        virtual void onInitialize();
        virtual void onCleanup();
        virtual void onRender() const;

};//threeDText

#endif // THREE_D_TEXT_H
