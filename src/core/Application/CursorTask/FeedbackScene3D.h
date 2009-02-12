////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A 3D feedback scene using the OpenGL-based 3D API.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FEEDBACK_SCENE_3D_H
#define FEEDBACK_SCENE_3D_H

#include "FeedbackScene.h"

#include "Scene.h"
#include "Sphere.h"
#include "Cuboids.h"
#include "DisplayWindow.h"

class FeedbackScene3D : public FeedbackScene
{
  typedef FeedbackScene3D self;

 public:
  FeedbackScene3D( GUI::DisplayWindow& );
  virtual ~FeedbackScene3D();

  virtual self& Initialize();

  virtual float CursorRadius() const;
  virtual float CursorXPosition() const;
  virtual float CursorYPosition() const;
  virtual float CursorZPosition() const;
  virtual self& SetCursorPosition( float x, float y, float z );
  virtual self& SetCursorVisible( bool );
  virtual self& SetCursorColor( RGBColor );

  virtual int   NumTargets() const;
  virtual bool  TargetHit( int ) const;
  virtual float CursorTargetDistance( int ) const;
  virtual self& SetTargetVisible( bool, int );
  virtual self& SetTargetColor( RGBColor, int );

 private:
  void ClearObjects();

  GUI::DisplayWindow&  mDisplay;
  Scene*               mpScene;
  sphere*              mpCursor;
  invertedCuboid*      mpBoundary;
  std::vector<cuboid*> mTargets;
};

#endif // FEEDBACK_SCENE_3D_H
