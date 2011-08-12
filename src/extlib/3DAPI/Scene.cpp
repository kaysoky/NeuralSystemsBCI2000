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
#include "PCHIncludes.h"
#pragma hdrstop

#include "Scene.h"

#include "GraphDisplay.h"
#include "DisplayWindow.h"
#include "BCIDirectory.h"
#include "BCIError.h"
#include "OSError.h"
#include "BCIException.h"

#if _MSC_VER
# include <direct.h>
#elif _WIN32
# include <dir.h>
#else
# include <dirent.h>
#endif

#ifndef __BORLANDC__
# include <QPainter>
#endif // __BORLANDC__

using namespace GUI;
using namespace std;

Scene::Scene( GraphDisplay& inDisplay )
: GraphObject( inDisplay, SceneDisplayZOrder ),
  mInitialized( false ),
#ifdef __BORLANDC__
  mGLRC( NULL ),
#else // __BORLANDC__
  mpGLScene( NULL ),
#endif // __BORLANDC__
  mContextHandle( NULL ),
  mBitDepth( 16 ),
  mDoubleBuffering( false ),
  mDisableVsync( false ),
  mHardwareAccelerated( false ),
  mfpOnCollide( NULL )
{
#ifndef __BORLANDC__
  QWidget* parentWidget = NULL;
  parentWidget = dynamic_cast<QWidget*>( Display().Context().handle.device );
  if( parentWidget )
  {
    mpGLScene = new class GLScene( parentWidget );
    mpGLScene->resize(
      static_cast<int>( Display().Context().rect.right - Display().Context().rect.left ),
      static_cast<int>( Display().Context().rect.bottom - Display().Context().rect.top - 1 )
    );
  }
#endif // __BORLANDC__
}

Scene::~Scene()
{
  DeleteObjects();
#ifndef __BORLANDC__
  delete mpGLScene;
  mpGLScene = NULL;
#endif // __BORLANDC__
}

void
Scene::Add( primObj* p )
{
  if( mInitialized )
    bcierr << "Cannot add object to scene in initialized state. "
           << "Call Scene::Cleanup() to de-initialize the scene."
           << endl;
  else
    mObjects.insert( p );
}

void
Scene::Remove( primObj* p )
{
  if( mInitialized )
    bcierr << "Cannot remove object from scene in initialized state. "
           << "Call Scene::Cleanup() to de-initialize the scene."
           << endl;
  else
    mObjects.erase( p );
}

void
Scene::DeleteObjects()
{
  Cleanup();
  while( !mObjects.empty() )
    delete *mObjects.begin();
}


void
Scene::OnPaint( const DrawContext& inDC )
{
#ifdef __BORLANDC__
  // OpenGL is not compatible with clipping, so we remove the clipping region.
  // We also don't restore the clipping region to make sure that all objects
  // further to the top are drawn properly.
  ::SelectClipRgn( ( HDC )inDC.handle, NULL );
  if( !::wglMakeCurrent( ( HDC )inDC.handle, mGLRC ) )
    bciexception << OSError().Message();
  GUI::Rect fullRect = { 0, 0, 1, 1 };
  fullRect = Display().NormalizedToPixelCoords( fullRect );
  int windowHeight = fullRect.bottom - fullRect.top;
  ::glViewport(
    inDC.rect.left,
    windowHeight - inDC.rect.bottom,
    inDC.rect.right - inDC.rect.left,
    inDC.rect.bottom - inDC.rect.top
  );

  ::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  ::glMatrixMode( GL_MODELVIEW );
  ::glLoadIdentity();
  mCameraAndLight.apply();
  ::glEnable( GL_NORMALIZE );

  DrawingOrderedSetOfObjects s;
  for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
    s.insert( *i );

  for( DrawingOrderedIterator i = s.begin(); i != s.end(); ++i )
    ( *i )->render();

  ::glFlush();
  ::wglMakeCurrent( NULL, NULL );
#else // __BORLANDC__
  mpGLScene->RenderObjects( mObjects, &mCameraAndLight );
#endif // __BORLANDC__
}

