////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A 3D scene viewed through a rectangular region.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SCENE_H
#define SCENE_H

#include "GraphObject.h"
#include "primObj.h"
#include "cameraNlight.h"

#include <set>

class Scene : public GUI::GraphObject
{
 public:
  Scene( GUI::GraphDisplay& );
  virtual ~Scene();

  // Properties
  Scene& SetImagePath( const std::string& p )
    { mImagePath = p; Change(); return *this; }
  const std::string& ImagePath() const
    { return mImagePath; }

  Scene& SetCameraX( float f )
    { mCameraAndLight.setCamViewPoint( f, CameraY(), CameraZ() ); return *this; }
  float CameraX() const
    { return mCameraAndLight.getCamViewX(); }
  Scene& SetCameraY( float f )
    { mCameraAndLight.setCamViewPoint( CameraX(), f, CameraZ() ); return *this; }
  float CameraY() const
    { return mCameraAndLight.getCamViewY(); }
  Scene& SetCameraZ( float f )
    { mCameraAndLight.setCamViewPoint( CameraX(), CameraY(), f ); return *this; }
  float CameraZ() const
    { return mCameraAndLight.getCamViewZ(); }

  cameraNLight& CameraAndLight()
    { return mCameraAndLight; }

  Scene& SetBitDepth( int inBitDepth )
    { mBitDepth = inBitDepth; Change(); return *this; }
  int BitDepth() const
    { return mBitDepth; }
  // Double buffering may result in faster rendering but other GraphObjects will
  // not be visible on the screen.
  Scene& SetDoubleBuffering( bool inDoubleBuffering )
    { mDoubleBuffering = inDoubleBuffering; Change(); return *this; }
  bool DoubleBuffering() const
    { return mDoubleBuffering; }
  // Some graphics cards allow switching off buffer synchronization on vsync.
  Scene& SetDisableVsync( bool inDisableVsync )
    { mDisableVsync = inDisableVsync; return *this; }
  bool DisableVsync() const
    { return mDisableVsync; }
  // False for the generic software OpenGL implementation.
  bool HardwareAccelerated() const
    { return mHardwareAccelerated; }

  void Add( primObj* );
  void Remove( primObj* );
  void DeleteObjects();

  typedef std::set<primObj*> SetOfObjects;
  typedef Scene::SetOfObjects::const_iterator ObjectIterator;
  const SetOfObjects& Objects()
    { return mObjects; }

  typedef std::set<primObj*, primObj::compareByDrawingOrder> DrawingOrderedSetOfObjects;
  typedef Scene::DrawingOrderedSetOfObjects::const_iterator DrawingOrderedIterator;

  void Move( float deltaT ); // moves all objects according to their velocities, and tests for
                             // collisions

  typedef void (*OnCollideFunc)( sceneObj&, sceneObj& );
  Scene& SetOnCollide( OnCollideFunc c )
    { mfpOnCollide = c; return *this; }

 protected:
  // GraphObject event handlers
  void OnPaint( const GUI::DrawContext& );
  void OnChange( GUI::DrawContext& );

 private:
  void Initialize();
  void Cleanup();

 private:
  bool  mInitialized;
  
  HGLRC mGLRC;
  void* mContextHandle;
  int   mBitDepth;
  bool  mDoubleBuffering,
        mDisableVsync,
        mHardwareAccelerated;

  // Scene Objects
  SetOfObjects  mObjects;
  cameraNLight  mCameraAndLight;
  std::string   mImagePath; // path to images to load

  OnCollideFunc mfpOnCollide;
};

#endif // SCENE_H
