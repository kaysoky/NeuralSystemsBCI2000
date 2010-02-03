///////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: A 2D cursor displayed on top of the 2D overlay.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#ifndef TWO_D_CURSOR_H
#define TWO_D_CURSOR_H

#include "glheaders.h"
#include "primObj.h"

class twoDCursor : public overlayObj
{
private:
        CVector2 mOrigin;
        float    mRColor,
                 mGColor,
                 mBColor;
        float    mRadius;

public:
/**************************************CONTRUCTRO******************************/
        twoDCursor(Scene& inScene)
          :overlayObj(inScene),mRadius(1),mRColor(0),mGColor(0),mBColor(0)
          {mOrigin.x=mOrigin.y=0;setDrawingOrder(2);}
/***********************************Modifier***********************************/
        void setRadius(float r)                  {mRadius=r;}
        void setColor(float r, float g, float b) {mRColor=r;mGColor=g;mBColor=b;}
        void setOrigin(const CVector2 v)         {mOrigin=v;}
        void setOrigin(float x, float y)         {mOrigin.x=x;mOrigin.y=y;}
/***************************Accessor*******************************************/
        float getRadius() const            {return mRadius;}
        float getRColor() const            {return mRColor;}
        float getGColor() const            {return mGColor;}
        float getBColor() const            {return mBColor;}
        const CVector2& getOrigin() const  {return mOrigin;}
        float getOriginX() const           {return mOrigin.x;}
        float getOriginY() const           {return mOrigin.y;}

protected:
        void onRender() const;

};//twoDCursor

#endif // TWO_D_CURSOR_H