void
Scene::OnChange( DrawContext& inDC )
{
#ifdef __BORLANDC__
  if( mContextHandle != inDC.handle )
  {
    mHardwareAccelerated = false;

    Cleanup();
    if( mGLRC )
      ::wglDeleteContext( mGLRC );

    PIXELFORMATDESCRIPTOR pfd =
    {
      sizeof( PIXELFORMATDESCRIPTOR ),
      1, // Version number
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
      PFD_TYPE_RGBA,
      mBitDepth,
      0, 0, 0, 0, 0, 0, // Color bits ignored
      0, // No alpha buffer
      0, // Shift bit ignored
      0, // No accumulation buffer
      0, 0, 0, 0, // Accumulation bits ignored
      16, // 16Bit Z-Buffer (Depth buffer)
      0, // No stencil buffer
      0, // No auxiliary buffer
      PFD_MAIN_PLANE, // Main drawing layer
      0, // Reserved
      0, 0, 0 // Layer masks ignored
    };
    if( mDoubleBuffering )
      pfd.dwFlags |= PFD_DOUBLEBUFFER;

    int pixelFormat = ::ChoosePixelFormat( ( HDC )inDC.handle, &pfd );
    ::SetPixelFormat( ( HDC )inDC.handle, pixelFormat, &pfd );
    if( ::DescribePixelFormat( ( HDC )inDC.handle, pixelFormat, sizeof( PIXELFORMATDESCRIPTOR ), &pfd ) )
      mHardwareAccelerated = ( ( pfd.dwFlags & PFD_GENERIC_FORMAT ) == 0 );

    mGLRC = ::wglCreateContext( ( HDC )inDC.handle );
    ::wglMakeCurrent( ( HDC )inDC.handle, mGLRC );
    Initialize();

    if( mDisableVsync )
    {
      mDisableVsync = false;
      // glGetString will return non-NULL only the first time it is called.
      static const char* extensions = NULL;
      if( extensions == NULL )
        extensions = ::glGetString( GL_EXTENSIONS );
      if( extensions != NULL && string( extensions ).find( "WGL_EXT_swap_control" ) != string::npos )
      {  // Switch off VSYNC if possible.
         typedef void (APIENTRY *wglSwapProc)( int );
         wglSwapProc wglSwapIntervalEXT
           = reinterpret_cast<wglSwapProc>( ::wglGetProcAddress( "wglSwapIntervalEXT" ) );
         if( wglSwapIntervalEXT != NULL )
         {
           wglSwapIntervalEXT( 0 );
           typedef int (*wglGetSwapProc)();
           wglGetSwapProc wglGetSwapIntervalEXT
             = reinterpret_cast<wglGetSwapProc>( ::wglGetProcAddress( "wglGetSwapIntervalEXT" ) );
             if( wglGetSwapIntervalEXT != NULL )
               mDisableVsync = ( wglGetSwapIntervalEXT() == 0 );
         }
      }
    }
    ::wglMakeCurrent( NULL, NULL );
  }
  mContextHandle = inDC.handle;
#else // __BORLANDC__
  Initialize();
#endif // __BORLANDC__
}


void
Scene::Initialize()
{
  Cleanup();

#ifdef __BORLANDC_
  ::glClearColor( 0, 0, 0, 0.5 ); // background color
  ::glClearDepth( 1 );
#endif // __BORLANDC__

  string cwd = BCIDirectory::GetCWD();
  if( !mImagePath.empty() )
    ::chdir( BCIDirectory::AbsolutePath( mImagePath ).c_str() );

  for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
    ( *i )->initialize();

  ::chdir( cwd.c_str() );

  mInitialized = true;
}

void
Scene::Cleanup()
{
#ifdef __BORLANDC__
  if( mInitialized )
  {
    HGLRC glrc = ::wglGetCurrentContext();
    HDC   dc = ::wglGetCurrentDC();
    ::wglMakeCurrent( ( HDC )mContextHandle, mGLRC );

    for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
      ( *i )->cleanup();

    ::wglMakeCurrent( dc, glrc );
  }
#else // __BORLANDC__
  if( mInitialized )
    for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
      ( *i )->cleanup();
#endif // __BORLANDC__
  mInitialized = false;
}

