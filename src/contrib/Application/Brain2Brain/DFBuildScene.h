////////////////////////////////////////////////////////////////////////////////
// $Id: DFBuildScene.h 3798 2012-02-01 18:07:06Z dsarma $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An abstract class interface for feedback scenes.
//   A feedback scene consists of a cursor and a number of targets.
// Adapted: dsarma
// 
////////////////////////////////////////////////////////////////////////////////
#ifndef DFBUILD_SCENE_H
#define DFBUILD_SCENE_H

#include "Environment.h"
#include "Color.h"
#include <string>

class DFBuildScene : protected Environment
{
 typedef DFBuildScene self;

 public:
  DFBuildScene()
    {}
  virtual ~DFBuildScene()
    {}

  virtual self& Initialize() = 0;

  virtual float CursorRadius() const = 0;
  virtual float CursorXPosition() const = 0;
  virtual float CursorYPosition() const = 0;
  virtual float CursorZPosition() const = 0;
  virtual self& SetCursorPosition( float x, float y, float z ) = 0;
  virtual self& SetCursorVisible( bool ) = 0;
  virtual self& SetCursorColor( RGBColor ) = 0;

  virtual int   NumTargets() const = 0;
  virtual bool  TargetHit( int ) const = 0;
  virtual float CursorTargetDistance( int ) const = 0;
  virtual self& SetTargetVisible( bool, int ) = 0;
  virtual self& SetTargetColor( RGBColor, int ) = 0;
};

#endif // DFBUILD_SCENE_H
