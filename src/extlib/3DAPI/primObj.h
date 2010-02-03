////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: primObj is a base class for all objects in the 3D environment.
//   Two classes are derived from primObj:
//     sceneObj representing 3D scene objects, and
//     overlayObj representing objects in the 2D overlay.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef PRIM_OBJ_H
#define PRIM_OBJ_H

#include "glheaders.h"
#include "component.h"
#include "geomObj.h"
#include <string>

class Scene;

class primObj
{
 private:
  bool  mVisible;
  // Drawing order is used to sort 2D objects from bottom to top, with
  // larger values corresponding to the top.
  // For 3D objects, drawing order should be set to <= 0.
  float mDrawingOrder;

 protected:
  Scene& mScene;

 public:
  explicit primObj( Scene& );
  virtual ~primObj();
  
 private:
  primObj( const primObj& );
  primObj& operator=( const primObj& );

 public:
  void  setVisible( bool b )       {mVisible=b;}
  bool  getVisible() const         {return mVisible;}
  void  setDrawingOrder( float f ) {mDrawingOrder=f;}
  float getDrawingOrder() const    {return mDrawingOrder;}

  // primObj events:
  // Initialization/setup prior to first rendering
  void initialize()                {onInitialize();}
  // Cleaning up when rendering context becomes obsolete
  void cleanup()                   {onCleanup();}
  // Rendering
  virtual void render() const = 0;

  struct compareByDrawingOrder
  {
    bool operator()( primObj* p1, primObj* p2 ) const
    {
      if( p1 == NULL || p2 == NULL )
        return p1 < p2;
      if( p1->getDrawingOrder() == p2->getDrawingOrder() )
        return p1 < p2;
      return p1->getDrawingOrder() < p2->getDrawingOrder();
    }
  };

 protected:
  virtual void onInitialize() = 0;
  virtual void onCleanup() {}
  virtual void onRender() const = 0;
};


class overlayObj : public primObj
{
 public:
  overlayObj( Scene& inScene) : primObj( inScene ) { setDrawingOrder(1); }
  virtual ~overlayObj() {}
  virtual void render() const;

 protected:
  virtual void onInitialize()   {}
  virtual void onRender() const {}
};


class sceneObj : public primObj, public geomObj
{
 private:
  int mBrightness;         //0..1
  int mTransparency;       //0..1

  GLfloat mRColor;         //0..1
  GLfloat mBColor;
  GLfloat mGColor;

  std::string  mTextureFile;     //texture file
  GLuint       mTextureHandle;   //texture data handle

/***************************CONSTRUCTOR****************************************/
 public:
  sceneObj( Scene& );
  virtual ~sceneObj() {}

 private:
  sceneObj( const sceneObj& );
  sceneObj& operator=( const sceneObj& );

 public:
/****************************General Properties********************************/
  void setBrightness(int bri)             {mBrightness = bri;}
  int  getBrightness() const              {return mBrightness;}

  void setTransparency(int tra)           {mTransparency = tra;}
  int  getTransparency() const            {return mTransparency;}

  void setColor(GLfloat R, GLfloat G, GLfloat B)
  {
    mRColor = R;
    mGColor = G;
    mBColor = B;
  }
  GLfloat getColorR() const               {return mRColor;}
  GLfloat getColorG() const               {return mGColor;}
  GLfloat getColorB() const               {return mBColor;}

  void  setTexture( const std::string& t ){mTextureFile = t;}
  const std::string& getTexture() const   {return mTextureFile;}

  // Events inherited from primObj: 
  virtual void render() const;

 protected:
  virtual void onInitialize();
  virtual void onCleanup();
  virtual void onRender() const {}
};//sceneObj

#endif // PRIM_OBJ_H