void
Scene::Move( float inDeltaT )
{
  typedef set<sceneObj*> SetOfSceneObjects;
  typedef SetOfSceneObjects::const_iterator SceneIterator;
  SetOfSceneObjects sceneObjects;
  for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
  {
    sceneObj* p = dynamic_cast<sceneObj*>( *i );
    if( p )
      sceneObjects.insert( p );
  }
  // Apply movement
  for( SceneIterator i = sceneObjects.begin(); i != sceneObjects.end(); ++i )
    ( *i )->move( inDeltaT );
  // Test for collisions
  if( mfpOnCollide )
    for( SceneIterator i = sceneObjects.begin(); i != sceneObjects.end(); ++i )
      for( SceneIterator j = sceneObjects.begin(); j != i; ++j )
        if( sceneObj::VolumeIntersection( **i, **j ) )
          mfpOnCollide( **i, **j );
}

// GL Scene
#ifndef __BORLANDC__
// Constructs GLScene
// Parameters: Pointer to the Parent QWidget
GLScene::GLScene( QWidget *parent )
: QGLWidget( parent ),
  mCameraAndLight( NULL )
{
  this->show();
}

// Deconstructs GLScene
GLScene::~GLScene()
{
}

// Divs up overlay and scene objects, then calls repaint
// Parameters: Set of primObjects to render
void
GLScene::RenderObjects( SetOfObjects &objSet, cameraNLight* cl )
{
  mSceneObjectSet.clear();
  mOverlayObjectSet.clear();

  mCameraAndLight = cl;

  // Divide the overlay objects and the scene objects
  for( ObjectIterator itr = objSet.begin(); itr != objSet.end(); itr++ )
  {
    sceneObj* sObj = NULL;
    sObj = dynamic_cast<sceneObj*>( *itr );
    if( sObj )
      mSceneObjectSet.insert( sObj );

    overlayObj* oObj = NULL;
    oObj = dynamic_cast<overlayObj*>( *itr );
    if( oObj )
      mOverlayObjectSet.insert( oObj );
  }

  // Ensure the paint functions are called
  this->updateGL();
}

// Virtual interface of the initializeGL function from QGLWidget
// Sets up the GLScene for rendering 3d objects
void
GLScene::initializeGL()
{
  ::glClearColor( 0, 0, 0, 0.5 ); // background color
  ::glClearDepth( 1 );
}

// Virtual interface of the resizeGL function from QGLWidget
// Resizes the GLScene viewport
void
GLScene::resizeGL( int w, int h )
{
  // resize the widget
}

// Virtual interface of the paintGL function from QGLWidget
// Handles all GL related paint functions
void
GLScene::paintGL()
{
  glViewport(
    this->pos().x(),
    this->pos().y(),
    this->width(),
    this->height()
  );

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  if( mCameraAndLight )
    mCameraAndLight->apply();
  glEnable( GL_NORMALIZE );

  if( !mSceneObjectSet.empty() )
  {
    DrawingOrderedSetOfObjects s;
    for( SceneObjectIterator i = mSceneObjectSet.begin(); i != mSceneObjectSet.end(); ++i )
      s.insert( *i );

    for( DrawingOrderedIterator i = s.begin(); i != s.end(); ++i )
      ( *i )->render();
  }

  glFlush();
#if _WIN32
  wglMakeCurrent( NULL, NULL );
#endif
}

// Virtual interface of the paintOverlayGL function from QGLWidget
// Handles all overlay related GL Paint functions
void
GLScene::paintOverlayGL()
{
  glViewport(
    this->pos().x(),
    this->pos().y(),
    this->width(),
    this->height()
  );

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  if( mCameraAndLight )
    mCameraAndLight->apply();
  glEnable( GL_NORMALIZE );

  if( !mOverlayObjectSet.empty() )
  {
    DrawingOrderedSetOfObjects s;
    for( OverlayObjectIterator i = mOverlayObjectSet.begin(); i != mOverlayObjectSet.end(); ++i )
      s.insert( *i );

    for( DrawingOrderedIterator i = s.begin(); i != s.end(); ++i )
      ( *i )->render();
  }

  glFlush();
#if _WIN32
  wglMakeCurrent( NULL, NULL );
#endif
}
#endif // __BORLANDC__


