////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A 3D scene viewed through a rectangular region.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Scene.h"

#include "GraphDisplay.h"
#include "BCIDirectory.h"
#include "BCIError.h"
#include "OSError.h"

#include <dir.h>

using namespace GUI;
using namespace std;

Scene::Scene( GraphDisplay& inDisplay )
: GraphObject( inDisplay, SceneDisplayZOrder ),
  mInitialized( false ),
  mGLRC( NULL ),
  mContextHandle( NULL ),
  mBitDepth( 16 ),
  mDoubleBuffering( false ),
  mDisableVsync( false ),
  mHardwareAccelerated( false ),
  mfpOnCollide( NULL )
{
}

Scene::~Scene()
{
  DeleteObjects();
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
  for( SetOfObjects::const_iterator i = mObjects.begin(); i != mObjects.end(); ++i )
    delete *i;
  mObjects.clear();
}


void
Scene::OnPaint( const DrawContext& inDC )
{
  // OpenGL is not compatible with clipping, so we remove the clipping region.
  // We also don't restore the clipping region to make sure that all objects
  // further to the top are drawn properly.
  ::SelectClipRgn( inDC.handle, NULL );
  if( !::wglMakeCurrent( inDC.handle, mGLRC ) )
    throw OSError().Message();
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

  SetOfObjects s;
  for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
    s.insert( *i );
  mObjects = s;

  for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
    ( *i )->render();

  ::glFlush();
  ::wglMakeCurrent( NULL, NULL );
}

void
Scene::OnChange( DrawContext& inDC )
{
  if( mContextHandle != inDC.handle )
  {
    mHardwareAccelerated = false;

    if( mGLRC )
    {
      ::wglMakeCurrent( mContextHandle, mGLRC );
      Cleanup();
      ::wglDeleteContext( mGLRC );
    }

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

    int pixelFormat = ::ChoosePixelFormat( inDC.handle, &pfd );
    ::SetPixelFormat( inDC.handle, pixelFormat, &pfd );
    if( ::DescribePixelFormat( inDC.handle, pixelFormat, sizeof( PIXELFORMATDESCRIPTOR ), &pfd ) )
      mHardwareAccelerated = ( ( pfd.dwFlags & PFD_GENERIC_FORMAT ) == 0 );

    mGLRC = ::wglCreateContext( inDC.handle );
    ::wglMakeCurrent( inDC.handle, mGLRC );
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
}


void
Scene::Initialize()
{
  Cleanup();

  ::glClearColor( 0, 0, 0, 0.5 ); // background color
  ::glClearDepth( 1 );

  string cwd = BCIDirectory::GetCWD();
  if( !mImagePath.empty() )
    ::chdir( BCIDirectory::AbsolutePath( mImagePath ).c_str() );

  for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
    ( *i )->initialize();
  mInitialized = true;

  ::chdir( cwd.c_str() );
}

void
Scene::Cleanup()
{
  if( mInitialized )
    for( ObjectIterator i = mObjects.begin(); i != mObjects.end(); ++i )
      ( *i )->cleanup();
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


