////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A 3D scene viewed through a rectangular region.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef SCENE_H
#define SCENE_H

#include "GraphObject.h"
#include "primObj.h"
#include "cameraNlight.h"

#ifndef __BORLANDC__
#include <QGLWidget>
#endif // __BORLANDC__

#include <set>

#ifndef __BORLANDC__
// A descendent class of QGLWidget is required to render GL with QT
class GLScene : public QGLWidget
{
 public:
  GLScene( QWidget* parent=0 );
  ~GLScene();

  // Helper Typedefs
  typedef std::set<primObj*> SetOfObjects;
  typedef GLScene::SetOfObjects::const_iterator ObjectIterator;

  typedef std::set<sceneObj*, primObj::compareByDrawingOrder> SetOfSceneObjects;
  typedef GLScene::SetOfSceneObjects::const_iterator SceneObjectIterator;

  typedef std::set<overlayObj*, primObj::compareByDrawingOrder> SetOfOverlayObjects;
  typedef GLScene::SetOfOverlayObjects::const_iterator OverlayObjectIterator;

  typedef std::set<primObj*, primObj::compareByDrawingOrder> DrawingOrderedSetOfObjects;
  typedef GLScene::DrawingOrderedSetOfObjects::const_iterator DrawingOrderedIterator;

  // Renders a set of objects onto the GLScene
  void RenderObjects( SetOfObjects &objSet, cameraNLight* cl );

 protected:
  // Virtual interface with QGLWidget
  virtual void initializeGL();
  virtual void resizeGL( int w, int h );
  virtual void paintGL();
  virtual void paintOverlayGL();

 private:
  // Set of overlay and scene objects
  SetOfSceneObjects mSceneObjectSet;
  SetOfOverlayObjects mOverlayObjectSet;

  // Camera/Lighting
  cameraNLight* mCameraAndLight;
};
#endif // __BORLANDC__

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

  // GLScene if we're using Qt
#ifndef __BORLANDC__
  const class GLScene& GLScene() const
    { return *mpGLScene; }
#endif // __BORLANDC__

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
  
#ifdef __BORLANDC__
  HGLRC mGLRC;
#else // __BORLANDC__
  // GLScene if we're using Qt
  class GLScene* mpGLScene;
#endif // __BORLANDC__
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
